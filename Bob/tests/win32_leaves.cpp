// win32_leaves.cpp — leaf Win32 impls for bob_lib3d_smoke.  The real
// frontend gets these (with SDL backends) from RowanMFC/MFC_stub.cpp;
// pulling the whole lib in would drag the mig game object graph the smoke
// doesn't exercise.  These return success-shaped answers so lib3d's real
// code paths keep running — the emulation under test sits below them.

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

BOOL GetWindowRect(HWND, LPRECT r)
{
    if (r) { r->left = r->top = 0; r->right = 640; r->bottom = 480; }
    return TRUE;
}

BOOL EnumDisplaySettings(const char*, DWORD, DEVMODE*)
{
    return FALSE;   // end of enumeration — init path only needs "no more"
}

LONG ChangeDisplaySettings(DEVMODE*, DWORD)
{
    return DISP_CHANGE_SUCCESSFUL;
}

int StretchDIBits(HDC, int, int, int, int, int, int, int, int,
                  const VOID*, const BITMAPINFO*, UINT, DWORD)
{
    return 0;
}
