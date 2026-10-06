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

namespace {

SDL_Texture* g_ddTex = nullptr;
int g_ddTexW = 0, g_ddTexH = 0, g_ddTexBpp = 0;

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

void PresentPrimary(IDirectDrawSurface7* primary)
{
    if (!primary || !primary->desc.lpSurface) return;
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

    if (!g_ddTex || g_ddTexW != w || g_ddTexH != h || g_ddTexBpp != (int)bpp) {
        if (g_ddTex) SDL_DestroyTexture(g_ddTex);
        g_ddTex = SDL_CreateTexture(be->renderer, fmt,
                                    SDL_TEXTUREACCESS_STREAMING, w, h);
        g_ddTexW = w; g_ddTexH = h; g_ddTexBpp = (int)bpp;
    }
    if (!g_ddTex) return;

    if (SDL_UpdateTexture(g_ddTex, nullptr, primary->desc.lpSurface,
                          (int)primary->desc.lPitch) != 0)
        return;
    SDL_RenderCopy(be->renderer, g_ddTex, nullptr, nullptr);
    SDL_RenderPresent(be->renderer);
}

} // namespace

// Called once from bob_main.cpp; the hook is a plain function pointer so the
// install is just an assignment.
void BoBInstallDDPresent()
{
    RowanDDPresentHook = &PresentPrimary;
}
