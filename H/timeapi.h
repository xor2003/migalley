#ifndef __TIMEAPI_H__
#define __TIMEAPI_H__

/*
 * Portable replacement for Windows <timeapi.h>
 * For use on Linux with GCC (-m32 or -m64).
 *
 * Provides:
 *   - TIMECAPS struct
 *   - timeGetTime()
 *   - timeBeginPeriod() / timeEndPeriod()
 *   - timeGetDevCaps()
 *
 * These are stubs using POSIX clock_gettime().
 */

#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

/* Return codes */
#define TIMERR_NOERROR 0
#define TIMERR_NOCANDO 1
#ifndef MMSYSERR_NOERROR
#define MMSYSERR_NOERROR 0
#endif

/* TIMECAPS structure */
typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *LPTIMECAPS;

/* ------------------------------------------------------------------
   Portable implementations
   ------------------------------------------------------------------ */

/* timeGetTime: returns system uptime in milliseconds */
static inline DWORD timeGetTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)((ts.tv_sec * 1000u) + (ts.tv_nsec / 1000000u));
}

/* timeBeginPeriod / timeEndPeriod:
   On Windows, these request higher timer resolution.
   On Linux, we can ignore and return success. */
static inline MMRESULT timeBeginPeriod(UINT uPeriod) {
    (void)uPeriod;
    return TIMERR_NOERROR;
}

static inline MMRESULT timeEndPeriod(UINT uPeriod) {
    (void)uPeriod;
    return TIMERR_NOERROR;
}

/* timeGetDevCaps: reports timer resolution capabilities */
static inline MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc) {
    if (!ptc || cbtc < sizeof(TIMECAPS))
        return TIMERR_NOCANDO;

    ptc->wPeriodMin = 1;    /* 1 ms resolution */
    ptc->wPeriodMax = 1000; /* 1 second */
    return TIMERR_NOERROR;
}

/* ------------------------------------------------------------------
   BoB extension (T118): the winmm periodic event-timer surface
   (timeSetEvent/timeKillEvent). BoB's stub3d drives Mast3d's frame
   ticks off a TIME_PERIODIC timer, so these are REAL implementations
   on SDL timers — a static per-TU slot table keeps them header-only.
   ------------------------------------------------------------------ */

#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef DWORD_PTR
typedef unsigned long DWORD_PTR;
#endif

#ifndef TIME_PERIODIC
#define TIME_ONESHOT            0x0000
#define TIME_PERIODIC           0x0001
#define TIME_CALLBACK_FUNCTION  0x0000

typedef void (CALLBACK *LPTIMECALLBACK)(UINT uTimerID, UINT uMsg,
                                        DWORD_PTR dwUser,
                                        DWORD_PTR dw1, DWORD_PTR dw2);

struct WinmmEventSlot {
    LPTIMECALLBACK proc;
    DWORD_PTR      user;
    UINT           id;
    SDL_TimerID    sdlid;
    bool           periodic;
};

inline WinmmEventSlot* winmm_slots() { static WinmmEventSlot t[64]; return t; }
inline unsigned&       winmm_next()  { static unsigned n;      return n; }

inline Uint32 SDLCALL winmm_trampoline(Uint32 interval, void* param)
{
    WinmmEventSlot* ev = static_cast<WinmmEventSlot*>(param);
    if (ev->proc)
        ev->proc(ev->id, 0, ev->user, 0, 0);
    return ev->periodic ? interval : 0;
}

static inline UINT timeSetEvent(UINT uDelay, UINT /*uResolution*/,
                                LPTIMECALLBACK lpTimeProc,
                                DWORD_PTR dwUser, UINT fuEvent)
{
    if (!lpTimeProc) return 0;
    unsigned slot = winmm_next()++ % 64;
    WinmmEventSlot* ev = winmm_slots() + slot;
    if (ev->sdlid)
        SDL_RemoveTimer(ev->sdlid);
    ev->proc     = lpTimeProc;
    ev->user     = dwUser;
    ev->id       = slot + 1;
    ev->periodic = (fuEvent & TIME_PERIODIC) != 0;
    ev->sdlid    = SDL_AddTimer(uDelay ? uDelay : 1, winmm_trampoline, ev);
    return slot + 1;
}

static inline MMRESULT timeKillEvent(UINT uTimerID)
{
    if (!uTimerID) return TIMERR_NOCANDO;
    SDL_TimerID id = winmm_slots()[(uTimerID - 1) % 64].sdlid;
    if (id)
        SDL_RemoveTimer(id);
    winmm_slots()[(uTimerID - 1) % 64].sdlid = 0;
    return TIMERR_NOERROR;
}
#endif /* !TIME_PERIODIC */

#endif /* __TIMEAPI_H__ */
