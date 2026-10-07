// dd_present.cpp — DirectDraw -> SDL present bridge (T123, bob exe only).
//
// The BoB build's compat/ddraw.h fires RowanDDPresentHook after Flip and
// after blits that target a primary/front-buffer surface.  This TU installs
// the hook: it uploads the primary surface's pixels to a streaming texture
// on the top-level SDL window and presents.  BoB's native 3D path is
// fullscreen-exclusive, so presenting to the root backend is correct — the
// 2D dialog path keeps its own compositing in MFC_stub.

#include "MFC_stub.h"   // g_hwndRegistry / WindowBackend / backend_from_hwnd
#include <ddraw.h>      // compat/ddraw.h via bob_overlay -> IDirectDrawSurface7
#include <mutex>
#include <cstring>

namespace {

// The 3D worker must not call SDL video functions: SDL window ops are
// main-thread-only and concurrent Xlib use of the same Display corrupts
// X11's malloc'd buffers (that was the "corrupted double-linked list" /
// deadlock we kept hitting).  The worker queues the latest primary surface
// here and PumpSDL presents it on the main thread — latest frame wins.
IDirectDrawSurface7* g_pendingPrimary = nullptr;
std::mutex           g_pendingMutex;

} // namespace

// Read by MFC_stub's dialog repaint path: while presents are recent the
// game owns the window (fullscreen-exclusive semantics) and MFC paints
// must not clear/present over the 3D frame.
volatile unsigned long RowanDDLastPresentMs = 0;

namespace {

WindowBackend* TopLevelBackend()
{
    // The DirectDraw primary covers the whole SDL window (exclusive
    // fullscreen semantics), so present through the root backend.
    WindowBackend* fallback = nullptr;
    for (auto& kv : g_hwndRegistry) {
        WindowBackend& be = kv.second;
        if (be.window && be.renderer && !be.isChild) return &be;
        if (be.window && be.renderer && !fallback) fallback = &be;
    }
    return fallback;
}

// Present via the window surface (software blit), not the backend renderer:
// MFC's SDL_RENDERER_ACCELERATED GL context is bound to the main thread, so
// SDL_UpdateTexture from the 3D worker thread fails with "window has not
// been made current".  GetWindowSurface/BlitScaled/UpdateWindowSurface are
// pure software paths safe to call from the worker.
void PresentPrimary(IDirectDrawSurface7* primary)
{
    if (!primary || !primary->desc.lpSurface) return;
    RowanDDLastPresentMs = SDL_GetTicks();
    WindowBackend* be = TopLevelBackend();
    if (!be) return;

    const int w = (int)primary->desc.dwWidth;
    const int h = (int)primary->desc.dwHeight;
    const DWORD bpp = primary->desc.ddpfPixelFormat.dwRGBBitCount;
    if (!w || !h) return;
    Uint32 fmt;
    if (bpp == 32)      fmt = SDL_PIXELFORMAT_ARGB8888;
    else if (bpp == 16) fmt = SDL_PIXELFORMAT_RGB565;
    else                return;   // palettized primaries unsupported

    // Wrap the DD frame and scale it onto the window surface.  Window
    // surfaces are shared with MFC's paint path, which is suppressed
    // while DD presents are active.
    SDL_Surface* ws = SDL_GetWindowSurface(be->window);
    if (!ws) return;
    SDL_Surface* src = SDL_CreateRGBSurfaceWithFormatFrom(
        primary->desc.lpSurface, w, h, (int)bpp,
        (int)primary->desc.lPitch, fmt);
    if (!src) return;
    SDL_BlitScaled(src, nullptr, ws, nullptr);
    SDL_FreeSurface(src);
    SDL_UpdateWindowSurface(be->window);
}

} // namespace

// 3D-worker entry point (the RowanDDPresentHook target): refcount the frame,
// replace any still-queued frame (latest wins), and nudge the main thread.
void QueuePresent(IDirectDrawSurface7* primary)
{
    if (!primary || !primary->desc.lpSurface) return;
    RowanDDLastPresentMs = SDL_GetTicks();
    primary->AddRef();
    IDirectDrawSurface7* old;
    {
        std::lock_guard<std::mutex> lk(g_pendingMutex);
        old = g_pendingPrimary;
        g_pendingPrimary = primary;
    }
    if (old) old->Release();
    SDL_Event e;
    std::memset(&e, 0, sizeof(e));
    e.type = SDL_USEREVENT;
    e.user.code = 0x44445052;   // 'DDPR'
    SDL_PushEvent(&e);
}

// PumpSDL calls this (main thread) when the queued 'DDPR' event arrives.
extern "C" void RowanDDPumpPresent()
{
    IDirectDrawSurface7* s;
    {
        std::lock_guard<std::mutex> lk(g_pendingMutex);
        s = g_pendingPrimary;
        g_pendingPrimary = nullptr;
    }
    if (!s) return;
    PresentPrimary(s);
    s->Release();
}

// Called once from bob_main.cpp; the hook is a plain function pointer so the
// install is just an assignment.
void BoBInstallDDPresent()
{
    RowanDDPresentHook = &QueuePresent;
}
