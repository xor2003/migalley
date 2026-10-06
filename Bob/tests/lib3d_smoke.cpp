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
#include "lib3d.h"

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

    ULong refs = lib->Release();
    printf("PASS lib3d init smoke (release refs=%lu)\n", (unsigned long)refs);
    fflush(stdout);
    _exit(0);   // skip static dtors — unrelated game-side stubs
}
