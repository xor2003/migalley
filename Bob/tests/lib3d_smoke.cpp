// lib3d_smoke.cpp — T120b smoke test for BoB's native CLib3D on the rowan
// DX7 emulation.  Drives the same call sequence stub3d.cpp's MainInit
// does: Lib3DCreate(IID_ILib3D) → Initialise → GetDrivers → GetModes →
// GetDefaultDriverAndMode → SetDriverAndMode → Release.  Exits 0 on
// success, prints which stage failed otherwise.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
// Test TU: reach Lib3D's private pDD*/pD3DDEV7 to drive the compat device
// directly (T123d raster check). Test-only access — no production use.
#define private public
#define protected public
#include "lib3d.h"
#undef private
#undef protected

int main()
{
    ULong* sqrtTable = (ULong*)calloc(0x10000, sizeof(ULong));
    HWND hwnd = (HWND)0;

    CLib3D* lib = Lib3DCreate(IID_ILib3D);
    if (!lib) { printf("FAIL Lib3DCreate\n"); return 1; }

    HRESULT hr = lib->Initialise(hwnd, sqrtTable);
    if (hr != S_OK) { printf("FAIL Initialise hr=0x%lx\n", (unsigned long)hr); return 1; }

    UIDrivers drivers;
    memset(&drivers, 0, sizeof(drivers));
    drivers.size = sizeof(drivers);
    hr = lib->GetDrivers(drivers);
    if (hr != S_OK || drivers.numDrivers == 0) {
        printf("FAIL GetDrivers hr=0x%lx num=%lu\n",
               (unsigned long)hr, (unsigned long)drivers.numDrivers);
        return 1;
    }
    printf("drivers: %lu (%s)\n", (unsigned long)drivers.numDrivers,
           drivers.driver[0].name);

    UIModes modes;
    memset(&modes, 0, sizeof(modes));
    modes.size = sizeof(modes);
    hr = lib->GetModes(drivers.driver[0].hDriver, modes);
    if (hr != S_OK || modes.numModes == 0) {
        printf("FAIL GetModes hr=0x%lx num=%lu\n",
               (unsigned long)hr, (unsigned long)modes.numModes);
        return 1;
    }
    printf("modes: %lu first=%lux%lux%lu\n", (unsigned long)modes.numModes,
           (unsigned long)modes.mode[0].width,
           (unsigned long)modes.mode[0].height,
           (unsigned long)modes.mode[0].bpp);

    HDRIVER hDriver; HMODE hMode;
    hr = lib->GetDefaultDriverAndMode(hDriver, hMode);
    // hDriver==0 is legitimate: handles are list indices, driver 0 is valid.
    if (hr != S_OK || hMode == 0) {
        printf("FAIL GetDefaultDriverAndMode hr=0x%lx drv=%lu mode=%lu\n",
               (unsigned long)hr, (unsigned long)hDriver, (unsigned long)hMode);
        return 1;
    }

    hr = lib->SetDriverAndMode(hDriver, hMode, hwnd);
    if (hr != S_OK) {
        printf("FAIL SetDriverAndMode hr=0x%lx\n", (unsigned long)hr);
        return 1;
    }

    // T123d — rasterizer smoke: draw one red screen-space triangle through
    // the D3D device, flip, and verify pixels landed on the primary.
    // Drive the device directly so the check covers the compat path
    // (vertex buffer → decode → EmitPrim → surface → Flip) end to end.
    Lib3D* L = static_cast<Lib3D*>(lib);
    if (!L->pD3DDEV7 || !L->pDDSB7 || !L->pDDSP7) {
        printf("FAIL surfaces/dev pD3DDEV7=%p back=%p prim=%p\n",
               (void*)L->pD3DDEV7, (void*)L->pDDSB7, (void*)L->pDDSP7);
        return 1;
    }

    struct TV { float x,y,z,rhw; DWORD col,spec; float u,v; };
    D3DVERTEXBUFFERDESC vbd;
    memset(&vbd,0,sizeof(vbd));
    vbd.dwSize = sizeof(vbd);
    vbd.dwCaps = D3DVBCAPS_WRITEONLY;
    vbd.dwFVF  = D3DFVF_TLVERTEX;
    vbd.dwNumVertices = 3;
    LPDIRECT3DVERTEXBUFFER7 vb = NULL;
    LPDIRECT3D7 d3d = NULL;
    if (L->pD3DDEV7->QueryInterface(IID_IDirect3D7,(LPVOID*)&d3d)!=D3D_OK || !d3d) {
        printf("FAIL QI IDirect3D7\n"); return 1;
    }
    if (d3d->CreateVertexBuffer(&vbd,&vb,0)!=D3D_OK || !vb) {
        printf("FAIL CreateVertexBuffer\n"); return 1;
    }
    TV* verts=NULL;
    if (vb->Lock(DDLOCK_WAIT,(LPVOID*)&verts,NULL)!=DD_OK || !verts) {
        printf("FAIL vb Lock\n"); return 1;
    }
    const DWORD red = 0xFFFF0000;
    float vw = (float)L->pDDSB7->desc.dwWidth, vh = (float)L->pDDSB7->desc.dwHeight;
    verts[0]=(TV){vw*0.25f, vh*0.75f, 0.0f, 1.0f, red, 0, 0.f, 0.f};
    verts[1]=(TV){vw*0.50f, vh*0.25f, 0.0f, 1.0f, red, 0, 0.f, 0.f};
    verts[2]=(TV){vw*0.75f, vh*0.75f, 0.0f, 1.0f, red, 0, 0.f, 0.f};
    vb->Unlock();

    L->pD3DDEV7->SetRenderTarget(L->pDDSB7,0);
    L->pD3DDEV7->BeginScene();
    L->pD3DDEV7->Clear(0,NULL,D3DCLEAR_TARGET,0xFF202020,0,0);
    hr = L->pD3DDEV7->DrawPrimitiveVB(D3DPT_TRIANGLELIST, vb, 0, 3, 0);
    L->pD3DDEV7->EndScene();
    if (hr != D3D_OK) { printf("FAIL DrawPrimitiveVB hr=0x%lx\n",(unsigned long)hr); return 1; }
    L->pDDSP7->Flip(NULL, DDFLIP_WAIT);

    // sample the primary across the triangle's bounding box — any pixel
    // with a red channel >> the 0x20 clear color proves rasterisation.
    int hit = 0;
    const BYTE* fb = (const BYTE*)L->pDDSP7->desc.lpSurface;
    const DWORD pitch = (DWORD)L->pDDSP7->desc.lPitch;
    for (int y=(int)(vh*0.3f); y<(int)(vh*0.7f); y++)
        for (int x=(int)(vw*0.28f); x<(int)(vw*0.72f); x++) {
            const BYTE* px = fb + y*pitch + x*2;
            unsigned v = px[0] | (px[1]<<8);      // RGB565
            if (((v>>11)&0x1f) > 8) { hit++; break; }
        }
    vb->Release(); d3d->Release();
    if (!hit) { printf("FAIL raster: no red pixel on primary\n"); return 1; }

    ULong refs = lib->Release();
    printf("PASS lib3d init+triangle smoke (release refs=%lu)\n", (unsigned long)refs);
    fflush(stdout);
    _exit(0);   // skip static dtors — unrelated game-side stubs
}
