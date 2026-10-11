// mssw_compat.h, a miles stub
#ifndef MSSW_COMPAT_H
#define MSSW_COMPAT_H

#include <SDL2/SDL.h>
#include <AL/al.h>
#include <AL/alc.h>

// Force natural layout - straddles DOSDEFS.H's leaked pack(1) across TUs (ODR).
#pragma pack(push)
#pragma pack()

///// RERUN Miles to SDL Start
#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----- MSS 4.0e type aliases -----
typedef struct _SEQUENCE* HSEQUENCE;
typedef MDI_DRIVER* HMDIDRIVER;

typedef SLong (*MIDIEventCallback)(HMDIDRIVER mdi,
                                   HSEQUENCE seq,
                                   SLong status,
                                   SLong data1,
                                   SLong data2);

typedef struct _MDI_DRIVER {
    int opened;
    int driver_id;
    int master_volume; // 0–127
    MIDIEventCallback callback;
} MDI_DRIVER;

typedef struct _DIG_DRIVER* HDIGDRIVER;
typedef struct _MDI_DRIVER* HMDIDRIVER;
typedef struct _SAMPLE* HSAMPLE;

// Basic types
typedef void* HSTREAM;
typedef void* H3DSAMPLE;
typedef void* HPROVIDER;

typedef void* HDLSFILEID;
typedef void* HWAVESYNTH;
typedef void* HDLSDEVICE;

#define HMIDI MDI_DRIVER*

// ------------------------------------------------------------------
// Sample format constants
// ------------------------------------------------------------------
#define DIG_F_MONO_8   0x0001  // 8-bit mono format
#define DIG_F_STEREO_8 0x0002  // (optional) 8-bit stereo
#define DIG_F_MONO_16  0x0004  // (optional) 16-bit mono
#define DIG_F_STEREO_16 0x0008 // (optional) 16-bit stereo

// ----- Status flags -----
#define SMP_FREE     0x0001
#define SMP_DONE     0x0002
#define SMP_PLAYING  0x0004
#define SMP_STOPPED  0x0008

#define SEQ_FREE     0x0001
#define SEQ_DONE     0x0002
#define SEQ_PLAYING  0x0004
#define SEQ_STOPPED  0x0008

#ifndef MIDI_NULL_DRIVER
#define MIDI_NULL_DRIVER 0xFFFF
#endif

#define WAVE_FORMAT_PCM 1

// ------------------------------------------------------------------
// File type constants
// ------------------------------------------------------------------
#define AILFILETYPE_XMIDI      1   // Standard XMIDI file
#define AILFILETYPE_XMIDI_DLS  2   // XMIDI with DLS instrument set
#define AILFILETYPE_XMIDI_MLS  3   // XMIDI with MLS instrument set

// ------------------------------------------------------------------
// Constant used by AIL_DLS_close
// ------------------------------------------------------------------
#define RETURN_TO_GM_ONLY_STATE 1

// ----- Shim structures -----
typedef struct _SAMPLE {
    Mix_Chunk* chunk;
    int channel;
    int status;
    int volume;        // 0..127
    int pan;           // 0..127
    int loop_count;    // SDL_mixer loops parameter
    void (*EOS)(HSAMPLE);
    long playback_rate;

    long format;       // MSS-side format code (optional)
    long flags;        // DIG_F_* hints
    const void* address;
    long length;

    int orig_rate;             // parsed from WAV if available
    int channels;              // 1 or 2
    SDL_AudioFormat orig_format; // AUDIO_U8 or AUDIO_S16SYS
} SAMPLE;

typedef struct _SEQUENCE {
    Mix_Music* music;
    int status;
    int volume;   // 0..127
    int track;
    int initialized;
} SEQUENCE;

typedef SEQUENCE* HSEQUENCE;

// ----- Initialization -----
static inline int AIL_startup(void) {
    // Try to find a soundfont if not configured
    if (!SDL_getenv("SDL_SOUNDFONTS")) {
        const char* fonts[] = {
            "/usr/share/sounds/sf2/FluidR3_GM.sf2",
            "/usr/share/soundfonts/FluidR3_GM.sf2",
            "/usr/share/sounds/sf2/default.sf2",
            "/usr/share/soundfonts/default.sf2",
            NULL
        };
        for (int i = 0; fonts[i]; i++) {
            FILE* f = fopen(fonts[i], "rb");
            if (f) {
                fclose(f);
                SDL_setenv("SDL_SOUNDFONTS", fonts[i], 1);
                break;
            }
        }
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) return 0;
    Mix_AllocateChannels(64);
    return 1;
}

static inline void AIL_shutdown(void) {
    Mix_CloseAudio();
}

static inline void AIL_set_DirectSound_HWND(HDIGDRIVER dig, void* hwnd) {
    (void)dig;
    (void)hwnd;
    // No-op in SDL2: DirectSound HWND binding not required
}

// ----- Sample API -----
static inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER d) {
    (void)d;
    SAMPLE* s = (SAMPLE*)SDL_calloc(1, sizeof(SAMPLE));
    if (!s) return NULL;
    s->status = SMP_FREE;
    s->volume = 127;
    s->pan = 64;
    return (HSAMPLE)s;
}

static inline void AIL_release_sample_handle(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (s->channel >= 0) Mix_HaltChannel(s->channel);
    if (s->chunk) Mix_FreeChunk(s->chunk);
    SDL_free(s);
}

// Minimal RIFF parser to compute WAV length
static long detect_wav_length(const void* address) {
    const unsigned char* buf = (const unsigned char*)address;
    if (!buf) return 0;

    // Must start with "RIFF"
    if (memcmp(buf, "RIFF", 4) != 0) return 0;
    // Must have "WAVE" at offset 8
    if (memcmp(buf + 8, "WAVE", 4) != 0) return 0;

    // Read little-endian 32-bit size at offset 4
    uint32_t riffSize = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);

    // Total length = riffSize + 8
    return (long)riffSize + 8;
}


static int parse_wav_fmt(const unsigned char* buf, int len,
                         int* out_rate, int* out_channels, SDL_AudioFormat* out_fmt)
{
    if (!buf || len < 44) return 0;
    const unsigned char* p = buf + 12;
    const unsigned char* end = buf + len;
    while (p + 8 <= end) {
        uint32_t ckid = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
        uint32_t cksz = p[4] | (p[5]<<8) | (p[6]<<16) | (p[7]<<24);
        if (p + 8 + cksz > end) break;
        if (ckid == 0x20746D66) { // "fmt "
            uint16_t audioFormat   = p[8] | (p[9] << 8);
            uint16_t numChannels   = p[10] | (p[11] << 8);
            uint32_t sampleRate    = p[12] | (p[13] << 8) | (p[14] << 16) | (p[15] << 24);
            uint16_t bitsPerSample = p[22] | (p[23] << 8);
            if (audioFormat == WAVE_FORMAT_PCM) {
                *out_rate     = (int)sampleRate;
                *out_channels = (int)numChannels;
                *out_fmt      = (bitsPerSample == 8) ? AUDIO_U8 : AUDIO_S16SYS;
                return 1;
            }
            break;
        }
        p += 8 + cksz;
    }
    return 0;
}

static inline int AIL_set_sample_file(HSAMPLE h, const void* address, long length) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s || !address) return 0;

    if (s->chunk) { Mix_FreeChunk(s->chunk); s->chunk = NULL; }

    if (length == -1) {
        length = detect_wav_length(address);
        if (length <= 0) return 0;
    }

    int rate = 0, chans = 1;
    SDL_AudioFormat fmt = AUDIO_S16SYS;
    parse_wav_fmt((const unsigned char*)address, (int)length, &rate, &chans, &fmt);
    s->orig_rate   = rate ? rate : 44100;
    s->channels    = chans;
    s->orig_format = fmt;

    SDL_RWops* rw = SDL_RWFromConstMem(address, length);
    if (!rw) return 0;

    s->chunk = Mix_LoadWAV_RW(rw, 1);
    if (!s->chunk) return 0;

    s->address = address;
    s->length  = length;
    s->status  = SMP_DONE;
    return 1;
}


static inline void AIL_set_sample_volume(HSAMPLE h, int vol) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (vol < 0) vol = 0; if (vol > 127) vol = 127;
    s->volume = vol;
    if (s->channel >= 0) Mix_Volume(s->channel, (vol * 128) / 127);
}

static inline void AIL_set_sample_pan(HSAMPLE h, int pan) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (pan < 0) pan = 0; if (pan > 127) pan = 127;
    s->pan = pan;
    if (s->channel >= 0) {
        int left  = (127 - pan) * 255 / 127;
        int right = pan * 255 / 127;
        Mix_SetPanning(s->channel, (Uint8)left, (Uint8)right);
    }
}

// Initialize a sample to default state
static inline void AIL_init_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    s->channel    = -1;
    s->status     = SMP_DONE;
    s->volume     = 127;
    s->pan        = 64;
    s->loop_count = 0;
    s->EOS        = NULL;
    // Reset chunk pointer left intact; caller sets via AIL_set_sample_file
}

// Stop playback immediately
static inline void AIL_stop_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (s->channel >= 0) {
        Mix_HaltChannel(s->channel);
        s->channel = -1;
    }
    s->status = SMP_STOPPED;
}

// Resume playback if paused/stopped
static inline void AIL_resume_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (s->channel >= 0) {
        Mix_Resume(s->channel);
        s->status = SMP_PLAYING;
    }
}

// Set playback rate (pitch) in Hz
static inline void AIL_set_sample_playback_rate(HSAMPLE h, long rate) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    s->playback_rate = rate;
    // SDL_mixer does not support per-channel resampling directly.
    // Options:
    //  - Pre-resample assets offline to desired rate
    //  - Use SDL_AudioDevice + custom resampler (libsamplerate, speexdsp)
    // For now, we store the requested rate for fidelity tracking.
}


static inline int AIL_start_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s || !s->chunk) return 0;
    int loops = s->loop_count;
    int ch = Mix_PlayChannel(-1, s->chunk, loops);
    if (ch < 0) return 0;
    
     // RERUN volume and pan might have been set on channel -1, so redo just in case...
    Mix_Volume(ch, s->volume);
    int pan = s->pan;
    if (ch>= 0) {
        int left  = (127 - pan) * 255 / 127;
        int right = pan * 255 / 127;
        Mix_SetPanning(ch, (Uint8)left, (Uint8)right);
    }

    s->channel = ch;
    s->status = SMP_PLAYING;
    return 1;
}

static inline void AIL_end_sample(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return;
    if (s->channel >= 0) Mix_HaltChannel(s->channel);
    s->status = SMP_STOPPED;
}

static inline int AIL_sample_status(HSAMPLE h) {
    SAMPLE* s = (SAMPLE*)h;
    if (!s) return SMP_FREE;
    if (s->channel >= 0 && Mix_Playing(s->channel)) return SMP_PLAYING;
    return SMP_DONE;
}

// ----- Sequence (music) API -----
static inline HSEQUENCE AIL_allocate_sequence_handle(HMDIDRIVER d) {
    (void)d;
    SEQUENCE* m = (SEQUENCE*)SDL_calloc(1, sizeof(SEQUENCE));
    if (!m) return NULL;
    m->status = SEQ_FREE;
    m->volume = 127;
    return (HSEQUENCE)m;
}

static inline void AIL_release_sequence_handle(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return;
    if (m->music) Mix_FreeMusic(m->music);
    SDL_free(m);
}

static inline int AIL_set_sequence_file(HSEQUENCE h, const char* path) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return 0;
    if (m->music) Mix_FreeMusic(m->music);
    m->music = Mix_LoadMUS(path);
    return m->music != NULL;
}

static inline int AIL_start_sequence(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m || !m->music) return 0;

    // MSS semantics: start playback once, looping handled elsewhere
    if (Mix_PlayMusic(m->music, 1) == 0) {  // play once
        m->status = SEQ_PLAYING;
        return 1;
    }
    return 0;
}


static inline void AIL_stop_sequence(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return;
    Mix_HaltMusic();
    m->status = SEQ_STOPPED;
}

static inline int AIL_sequence_status(HSEQUENCE h) {
    SEQUENCE* m = (SEQUENCE*)h;
    if (!m) return SEQ_FREE;
    if (Mix_PlayingMusic()) return SEQ_PLAYING;
    return SEQ_DONE;
}

inline int AIL_midiOutOpen(HMDIDRIVER* mdi,
                           void* midiOutDev,
                           unsigned int uDriverID)
{
    (void)midiOutDev; // unused in SDL2

    HMDIDRIVER d = (HMDIDRIVER)SDL_calloc(1, sizeof(*d));
    if (!d) return -1;

    d->opened = 1;
    d->driver_id = (int)uDriverID;

    if (mdi) *mdi = d;

    return 0; // success
}


typedef struct _DLS_SYNTH {
    int sample_rate;
    int bits;
    int channels;
    int opened;
} DLS_SYNTH;

inline void* AIL_DLS_open(void* dls,
                          HDIGDRIVER dig,
                          const char* DLS_dll_name,
                          unsigned int flags,
                          int sample_value,
                          int bits,
                          int channels)
{
    (void)dls;
    (void)dig;
    (void)DLS_dll_name;
    (void)flags;

    DLS_SYNTH* synth = (DLS_SYNTH*)SDL_calloc(1, sizeof(DLS_SYNTH));
    if (!synth) return NULL;

    synth->sample_rate = sample_value ? sample_value : 44100;
    synth->bits = bits;
    synth->channels = channels;
    synth->opened = 1;

    // SDL2_mixer already initialized via Mix_OpenAudio
    // If you want to enforce sample rate/format, you could call Mix_OpenAudio here.

    return synth;
}

typedef struct _WAVE_SYNTH {
    HDIGDRIVER dig;
    HMDIDRIVER mdi;
    void*      config;
    int        flags;
    int        opened;
} WAVE_SYNTH;

inline void* AIL_create_wave_synthesizer(HDIGDRIVER dig,
                                         HMDIDRIVER mdi,
                                         void* tmpptr,
                                         int flags)
{
    WAVE_SYNTH* synth = (WAVE_SYNTH*)SDL_calloc(1, sizeof(WAVE_SYNTH));
    if (!synth) return NULL;

    synth->dig    = dig;
    synth->mdi    = mdi;
    synth->config = tmpptr;
    synth->flags  = flags;
    synth->opened = 1;

    // SDL2_mixer already initialized via Mix_OpenAudio.
    // Actual MIDI playback will be handled by Mix_LoadMUS / Mix_PlayMusic.

    return synth;
}


// ------------------------------------------------------------------
// Digital driver
// ------------------------------------------------------------------
inline HDIGDRIVER AIL_open_digital_driver(int rate, int bits, int channels) { return nullptr; }
inline void AIL_close_digital_driver(HDIGDRIVER driver) {}

// ------------------------------
// Helpers: flag decoding
// ------------------------------
static inline int sample_channels_from_flags(long flags) {
    if (flags & (DIG_F_STEREO_8 | DIG_F_STEREO_16)) return 2;
    if (flags & (DIG_F_MONO_8   | DIG_F_MONO_16))   return 1;
    return 1;
}

static inline SDL_AudioFormat sample_format_from_flags(long flags) {
    if (flags & (DIG_F_MONO_8 | DIG_F_STEREO_8))     return AUDIO_U8;
    if (flags & (DIG_F_MONO_16 | DIG_F_STEREO_16))   return AUDIO_S16SYS;
    return AUDIO_S16SYS;
}

inline void AIL_set_sample_type(HSAMPLE sample, SLong format, SLong flags) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s) return;

    s->format = format;
    s->flags  = flags;

    if (s->orig_rate == 0) s->orig_rate = 44100;
    if (s->channels   == 0) s->channels = sample_channels_from_flags(flags);
    if (s->orig_format == 0) s->orig_format = sample_format_from_flags(flags);
}



// Convert 8-bit unsigned mono to 16-bit signed mono
static uint8_t* u8_to_s16le(const uint8_t* b, int n, int* out_bytes) {
    int samples = n;
    int outn = samples * 2;
    uint8_t* out = (uint8_t*)malloc(outn);
    if (!out) { *out_bytes = 0; return NULL; }
    for (int i = 0; i < samples; i++) {
        int16_t s = ((int)b[i] - 128) << 8; // scale to full 16-bit
        out[2*i] = (uint8_t)(s & 0xFF);
        out[2*i+1] = (uint8_t)((s >> 8) & 0xFF);
    }
    *out_bytes = outn;
    return out;
}

// Inside your AIL_set_sample_address implementation:
static inline void AIL_set_sample_address(HSAMPLE sample, const void* start, long len) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s) return;

    s->address = start;
    s->length  = len;

    if (s->chunk) { Mix_FreeChunk(s->chunk); s->chunk = NULL; }

    if (!start || len <= 0) return;

    const uint8_t* buf = (const uint8_t*)start;

    // --- WAV path ---
    if (len >= 12 && memcmp(buf, "RIFF", 4) == 0 && memcmp(buf + 8, "WAVE", 4) == 0) {
        int rate = 0, chans = 1;
        SDL_AudioFormat fmt = AUDIO_S16SYS;
        parse_wav_fmt(buf, (int)len, &rate, &chans, &fmt);

        SDL_RWops* rw = SDL_RWFromConstMem(start, len);
        if (rw) {
            Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1);
            if (chunk) {
                // Convert once into device format (e.g. 44100 Hz, AUDIO_S16SYS)
                SDL_AudioCVT cvt;
                if (SDL_BuildAudioCVT(&cvt, fmt, chans, rate,
                                      AUDIO_S16SYS, chans, 44100) >= 0)
                {
                    Uint8* newbuf = (Uint8*)SDL_malloc(chunk->alen * cvt.len_mult);
                    memcpy(newbuf, chunk->abuf, chunk->alen);
                    cvt.buf = newbuf;
                    cvt.len = chunk->alen;
                    if (SDL_ConvertAudio(&cvt) == 0) {
                        Mix_FreeChunk(chunk);
                        s->chunk = Mix_QuickLoad_RAW(newbuf, cvt.len_cvt);
                        s->orig_rate   = 44100;
                        s->channels    = chans;
                        s->orig_format = AUDIO_S16SYS;
                        s->status      = SMP_DONE;
                    } else {
                        SDL_free(newbuf);
                        Mix_FreeChunk(chunk);
                    }
                } else {
                    // No conversion needed
                    s->chunk = chunk;
                    s->orig_rate   = rate;
                    s->channels    = chans;
                    s->orig_format = fmt;
                    s->status      = SMP_DONE;
                }
            }
        }
        return;
    }

    // --- Radio messages path
    if (s->format == DIG_F_MONO_8) {
        int out_len = 0;

        uint8_t* pcm = u8_to_s16le(buf, len, &out_len);
        
        if (pcm && out_len > 0) {
            // Build converter from 22050 Hz mono → 44100 Hz mono
            SDL_AudioCVT cvt;
            if (SDL_BuildAudioCVT(&cvt,
                                AUDIO_U8, 1, 22050,       // source: unsigned 8‑bit mono at 22050 Hz
                                AUDIO_S16SYS, 1, 44100) >= 0) // target: signed 16‑bit mono at 44100 Hz
            {
                cvt.len = len; // length in bytes of the original U8 buffer
                cvt.buf = (Uint8*)SDL_malloc(len * cvt.len_mult);
                memcpy(cvt.buf, buf, len);

                if (SDL_ConvertAudio(&cvt) == 0) {
                    s->chunk = Mix_QuickLoad_RAW(cvt.buf, cvt.len_cvt);
                    s->orig_rate   = 44100;
                    s->channels    = 1;
                    s->orig_format = AUDIO_S16SYS;
                    s->status      = SMP_DONE;
                } else {
                    SDL_free(cvt.buf);
                }
            }
        }
        return;
    }
}

// ------------------------------------------------------------------
// Stream handling
// ------------------------------------------------------------------
inline HSTREAM AIL_open_stream(HDIGDRIVER driver, const char* filename, int flags) { return nullptr; }
inline void AIL_close_stream(HSTREAM stream) {}
inline void AIL_start_stream(HSTREAM stream) {}
inline void AIL_pause_stream(HSTREAM stream, int pause) {}
inline int AIL_stream_volume(HSTREAM stream) { return 0; }
inline void AIL_set_stream_volume(HSTREAM stream, int volume) {}

// ------------------------------------------------------------------
// 3D sample handling
// ------------------------------------------------------------------
inline void AIL_start_3D_sample(H3DSAMPLE sample) {}
inline void AIL_stop_3D_sample(H3DSAMPLE sample) {}
inline void AIL_resume_3D_sample(H3DSAMPLE sample) {}
inline void AIL_end_3D_sample(H3DSAMPLE sample) {}
inline int AIL_3D_sample_volume(H3DSAMPLE sample) { return 0; }
inline void AIL_set_3D_sample_volume(H3DSAMPLE sample, float volume) {}
inline int AIL_3D_sample_playback_rate(H3DSAMPLE sample) { return 0; }
inline void AIL_set_3D_sample_playback_rate(H3DSAMPLE sample, int rate) {}

// ------------------------------------------------------------------
// Preferences / misc
// ------------------------------------------------------------------
inline int AIL_set_preference(unsigned int number, int value) { return 0; }

inline HDLSFILEID AIL_DLS_load_file(const char* filename) { return nullptr; }
inline void AIL_DLS_unload_file(HDLSFILEID file) {}
inline HWAVESYNTH AIL_open_wave_synthesizer() { return nullptr; }
inline void AIL_close_wave_synthesizer(HWAVESYNTH synth) {}
inline HDLSDEVICE AIL_DLS_open_device(HWAVESYNTH synth) { return nullptr; }
inline void AIL_DLS_close_device(HDLSDEVICE device) {}

// Return the current volume of a sequence.
// In the stub, we just return 0 (silence).
typedef struct _SEQUENCE    * HSEQUENCE;       // Handle to sequence
inline int AIL_sequence_volume(HSEQUENCE sequence) { return 0; }

// In Miles, this returns the current branch index of an XMIDI sequence.
// The second parameter is the branch code (signed long).
inline int AIL_branch_index(HSEQUENCE sequence, long newcode) {
    (void)sequence; // suppress unused warnings
    (void)newcode;
    return 0;
}

// In Miles, this sets the playback volume of a sequence (0–127).
inline void AIL_set_sequence_volume(HSEQUENCE seq, int chan, int vol) {
    (void)seq;
    (void)chan;
    (void)vol;
    // no-op
}

#define WAVE_FORMAT_PCM         1

// In Miles: opens a digital audio driver with given format.
typedef struct _DIG_DRIVER {
    int sample_rate;
    int channels;
    int format;
    int opened;
    int master_volume; // 0–127
} DIG_DRIVER;


static inline void AIL_set_digital_master_volume(HDIGDRIVER dig, int volume) {
    if (!dig) return;

    DIG_DRIVER* d = (DIG_DRIVER*)dig;

    // Clamp to 0–127
    if (volume < 0) volume = 0;
    if (volume > 127) volume = 127;

    d->master_volume = volume;

    // Map Miles scale (0–127) to SDL scale (0–128)
    int sdlVol = (volume * 128) / 127;

    // Apply to all channels and music
    Mix_Volume(-1, sdlVol);       // all sample channels
    Mix_VolumeMusic(sdlVol);      // music volume
}

inline int AIL_waveOutOpen(HDIGDRIVER* driver,
                           void* waveout,   // unused in SDL2
                           int id,          // unused in SDL2
                           void* format)    // pointer to WAVEFORMAT/PCMWAVEFORMAT
{
    (void)waveout;
    (void)id;

    DIG_DRIVER* d = (DIG_DRIVER*)SDL_calloc(1, sizeof(DIG_DRIVER));
    if (!d) return -1;

    // Default values
    int freq = 44100;
    int channels = 2;
    int fmt = AUDIO_S16SYS;

    if (format) {
        PCMWAVEFORMAT* wf = (PCMWAVEFORMAT*)format;
        freq = wf->wf.nSamplesPerSec;
        channels = wf->wf.nChannels;
        fmt = (wf->wBitsPerSample == 16) ? AUDIO_S16SYS : AUDIO_U8;
    }

    if (Mix_OpenAudio(freq, fmt, channels, 2048) != 0) {
        SDL_free(d);
        return -1;
    }

    d->sample_rate = freq;
    d->channels = channels;
    d->format = fmt;
    d->opened = 1;

    if (driver) *driver = d;
    return 0; // success
}

// In Miles: closes the audio driver and releases resources.
inline void AIL_waveOutClose(HDIGDRIVER driver) {
    if (!driver) return;

    DIG_DRIVER* d = (DIG_DRIVER*)driver;

    if (d->opened) {
        Mix_CloseAudio();   // shut down SDL_mixer audio device
        d->opened = 0;
    }

    SDL_free(d); // free the driver struct
}


// In Miles: returns a string describing the last error.
inline const char* AIL_last_error() {
    return "No error";
}

static long detect_midi_length(const void* address) {
    const unsigned char* buf = (const unsigned char*)address;
    if (!buf) return 0;

    // XMIDI: "FORM" <size:4> <type:4> ...
    if (memcmp(buf, "FORM", 4) == 0) {
        uint32_t size = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
        return (long)size + 8;
    }
    
    // Standard MIDI: "MThd" <size:4> ...
    if (memcmp(buf, "MThd", 4) == 0) {
        long pos = 0;
        uint32_t len = (buf[pos+4] << 24) | (buf[pos+5] << 16) | (buf[pos+6] << 8) | buf[pos+7];
        int ntrks = (buf[pos+10] << 8) | buf[pos+11];
        pos += 8 + len;
        
        for (int i = 0; i < ntrks; i++) {
            if (memcmp(buf + pos, "MTrk", 4) != 0) break;
            uint32_t tlen = (buf[pos+4] << 24) | (buf[pos+5] << 16) | (buf[pos+6] << 8) | buf[pos+7];
            pos += 8 + tlen;
        }
        return pos;
    }
    return 0;
}

inline int AIL_init_sequence(HSEQUENCE seq, void* data, int track) {
    if (!seq || !data) return -1;

    // Free any existing music
    if (seq->music) {
        Mix_FreeMusic(seq->music);
        seq->music = NULL;
    }

    // Wrap the MIDI/XMIDI data in an SDL_RWops
    // NOTE: Miles often passed -1 for length, but here we assume caller knows size.
    // We use detect_midi_length to find the size.
    long length = detect_midi_length(data);
    if (length <= 0) return -1;

    SDL_RWops* rw = SDL_RWFromConstMem(data, (int)length);
    if (!rw) return -1;

    // Load the MIDI/XMIDI sequence
    seq->music = Mix_LoadMUS_RW(rw, 1); // auto-free RWops
    if (!seq->music) return -1;

    seq->track = track;
    seq->initialized = 1;

    return 0; // success
}


// Real Miles: stops playback of a sequence immediately.
// Stub: do nothing.
inline void AIL_end_sequence(HSEQUENCE seq) {
    (void)seq;
    // no-op
}


// Real Miles: registers a callback for XMIDI trigger events.
typedef void (*AILTRIGGERCB)(HSEQUENCE seq, SLong channel, SLong value);
inline void AIL_register_trigger_callback(HSEQUENCE seq, AILTRIGGERCB callback) {
    (void)seq;
    (void)callback;
}

inline void AIL_resume_sequence(HSEQUENCE seq) {
    (void)seq;
}

// Real Miles: locks and unlock internal data structures (thread safety).
inline void AIL_lock() {
    // no-op
}
inline void AIL_unlock() {
    // no-op
}

// ------------------------------------------------------------------
// File type detection
// ------------------------------------------------------------------
// Real Miles: inspects file contents and returns one of the above constants.
// Parameters:
//   filename = path to file
// Stub: always return XMIDI.
inline long AIL_file_type(void* headptr, long size) {
    (void)headptr;
    (void)size;
    return AILFILETYPE_XMIDI;
}

// ------------------------------------------------------------------
// Current sample position (in samples/bytes depending on API usage)
// ------------------------------------------------------------------
inline SLong AIL_sample_position(HSAMPLE sample) {
    (void)sample;
    return 0;
}

inline void AIL_set_sample_loop_count(HSAMPLE sample, SLong count) {
    SAMPLE* s = (SAMPLE*)sample;
    if (!s) return;

    int loops = 0;

    if (count <= 0) {
        loops = -1; // infinite loop
    } else {
        loops = count - 1; // Miles: N = loop N-1 times
    }

    s->loop_count = loops;
}


// ------------------------------------------------------------------
// Destroy wave synthesizer
// ------------------------------------------------------------------
inline void AIL_destroy_wave_synthesizer(HWAVESYNTH wavesynth) {
    (void)wavesynth;
    // no-op
}

// ------------------------------------------------------------------
// Close DLS wave synthesizer
// ------------------------------------------------------------------
inline void AIL_DLS_close(HDLSDEVICE dlswavesynth, SLong state) {
    (void)dlswavesynth;
    (void)state;
    // no-op
}

// ------------------------------------------------------------------
// Release timer handle
// ------------------------------------------------------------------
inline void AIL_release_timer_handle(SLong timer) {
    (void)timer;
    // no-op
}


inline void AIL_midiOutClose(HMIDI midi) {
    (void)midi;
    // no-op
}

static inline void AIL_set_XMIDI_master_volume(HMDIDRIVER mdi, int volume) {
    if (!mdi) return;

    // Clamp to 0–127
    if (volume < 0) volume = 0;
    if (volume > 127) volume = 127;

    mdi->master_volume = volume;

    // Map Miles scale (0–127) to SDL scale (0–128)
    int sdlVol = (volume * 128) / 127;

    // Apply to music (MIDI playback)
    Mix_VolumeMusic(sdlVol);
}

static inline SLong AIL_extract_DLS(void* ptr,
                                    SLong size,
                                    void** xmiPtr,
                                    SLong* xmiSize,
                                    void** dlsPtr,
                                    SLong* dlsSize)
{
    if (!ptr || size <= 0) return -1;

    // For now, assume the block is just XMIDI data
    if (xmiPtr) *xmiPtr = ptr;
    if (xmiSize) *xmiSize = size;

    // No DLS support in SDL2 shim
    if (dlsPtr) *dlsPtr = NULL;
    if (dlsSize) *dlsSize = 0;

    return 0; // success
}


static inline HDLSFILEID AIL_DLS_load_memory(void* wavesynth,
                                             void* dlsPtr,
                                             unsigned int flags)
{
    (void)wavesynth;
    (void)dlsPtr;
    (void)flags;

    if (!dlsPtr) return NULL;

    // Allocate a dummy handle to represent the loaded DLS bank
    void* fakeHandle = SDL_calloc(1, 1); // just a unique pointer
    return (HDLSFILEID)fakeHandle;
}

static inline SLong AIL_find_DLS(void* ptr,
                                 SLong size,
                                 void** xmiPtr,
                                 SLong* xmiSize,
                                 void** dlsPtr,
                                 SLong* dlsSize)
{
    if (!ptr || size <= 0) return -1;

    // For now, assume the block is just XMIDI data
    if (xmiPtr) *xmiPtr = ptr;
    if (xmiSize) *xmiSize = size;

    // No DLS support in SDL2 shim
    if (dlsPtr) *dlsPtr = NULL;
    if (dlsSize) *dlsSize = 0;

    return 0; // success
}


static inline void AIL_register_event_callback(HMDIDRIVER mdi,
                                               MIDIEventCallback cb)
{
    if (!mdi) return;
    mdi->callback = cb;
}

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
#endif // MSSW_COMPAT_H
