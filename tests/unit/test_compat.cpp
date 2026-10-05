//------------------------------------------------------------------------------
// Unit tests for the WIN32 compatibility layer (H/WIN32_COMPAT.H).
//
// This is OUR code (not 1998 code), it backs the game's thread/event/timer
// plumbing (MIG.cpp htable input events, STUB3D semaphores/mutexes,
// CEvent::Lock). Sync bugs here are the volatile-flag race class that had
// to be fixed upstream, so the contract is pinned directly.
//
// Kept in its own TU: WIN32_COMPAT.H defines the whole Win32 surface and
// would collide with the game headers included by unit_tests.cpp.
//------------------------------------------------------------------------------
#include "harness.h"
#include "WIN32_COMPAT.H"

//------------------------------------------------------------------------------
void test_win32_events()
{
    // Auto-reset event: unsignalled must NOT satisfy a wait. (Earlier code
    // mutex-locked the event and always returned OBJECT_0 while leaving the
    // mutex held - any later SetEvent then deadlocked.)
    HANDLE ev = CreateEvent(nullptr, false, false, nullptr);
    CHECK(ev != nullptr);
    CHECK_EQ(WaitForSingleObject(ev, 0), WAIT_TIMEOUT);   // not signaled
    CHECK(SetEvent(ev));
    CHECK_EQ(WaitForSingleObject(ev, 0), WAIT_OBJECT_0);  // signaled -> got it
    CHECK_EQ(WaitForSingleObject(ev, 0), WAIT_TIMEOUT);   // auto-reset consumed
    CloseHandle(ev);

    // Manual-reset event: signal persists until ResetEvent.
    HANDLE mev = CreateEvent(nullptr, true, false, nullptr);
    CHECK(SetEvent(mev));
    CHECK_EQ(WaitForSingleObject(mev, 0), WAIT_OBJECT_0);
    CHECK_EQ(WaitForSingleObject(mev, 0), WAIT_OBJECT_0); // manual: still set
    CHECK(ResetEvent(mev));
    CHECK_EQ(WaitForSingleObject(mev, 0), WAIT_TIMEOUT);
    CloseHandle(mev);

    // Initial state honored.
    HANDLE iev = CreateEvent(nullptr, false, true, nullptr);
    CHECK_EQ(WaitForSingleObject(iev, 0), WAIT_OBJECT_0);
    CloseHandle(iev);

    // Signals do not queue: SetEvent twice still leaves one consume on an
    // auto-reset event (the flag is boolean, not a count).
    HANDLE sev = CreateEvent(nullptr, false, false, nullptr);
    CHECK(SetEvent(sev));
    CHECK(SetEvent(sev));
    CHECK_EQ(WaitForSingleObject(sev, 0), WAIT_OBJECT_0);
    CHECK_EQ(WaitForSingleObject(sev, 0), WAIT_TIMEOUT);
    CloseHandle(sev);
}

//------------------------------------------------------------------------------
void test_win32_semaphore()
{
    HANDLE sem = CreateSemaphore(nullptr, 1, 2, nullptr);
    CHECK(sem != nullptr);
    CHECK_EQ(WaitForSingleObject(sem, 0), WAIT_OBJECT_0); // count 1 -> acquired
    CHECK_EQ(WaitForSingleObject(sem, 0), WAIT_TIMEOUT);  // count 0 -> blocks

    LONG prev = -1;
    CHECK(ReleaseSemaphore(sem, 1, &prev));
    CHECK_EQ(prev, 0);                                  // was drained
    CHECK_EQ(WaitForSingleObject(sem, 0), WAIT_OBJECT_0);
    CloseHandle(sem);

    // Invalid handle -> WAIT_FAILED, never a lock/deadlock. (Return type is
    // signed: -1 == 0xFFFFFFFF WAIT_FAILED once converted - cast to DWORD.)
    CHECK_EQ((DWORD)WaitForSingleObject(nullptr, 0), (DWORD)WAIT_FAILED);
    CHECK_EQ((DWORD)WaitForSingleObject(INVALID_HANDLE_VALUE, 0),
             (DWORD)WAIT_FAILED);

    // Initial count 0 -> immediate timeout without consuming anything.
    HANDLE zsem = CreateSemaphore(nullptr, 0, 2, nullptr);
    CHECK_EQ(WaitForSingleObject(zsem, 0), WAIT_TIMEOUT);

    // Win32 contract: ReleaseSemaphore fails without releasing if the
    // count would exceed lMaximumCount. Fill to max, then overflow.
    CHECK(ReleaseSemaphore(zsem, 1, nullptr));
    CHECK(ReleaseSemaphore(zsem, 1, nullptr));      // count = 2 = max
    CHECK_EQ(ReleaseSemaphore(zsem, 1, nullptr), false); // would exceed
    // Overflow left the count intact: drain twice, third blocks.
    CHECK_EQ(WaitForSingleObject(zsem, 0), WAIT_OBJECT_0);
    CHECK_EQ(WaitForSingleObject(zsem, 0), WAIT_OBJECT_0);
    CHECK_EQ(WaitForSingleObject(zsem, 0), WAIT_TIMEOUT);
    CloseHandle(zsem);

    // Releasing 0 is a legal no-op that still reports prevCount.
    HANDLE nsem = CreateSemaphore(nullptr, 1, 3, nullptr);
    LONG prev2 = -1;
    CHECK(ReleaseSemaphore(nsem, 0, &prev2));
    CHECK_EQ(prev2, 1);
    CHECK_EQ(WaitForSingleObject(nsem, 0), WAIT_OBJECT_0);
    CloseHandle(nsem);
}

//------------------------------------------------------------------------------
void test_win32_mutex()
{
    HANDLE mx = CreateMutex(nullptr, false, nullptr);
    CHECK(mx != nullptr);
    CHECK_EQ(WaitForSingleObject(mx, 0), WAIT_OBJECT_0);  // acquire
    CHECK_EQ(WaitForSingleObject(mx, 0), WAIT_TIMEOUT);   // held by self
    CHECK(ReleaseMutex(mx));
    CHECK_EQ(WaitForSingleObject(mx, 0), WAIT_OBJECT_0);  // re-acquire
    CHECK(ReleaseMutex(mx));
    CloseHandle(mx);
}

//------------------------------------------------------------------------------
void test_win32_timing()
{
    LARGE_INTEGER freq = {}, c1 = {}, c2 = {};
    CHECK(QueryPerformanceFrequency(&freq) != 0);
    CHECK(freq.QuadPart > 0);
    CHECK(QueryPerformanceCounter(&c1) != 0);
    Sleep(0);
    CHECK(QueryPerformanceCounter(&c2) != 0);
    CHECK(c2.QuadPart >= c1.QuadPart);                  // monotonic
    Sleep(1);                                            // must not spin

    // Frequency is a stable constant across calls.
    LARGE_INTEGER f2 = {};
    CHECK(QueryPerformanceFrequency(&f2) != 0);
    CHECK_EQ(f2.QuadPart, freq.QuadPart);

    // A real timed wait actually sleeps ~the timeout (measured in ms via
    // QPC; generous upper bound for loaded CI runners).
    HANDLE ev = CreateEvent(nullptr, false, false, nullptr);
    LARGE_INTEGER t0 = {}, t1 = {};
    QueryPerformanceCounter(&t0);
    DWORD wr = WaitForSingleObject(ev, 50);
    QueryPerformanceCounter(&t1);
    CloseHandle(ev);
    CHECK_EQ(wr, WAIT_TIMEOUT);
    long long elapsed_ms =
        (long long)((t1.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart);
    CHECK(elapsed_ms >= 40);                            // waited, not poll
    CHECK(elapsed_ms < 5000);                           // and not forever
}

//------------------------------------------------------------------------------
// File shim: CreateFileA/ReadFile/WriteFile/SetFilePointer/GetFileSize/
// SetEndOfFile/CloseHandle over real fds via /tmp.
void test_win32_files()
{
    static char fn[] = "/tmp/migw32files.bin";
    unlink(fn);

    // OPEN_EXISTING on a missing file must fail (no O_CREAT).
    HANDLE h = CreateFileA(fn, GENERIC_READ, 0, nullptr,
                           OPEN_EXISTING, 0, nullptr);
    CHECK_EQ((uintptr_t)h, (uintptr_t)INVALID_HANDLE_VALUE);
    CHECK(GetLastError() != 0);                          // errno surfaced

    // CREATE_ALWAYS makes a fresh file; CREATE_NEW then refuses (O_EXCL).
    h = CreateFileA(fn, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    CREATE_ALWAYS, 0, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE);

    const char payload[] = "0123456789";
    DWORD wrote = 0;
    CHECK(WriteFile(h, payload, 10, &wrote, nullptr));
    CHECK_EQ(wrote, 10u);

    // GetFileSize sees the size WITHOUT moving the file position
    // (position stays at 10 where the write left it).
    DWORD hi = 0xdead;
    CHECK_EQ(GetFileSize(h, &hi), 10u);
    CHECK_EQ(hi, 0u);
    DWORD back = 0;
    char tail[4] = {};
    CHECK(ReadFile(h, tail, 1, &back, nullptr));         // at EOF: 0 bytes
    CHECK_EQ(back, 0u);

    // SetFilePointer: all three anchors.
    CHECK_EQ(SetFilePointer(h, 4, nullptr, FILE_BEGIN), 4u);
    CHECK_EQ(SetFilePointer(h, 2, nullptr, FILE_CURRENT), 6u);
    CHECK_EQ(SetFilePointer(h, -3, nullptr, FILE_END), 7u);

    // high-dword path: (0, pos) behaves like the low-only call.
    LONG hihi = 0;
    CHECK_EQ(SetFilePointer(h, 5, &hihi, FILE_BEGIN), 5u);
    CHECK_EQ(hihi, 0);

    // Read back from position 5: "56789".
    char buf[8] = {};
    CHECK(ReadFile(h, buf, 5, &back, nullptr));
    CHECK_EQ(back, 5u);
    CHECK(memcmp(buf, "56789", 5) == 0);

    // SetEndOfFile truncates at the current position.
    CHECK_EQ(SetFilePointer(h, 4, nullptr, FILE_BEGIN), 4u);
    CHECK(SetEndOfFile(h));
    CHECK_EQ(GetFileSize(h, nullptr), 4u);

    // Extending: seek beyond EOF then SetEndOfFile zero-fills (POSIX
    // ftruncate semantics; Win32 also zero-fills on extend).
    CHECK_EQ(SetFilePointer(h, 8, nullptr, FILE_BEGIN), 8u);
    CHECK(SetEndOfFile(h));
    CHECK_EQ(GetFileSize(h, nullptr), 8u);
    CHECK_EQ(SetFilePointer(h, 6, nullptr, FILE_BEGIN), 6u);
    memset(buf, 0xAA, sizeof buf);
    CHECK(ReadFile(h, buf, 2, &back, nullptr));
    CHECK_EQ(buf[0], '\0');
    CHECK_EQ(buf[1], '\0');

    CloseHandle(h);

    // CREATE_NEW on the existing file must fail (O_EXCL).
    h = CreateFileA(fn, GENERIC_WRITE, 0, nullptr, CREATE_NEW, 0, nullptr);
    CHECK_EQ((uintptr_t)h, (uintptr_t)INVALID_HANDLE_VALUE);

    // OPEN_ALWAYS opens the existing file in place (no truncation).
    h = CreateFileA(fn, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    OPEN_ALWAYS, 0, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE);
    CHECK_EQ(GetFileSize(h, nullptr), 8u);
    CloseHandle(h);

    // Read-only handle rejects writes at the syscall level.
    h = CreateFileA(fn, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    CHECK(h != INVALID_HANDLE_VALUE);
    CHECK(!WriteFile(h, "x", 1, &wrote, nullptr));
    CloseHandle(h);

    // DeleteFile + gone.
    CHECK(DeleteFile(fn));
    CHECK(!DeleteFile(fn));
}
