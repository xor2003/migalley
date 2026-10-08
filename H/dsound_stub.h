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

// ---- SDL2 software-mixer backend -----------------------------------------
// Real DX7-semantic implementation: secondary buffers hold PCM bytes the
// game Lock()s and fills; an SDL audio callback mixes every playing buffer
// into the output applying volume (hundredths-dB), pan, resampling for
// SetFrequency, and approximate 3D distance attenuation + bearing panning
// from the listener state. No vtables required: BoB calls these through
// C++ method syntax on our objects only.

#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <math.h>

namespace rowan_ds {

struct Listener {
    float px,py,pz, vx,vy,vz, fx,fy,fz, ux,uy,uz;
    float distFactor, doppler, rolloff;
    Listener() : px(0),py(0),pz(0), vx(0),vy(0),vz(0),
                 fx(0),fy(0),fz(1), ux(0),uy(1),uz(0),
                 distFactor(1),doppler(1),rolloff(1) {}
};

struct MixState {
    SDL_AudioDeviceID dev;
    int freq, ch;
    std::vector<IDirectSoundBuffer*> bufs;
    Listener lis;
    bool openFailed;
    // env-gated diagnostics (ROWAN_DEBUG_SOUND): API call counters + mix stats
    unsigned long nCreate, nLock, nUnlock, nPlay, nStop, nFramesMixed;
    int dbgSound, dbgLogCtr, dbgPeak;
    MixState() : dev(0), freq(22050), ch(2), openFailed(false),
                 nCreate(0), nLock(0), nUnlock(0), nPlay(0), nStop(0),
                 nFramesMixed(0), dbgSound(-1), dbgLogCtr(0), dbgPeak(0) {}
};

inline MixState& S() { static MixState s; return s; }
inline void Lock_()  { if (S().dev) SDL_LockAudioDevice(S().dev); }
inline void Unlock_(){ if (S().dev) SDL_UnlockAudioDevice(S().dev); }

void AudioCb(void* userdata, Uint8* stream, int len);  // defined below IDirectSoundBuffer

inline void CloseDeviceAtExit()
{
    MixState& s = S();
    if (s.dev) { SDL_CloseAudioDevice(s.dev); s.dev = 0; }
}

inline void EnsureDevice()
{
    MixState& s = S();
    if (s.dev || s.openFailed) return;
    if (!SDL_WasInit(SDL_INIT_AUDIO)) SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = s.freq;
    want.format = AUDIO_S16SYS;
    want.channels = (Uint8)s.ch;
    want.samples = 1024;
    want.callback = AudioCb;
    s.dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (s.dev) {
        SDL_PauseAudioDevice(s.dev, 0);
        // Close the device before MixState's static dtor frees bufs at
        // exit: SDL_CloseAudioDevice joins the audio thread, and this
        // atexit (registered after S() was constructed) runs before it.
        std::atexit(CloseDeviceAtExit);
        fprintf(stderr, "[dsound] SDL2 audio device open: %d Hz %d ch\n", s.freq, s.ch);
    } else {
        s.openFailed = true;
        fprintf(stderr, "[dsound] SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    }
}

} // namespace rowan_ds

// IID d1 values (DX7 SDK; see Bob/port/cguid.h)
#define ROWAN_IID_DS3DLISTENER_D1 0x279AFA84UL
#define ROWAN_IID_DS3DBUFFER_D1   0x279AFA86UL
#define ROWAN_IID_DSNOTIFY_D1     0xB0210783UL

// first GUID word regardless of which GUID definition won (WIN32_COMPAT.H
// uses Data1; the fallback above uses d1 — same layout, read it by memcpy)
inline unsigned long RowanGuidD1(const GUID& g)
{
    unsigned long v; memcpy(&v, &g, sizeof v); return v;
}

struct IDirectSound3DBuffer;    // fwd
struct IDirectSound3DListener;  // fwd
IDirectSound3DListener* RowanDSListener();  // defined below IDirectSound3DListener

struct IDirectSoundBuffer {
    long refCnt;
    // ---- backend state ----
    std::vector<unsigned char> pcm;  // PCM bytes, Lock() writes into this
    WAVEFORMATEX fmt;
    unsigned long capsFlags;
    bool   primary, is3d;
    bool   playing, looping, lost;
    double cursor;                   // fractional frame position
    long   vol;                      // hundredths of dB (0..-10000)
    long   panv;                     // -10000..10000
    unsigned long freq;              // Hz, 0 = format rate
    // 3D state
    float  px,py,pz, pvx,pvy,pvz, mind, maxd;
    unsigned long mode3d;
    IDirectSound3DBuffer* buf3d;     // facade, created on first QI

    IDirectSoundBuffer()
        : refCnt(1), capsFlags(0), primary(false), is3d(false),
          playing(false), looping(false), lost(false), cursor(0.0),
          vol(0), panv(0), freq(0),
          px(0),py(0),pz(0), pvx(0),pvy(0),pvz(0),
          mind(1.0f), maxd(1000000000.0f), mode3d(DS3DMODE_NORMAL),
          buf3d(0)
    { memset(&fmt, 0, sizeof fmt); }

    ~IDirectSoundBuffer();   // defined after IDirectSound3DBuffer (complete type needed)

    HRESULT QueryInterface(const GUID& riid, void** ppv);   // defined below IDirectSound3DBuffer
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r = refCnt>0 ? --refCnt : 0; if(!r) delete this; return r; }

    int FrameBytes() const
    {
        int fb = fmt.nBlockAlign;
        if (!fb) fb = (fmt.nChannels ? fmt.nChannels : 1) * ((fmt.wBitsPerSample ? fmt.wBitsPerSample : 16) / 8);
        return fb;
    }
    unsigned long Frames() const { int fb = FrameBytes(); return fb ? (unsigned long)(pcm.size() / fb) : 0; }

    HRESULT Lock(unsigned long off, unsigned long bytes, void** pp1, unsigned long* pn1,
                 void** pp2, unsigned long* pn2, unsigned long flags)
    {
        if (primary || pcm.empty()) return DSERR_GENERIC;
        if (off >= pcm.size()) off = 0;
        if (flags & DSBLOCK_ENTIREBUFFER) { off = 0; bytes = (unsigned long)pcm.size(); }
        if (!bytes) bytes = (unsigned long)pcm.size() - off;
        unsigned long size = (unsigned long)pcm.size();
        unsigned long first = bytes;
        if (off + first > size) first = size - off;
        if (pp1) *pp1 = pcm.data() + off;
        if (pn1) *pn1 = first;
        if (off + bytes > size) {              // wrap region
            if (pp2) *pp2 = pcm.data();
            if (pn2) *pn2 = bytes - first;
        } else {
            if (pp2) *pp2 = 0;
            if (pn2) *pn2 = 0;
        }
        rowan_ds::S().nLock++;
        return DS_OK;
    }
    HRESULT Unlock(void*, unsigned long, void*, unsigned long) { rowan_ds::S().nUnlock++; return DS_OK; }

    HRESULT Play(unsigned long, unsigned long, unsigned long dwFlags)
    {
        if (primary) return DS_OK;
        rowan_ds::EnsureDevice();
        rowan_ds::Lock_();
        playing = true;
        looping = (dwFlags & DSBPLAY_LOOPING) != 0;
        if (cursor >= Frames()) cursor = 0;
        rowan_ds::S().nPlay++;
        rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT Stop() { rowan_ds::Lock_(); playing = false; rowan_ds::S().nStop++; rowan_ds::Unlock_(); return DS_OK; }
    HRESULT SetCurrentPosition(unsigned long pos)
    {
        rowan_ds::Lock_();
        cursor = (double)pos / FrameBytes();
        if (cursor >= Frames()) cursor = Frames() ? Frames() - 1 : 0;
        rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT GetCurrentPosition(unsigned long* p, unsigned long* w)
    {
        rowan_ds::Lock_();
        unsigned long bytepos = (unsigned long)(cursor * FrameBytes());
        if (p) *p = bytepos;
        if (w) *w = bytepos;
        rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT SetFrequency(unsigned long f)
    {
        if (f && (f < DSBFREQUENCY_MIN || f > DSBFREQUENCY_MAX)) return DSERR_INVALIDPARAM;
        rowan_ds::Lock_(); freq = f; rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT GetFrequency(unsigned long* p) { if (p) *p = freq ? freq : fmt.nSamplesPerSec; return DS_OK; }
    HRESULT SetVolume(long v)
    {
        if (v < DSBVOLUME_MIN) v = DSBVOLUME_MIN;
        if (v > DSBVOLUME_MAX) v = DSBVOLUME_MAX;
        rowan_ds::Lock_(); vol = v; rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT GetVolume(long* p) { if (p) *p = vol; return DS_OK; }
    HRESULT SetPan(long p)
    {
        if (p < DSBPAN_LEFT) p = DSBPAN_LEFT;
        if (p > DSBPAN_RIGHT) p = DSBPAN_RIGHT;
        rowan_ds::Lock_(); panv = p; rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT GetPan(long* p) { if (p) *p = panv; return DS_OK; }
    HRESULT GetStatus(unsigned long* p)
    {
        if (p) {
            rowan_ds::Lock_();
            *p = (playing ? DSBSTATUS_PLAYING : 0)
               | (looping ? DSBSTATUS_LOOPING : 0)
               | (lost    ? DSBSTATUS_BUFFERLOST : 0);
            rowan_ds::Unlock_();
        }
        return DS_OK;
    }
    HRESULT Restore() { lost = false; return DS_OK; }
    HRESULT SetFormat(LPCWAVEFORMATEX wfx)
    {
        if (!primary || !wfx) return DSERR_GENERIC;
        // honor the primary output rate if sane; reopen device
        int f = (int)wfx->nSamplesPerSec;
        rowan_ds::MixState& s = rowan_ds::S();
        if (f >= 8000 && f <= 96000 && f != s.freq) {
            s.freq = f;
            s.ch = (wfx->nChannels >= 1 && wfx->nChannels <= 2) ? wfx->nChannels : 2;
            if (s.dev) { SDL_CloseAudioDevice(s.dev); s.dev = 0; }
            rowan_ds::EnsureDevice();
        }
        return DS_OK;
    }
    HRESULT GetFormat(LPWAVEFORMATEX p, unsigned long n, unsigned long* written)
    {
        if (p) { memcpy(p, &fmt, n < sizeof fmt ? n : sizeof fmt); }
        if (written) *written = sizeof fmt;
        return DS_OK;
    }
    HRESULT GetCaps(void*) { return DS_OK; }
    HRESULT Initialize(IDirectSound*, LPCDSBUFFERDESC) { return DSERR_GENERIC; }
};

// 3D buffer facade — forwards into the owning secondary buffer's state.
struct IDirectSound3DBuffer {
    long refCnt;
    IDirectSoundBuffer* owner;
    IDirectSound3DBuffer(IDirectSoundBuffer* o) : refCnt(0), owner(o) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv = 0; return DSERR_GENERIC; }
    unsigned long AddRef()  { if (owner) owner->AddRef(); return ++refCnt; }
    unsigned long Release()
    {
        unsigned long r = refCnt ? --refCnt : 0;
        IDirectSoundBuffer* o = owner;
        if (!r) delete this;
        if (o) o->Release();
        return r;
    }
    HRESULT SetMaxDistance(D3DVALUE d, unsigned long) { if(owner){owner->maxd=d;} return DS_OK; }
    HRESULT GetMaxDistance(D3DVALUE* p) { if(p)*p=owner?owner->maxd:0; return DS_OK; }
    HRESULT SetMinDistance(D3DVALUE d, unsigned long) { if(owner){owner->mind=d>0?d:0.001f;} return DS_OK; }
    HRESULT GetMinDistance(D3DVALUE* p) { if(p)*p=owner?owner->mind:0; return DS_OK; }
    HRESULT SetMode(unsigned long m, unsigned long) { if(owner)owner->mode3d=m; return DS_OK; }
    HRESULT GetMode(unsigned long* p) { if(p)*p=owner?owner->mode3d:0; return DS_OK; }
    HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, unsigned long)
        { if(owner){rowan_ds::Lock_();owner->px=x;owner->py=y;owner->pz=z;rowan_ds::Unlock_();} return DS_OK; }
    HRESULT GetPosition(void* v)
        { if(v&&owner){((D3DVALUE*)v)[0]=owner->px;((D3DVALUE*)v)[1]=owner->py;((D3DVALUE*)v)[2]=owner->pz;} return DS_OK; }
    HRESULT SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, unsigned long)
        { if(owner){owner->pvx=x;owner->pvy=y;owner->pvz=z;} return DS_OK; }
    HRESULT GetVelocity(void* v)
        { if(v&&owner){((D3DVALUE*)v)[0]=owner->pvx;((D3DVALUE*)v)[1]=owner->pvy;((D3DVALUE*)v)[2]=owner->pvz;} return DS_OK; }
};

inline HRESULT IDirectSoundBuffer::QueryInterface(const GUID& riid, void** ppv)
{
    if (!ppv) return DSERR_INVALIDPARAM;
    *ppv = 0;
    if (RowanGuidD1(riid) == ROWAN_IID_DS3DBUFFER_D1) {
        if (!is3d || primary) return DSERR_GENERIC;
        if (!buf3d) buf3d = new IDirectSound3DBuffer(this);
        buf3d->AddRef();
        *ppv = buf3d;
        return S_OK;
    }
    if (RowanGuidD1(riid) == ROWAN_IID_DS3DLISTENER_D1 && primary) {
        *ppv = RowanDSListener();
        return *ppv ? S_OK : DSERR_GENERIC;
    }
    return DSERR_GENERIC;
}

inline IDirectSoundBuffer::~IDirectSoundBuffer()
{
    rowan_ds::Lock_();
    std::vector<IDirectSoundBuffer*>& v = rowan_ds::S().bufs;
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == this) { v.erase(v.begin()+i); break; }
    rowan_ds::Unlock_();
    delete buf3d;   // facade only forwards into us — plain delete, no Release chain
}

// 3D listener facade — writes into the global mixer listener state.
struct IDirectSound3DListener {
    long refCnt;
    IDirectSound3DListener() : refCnt(1) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv = 0; return DSERR_GENERIC; }
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r=--refCnt; if(!r) delete this; return r; }
    HRESULT CommitDeferredSettings() { return DS_OK; }
    HRESULT SetDistanceFactor(D3DVALUE f, unsigned long) { rowan_ds::S().lis.distFactor=f; return DS_OK; }
    HRESULT SetDopplerFactor(D3DVALUE f, unsigned long)  { rowan_ds::S().lis.doppler=f;    return DS_OK; }
    HRESULT SetRolloffFactor(D3DVALUE f, unsigned long)  { rowan_ds::S().lis.rolloff=f;    return DS_OK; }
    HRESULT SetOrientation(D3DVALUE fx,D3DVALUE fy,D3DVALUE fz,
                           D3DVALUE ux,D3DVALUE uy,D3DVALUE uz, unsigned long)
    {
        rowan_ds::Listener& l = rowan_ds::S().lis;
        rowan_ds::Lock_();
        l.fx=fx; l.fy=fy; l.fz=fz; l.ux=ux; l.uy=uy; l.uz=uz;
        rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, unsigned long)
    {
        rowan_ds::Listener& l = rowan_ds::S().lis;
        rowan_ds::Lock_(); l.px=x; l.py=y; l.pz=z; rowan_ds::Unlock_();
        return DS_OK;
    }
    HRESULT SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, unsigned long)
    {
        rowan_ds::Listener& l = rowan_ds::S().lis;
        l.vx=x; l.vy=y; l.vz=z; return DS_OK;
    }
};

// one shared listener facade (3D listener is a singleton resource in DS)
inline IDirectSound3DListener* RowanDSListener()
{
    static IDirectSound3DListener* l = new IDirectSound3DListener();
    l->AddRef();
    return l;
}

namespace rowan_ds {

// decode one source frame (mono or stereo, 8/16-bit PCM) into L/R floats
inline void DecodeFrame(const IDirectSoundBuffer* b, unsigned long frame, float& L, float& R)
{
    const unsigned char* d = b->pcm.data();
    int ch = b->fmt.nChannels ? b->fmt.nChannels : 1;
    int bits = b->fmt.wBitsPerSample ? b->fmt.wBitsPerSample : 16;
    unsigned long off = frame * b->FrameBytes();
    if (off >= b->pcm.size()) { L = R = 0; return; }
    if (bits == 16) {
        const short* s = (const short*)(d + off);
        L = (float)s[0] / 32768.0f;
        R = (ch == 2 && off + 3 < b->pcm.size()) ? (float)s[1] / 32768.0f : L;
    } else {
        L = ((float)d[off] - 128.0f) / 128.0f;
        R = (ch == 2 && off + 1 < b->pcm.size()) ? ((float)d[off+1] - 128.0f) / 128.0f : L;
    }
}

inline void AudioCb(void*, Uint8* stream, int len)
{
    MixState& s = S();
    int frames = len / (2 * s.ch);
    short* out = (short*)stream;
    memset(stream, 0, len);

    for (size_t bi = 0; bi < s.bufs.size(); ++bi) {
        IDirectSoundBuffer* b = s.bufs[bi];
        if (!b->playing || b->lost || b->pcm.empty()) continue;
        unsigned long nfr = b->Frames();
        if (!nfr) { b->playing = false; continue; }

        unsigned long srcRate = b->freq ? b->freq : (b->fmt.nSamplesPerSec ? b->fmt.nSamplesPerSec : 22050);
        double step = (double)srcRate / s.freq;
        float g = (float)pow(10.0, b->vol / 2000.0);
        // pan: DS pan is hundredths-dB attenuation of the opposite channel
        float pl = g, pr = g;
        if (b->panv > 0)      pl = g * (float)pow(10.0, -b->panv / 2000.0);
        else if (b->panv < 0) pr = g * (float)pow(10.0,  b->panv / 2000.0);

        // 3D: distance attenuation + bearing pan from listener state
        if (b->is3d && b->mode3d != DS3DMODE_DISABLE) {
            Listener& l = s.lis;
            float rx = b->px - l.px, ry = b->py - l.py, rz = b->pz - l.pz;
            if (b->mode3d == DS3DMODE_HEADRELATIVE) { rx = b->px; ry = b->py; rz = b->pz; }
            float dist = sqrtf(rx*rx + ry*ry + rz*rz) * l.distFactor;
            float att = 1.0f;
            if (dist > b->mind) {
                att = b->mind / (b->mind + l.rolloff * (dist - b->mind));
                if (dist > b->maxd) {
                    if (b->capsFlags & DSBCAPS_MUTE3DATMAXDISTANCE) { b->playing = false; continue; }
                    att = b->mind / (b->mind + l.rolloff * (b->maxd - b->mind));
                }
            }
            // bearing pan: right = front x top (LH cross -> screen right)
            float crx = l.fy*l.uz - l.fz*l.uy;
            float cry = l.fz*l.ux - l.fx*l.uz;
            float crz = l.fx*l.uy - l.fy*l.ux;
            float rl = sqrtf(crx*crx + cry*cry + crz*crz);
            float pan3 = 0;
            if (rl > 0.001f && dist > 0.001f)
                pan3 = (rx*crx + ry*cry + rz*crz) / (dist * rl * l.distFactor);
            if (pan3 > 1) pan3 = 1; if (pan3 < -1) pan3 = -1;
            float g3 = g * att;
            pl = g3 * (1.0f - (pan3 > 0 ? pan3 : 0) * 0.7f);
            pr = g3 * (1.0f + (pan3 < 0 ? pan3 : 0) * 0.7f);
        }

        for (int i = 0; i < frames; ++i) {
            unsigned long f = (unsigned long)b->cursor;
            if (f >= nfr) {
                if (b->looping) { b->cursor = fmod(b->cursor, (double)nfr); f = (unsigned long)b->cursor; }
                else { b->playing = false; break; }
            }
            float L, R;
            DecodeFrame(b, f, L, R);
            int accL = out[i*s.ch]   + (int)(L * pl * 32767.0f);
            int accR = out[i*s.ch+1] + (int)(R * pr * 32767.0f);
            if (accL > 32767) accL = 32767; if (accL < -32768) accL = -32768;
            if (accR > 32767) accR = 32767; if (accR < -32768) accR = -32768;
            out[i*s.ch]   = (short)accL;
            out[i*s.ch+1] = (short)accR;
            b->cursor += step;
        }
    }

    if (s.dbgSound < 0) s.dbgSound = getenv("ROWAN_DEBUG_SOUND") ? 1 : 0;
    if (s.dbgSound) {
        s.nFramesMixed += frames;
        for (int i = 0; i < frames * s.ch; ++i)
            if (out[i] > s.dbgPeak || -out[i] > s.dbgPeak)
                s.dbgPeak = out[i] > 0 ? out[i] : -out[i];
        if (++s.dbgLogCtr >= 200) {   // ~9s at 22050Hz/1024
            s.dbgLogCtr = 0;
            int live = 0;
            for (size_t i = 0; i < s.bufs.size(); ++i) if (s.bufs[i]->playing) live++;
            fprintf(stderr, "[dsound] bufs=%u live=%d create=%lu lock=%lu unlock=%lu play=%lu stop=%lu mixed=%lu peak=%d\n",
                    (unsigned)s.bufs.size(), live, s.nCreate, s.nLock, s.nUnlock,
                    s.nPlay, s.nStop, s.nFramesMixed, s.dbgPeak);
            s.dbgPeak = 0;
        }
    }
}

} // namespace rowan_ds

struct IDirectSound {
    long refCnt;
    IDirectSound() : refCnt(1) {}
    HRESULT QueryInterface(const GUID&, void** ppv) { *ppv = 0; return DSERR_GENERIC; }
    unsigned long AddRef()  { return ++refCnt; }
    unsigned long Release() { unsigned long r=--refCnt; if(!r) delete this; return r; }
    HRESULT CreateSoundBuffer(LPCDSBUFFERDESC d, LPDIRECTSOUNDBUFFER* pp, IUnknown*)
    {
        if (!pp) return DSERR_INVALIDPARAM;
        *pp = 0;
        if (!d) return DSERR_INVALIDPARAM;
        IDirectSoundBuffer* b = new IDirectSoundBuffer();
        b->capsFlags = d->dwFlags;
        if (d->dwFlags & DSBCAPS_PRIMARYBUFFER) {
            b->primary = true;
        } else {
            if (d->lpwfxFormat) b->fmt = *d->lpwfxFormat;
            b->pcm.assign(d->dwBufferBytes ? d->dwBufferBytes : 1, 0);
            b->is3d = (d->dwFlags & DSBCAPS_CTRL3D) != 0;
            rowan_ds::Lock_();
            rowan_ds::S().bufs.push_back(b);
            rowan_ds::Unlock_();
        }
        *pp = b;
        rowan_ds::S().nCreate++;
        return DS_OK;
    }
    HRESULT GetCaps(LPDSCAPS caps)
    {
        if (caps) {
            memset(caps, 0, sizeof(*caps));
            caps->dwSize = sizeof(*caps);
            caps->dwMinSecondarySampleRate = DSBFREQUENCY_MIN;
            caps->dwMaxSecondarySampleRate = 48000;
            caps->dwPrimaryBuffers = 1;
            caps->dwMaxHwMixingAllBuffers = caps->dwFreeHwMixingAllBuffers = 128;
            caps->dwMaxHw3DAllBuffers     = caps->dwFreeHw3DAllBuffers     = 64;
            caps->dwFlags = DSCAPS_SECONDARYMONO | DSCAPS_SECONDARYSTEREO
                          | DSCAPS_SECONDARY8BIT | DSCAPS_SECONDARY16BIT;
        }
        return DS_OK;
    }
    HRESULT SetCooperativeLevel(void*, unsigned long) { return DS_OK; }
    HRESULT Compact() { return DS_OK; }
    HRESULT Initialize(const GUID*) { return DS_OK; }
};

struct IDirectSoundNotify {
    HRESULT SetNotificationPositions(unsigned long, LPCDSBPOSITIONNOTIFY) { return DS_OK; }
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
    if (!ppDS) return DSERR_INVALIDPARAM;
    rowan_ds::EnsureDevice();
    *ppDS = new IDirectSound();
    return DS_OK;
}

static inline HRESULT DirectSoundEnumerateA(LPDSENUMCALLBACK cb, void* ctx)
{
    // Report one synthetic device so callers that enumerate get an entry;
    // DirectSoundCreate always opens the SDL2 default device regardless.
    if (cb) cb(NULL, "SDL2 Audio Device", "sdl2", ctx);
    return DS_OK;
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
