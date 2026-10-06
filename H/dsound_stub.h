// dsound.h - Linux stub for DirectSound (DX7)
// Non-functional placeholder for compilation only.

#pragma once

typedef long HRESULT;
#define DS_OK            0
#define DSERR_GENERIC   -1

typedef struct IDirectSound       IDirectSound;
typedef struct IDirectSoundBuffer IDirectSoundBuffer;

struct IDirectSound       { void* unused; };
struct IDirectSoundBuffer { void* unused; };

/* LP-prefixed spellings used by the DirectMusic headers */
typedef IDirectSound*       LPDIRECTSOUND;
typedef IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
typedef struct IDirectSound3DListener { void* unused; } IDirectSound3DListener;
typedef IDirectSound3DListener*         LPDIRECTSOUND3DLISTENER;

/* Buffer descriptor — fields sized like the real DX7 struct */
typedef struct {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwBufferBytes;
    unsigned long dwReserved;
    void*         lpwfxFormat;
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef struct {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwBufferBytes;
    unsigned long dwReserved;
    void*         lpwfxFormat;
    void*         guid3DAlgorithm;
} DSBUFFERDESC1, *LPDSBUFFERDESC1;

// Cooperative levels
#define DSSCL_NORMAL 0

// Function stubs
static inline HRESULT DirectSoundCreate(const void* lpGuid,
                                        IDirectSound** ppDS,
                                        void* pUnkOuter)
{
    (void)lpGuid; (void)ppDS; (void)pUnkOuter;
    return DSERR_GENERIC;
}

