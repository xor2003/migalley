#include "WIN32_COMPAT.H"

#include "dinput_stub.h"

#include <deque>

// Single-definition GUIDs (declared extern in the header so gc-sections
// cannot discard a TU-local copy that game code still references).
const GUID GUID_XAxis = { 0xA36D02E0, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_YAxis = { 0xA36D02E1, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_ZAxis = { 0xA36D02E2, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_RxAxis = { 0xA36D02E3, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_RyAxis = { 0xA36D02E4, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_RzAxis = { 0xA36D02E5, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Slider = { 0xA36D02E6, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_POV = { 0xA36D02F2, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Button = { 0xA36D02F0, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Key = { 0x55728220, 0xD33C, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID IID_IDirectInputDeviceA = {
    0x5944e680, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};
const GUID IID_IDirectInputDevice2A = {
    0x5944e682, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};
const GUID IID_IDirectInputDevice2 = {
    0x5944e682, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};
const GUID IID_IDirectInputA = { 0x89521360, 0xaa8a, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_SysMouse = { 0x6f1d2b60, 0xd5a0, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_SysKeyboard = { 0x6f1d2b61, 0xd5a0, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_ConstantForce = {
    0x13541c2b, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_RampForce = {
    0x13541c2c, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Square = {
    0x13541c2d, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Sine = {
    0x13541c2e, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Triangle = {
    0x13541c2f, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_SawtoothUp = {
    0x13541c30, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_SawtoothDown = {
    0x13541c31, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Damper = {
    0x13541c33, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Inertia = {
    0x13541c34, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Friction = {
    0x13541c35, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_CustomForce = {
    0x13541c36, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
const GUID GUID_Spring = {
    0x13541C20, 0x8E33, 0x11D0,
    {0x9A, 0x7F, 0x00, 0xA0, 0xC9, 0x0C, 0xA9, 0x26}
};

// ---------------------------------------------------------------------------
// Buffered keyboard input (system keyboard, c_dfDIKeyboard data format).
//
// SDL delivers real KEYDOWN/KEYUP events; an SDL event watch records them as
// DIDEVICEOBJECTDATA entries — dwOfs = set-1 make code, dwData bit 7 = down —
// so taps shorter than one frame are never lost the way a keyboard-state
// diff would lose them.  When the game arms a notification handle via
// IDirectInputDevice2::SetEventNotification we SetEvent it whenever new data
// lands, which is what wakes CMIGApp::Run() -> Inst3d::OnKeyInput().
// ---------------------------------------------------------------------------

static pthread_mutex_t              g_kbMutex = PTHREAD_MUTEX_INITIALIZER;
static std::deque<DIDEVICEOBJECTDATA> g_kbQueue;
static HANDLE                       g_kbNotify = NULL;
static DWORD                        g_kbSeq = 0;
static bool                         g_kbWatch = false;
static bool                         g_kbDown[256] = {};

static int SDLCALL RowanDIKeyWatchFn(void* /*userdata*/, SDL_Event* e)
{
    static const bool dbg = getenv("ROWAN_DEBUG_INPUT") != nullptr;
    if (e->type == SDL_WINDOWEVENT &&
        e->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        // DInput unacquire semantics: every held key must report a release,
        // otherwise the game's shift-column state latches until re-pressed.
        pthread_mutex_lock(&g_kbMutex);
        for (int k = 0; k < 256; ++k)
            if (g_kbDown[k]) {
                g_kbDown[k] = false;
                if (g_kbQueue.size() < 512) {
                    DIDEVICEOBJECTDATA d = {};
                    d.dwOfs       = (DWORD)k;
                    d.dwData      = 0;
                    d.dwTimeStamp = e->window.timestamp;
                    d.dwSequence  = g_kbSeq++;
                    g_kbQueue.push_back(d);
                }
            }
        HANDLE h = g_kbNotify;
        pthread_mutex_unlock(&g_kbMutex);
        if (dbg)
            fprintf(stderr, "[key] focus lost: released held keys\n");
        if (h)
            SetEvent(h);
        return 1;
    }
    if (e->type != SDL_KEYDOWN && e->type != SDL_KEYUP)
        return 1;
    int dik = RowanSdlToDik(e->key.keysym.scancode);
    if (dbg)
        fprintf(stderr, "[key] sdl sc=%d dik=%02x %s\n",
                e->key.keysym.scancode, dik,
                e->type == SDL_KEYDOWN ? "down" : "up");
    if (!dik)
        return 1;
    pthread_mutex_lock(&g_kbMutex);
    g_kbDown[dik & 0xFF] = (e->type == SDL_KEYDOWN);
    if (g_kbQueue.size() < 512) {   // bound; drop oldest data beyond it
        DIDEVICEOBJECTDATA d = {};
        d.dwOfs       = (DWORD)(dik & 0xFF);
        d.dwData      = (e->type == SDL_KEYDOWN) ? 0x80 : 0;
        d.dwTimeStamp = e->key.timestamp;
        d.dwSequence  = g_kbSeq++;
        g_kbQueue.push_back(d);
    }
    HANDLE h = g_kbNotify;
    pthread_mutex_unlock(&g_kbMutex);
    if (h) {
        if (dbg)
            fprintf(stderr, "[key] setevent h=%p\n", h);
        SetEvent(h);
    }
    return 1;
}

void RowanDIKeyWatchEnsure()
{
    if (!g_kbWatch) {
        SDL_AddEventWatch(RowanDIKeyWatchFn, nullptr);
        g_kbWatch = true;
    }
}

void RowanDISetKeyNotify(HANDLE hEvent)
{
    pthread_mutex_lock(&g_kbMutex);
    g_kbNotify = hEvent;
    if (hEvent) {
        g_kbQueue.clear();  // drop keys buffered while no consumer was armed
        memset(g_kbDown, 0, sizeof g_kbDown);
    }
    pthread_mutex_unlock(&g_kbMutex);
    if (hEvent)
        RowanDIKeyWatchEnsure();
}

DWORD RowanDIKeyDrain(DIDEVICEOBJECTDATA* out, DWORD want, bool peek)
{
    pthread_mutex_lock(&g_kbMutex);
    DWORD n = want;
    if (n > g_kbQueue.size())
        n = (DWORD)g_kbQueue.size();
    for (DWORD i = 0; i < n; ++i)
        out[i] = g_kbQueue[i];
    if (!peek)
        for (DWORD i = 0; i < n; ++i)
            g_kbQueue.pop_front();
    pthread_mutex_unlock(&g_kbMutex);
    return n;
}

static const unsigned long SDL_JOY_GUID_MAGIC = 0x53444C4A; // 'SDLJ'

static bool IsJoystickGUID(const GUID& guid) {
    return guid.Data2 == (unsigned short)(SDL_JOY_GUID_MAGIC >> 16) &&
           guid.Data3 == (unsigned short)(SDL_JOY_GUID_MAGIC & 0xFFFF);
}

HRESULT IDirectInputA::QueryInterface(const IID& iid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (memcmp(&iid, &IID_IDirectInputA, sizeof(IID)) == 0) {
        *ppv = this;
        AddRef();
        return DI_OK;
    }
    return E_NOINTERFACE;
}

ULONG IDirectInputA::AddRef() {
    return ++refCount;
}

ULONG IDirectInputA::Release() {
    ULONG r = --refCount;
    if (r == 0) delete this;
    return r;
}

HRESULT IDirectInputA::CreateDevice(const GUID& rguid,
                    LPDIRECTINPUTDEVICEA* lplpDevice,
                    LPUNKNOWN pUnkOuter)
{
    if (!lplpDevice) return DIERR_GENERIC;

    // Create the device object
    // Note: We ignore pUnkOuter for aggregation in this stub
    auto* dev = new IDirectInputDevice2A();
    *lplpDevice = dev;

    // Check for System Mouse
    if (memcmp(&rguid, &GUID_SysMouse, sizeof(GUID)) == 0) {
        dev->isMouse = true;
        return DI_OK;
    }
    // Check for System Keyboard
    else if (memcmp(&rguid, &GUID_SysKeyboard, sizeof(GUID)) == 0) {
        dev->isKeyboard = true;
        return DI_OK;
    }
    // Check for our SDL Joystick GUID pattern
    else if (IsJoystickGUID(rguid)) {
        dev->deviceIndex = (int)rguid.Data1 - 3; // EnumDevices encodes index+3
        dev->sdlJoy = SDL_JoystickOpen(dev->deviceIndex);
        if (!dev->sdlJoy) {
            delete dev;
            *lplpDevice = nullptr;
            return DIERR_NOTFOUND;
        }
        return DI_OK;
    }
    // Fallback: If the game passes IID instead of GUID (buggy game code?), handle gracefully
    else if (memcmp(&rguid, &IID_IDirectInputDeviceA, sizeof(GUID)) == 0 ||
             memcmp(&rguid, &IID_IDirectInputDevice2A, sizeof(GUID)) == 0) {
        // Just return a dummy device, likely keyboard or mouse logic required by game
        return DI_OK;
    }
    
    delete dev;
    *lplpDevice = nullptr;
    return DIERR_NOTINITIALIZED;
}

HRESULT IDirectInputA::EnumDevices(DWORD dwDevType,
                    LPDIENUMDEVICESCALLBACK lpCallback,
                    LPVOID pvRef,
                    DWORD dwFlags)
{
    // same SDL-backed enumeration as before
    if (!lpCallback) return DIERR_GENERIC;

    // DirectInput: dwDevType==0 enumerates every attached device (BoB's
    // SController::BuildEnumerationTables relies on this to find the mouse
    // and all joysticks in one pass).
    const bool all = (dwDevType == 0);

    if (all || (dwDevType & DIDEVTYPE_MOUSE)) {
        DIDEVICEINSTANCE inst{};
        inst.dwSize = sizeof(inst);
        inst.guidInstance = GUID_SysMouse;
        inst.guidProduct = GUID_SysMouse;
        inst.dwDevType = DIDEVTYPE_MOUSE;
        snprintf(inst.tszInstanceName, MAX_PATH, "SDL Mouse");
        snprintf(inst.tszProductName, MAX_PATH, "System Mouse");
        lpCallback(&inst, pvRef);
    }

    if (all || (dwDevType & DIDEVTYPE_KEYBOARD)) {
        DIDEVICEINSTANCE inst{};
        inst.dwSize = sizeof(inst);
        inst.guidInstance = GUID_SysKeyboard;
        inst.guidProduct = GUID_SysKeyboard;
        inst.dwDevType = DIDEVTYPE_KEYBOARD;
        snprintf(inst.tszInstanceName, MAX_PATH, "SDL Keyboard");
        snprintf(inst.tszProductName, MAX_PATH, "System Keyboard");
        lpCallback(&inst, pvRef);
    }

    if (all || (dwDevType & DIDEVTYPE_JOYSTICK)) {
        int numJoy = SDL_NumJoysticks();
        for (int i = 0; i < numJoy; ++i) {
            DIDEVICEINSTANCE inst{};
            inst.dwSize = sizeof(inst);
            // Embed index+3 in GUID Data1 — games treat Data1==0 as a
            // device-list terminator and 1/2 as joystick/mouse aliases.
            inst.guidInstance.Data1 = (unsigned long)(i + 3);
            inst.guidInstance.Data2 = (unsigned short)(SDL_JOY_GUID_MAGIC >> 16);
            inst.guidInstance.Data3 = (unsigned short)(SDL_JOY_GUID_MAGIC & 0xFFFF);

            inst.dwDevType = DIDEVTYPE_JOYSTICK;
            snprintf(inst.tszInstanceName, MAX_PATH, "SDL Joystick %d", i);
            snprintf(inst.tszProductName, MAX_PATH, "%s", SDL_JoystickNameForIndex(i));
            lpCallback(&inst, pvRef);
        }
    }
    return DI_OK;
}

HRESULT IDirectInputA::GetDeviceStatus(const GUID& rguid)
{
    if (memcmp(&rguid, &GUID_SysMouse, sizeof(GUID)) == 0 ||
        memcmp(&rguid, &GUID_SysKeyboard, sizeof(GUID)) == 0) {
        // System mouse/keyboard are always present
        return DI_OK;
    }
    if (IsJoystickGUID(rguid))
    {
        int idx = (int)rguid.Data1 - 3;
        if (idx >= 0 && idx < SDL_NumJoysticks()) {
            SDL_Joystick* joy = SDL_JoystickOpen(idx);
            if (joy) {
                bool attached = SDL_JoystickGetAttached(joy);
                SDL_JoystickClose(joy);
                if (attached) return DI_OK;
            }
        }
        return DI_NOTATTACHED;
    }
    // Unknown GUID → pretend not attached
    return DI_NOTATTACHED;
}

HRESULT IDirectInputA::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    return DI_OK;
}

HRESULT DirectInputCreate(HINSTANCE hinst,
                                 uint32_t dwVersion,
                                 LPDIRECTINPUT* ppDI,
                                 LPUNKNOWN punkOuter) {
    if (ppDI) {
        *ppDI = new IDirectInputA();
    }
    return DI_OK;
}
