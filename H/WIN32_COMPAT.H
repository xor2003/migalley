#pragma once
#include <stdlib.h>   // abs, labs, llabs
#include <math.h>     // fabs, fabsf, fabsl if needed
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unordered_map>
#include "io_compat.h"
#include "SDL.h"
#include "MigCstring.h"

// Minimal Windows/MFC compatibility shims for Linux/gcc
#ifndef WIN32_COMPAT_H
#define WIN32_COMPAT_H

/* ------------------------------------------------------------------
   Windows-style max/min macros
   ------------------------------------------------------------------ */
#ifndef maxstub
#define maxstub(a,b) (( (a) > (b) ) ? (a) : (b))
#endif

#ifndef ZeroMemory
#include <string.h>  // for memset
#define ZeroMemory(Destination,Length) memset((Destination), 0, (Length))
#endif

#ifndef Sleep
#include <sched.h>    // for sched_yield
static inline void Sleep(unsigned int dwMilliseconds)
{
    if (dwMilliseconds == 0) {
        // Yield the processor, like Windows Sleep(0)
        sched_yield();
    } else {
        // usleep takes microseconds
        usleep(dwMilliseconds * 1000);
    }
}
#endif

typedef unsigned short VARTYPE;
typedef long HRESULT;
typedef unsigned long LCID; // locale ID 
typedef long DISPID; // dispatch ID 

#define FAR
#define __FAR
#ifndef WINAPI
#  if defined(__i386__) || defined(_M_IX86)
#    define WINAPI __attribute__((stdcall))
#  else
#    define WINAPI
#  endif
#endif

#define PASCAL WINAPI

// Basic character and string types
typedef char CHAR;
typedef char* LPSTR;
typedef const CHAR* LPCSTR;
typedef LPCSTR PCSTR;
typedef char* LPTSTR;
typedef const CHAR* LPCTSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef const void* LPCVOID;/* Long pointer to constant void */

typedef unsigned long ULONG;
typedef ULONG *PULONG;

typedef void VOID;
typedef void* LPVOID;
typedef const void* LPCVOID;

typedef uint32_t DWORD;
typedef DWORD* LPDWORD;

typedef uint16_t WORD;
typedef WORD* LPWORD;

typedef uint8_t BYTE;
typedef BYTE* LPBYTE;

using UINT_PTR = std::uintptr_t;


#ifndef AFX_CDECL
#if defined(__GNUC__)
#define AFX_CDECL
#else
#define AFX_CDECL __cdecl
#endif
#endif

// TCHAR/LPCTSTR compatibility
#ifndef TCHAR
typedef char TCHAR;   // or wchar_t if you want to mimic UNICODE builds
#endif

#ifndef LPCTSTR
typedef const TCHAR* LPCTSTR;
#endif

// Some MSVC headers use the __cdecl keyword; on non-MSVC compilers make it a no-op
#ifndef __cdecl
#define __cdecl
#endif

// Calling convention macros
#ifndef __thiscall
#define __thiscall
#endif

// Inline assembly macros
#ifndef _asm
#define _asm asm
#endif

// Debug/int3
#ifndef INT3
#define INT3 ((void)0)
#endif

// Floating point control
//#include "MATHASM.H"

// BOOL/TRUE/FALSE if not defined elsewhere
#ifndef BOOL
typedef int BOOL;
#endif
typedef BOOL* PBOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Simple stub for AfxTrace used in some headers — defined as inline to avoid link errors
inline void AfxTrace(const char*, ...) {}

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef PVOID
typedef void* PVOID;
#endif

#ifndef BOOLEAN
typedef unsigned char BOOLEAN;
#endif

#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef long LONG;
typedef LONG* LPLONG;  // pointer to LONG

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

typedef RECT* PRECT;
typedef RECT* LPRECT;
typedef const RECT* LPCRECT;
#endif


// Scroll Bar Constants
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3

#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#define GW_CHILD            5
#define GW_HWNDNEXT         2

/// Windows GUI stuff
// SDL2 window replaces HWND
//struct SDL_Window;
//using HWND = SDL_Window*;
struct HWND__ { int unused; };
typedef HWND__* HWND;

BOOL GetWindowRect(HWND hWnd, LPRECT lpRect);

typedef void *HANDLE;

static inline BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase)
{
    // Linux stub: no-op
    (void)hWnd;
    (void)lpRect;
    (void)bErase;
    return TRUE;
}

#ifndef IDOK
#define IDOK        1
#endif
#ifndef IDCANCEL
#define IDCANCEL    2
#endif
#ifndef IDABORT
#define IDABORT     3
#endif
#ifndef IDRETRY
#define IDRETRY     4
#endif
#ifndef IDIGNORE
#define IDIGNORE    5
#endif
#ifndef IDYES
#define IDYES       6
#endif
#ifndef IDNO
#define IDNO        7
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFFu
#endif


// guid stub
#ifndef GUID_DEFINED
#define GUID_DEFINED
// Match Windows GUID layout
typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} GUID;

typedef GUID* LPGUID;
typedef const GUID* LPCGUID;

// Optional: define IID/CLSID aliases if your code uses them
typedef GUID IID;
typedef GUID CLSID;

/* Reference to a GUID (by const pointer) */
//typedef const GUID* REFGUID;
//typedef const GUID* REFIID;
//typedef const GUID* REFCLSID;
typedef const GUID& REFGUID;
typedef const GUID& REFIID;
typedef const GUID& REFCLSID;


/* Common aliases */
typedef GUID IID;
typedef GUID CLSID;

/* ------------------------------------------------------------------
   Comparison helpers (mimic Windows InlineIsEqualGUID)
   ------------------------------------------------------------------ */

#ifdef __cplusplus
#include <algorithm>  /* for std::equal */

inline bool operator==(const GUID& a, const GUID& b) {
    return a.Data1 == b.Data1 &&
           a.Data2 == b.Data2 &&
           a.Data3 == b.Data3 &&
           std::equal(std::begin(a.Data4), std::end(a.Data4), std::begin(b.Data4));
}

inline bool operator!=(const GUID& a, const GUID& b) {
    return !(a == b);
}
#endif /* __cplusplus */

/* C‑style helper macro, like Windows IsEqualGUID */
#define IsEqualGUID(g1,g2) \
    ((g1).Data1 == (g2).Data1 && \
     (g1).Data2 == (g2).Data2 && \
     (g1).Data3 == (g2).Data3 && \
     (g1).Data4[0] == (g2).Data4[0] && \
     (g1).Data4[1] == (g2).Data4[1] && \
     (g1).Data4[2] == (g2).Data4[2] && \
     (g1).Data4[3] == (g2).Data4[3] && \
     (g1).Data4[4] == (g2).Data4[4] && \
     (g1).Data4[5] == (g2).Data4[5] && \
     (g1).Data4[6] == (g2).Data4[6] && \
     (g1).Data4[7] == (g2).Data4[7])

#endif // GUID_DEFINED


// WIN32_COMPAT.H
#ifndef MB_OK
#define MB_OK 0x00000000L
#endif
#ifndef MB_OKCANCEL
#define MB_OKCANCEL 0x00000001L 
#endif

#if 0
inline int MessageBox(void* /*hWnd*/, const char* text, const char* caption, unsigned /*type*/) {
    // For now, just print to stderr or stdout
    fprintf(stderr, "[MessageBox: %s] %s\n", caption ? caption : "", text ? text : "");
    return 0; // IDOK
}
#else
inline int MessageBox(void* /*hWnd*/, const CString& text, const CString& caption, unsigned /*type*/)
{
    // For now, just print to stderr
    std::fprintf(stderr, "[MessageBox: %s] %s\n",
                 caption.c_str(),   // CString::c_str() returns const char*
                 text.c_str());

    return IDOK; // Always return OK for MB_OK
}

inline int MessageBox(const CString& text, const CString& caption)
{
    std::fprintf(stderr, "[MessageBox: %s] %s\n",
                 caption.c_str(),
                 text.c_str());
    return IDOK;
}

inline int MessageBox(const CString& text, const CString& caption, unsigned /*type*/)
{
    std::fprintf(stderr, "[MessageBox: %s] %s\n",
                 caption.c_str(),
                 text.c_str());
    return IDOK;
}

#endif

// Simple OutputDebugString stub
#define OutputDebugString(msg) fprintf(stderr, "%s", msg)

/* COM interface declaration stubs for GCC/Linux */

#ifndef DECLARE_INTERFACE_
# define DECLARE_INTERFACE_(iface, base) struct iface : public base
#endif

#ifndef STDMETHOD
# define STDMETHOD(method) virtual HRESULT method
#endif

#ifndef STDMETHOD_
# define STDMETHOD_(type,method) virtual type method
#endif

#ifndef PURE
# define PURE = 0
#endif

/* In C++ mode, THIS and THIS_ are empty */
#ifndef THIS
# define THIS
#endif

#ifndef THIS_
# define THIS_
#endif

/* ------------------------------------------------------------------
   Windows memory copy compatibility
   ------------------------------------------------------------------ */
#ifndef CopyMemory
#define CopyMemory(Destination, Source, Length) \
    memcpy((Destination), (Source), (Length))
#endif

/* ------------------------------------------------------------------
   Win32 file I/O compatibility
   ------------------------------------------------------------------ */

/* Desired access */
#ifndef GENERIC_READ
#define GENERIC_READ    0x80000000
#endif
#ifndef GENERIC_WRITE
#define GENERIC_WRITE   0x40000000
#endif

/* Share modes */
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif
#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE 0x00000002
#endif

/* Creation disposition */
#ifndef CREATE_NEW
#define CREATE_NEW      1
#endif
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS   2
#endif
#ifndef OPEN_EXISTING
#define OPEN_EXISTING   3
#endif
#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS     4
#endif
#ifndef TRUNCATE_EXISTING
#define TRUNCATE_EXISTING 5
#endif

/* Invalid handle */
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

/* ------------------------------------------------------------------
   CreateFile replacement
   ------------------------------------------------------------------ */
static inline HANDLE CreateFileA(LPCSTR lpFileName,
                                 DWORD dwDesiredAccess,
                                 DWORD dwShareMode,
                                 void* lpSecurityAttributes,
                                 DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes,
                                 HANDLE hTemplateFile)
{
    (void)dwShareMode; (void)lpSecurityAttributes;
    (void)dwFlagsAndAttributes; (void)hTemplateFile;

    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE))
        flags = O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE)
        flags = O_WRONLY;
    else
        flags = O_RDONLY;

    switch (dwCreationDisposition) {
        case CREATE_NEW:        flags |= O_CREAT | O_EXCL; break;
        case CREATE_ALWAYS:     flags |= O_CREAT | O_TRUNC; break;
        case OPEN_EXISTING:     /* no O_CREAT */ break;
        case OPEN_ALWAYS:       flags |= O_CREAT; break;
        case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    }

    int fd = open(lpFileName, flags, 0666);
    if (fd == -1) return INVALID_HANDLE_VALUE;
    return (HANDLE)(intptr_t)fd;
}
#define CreateFile CreateFileA

/* ------------------------------------------------------------------
   GetFileSize replacement
   ------------------------------------------------------------------ */
static inline DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh)
{
    int fd = (int)(intptr_t)hFile;
    // Win32 leaves the file pointer untouched — save/restore, not
    // reset-to-start (callers stream reads between size queries).
    off_t save = lseek(fd, 0, SEEK_CUR);
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, save, SEEK_SET);
    if (size == (off_t)-1) return (DWORD)-1;

    if (lpFileSizeHigh) {
        uint64_t u = (uint64_t)size;
        *lpFileSizeHigh = (DWORD)(u >> 32);
    }
    return (DWORD)(size & 0xFFFFFFFF);
}

/* ------------------------------------------------------------------
   ReadFile replacement
   ------------------------------------------------------------------ */
static inline BOOL ReadFile(HANDLE hFile,
                            LPVOID lpBuffer,
                            DWORD nNumberOfBytesToRead,
                            DWORD* lpNumberOfBytesRead,
                            void* lpOverlapped)
{
    (void)lpOverlapped;
    ssize_t ret = read((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToRead);
    if (ret < 0) return 0; // failure
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)ret;
    return 1; // success
}

/* ------------------------------------------------------------------
   WriteFile replacement
   ------------------------------------------------------------------ */
static inline BOOL WriteFile(HANDLE hFile,
                             LPCVOID lpBuffer,
                             DWORD nNumberOfBytesToWrite,
                             DWORD* lpNumberOfBytesWritten,
                             void* lpOverlapped)
{
    (void)lpOverlapped;
    ssize_t ret = write((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToWrite);
    if (ret < 0) return 0; // failure
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)ret;
    return 1; // success
}

static inline BOOL SetEndOfFile(HANDLE hFile)
{
    uintptr_t h = (uintptr_t)hFile;
    // Only operate on file descriptors (small values)
    if (h == 0 || h == (uintptr_t)INVALID_HANDLE_VALUE || h > 0x10000)
        return 0; // not a file descriptor

    int fd = (int)h;
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos == (off_t)-1)
        return 0;  // failure

    return (ftruncate(fd, pos) == 0);
}

#ifndef FILE_BEGIN
#define FILE_BEGIN   SEEK_SET
#endif
#ifndef FILE_CURRENT
#define FILE_CURRENT SEEK_CUR
#endif
#ifndef FILE_END
#define FILE_END     SEEK_END
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

static inline DWORD SetFilePointer(HANDLE hFile,
                                   LONG lDistanceToMove,
                                   LONG* lpDistanceToMoveHigh,
                                   DWORD dwMoveMethod)
{
    int fd = (int)(intptr_t)hFile;

    // Always do math in 64 bits. With a high half present the low
    // parameter is the *unsigned* low 32 of the pair — sign-extending
    // it first then |= corrupts negative-low/nonzero-high pairs.
    int64_t offset = (int64_t)lDistanceToMove;
    if (lpDistanceToMoveHigh) {
        offset = ((int64_t)(*lpDistanceToMoveHigh) << 32)
                 | (int64_t)(uint32_t)lDistanceToMove;
    }

    off_t pos = lseek(fd, (off_t)offset, (int)dwMoveMethod);
    if (pos == (off_t)-1)
        return INVALID_SET_FILE_POINTER;

    if (lpDistanceToMoveHigh) {
        int64_t pos64 = (int64_t)pos;
        *lpDistanceToMoveHigh = (LONG)(pos64 >> 32);
    }

    return (DWORD)(pos & 0xFFFFFFFF);
}


#ifndef FILE_ATTRIBUTE_TEMPORARY
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#endif

#ifndef GetLastError
static inline DWORD GetLastError(void)
{
    return (DWORD)errno;
}
#endif

#ifndef FlushFileBuffers
static inline BOOL FlushFileBuffers(HANDLE hFile)
{
    int fd = (int)(intptr_t)hFile;
    return (fsync(fd) == 0);
}
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif

#ifndef FILE_FLAG_RANDOM_ACCESS
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#endif

#ifndef DeleteFile
static inline BOOL DeleteFile(const char* lpFileName)
{
    return (unlink(lpFileName) == 0);
}
#endif

#include <sys/sysinfo.h>

typedef struct {
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
} MEMORYSTATUS;

static inline void GlobalMemoryStatus(MEMORYSTATUS* lpBuffer)
{
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        lpBuffer->dwTotalPhys = 100000000; //RERUN, not yet -> info.totalram;
        lpBuffer->dwAvailPhys = 100000000; //RERUN, not yet -> info.freeram;
        lpBuffer->dwTotalPageFile = info.totalswap;
        lpBuffer->dwAvailPageFile = info.freeswap;
        //RERUN lpBuffer->dwTotalVirtual = info.totalram + info.totalswap;
        //RERUN lpBuffer->dwAvailVirtual = info.freeram + info.freeswap;
        lpBuffer->dwTotalVirtual = 100000000; //RERUN
        lpBuffer->dwAvailVirtual = 100000000; //RERUN
        lpBuffer->dwMemoryLoad = (unsigned long)
            (100 - (lpBuffer->dwAvailPhys * 100 / lpBuffer->dwTotalPhys));
    } else {
        // fallback: zero everything
        memset(lpBuffer, 0, sizeof(*lpBuffer));
    }
}

#include <time.h>

typedef union {
    struct { uint32_t LowPart; int32_t HighPart; };
    int64_t QuadPart;
} LARGE_INTEGER;

static inline int QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;
    int64_t ns = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    lpPerformanceCount->QuadPart = ns;
    lpPerformanceCount->LowPart  = (uint32_t)(ns & 0xFFFFFFFF);
    lpPerformanceCount->HighPart = (int32_t)(ns >> 32);
    return 1; // TRUE
}

static inline int QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
    // CLOCK_MONOTONIC is defined in nanoseconds
    int64_t freq = 1000000000LL;
    lpFrequency->QuadPart = freq;
    lpFrequency->LowPart  = (uint32_t)(freq & 0xFFFFFFFF);
    lpFrequency->HighPart = (int32_t)(freq >> 32);
    return 1; // TRUE
}


typedef int32_t   LRESULT;
typedef DWORD*    PDWORD;
typedef WORD*     PWORD;
typedef GUID INTERFACEID;
typedef unsigned int UINT;   // 32-bit unsigned
typedef UINT* PUINT;
typedef UINT* LPUINT;
typedef long         LPARAM; // 32-bit signed (with -m32)
typedef UINT_PTR        WPARAM;

#ifndef INT
typedef int INT;
#endif
typedef INT* PINT;

typedef void* HMIDIOUT; // a handle to an open MIDI output device. 
// It’s used with functions like midiOutOpen, midiOutShortMsg, and 
// midiOutClose to send MIDI messages to hardware or software synthesizers.
typedef HMIDIOUT* LPHMIDIOUT;
typedef struct MIDIHDR {
    char*     lpData;
    uint32_t  dwBufferLength;
    uint32_t  dwBytesRecorded;
    uintptr_t dwUser;
    uint32_t  dwFlags;
    struct MIDIHDR* lpNext;
    uintptr_t reserved;
    uint32_t  dwOffset;
    uintptr_t dwReserved[8];
} MIDIHDR, *PMIDIHDR, *NPMIDIHDR, *LPMIDIHDR;

// Flag defines
#define MHDR_DONE     0x00000001
#define MHDR_PREPARED 0x00000002
#define MHDR_INQUEUE  0x00000004
#define MHDR_ISSTRM   0x00000008

typedef uint32_t MMRESULT;

#define MMSYSERR_NOERROR     0
#define MMSYSERR_INVALHANDLE 5
#define MMSYSERR_INVALPARAM  11
#define MIDIERR_UNPREPARED   64
#define MIDIERR_NOTREADY     67

// Stub functions
inline MMRESULT midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh) {
    if (!pmh || cbmh < sizeof(MIDIHDR)) return MMSYSERR_INVALPARAM;
    pmh->dwFlags |= MHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

inline MMRESULT midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh) {
    if (!pmh || !(pmh->dwFlags & MHDR_PREPARED)) return MIDIERR_UNPREPARED;
    // In a real implementation, you’d send pmh->lpData of length pmh->dwBufferLength
    pmh->dwFlags |= MHDR_DONE;
    return MMSYSERR_NOERROR;
}

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef INOUT
#define INOUT
#endif

#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

// HDC: A handle (opaque pointer) to a device context.
#ifndef HDC
typedef struct HDC__ { int unused; } *HDC;
#endif

#ifndef _MBS_FUNCS_STUB
#define _MBS_FUNCS_STUB
// Return the next character (as unsigned int) from a multibyte string
inline unsigned int _mbsnextc(const unsigned char* p) {
    return static_cast<unsigned int>(*p);
}

// Advance pointer to the next character
inline const unsigned char* _mbsinc(const unsigned char* p) {
    return p + 1;
}
#endif

inline BOOL KillTimer(void* /*hWnd*/, unsigned int /*uIDEvent*/) {
    return TRUE; // pretend success
}

inline BOOL SetCursorPos(int X, int Y) {
    // SDL_WarpMouseGlobal moves the mouse cursor in global screen coordinates
    if (SDL_WarpMouseGlobal(X, Y) == 0) {
        return 1; // TRUE on success
    } else {
        return 0; // FALSE on failure
    }
}

//// Window Event replacement by Posix

#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT  0x00000102L   // 258
#define WAIT_FAILED   0xFFFFFFFFL

/// Semaphore and Mutex stubs
#include <pthread.h>
#include <semaphore.h>

enum class HandleType { Unknown, Event, Mutex, Semaphore, Thread };

struct HandleBase {
    HandleType type = HandleType::Unknown;
    virtual ~HandleBase() = default;
};

// Internal wrapper types
struct LinuxSemaphore : public HandleBase {
    sem_t sem;
    int   maxCount = 0;   // optional: track maximum count
    LinuxSemaphore() { type = HandleType::Semaphore; }
};

struct PosixEventObject : public HandleBase {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            signaled = false;
    bool            manualReset = false;
    PosixEventObject() { type = HandleType::Event; }
};
// Drop-in replacement for CreateEvent
inline HANDLE CreateEvent(void* lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName)
{
    PosixEventObject* ev = new PosixEventObject;
    pthread_mutex_init(&ev->mutex, nullptr);
    pthread_cond_init(&ev->cond, nullptr);
    ev->signaled = bInitialState;
    ev->manualReset = bManualReset;
    return (HANDLE)ev;
}

// Drop-in replacement for SetEvent
inline bool SetEvent(HANDLE hEvent) {
    PosixEventObject* ev = (PosixEventObject*)hEvent;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = true;
    if (ev->manualReset)
        pthread_cond_broadcast(&ev->cond);
    else
        pthread_cond_signal(&ev->cond);
    pthread_mutex_unlock(&ev->mutex);
    return true;
}

// Drop-in replacement for ResetEvent
inline bool ResetEvent(HANDLE hEvent) {
    PosixEventObject* ev = (PosixEventObject*)hEvent;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = false;
    pthread_mutex_unlock(&ev->mutex);
    return true;
}

static inline bool CloseHandle(HANDLE hObject) {
    uintptr_t h = (uintptr_t)hObject;
    if (h == 0 || h == (uintptr_t)INVALID_HANDLE_VALUE) return false;

    if (h < 0x10000) {
        return (close((int)h) == 0);
    }

    HandleBase* hb = (HandleBase*)hObject;
    if (hb->type == HandleType::Semaphore) {
        LinuxSemaphore* s = (LinuxSemaphore*)hb;
        sem_destroy(&s->sem);
        delete s;
        return true;
    } else if (hb->type == HandleType::Thread) {
        // LinuxThread defined below, but we can cast to HandleBase to delete if virtual dtor
        delete hb;
        return true;
    }

    PosixEventObject* ev = (PosixEventObject*)hObject;
    pthread_mutex_destroy(&ev->mutex);
    if (hb->type == HandleType::Event) {
        pthread_cond_destroy(&ev->cond);
    }
    delete ev;
    return true;
}

// Drop-in replacement for WaitForSingleObject
inline int WaitForSingleObject(HANDLE hEvent, int timeout) {
    uintptr_t h = (uintptr_t)hEvent;
    if (h == 0 || h == (uintptr_t)INVALID_HANDLE_VALUE) return WAIT_FAILED;
    if (h < 0x10000) return WAIT_FAILED;

    HandleBase* hb = (HandleBase*)hEvent;
    if (hb->type == HandleType::Semaphore) {
        LinuxSemaphore* lsem = (LinuxSemaphore*)hEvent;
        if (timeout == INFINITE) {
            return sem_wait(&lsem->sem) == 0 ? WAIT_OBJECT_0 : WAIT_FAILED;
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            if (sem_timedwait(&lsem->sem, &ts) == 0)
                return WAIT_OBJECT_0;
            else if (errno == ETIMEDOUT)
                return WAIT_TIMEOUT;
            else
                return WAIT_FAILED;
        }
    }

    PosixEventObject* ev = (PosixEventObject*)hEvent;

    if (hb->type == HandleType::Mutex) {
        if (timeout == INFINITE) {
            int ret = pthread_mutex_lock(&ev->mutex);
            return (ret == 0) ? WAIT_OBJECT_0 : WAIT_FAILED;  // Map success/failure
        } else {
            struct timespec ts;
            if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
                return WAIT_FAILED;

            /* add the millisecond interval */
            const long long NSEC_PER_MSEC = 1000000LL;
            const long long NSEC_PER_SEC  = 1000000000LL;

            long long nsec = (long long)timeout * NSEC_PER_MSEC;
            ts.tv_sec  += nsec / NSEC_PER_SEC;
            ts.tv_nsec += nsec % NSEC_PER_SEC;

            /* carry over if nanoseconds overflow */
            if (ts.tv_nsec >= NSEC_PER_SEC) {
                ts.tv_nsec -= NSEC_PER_SEC;
                ++ts.tv_sec;
            }

            int rc = pthread_mutex_timedlock(&ev->mutex, &ts);
            if (rc == 0)          return WAIT_OBJECT_0;
            if (rc == ETIMEDOUT)  return WAIT_TIMEOUT;
            return WAIT_FAILED;   // any other error (e.g. EINVAL, EDEADLK, …)
        }
    }

    // Event: wait on `signaled` under the mutex; auto-reset consumes the
    // signal unless manualReset was requested at CreateEvent.
    if (timeout == INFINITE) {
        pthread_mutex_lock(&ev->mutex);
        while (!ev->signaled)
            pthread_cond_wait(&ev->cond, &ev->mutex);
        if (!ev->manualReset) ev->signaled = false;
        pthread_mutex_unlock(&ev->mutex);
        return WAIT_OBJECT_0;
    } else {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
            return WAIT_FAILED;

        const long long NSEC_PER_MSEC = 1000000LL;
        const long long NSEC_PER_SEC  = 1000000000LL;

        long long nsec = (long long)timeout * NSEC_PER_MSEC;
        ts.tv_sec  += nsec / NSEC_PER_SEC;
        ts.tv_nsec += nsec % NSEC_PER_SEC;
        if (ts.tv_nsec >= NSEC_PER_SEC) {
            ts.tv_nsec -= NSEC_PER_SEC;
            ++ts.tv_sec;
        }

        pthread_mutex_lock(&ev->mutex);
        int rc = 0;
        while (!ev->signaled && rc != ETIMEDOUT)
            rc = pthread_cond_timedwait(&ev->cond, &ev->mutex, &ts);
        bool got = ev->signaled;
        if (got && !ev->manualReset) ev->signaled = false;
        pthread_mutex_unlock(&ev->mutex);
        if (got)             return WAIT_OBJECT_0;
        if (rc == ETIMEDOUT) return WAIT_TIMEOUT;
        return WAIT_FAILED;
    }
}



// Drop-in replacement for CreateSemaphore
inline HANDLE CreateSemaphore(void* /*lpSemaphoreAttributes*/,
                              int lInitialCount,
                              int lMaximumCount,
                              const char* /*lpName*/) {
    LinuxSemaphore* s = new LinuxSemaphore;
    if (sem_init(&s->sem, 0, lInitialCount) != 0) {
        delete s;
        return nullptr;
    }
    // Note: POSIX semaphores don’t enforce max count, you’ll need to track it manually if required
    s->maxCount = lMaximumCount;
    return s;
}

// Drop-in replacement for CreateMutex
inline HANDLE CreateMutex(void* /*lpMutexAttributes*/,
                          bool /*bInitialOwner*/,
                          const char* /*lpName*/) {
    PosixEventObject* m = new PosixEventObject;
    m->type = HandleType::Mutex;
    if (pthread_mutex_init(&m->mutex, nullptr) != 0) {
        delete m;
        return nullptr;
    }
    return m;
}

// ReleaseSemaphore: increments the semaphore count
inline bool ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LONG* lpPreviousCount) {
    LinuxSemaphore* s = (LinuxSemaphore*)hSemaphore;
    if (!s) return false;

    int val;
    if (sem_getvalue(&s->sem, &val) != 0) return false;

    // Win32: fails without releasing if the count would exceed lMaximumCount
    // (POSIX sems have no max - enforce the CreateSemaphore value ourselves)
    if (val + lReleaseCount > s->maxCount) return false;

    // Optionally track previous count
    if (lpPreviousCount) *lpPreviousCount = val;

    // Increment by lReleaseCount
    for (int i = 0; i < lReleaseCount; i++) {
        if (sem_post(&s->sem) != 0) {
            printf("ReleaseSemaphore returns false!\n");
            return false;
        }
    }
    return true;
}

// ReleaseMutex: unlocks the mutex
inline bool ReleaseMutex(HANDLE hMutex) {
    PosixEventObject* m = (PosixEventObject*)hMutex;
    if (!m) return false;
    return pthread_mutex_unlock(&m->mutex) == 0;
}

////////////////////////////
// Windows thread code stubs
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID);

// Internal wrapper
struct LinuxThread : public HandleBase {
    pthread_t thread;
    LinuxThread() { type = HandleType::Thread; }
};

// Drop-in replacement for CreateThread
inline HANDLE CreateThread(void* /*lpThreadAttributes*/,
                           size_t /*dwStackSize*/,
                           LPTHREAD_START_ROUTINE lpStartAddress,
                           LPVOID lpParameter,
                           DWORD /*dwCreationFlags*/,
                           DWORD* /*lpThreadId*/) {
    LinuxThread* t = new LinuxThread;

    // Wrap the Windows-style function pointer into a pthread entry
    auto trampoline = [](void* arg) -> void* {
        auto pair = static_cast<std::pair<LPTHREAD_START_ROUTINE, LPVOID>*>(arg);
        LPTHREAD_START_ROUTINE func = pair->first;
        LPVOID param = pair->second;
        func(param); // Windows expects DWORD return, we ignore it here
        delete pair;
        return nullptr;
    };

    auto* pair = new std::pair<LPTHREAD_START_ROUTINE, LPVOID>(lpStartAddress, lpParameter);
    if (pthread_create(&t->thread, nullptr, trampoline, pair) != 0) {
        delete t;
        return nullptr;
    }
    return t;
}

// Drop-in replacement for SetThreadPriority
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_BELOW_NORMAL -1

inline int SetThreadPriority(HANDLE hThread, int /*nPriority*/) {
    // On Linux without root, changing priority has no effect.
    // We stub it out as success.
    return 1; // nonzero = success
}

// Optional cleanup
#if 0 
inline void CloseHandle(HANDLE h) {
    if (!h) return;
    LinuxThread* t = (LinuxThread*)h;
    pthread_detach(t->thread); // or pthread_join if you want to wait
    delete t;
}
#endif

// Stub of the MIDIOUTCAPS structure
// Windows defines MAXPNAMELEN as 32
#ifndef MAXPNAMELEN
#define MAXPNAMELEN 32
#endif

// MIDI technology types (from WinMM)
#define MOD_MIDIPORT   1   // Hardware port
#define MOD_SYNTH      2   // Internal synthesizer
#define MOD_SQSYNTH    3   // Square wave synthesizer
#define MOD_FMSYNTH    4   // FM synthesizer
#define MOD_MAPPER     5   // MIDI mapper

typedef struct {
    uint16_t wMid;                     // Manufacturer ID
    uint16_t wPid;                     // Product ID
    uint32_t vDriverVersion;           // Driver version
    char     szPname[MAXPNAMELEN];     // Product name
    uint16_t wTechnology;              // Device type
    uint16_t wVoices;                  // Number of voices
    uint16_t wNotes;                   // Max simultaneous notes
    uint16_t wChannelMask;             // Channel mask
    uint32_t dwSupport;                // Supported features
} MIDIOUTCAPS;

// Stub function that would normally fill this struct
inline int midiOutGetDevCaps(int deviceID, MIDIOUTCAPS* caps, unsigned int size) {
    if (!caps || size < sizeof(MIDIOUTCAPS)) return -1;
    caps->wMid = 0;
    caps->wPid = 0;
    caps->vDriverVersion = 0;
    strncpy(caps->szPname, "SDL2 Software Synth", sizeof(caps->szPname));
    caps->wTechnology = MOD_SYNTH;
    caps->wVoices = 64;
    caps->wNotes = 64;
    caps->wChannelMask = 0xFFFF;
    caps->dwSupport = 0;
    return 0; // success
}

typedef struct {
    uint16_t wFormatTag;        // PCM = 1
    uint16_t nChannels;         // 1 = mono, 2 = stereo
    uint32_t nSamplesPerSec;    // sample rate (e.g. 44100)
    uint32_t nAvgBytesPerSec;   // sample rate * block align
    uint16_t nBlockAlign;       // channels * bits/8
} WAVEFORMAT, *LPWAVEFORMAT;

typedef struct {
    WAVEFORMAT wf;              // nested WAVEFORMAT
    uint16_t   wBitsPerSample;  // bits per sample (8, 16, etc.)
} PCMWAVEFORMAT;

#define WAVE_MAPPER   (-1)

// WinMM MIDI constants
#ifndef MIDI_MAPPER
#define MIDI_MAPPER ((unsigned int)-1)
#endif

// Stub for midiOutGetNumDevs
inline unsigned int midiOutGetNumDevs(void) {
    // Pretend one MIDI device exists (SDL2_mixer backend)
    return 1;
}

// -----------------------------------------------------------------------------
// Minimal Win32 GDI compatibility stubs for Linux builds
// -----------------------------------------------------------------------------
// RGBQUAD structure
typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

// BITMAPINFOHEADER structure
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

// BITMAPINFO structure
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO;

#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

//struct MINMAXINFO {
//    struct { int x, y; } ptReserved;
//    struct { int x, y; } ptMaxSize;
//    struct { int x, y; } ptMaxPosition;
//    struct { int x, y; } ptMinTrackSize;
//    struct { int x, y; } ptMaxTrackSize;
//};

struct POINT
{
    long x;
    long y;
};

typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO;


struct PAINTSTRUCT {
    void* hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
};

inline void EndPaint(PAINTSTRUCT*) {}

inline int GetDeviceCaps(void*, int) {
    return 32; // pretend 32-bit color
}

#define BITSPIXEL 12

// -----------------------------------------------------------------------------
// BITMAP — minimal Win32-compatible bitmap info struct
// -----------------------------------------------------------------------------
struct BITMAP {
    int   bmType      = 0;
    int   bmWidth     = 0;
    int   bmHeight    = 0;
    int   bmWidthBytes = 0;
    unsigned short bmPlanes = 1;
    unsigned short bmBitsPixel = 32;
    void* bmBits     = nullptr;
};

// -----------------------------------------------------------------------------
// MSG — minimal Win32-compatible message struct
// -----------------------------------------------------------------------------
struct MSG {
    void*   hwnd;     // window handle (unused)
    UINT    message;  // message ID
    WPARAM  wParam;   // word parameter
    LPARAM  lParam;   // long parameter
    DWORD   time;     // timestamp
    int     pt_x;     // cursor position (x)
    int     pt_y;     // cursor position (y)
};

typedef HANDLE HMENU;

typedef unsigned long ULONG_PTR;

typedef struct tagMEASUREITEMSTRUCT {
    UINT  CtlType;
    UINT  CtlID;
    UINT  itemID;
    UINT  itemWidth;
    UINT  itemHeight;
    ULONG_PTR itemData;
} MEASUREITEMSTRUCT;

typedef MEASUREITEMSTRUCT* LPMEASUREITEMSTRUCT;

typedef struct tagDRAWITEMSTRUCT {
    UINT      CtlType;
    UINT      CtlID;
    UINT      itemID;
    UINT      itemAction;
    UINT      itemState;
    void*     hwndItem;
    HDC       hDC;
    RECT      rcItem;
    ULONG_PTR itemData;
} DRAWITEMSTRUCT;

typedef DRAWITEMSTRUCT* LPDRAWITEMSTRUCT;

#define WM_USER                0x0400
#ifndef WM_COMMAND
#define WM_COMMAND 0x0111
#endif
#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif
#ifndef WM_DESTROY
#define WM_DESTROY 0x0002
#endif
#ifndef WM_LBUTTONDOWN
#define WM_LBUTTONDOWN 0x0201
#endif
#ifndef WM_LBUTTONUP
#define WM_LBUTTONUP 0x0202
#endif
#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif
#ifndef MK_LBUTTON
#define MK_LBUTTON 0x0001
#endif
#ifndef MK_RBUTTON
#define MK_RBUTTON 0x0002
#endif
#ifndef WM_PAINT
#define WM_PAINT 0x000F
#endif
#ifndef WM_ERASEBKGND
#define WM_ERASEBKGND 0x0014
#endif
#ifndef WM_SIZE
#define WM_SIZE 0x0005
#endif
#ifndef WM_TIMER
#define WM_TIMER 0x0112
#endif
#ifndef WM_QUIT
#define WM_QUIT 0x0012
#endif
#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif
#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif
#ifndef WM_CHAR
#define WM_CHAR 0x0102
#endif

#ifndef WM_MOVE
#define WM_MOVE 0x0003
#endif
#ifndef WM_CREATE
#define WM_CREATE 0x0001
#endif

#ifndef WM_HSCROLL
#define WM_HSCROLL 0x0114
#endif

#ifndef WM_VSCROLL
#define WM_VSCROLL 0x0115
#endif

#ifndef WM_GETMINMAXINFO
#define WM_GETMINMAXINFO 0x0024
#endif

#ifndef WM_ACTIVATE
#define WM_ACTIVATE 0x0006
#endif

#ifndef WM_KILLFOCUS
#define WM_KILLFOCUS 0x0008
#endif

#ifndef WM_ENABLE
#define WM_ENABLE 0x000A
#endif

#ifndef WM_ACTIVATEAPP
#define WM_ACTIVATEAPP 0x001C
#endif

#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU 0x007B
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#ifndef WM_SHOWWINDOW
#define WM_SHOWWINDOW 0x0018
#endif

#ifndef WM_CANCELMODE
#define WM_CANCELMODE 0x001F
#endif

#ifndef WM_SETCURSOR
#define WM_SETCURSOR 0x0020
#endif

#ifndef WM_CHARTOITEM
#define WM_CHARTOITEM 0x002F
#endif

#ifndef WM_NCMOUSEMOVE
#define WM_NCMOUSEMOVE 0x00A0
#endif

#ifndef WM_NCLBUTTONDOWN
#define WM_NCLBUTTONDOWN 0x00A1
#endif

#ifndef WM_NCLBUTTONUP
#define WM_NCLBUTTONUP 0x00A2
#endif

#ifndef WM_RBUTTONDOWN
#define WM_RBUTTONDOWN 0x0204
#endif

#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP 0x0205
#endif

#ifndef WM_INITMENU
#define WM_INITMENU 0x0116
#endif

#ifndef WM_CAPTURECHANGED
#define WM_CAPTURECHANGED 0x0215
#endif

#ifndef EN_CHANGE
#define EN_CHANGE 0x0300
#endif

#ifndef EN_UPDATE
#define EN_UPDATE 0x0400
#endif

#ifndef WM_COMMANDHELP
#define WM_COMMANDHELP 0x0022
#endif
#ifndef WM_HELP
#define WM_HELP             0x0053
#endif

#define HELPINFO_WINDOW    0x0001
typedef struct tagHELPINFO {
    UINT      cbSize;
    int       iContextType;
    int       iCtrlId;
    HANDLE    hItemHandle;
    DWORD *dwContextId;
    POINT     MousePos;
} HELPINFO, *LPHELPINFO;


//#include "MESSAGES.H"

#ifndef MAKELPARAM
#define MAKELPARAM(l, h) ((LPARAM)(((WORD)(l)) | (((DWORD)((WORD)(h))) << 16)))
#endif

#ifndef MAKEWPARAM
#define MAKEWPARAM(l, h) ((WPARAM)(((WORD)(l)) | (((DWORD)((WORD)(h))) << 16)))
#endif

#ifndef RGB
#define RGB(r,g,b) ((unsigned int)(((r) & 0xFF) | (((g) & 0xFF) << 8) | (((b) & 0xFF) << 16)))
#endif

#define IDC_ARROW ((char*)32512)

#define SIZE_RESTORED 0
#define SIZE_MINIMIZED 1
#define SIZE_MAXIMIZED 2
#define SIZE_MAXSHOW   3
#define SIZE_MAXHIDE   4

#ifndef SetCursor
// Minimal stub for SetCursor on Linux: does nothing, returns nullptr
inline void* SetCursor(void* hCursor) { (void)hCursor; return nullptr; }
#endif

#define DIB_RGB_COLORS 0
#define DIB_PAL_COLORS 1

#define LB_ERR (-1)
#define TRANSPARENT 1
#define OPAQUE 2

#define TA_NOUPDATECP   0
#define TA_UPDATECP     1
#define TA_LEFT         0
#define TA_RIGHT        2
#define TA_CENTER       6
#define TA_TOP          0
#define TA_BOTTOM       8
#define TA_BASELINE     24

// Basic window styles
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L     // WS_BORDER | WS_DLGFRAME
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

// Common combinations
#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

// Composite styles
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)

#define WS_POPUPWINDOW      (WS_POPUP          | \
                             WS_BORDER         | \
                             WS_SYSMENU)

#define WS_CHILDWINDOW      (WS_CHILD)

#define WS_SIZEBOX    WS_THICKFRAME

#define DROPEFFECT_COPY  1
#define DROPEFFECT_MOVE  2
#define DROPEFFECT_LINK  4
#define DROPEFFECT_SCROLL 0x80000000

typedef HANDLE HGLOBAL;
typedef WORD CLIPFORMAT;

#ifndef __stdcall
#define __stdcall
#endif

struct TEXTMETRIC
{
    int tmHeight = 0;
    int tmAscent = 0;
    int tmDescent = 0;
    int tmInternalLeading = 0;
    int tmExternalLeading = 0;
    int tmAveCharWidth = 0;
    int tmMaxCharWidth = 0;
    int tmWeight = 0;
    int tmOverhang = 0;
    int tmDigitizedAspectX = 0;
    int tmDigitizedAspectY = 0;
    BYTE tmItalic = 0;
    BYTE tmUnderlined = 0;
    BYTE tmStruckOut = 0;
    BYTE tmFirstChar = 0;
    BYTE tmLastChar = 0;
    BYTE tmDefaultChar = 0;
    BYTE tmBreakChar = 0;
    BYTE tmPitchAndFamily = 0;
    BYTE tmCharSet = 0;
};

#define LF_FACESIZE 32

typedef struct tagLOGFONT
{
    int lfHeight;
    int lfWidth;
    int lfEscapement;
    int lfOrientation;
    int lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    char lfFaceName[LF_FACESIZE];
} LOGFONT;


#define DEFAULT_CHARSET          1
#define OUT_DEFAULT_PRECIS       0
#define CLIP_CHARACTER_PRECIS    1
#define DEFAULT_QUALITY          0
#define DEFAULT_PITCH            0

// ---------------------------------------------------------------------------
// IntelliMouse wheel support stubs (MSH_WHEEL.DLL legacy)
// ---------------------------------------------------------------------------
#define MSH_WHEELMODULE_CLASS   "MSHWHEEL_Class"
#define MSH_WHEELMODULE_TITLE   "MSH_WHEEL"
// Custom window message used by the old wheel module
#define MSH_MOUSEWHEEL          (WM_USER + 0x0200)
// Query for wheel support
#define MSH_WHEELSUPPORT        (WM_USER + 0x0201)
// Query for number of scroll lines
#define MSH_SCROLL_LINES        (WM_USER + 0x0202)
// FindWindow: always returns nullptr on Linux
inline HWND FindWindow(LPCTSTR /*lpClassName*/, LPCTSTR /*lpWindowName*/)
{
    return nullptr;
}
// RegisterWindowMessage: return a unique dummy message ID
inline UINT RegisterWindowMessage(LPCTSTR /*lpString*/)
{
    // Pick something safely above WM_USER
    static UINT nextMsg = 0xC000;
    return nextMsg++;
}

inline UINT RegisterWindowMessage(UINT /*msg*/)
{
    static UINT nextMsg = 0xC000;
    return nextMsg++;
}

// Win32 global SendMessage API stub
inline LRESULT SendMessage(HWND /*hWnd*/, UINT /*Msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return 0;
}

#define COLOR_SCROLLBAR      0
#define COLOR_3DDKSHADOW     21

#define MM_TEXT 1

#define WM_KEYFIRST 0x0100   // WM_KEYDOWN
#define WM_KEYLAST  0x0108   // WM_KEYLAST (WM_KEYUP, WM_CHAR, etc.)

#define PM_REMOVE 0x0001 // a flag for PeekMessage

typedef struct tagCY {
    LONG    Hi;   // high 32 bits
    ULONG   Lo;   // low 32 bits
} CY;

/* Minimal COM IUnknown stub for non‑Windows builds */
#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

typedef struct IUnknown IUnknown;

/* Define a dummy vtable with the three standard COM methods */
typedef struct IUnknownVtbl {
    HRESULT (*QueryInterface)(IUnknown* This, const void* riid, void** ppvObject);
    ULONG   (*AddRef)(IUnknown* This);
    ULONG   (*Release)(IUnknown* This);
} IUnknownVtbl;

/* The IUnknown struct itself just holds a vtable pointer */
struct IUnknown {
    const IUnknownVtbl* lpVtbl;
};

typedef IUnknown *LPUNKNOWN;

#endif /* __IUnknown_INTERFACE_DEFINED__ */

#define MF_GRAYED     0x00000001
#define MF_BYCOMMAND  0x00000000

#ifndef TPM_LEFTALIGN
#define TPM_LEFTALIGN   0x0000
#endif

#ifndef TPM_RIGHTBUTTON
#define TPM_RIGHTBUTTON 0x0002
#endif

typedef unsigned int COLORREF;
#ifndef PS_SOLID
#define PS_SOLID 0x00000000
#endif
#ifndef PS_NULL
#define PS_NULL 5
#endif
#ifndef DCB_RESET
#define DCB_RESET 0x0001
#endif
#ifndef DCB_SET
#define DCB_SET 0x0001
#endif
#ifndef DCB_ACCUMULATE
#define DCB_ACCUMULATE 0x0002
#endif
#ifndef DCB_DISABLE
#define DCB_DISABLE    0x0008
#endif
#ifndef DCB_ENABLE
#define DCB_ENABLE     0x0004
#endif

#ifndef SRCCOPY
#define SRCCOPY 0x00CC0020
#endif

int StretchDIBits(
    HDC hdc,
    int xDest, int yDest, int DestWidth, int DestHeight,
    int xSrc,  int ySrc,  int SrcWidth,  int SrcHeight,
    const void* lpBits,
    const BITMAPINFO* lpbmi,
    UINT usage,
    DWORD rop);

#define WHITE_BRUSH        0
#define LTGRAY_BRUSH       1
#define GRAY_BRUSH         2
#define DKGRAY_BRUSH       3
#define BLACK_BRUSH        4
#define NULL_BRUSH         5
#define HOLLOW_BRUSH       NULL_BRUSH

#define WHITE_PEN          6
#define BLACK_PEN          7
#define NULL_PEN           8

#define OEM_FIXED_FONT     10
#define ANSI_FIXED_FONT    11
#define ANSI_VAR_FONT      12
#define SYSTEM_FONT        13
#define DEVICE_DEFAULT_FONT 14
#define SYSTEM_FIXED_FONT  16

struct _BRUSH
{
    enum Type
    {
        SOLID,
        HATCHED,
        PATTERN,
        NULLTYPE
    };

    Type type;

    union
    {
        SDL_Color solid;

        struct
        {
            SDL_Color color;
            int hatchStyle;   // HS_HORIZONTAL, etc.
        } hatch;

        struct
        {
            SDL_Texture* texture;
        } pattern;
    };
    bool isStock;   // important!
};
typedef _BRUSH* HBRUSH;

inline HBRUSH GetStockObject(int obj)
{
    static _BRUSH brushes[] =
    {
        { _BRUSH::SOLID, {255,255,255,255}, true }, // WHITE
        { _BRUSH::SOLID, {192,192,192,255}, true }, // LTGRAY
        { _BRUSH::SOLID, {128,128,128,255}, true }, // GRAY
        { _BRUSH::SOLID, { 64, 64, 64,255}, true }, // DKGRAY
        { _BRUSH::SOLID, {  0,  0,  0,255}, true }, // BLACK
        { _BRUSH::NULLTYPE, {0,0,0,0}, true }     // NULL
    };

    switch (obj)
    {
        case WHITE_BRUSH:   return &brushes[0];
        case LTGRAY_BRUSH:  return &brushes[1];
        case GRAY_BRUSH:    return &brushes[2];
        case DKGRAY_BRUSH:  return &brushes[3];
        case BLACK_BRUSH:   return &brushes[4];
        case NULL_BRUSH:    return &brushes[5];
        default:            return nullptr;
    }
}

#define S_OK ((HRESULT)0L)
#define OLEMISC_INSIDEOUT        0x00000080L /*OLEMISC flags (from oleidl.h)*/
#define OLEIVERB_HIDE            (-3) /* OLE verbs (from oleidl.h) */

static inline BOOL IntersectRect(RECT* dst, const RECT* src1, const RECT* src2)
{
    if (!dst || !src1 || !src2)
        return FALSE;

    dst->left   = std::max(src1->left,   src2->left);
    dst->top    = std::max(src1->top,    src2->top);
    dst->right  = std::min(src1->right,  src2->right);
    dst->bottom = std::min(src1->bottom, src2->bottom);

    return (dst->right > dst->left) && (dst->bottom > dst->top);
}

/* TIMERQUEUE_STUB START */
/* Callback type */
typedef void (*WAITORTIMERCALLBACK)(PVOID, BOOLEAN);

struct TimerEntry {
    WAITORTIMERCALLBACK cb;
    PVOID param;
    uint32_t nextFire;
    uint32_t period;
};

inline std::vector<TimerEntry>& GetGlobalTimerList()
{
    static std::vector<TimerEntry> g_timers;
    return g_timers;
}

inline void register_periodic_callback(WAITORTIMERCALLBACK cb, PVOID param,
                                uint32_t due, uint32_t period)
{
    uint32_t now = SDL_GetTicks();
    GetGlobalTimerList().push_back({ cb, param, now + due, period });
}

/* The following is an example what to put in the main loop code 
uint32_t now = SDL_GetTicks();
for (auto& t : GetGlobalTimerList()) {
    if (now >= t.nextFire) {
        t.cb(t.param, 0);      // BOOLEAN = 0
        t.nextFire += t.period;
    }
}
*/

/* Flags */
#define WT_EXECUTEDEFAULT 0x00000000

/* CreateTimerQueue: return opaque handle */
static inline HANDLE CreateTimerQueue(void)
{
    return malloc(1);   /* any non-null pointer works */
}

/* CreateTimerQueueTimer:
   On Windows this schedules a periodic callback.
   In this port, you only need to register it with your own timer system.
*/
static inline int CreateTimerQueueTimer(
    HANDLE* phNewTimer,
    HANDLE TimerQueue,
    WAITORTIMERCALLBACK Callback,
    PVOID Parameter,
    DWORD DueTime,
    DWORD Period,
    ULONG Flags)
{
    (void)TimerQueue;
    (void)Flags;

    /* Allocate a fake timer handle */
    *phNewTimer = malloc(1);

    /* Register with your SDL timer/tick system */
    register_periodic_callback(Callback, Parameter, DueTime, Period);

    return 1; /* nonzero = success */
}

/* Optional: delete stubs */
static inline int DeleteTimerQueueTimer(HANDLE TimerQueue, HANDLE Timer, void* CompletionEvent)
{
    (void)TimerQueue;
    (void)CompletionEvent;
    free(Timer);
    return 1;
}
/* TIMERQUEUE_STUB END */

#ifndef E_POINTER
#define E_POINTER        ((HRESULT)0x80004003L)
#endif

#ifndef E_NOINTERFACE
#define E_NOINTERFACE    ((HRESULT)0x80004002L)
#endif

typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    void*  lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#define WA_INACTIVE    0
#define WA_ACTIVE      1
#define WA_CLICKACTIVE 2

#ifndef WS_EX_TOPMOST
#define WS_EX_TOPMOST 0x00000008
#endif

typedef long DISPID;
typedef wchar_t* BSTR;

struct VARIANTARG {
    // Minimal placeholder; expand only if needed
    unsigned short vt;
};

struct DISPPARAMS {
    VARIANTARG* rgvarg;            // arguments
    DISPID*     rgdispidNamedArgs; // named args
    unsigned int cArgs;            // number of args
    unsigned int cNamedArgs;       // number of named args
};

struct EXCEPINFO {
    unsigned short wCode;
    unsigned short wReserved;
    BSTR           bstrSource;
    BSTR           bstrDescription;
    BSTR           bstrHelpFile;
    unsigned long  dwHelpContext;
    void*          pvReserved;
    void*          pfnDeferredFillIn;
    long           scode;
};


#define GW_HWNDNEXT   2
#define GW_HWNDPREV   3
#define GW_OWNER      4
#define GW_CHILD      5

// --- Static control styles (WinUser.h) ---
#define SS_LEFT             0x00000000L
#define SS_CENTER           0x00000001L
#define SS_RIGHT            0x00000002L
#define SS_ICON             0x00000003L
#define SS_BLACKRECT        0x00000004L
#define SS_GRAYRECT         0x00000005L
#define SS_WHITERECT        0x00000006L
#define SS_BLACKFRAME       0x00000007L
#define SS_GRAYFRAME        0x00000008L
#define SS_WHITEFRAME       0x00000009L
#define SS_USERITEM         0x0000000AL
#define SS_SIMPLE           0x0000000BL
#define SS_LEFTNOWORDWRAP   0x0000000CL
#define SS_OWNERDRAW        0x0000000DL
#define SS_BITMAP           0x0000000EL
#define SS_ENHMETAFILE      0x0000000FL
#define SS_ETCHEDHORZ       0x00000010L
#define SS_ETCHEDVERT       0x00000011L
#define SS_ETCHEDFRAME      0x00000012L
#define SS_TYPEMASK         0x0000001FL

#define SS_REALSIZECONTROL  0x00000040L
#define SS_NOPREFIX         0x00000080L
#define SS_NOTIFY           0x00000100L
#define SS_CENTERIMAGE      0x00000200L
#define SS_RIGHTJUST        0x00000400L
#define SS_REALSIZEIMAGE    0x00000800L
#define SS_SUNKEN           0x00001000L
#define SS_EDITCONTROL      0x00002000L
#define SS_ENDELLIPSIS      0x00004000L
#define SS_PATHELLIPSIS     0x00008000L
#define SS_WORDELLIPSIS     0x0000C000L
#define SS_ELLIPSISMASK     0x0000C000L

#define LOWORD(l)   ((unsigned short)((l) & 0xFFFF))
#define HIWORD(l)   ((unsigned short)((l) >> 16))

#ifndef CBN_ERRSPACE
#define CBN_ERRSPACE      (-1)
#define CBN_SELCHANGE     1
#define CBN_DBLCLK        2
#define CBN_SETFOCUS      3
#define CBN_KILLFOCUS     4
#define CBN_EDITCHANGE    5
#define CBN_EDITUPDATE    6
#define CBN_DROPDOWN      7
#define CBN_CLOSEUP       8
#define CBN_SELENDOK      9
#define CBN_SELENDCANCEL  10
#endif

#ifndef LBS_NOTIFY
#define LBS_NOTIFY 0x0001
#endif
#ifndef LBN_SELCHANGE
#define LBN_SELCHANGE   1
#define LBN_DBLCLK      2
#define LBN_SELCANCEL   3
#define LBN_SETFOCUS    4
#define LBN_KILLFOCUS   5
#endif

// Scroll Info
#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)

typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}   SCROLLINFO, *LPSCROLLINFO;

#endif // WIN32_COMPAT_H
