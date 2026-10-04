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
}
