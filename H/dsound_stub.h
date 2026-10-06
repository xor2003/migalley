// dsound.h - Linux stub for DirectSound (DX7)
// Compile-surface only: every entry point fails with DSERR_GENERIC so the
// BoB sound layer binds without a working audio backend. Real backend is
// deferred (SDL_mixer path is the permanent route per the engine plan).

#pragma once

#include <string.h>

#ifndef __stdcall
#define __stdcall
#endif

#ifndef HRESULT
typedef long HRESULT;
#endif

#ifndef GUID_DEFINED
struct _GUID { unsigned long d1; unsigned short d2, d3; unsigned char d4[8]; };
typedef struct _GUID GUID;
#endif
typedef GUID* LPGUID;

#ifndef D3DVALUE_DEFINED
#define D3DVALUE_DEFINED
typedef float D3DVALUE;
typedef D3DVALUE* LPD3DVALUE;
#endif

#ifndef DS_OK
#define DS_OK            0
#define DSERR_GENERIC   -1
#define DSERR_INVALIDPARAM (-2)
#define DS_OK_BOOL      1
#endif

#define S_OK ((HRESULT)0)

class IUnknown; // fwd

typedef struct IDirectSound           IDirectSound;
typedef struct IDirectSoundBuffer     IDirectSoundBuffer;
typedef struct IDirectSound3DBuffer   IDirectSound3DBuffer;
typedef struct IDirectSound3DListener IDirectSound3DListener;
typedef struct IDirectSoundNotify     IDirectSoundNotify;

typedef IDirectSound*           LPDIRECTSOUND;
typedef IDirectSoundBuffer*     LPDIRECTSOUNDBUFFER;
typedef IDirectSound3DBuffer*   LPDIRECTSOUND3DBUFFER;
typedef IDirectSound3DListener* LPDIRECTSOUND3DLISTENER;
typedef IDirectSoundNotify*     LPDIRECTSOUNDNOTIFY;

typedef const GUID& REFIID_DSOUND; // keep decls obvious; real REFIID comes from compat when present

// ---- WAVEFORMATEX / PCMWAVEFORMAT ---------------------------------------
// WIN32_COMPAT.H supplies these when present; only define when missing.
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned long  nSamplesPerSec;
    unsigned long  nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
    unsigned short cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX, *PWAVEFORMATEX;
typedef const WAVEFORMATEX* LPCWAVEFORMATEX;
#endif
#ifndef _WAVEFORMAT_
#define _WAVEFORMAT_
typedef struct {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned long  nSamplesPerSec;
    unsigned long  nAvgBytesPerSec;
    unsigned short nBlockAlign;
} WAVEFORMAT, *LPWAVEFORMAT;
typedef struct {
    WAVEFORMAT wf;
    unsigned short wBitsPerSample;
} PCMWAVEFORMAT;
#endif
#define WAVE_FORMAT_PCM 1

// ---- capability/descriptor structs --------------------------------------
typedef struct {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwBufferBytes;
    unsigned long dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
} DSBUFFERDESC, *LPDSBUFFERDESC;
typedef const DSBUFFERDESC* LPCDSBUFFERDESC;

typedef struct {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwBufferBytes;
    unsigned long dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID          guid3DAlgorithm;
} DSBUFFERDESC1, *LPDSBUFFERDESC1;

typedef struct {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwMinSecondarySampleRate;
    unsigned long dwMaxSecondarySampleRate;
    unsigned long dwPrimaryBuffers;
    unsigned long dwMaxHwMixingAllBuffers;
    unsigned long dwMaxHwMixingStaticBuffers;
    unsigned long dwMaxHwMixingStreamingBuffers;
    unsigned long dwFreeHwMixingAllBuffers;
    unsigned long dwFreeHwMixingStaticBuffers;
    unsigned long dwFreeHwMixingStreamingBuffers;
    unsigned long dwMaxHw3DAllBuffers;
    unsigned long dwMaxHw3DStaticBuffers;
    unsigned long dwMaxHw3DStreamingBuffers;
    unsigned long dwFreeHw3DAllBuffers;
    unsigned long dwFreeHw3DStaticBuffers;
    unsigned long dwFreeHw3DStreamingBuffers;
    unsigned long dwTotalHwMemBytes;
    unsigned long dwFreeHwMemBytes;
    unsigned long dwMaxContigFreeHwMemBytes;
    unsigned long dwUnlockTransferRateHwBuffers;
    unsigned long dwPlayCpuOverheadSwBuffers;
    unsigned long dwReserved1;
    unsigned long dwReserved2;
} DSCAPS, *LPDSCAPS;

typedef struct {
    unsigned long dwSize;
    unsigned long dwPlayCursor;
    unsigned long dwWriteCursor;
} DSBCURSORPOS, *LPDSBCURSORPOS;

typedef struct {
    unsigned long dwOffset;
    unsigned long hEventNotify;
} DSBPOSITIONNOTIFY, *LPDSBPOSITIONNOTIFY;
typedef const DSBPOSITIONNOTIFY* LPCDSBPOSITIONNOTIFY;

// ---- flags/constants -----------------------------------------------------
#define DSSCL_NORMAL            0x00000001
#define DSSCL_PRIORITY          0x00000002
#define DSSCL_EXCLUSIVE         0x00000003
#define DSSCL_WRITEPRIMARY      0x00000004

#define DSBCAPS_PRIMARYBUFFER       0x00000001
#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_LOCHARDWARE         0x00000004
#define DSBCAPS_LOCSOFTWARE         0x00000008
#define DSBCAPS_CTRL3D              0x00000010
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRLVOLUME          0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY  0x00000100
#define DSBCAPS_CTRLFX              0x00000200
#define DSBCAPS_STICKYFOCUS         0x00004000
#define DSBCAPS_GLOBALFOCUS         0x00008000
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000
#define DSBCAPS_MUTE3DATMAXDISTANCE 0x00020000
#define DSBCAPS_LOCDEFER            0x00040000
#define DSBCAPS_CTRLDEFAULT         (DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY)

#define DSCAPS_EMULDRIVER           0x00000020
#define DSCAPS_CERTIFIED            0x00000040
#define DSCAPS_SECONDARYMONO        0x00000100
#define DSCAPS_SECONDARYSTEREO      0x00000200
#define DSCAPS_SECONDARY8BIT        0x00000400
#define DSCAPS_SECONDARY16BIT       0x00000800

#define DSBPLAY_LOOPING         0x00000001
#define DSBPLAY_LOCHARDWARE     0x00000002
#define DSBPLAY_LOCSOFTWARE     0x00000004
#define DSBPLAY_TERMINATEBY_TIME    0x00000008
#define DSBPLAY_TERMINATEBY_DISTANCE    0x000000010
#define DSBPLAY_TERMINATEBY_PRIORITY    0x000000020

#define DSBSTATUS_PLAYING       0x00000001
#define DSBSTATUS_BUFFERLOST    0x00000002
#define DSBSTATUS_LOOPING       0x00000004
#define DSBSTATUS_LOCHARDWARE   0x00000008
#define DSBSTATUS_LOCSOFTWARE   0x00000010
#define DSBSTATUS_TERMINATED    0x00000020

#define DSBLOCK_FROMWRITECURSOR 0x00000001
#define DSBLOCK_ENTIREBUFFER    0x00000002

#define DSBVOLUME_MIN           (-10000)
#define DSBVOLUME_MAX           0
#define DSBFREQUENCY_MIN        100
#define DSBFREQUENCY_MAX        200000
#define DSBFREQUENCY_ORIGINAL   0
#define DSBPAN_LEFT             (-10000)
#define DSBPAN_CENTER           0
#define DSBPAN_RIGHT            10000
#define DSBPOSITIONNOTIFY_MIN   0x00000000
#define DSBPOSITIONNOTIFY_MAX   0x000FFFFF

#define DS3DMODE_NORMAL         0x00000000
#define DS3DMODE_HEADRELATIVE   0x00000001
#define DS3DMODE_DISABLE        0x00000002
#define DS3D_IMMEDIATE          0x00000000
#define DS3D_DEFERRED           0x00000001

#ifndef GUID_NULL
#define GUID_NULL_ROWAN
#endif

// ---- interfaces ----------------------------------------------------------
// No vtables required: BoB calls these through C++ method syntax on our
// objects only; no reinterpreted IUnknown* call reaches them.

struct IDirectSound {
    long refCnt;
    IDirectSound(): refCnt(1) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv=0; return DSERR_GENERIC; }
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r=--refCnt; if(!r) delete this; return r; }
    HRESULT CreateSoundBuffer(LPCDSBUFFERDESC, LPDIRECTSOUNDBUFFER* pp, IUnknown*)
        { *pp=0; return DSERR_GENERIC; }
    HRESULT GetCaps(LPDSCAPS caps) { if(caps) memset(caps,0,sizeof(*caps)); return DS_OK; }
    HRESULT SetCooperativeLevel(void*, unsigned long) { return DS_OK; }
    HRESULT Compact() { return DS_OK; }
    HRESULT Initialize(const GUID*) { return DSERR_GENERIC; }
};

struct IDirectSoundBuffer {
    long refCnt;
    IDirectSoundBuffer(): refCnt(1) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv=0; return DSERR_GENERIC; }
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r=--refCnt; if(!r) delete this; return r; }
    HRESULT Lock(unsigned long, unsigned long, void** pp1, unsigned long* pn1,
                 void** pp2, unsigned long* pn2, unsigned long)
        { if(pp1)*pp1=0; if(pn1)*pn1=0; if(pp2)*pp2=0; if(pn2)*pn2=0; return DSERR_GENERIC; }
    HRESULT Unlock(void*, unsigned long, void*, unsigned long) { return DSERR_GENERIC; }
    HRESULT Play(unsigned long, unsigned long, unsigned long) { return DSERR_GENERIC; }
    HRESULT Stop() { return DSERR_GENERIC; }
    HRESULT SetCurrentPosition(unsigned long) { return DSERR_GENERIC; }
    HRESULT GetCurrentPosition(unsigned long* p, unsigned long* w)
        { if(p)*p=0; if(w)*w=0; return DS_OK; }
    HRESULT SetFrequency(unsigned long) { return DSERR_GENERIC; }
    HRESULT GetFrequency(unsigned long* p) { if(p)*p=0; return DS_OK; }
    HRESULT SetVolume(long) { return DSERR_GENERIC; }
    HRESULT GetVolume(long* p) { if(p)*p=DSBVOLUME_MIN; return DS_OK; }
    HRESULT SetPan(long) { return DSERR_GENERIC; }
    HRESULT GetPan(long* p) { if(p)*p=DSBPAN_CENTER; return DS_OK; }
    HRESULT GetStatus(unsigned long* p) { if(p)*p=0; return DS_OK; }
    HRESULT Restore() { return DSERR_GENERIC; }
    HRESULT SetFormat(LPCWAVEFORMATEX) { return DSERR_GENERIC; }
    HRESULT GetFormat(LPWAVEFORMATEX, unsigned long, unsigned long*) { return DSERR_GENERIC; }
    HRESULT GetCaps(void*) { return DSERR_GENERIC; }
    HRESULT Initialize(IDirectSound*, LPCDSBUFFERDESC) { return DSERR_GENERIC; }
};

struct IDirectSound3DBuffer {
    long refCnt;
    IDirectSound3DBuffer(): refCnt(1) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv=0; return DSERR_GENERIC; }
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r=--refCnt; if(!r) delete this; return r; }
    HRESULT SetMaxDistance(D3DVALUE, unsigned long) { return DSERR_GENERIC; }
    HRESULT GetMaxDistance(D3DVALUE* p) { if(p)*p=0; return DS_OK; }
    HRESULT SetMinDistance(D3DVALUE, unsigned long) { return DSERR_GENERIC; }
    HRESULT GetMinDistance(D3DVALUE* p) { if(p)*p=0; return DS_OK; }
    HRESULT SetMode(unsigned long, unsigned long) { return DSERR_GENERIC; }
    HRESULT GetMode(unsigned long* p) { if(p)*p=DS3DMODE_NORMAL; return DS_OK; }
    HRESULT SetPosition(D3DVALUE, D3DVALUE, D3DVALUE, unsigned long) { return DSERR_GENERIC; }
    HRESULT GetPosition(void*) { return DSERR_GENERIC; }
    HRESULT SetVelocity(D3DVALUE, D3DVALUE, D3DVALUE, unsigned long) { return DSERR_GENERIC; }
    HRESULT GetVelocity(void*) { return DSERR_GENERIC; }
};

struct IDirectSound3DListener {
    long refCnt;
    IDirectSound3DListener(): refCnt(1) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv=0; return DSERR_GENERIC; }
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r=--refCnt; if(!r) delete this; return r; }
    HRESULT CommitDeferredSettings() { return DS_OK; }
    HRESULT SetDistanceFactor(D3DVALUE, unsigned long) { return DS_OK; }
    HRESULT SetDopplerFactor(D3DVALUE, unsigned long) { return DS_OK; }
    HRESULT SetRolloffFactor(D3DVALUE, unsigned long) { return DS_OK; }
    HRESULT SetOrientation(D3DVALUE,D3DVALUE,D3DVALUE,D3DVALUE,D3DVALUE,D3DVALUE,unsigned long) { return DS_OK; }
    HRESULT SetPosition(D3DVALUE, D3DVALUE, D3DVALUE, unsigned long) { return DS_OK; }
    HRESULT SetVelocity(D3DVALUE, D3DVALUE, D3DVALUE, unsigned long) { return DS_OK; }
};

struct IDirectSoundNotify {
    HRESULT SetNotificationPositions(unsigned long, LPCDSBPOSITIONNOTIFY) { return DSERR_GENERIC; }
};

// ---- module functions ----------------------------------------------------
typedef int (DSENUMCALLBACKFN)(LPGUID, const char*, const char*, void*);
typedef DSENUMCALLBACKFN* LPDSENUMCALLBACK;
typedef int (__stdcall* LPDSENUMCALLBACKA)(LPGUID, const char*, const char*, void*);
typedef int (__stdcall* LPDSENUMCALLBACKW)(LPGUID, const unsigned short*, const unsigned short*, void*);

static inline HRESULT DirectSoundCreate(const GUID* lpGuid,
                                        IDirectSound** ppDS,
                                        IUnknown* pUnkOuter)
{
    (void)lpGuid; (void)pUnkOuter;
    if (ppDS) *ppDS = 0;
    return DSERR_GENERIC;
}

static inline HRESULT DirectSoundEnumerateA(LPDSENUMCALLBACK cb, void* ctx)
{
    // No devices: never invoke the callback — callers fall back to NULL GUID.
    (void)cb; (void)ctx; return DS_OK;
}
#define DirectSoundEnumerate DirectSoundEnumerateA

// SDK C-call macros (dsound.h ships these for C consumers; C++ callers use
// the methods directly but legacy code uses the macro spelling too).
#define IDirectSound_SetCooperativeLevel(p,h,f)   (p)->SetCooperativeLevel(h,f)
#define IDirectSound_CreateSoundBuffer(p,d,b,u)   (p)->CreateSoundBuffer(d,b,u)
#define IDirectSound_GetCaps(p,c)                 (p)->GetCaps(c)
#define IDirectSound_Release(p)                   (p)->Release()
#define IDirectSoundBuffer_Play(p,a,b,f)          (p)->Play(a,b,f)
#define IDirectSoundBuffer_Stop(p)                (p)->Stop()
#define IDirectSoundBuffer_Release(p)             (p)->Release()
#define IDirectSoundBuffer_SetVolume(p,v)         (p)->SetVolume(v)
#define IDirectSoundBuffer_SetPan(p,v)            (p)->SetPan(v)
#define IDirectSoundBuffer_SetFrequency(p,v)      (p)->SetFrequency(v)
#define IDirectSoundBuffer_SetCurrentPosition(p,v) (p)->SetCurrentPosition(v)
