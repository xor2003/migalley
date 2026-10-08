// bob_dinput_focus.cpp — regression test for the DirectInput shim's
// focus-loss behaviour in Hardware/dinput_stub.cpp.
//
// On SDL_WINDOWEVENT_FOCUS_LOST the watch must enqueue a release for every
// key still marked held — mirroring real DInput's unacquire semantics.
// Without it, a keydown whose keyup is lost to another window leaves the
// BoB keymap's shift column latched: every later key resolves a shifted
// action until the modifier is tapped again ("keyboard sometimes dead").
//
// The implementation TU is included directly so the test can drive the
// file-static watch handler and inspect the buffered-key queue.

#include "WIN32_COMPAT.H"
#include "dinput_stub.h"

#include "../tests/unit/harness.h"
extern int g_checks;
extern int g_failures;

#include "dinput_stub.cpp"

void test_dinput_focus_release()
{
    // Clean slate: the queue and held-key map are TU-local.
    g_kbQueue.clear();
    memset(g_kbDown, 0, sizeof g_kbDown);
    g_kbNotify = NULL;
    g_kbSeq = 0;

    // Physical keydown -> one queued event + held flag.
    SDL_Event kd;
    memset(&kd, 0, sizeof kd);
    kd.type = SDL_KEYDOWN;
    kd.key.keysym.scancode = SDL_SCANCODE_LSHIFT;
    RowanDIKeyWatchFn(nullptr, &kd);

    CHECK_EQ((long)g_kbQueue.size(), 1);
    if (g_kbQueue.empty()) return;
    DWORD dik = g_kbQueue.front().dwOfs;
    CHECK(dik != 0);
    CHECK(g_kbDown[dik & 0xFF]);

    // Focus loss while the key is still held -> synthesized release.
    SDL_Event fe;
    memset(&fe, 0, sizeof fe);
    fe.type = SDL_WINDOWEVENT;
    fe.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    RowanDIKeyWatchFn(nullptr, &fe);

    CHECK_EQ((long)g_kbQueue.size(), 2);
    const DIDEVICEOBJECTDATA& rel = g_kbQueue.back();
    CHECK_EQ((long)rel.dwOfs, (long)dik);
    CHECK_EQ((long)(rel.dwData & 0x80), 0);   // bit7 clear = release
    CHECK(!g_kbDown[dik & 0xFF]);

    // Keys not held produce no phantom releases on the next focus loss.
    RowanDIKeyWatchFn(nullptr, &fe);
    CHECK_EQ((long)g_kbQueue.size(), 2);
}

// The header's GUIDs were TU-local `static const` — --gc-sections could
// discard the copy a given TU referenced while another kept its own, so
// guidType comparisons compared distinct objects.  They are now single
// extern definitions in dinput_stub.cpp; pin the DX7 values and
// distinctness so a mis-merge of the definition set can't silently alias
// every device object class again.
void test_dinput_guid_identity()
{
    CHECK_EQ((long)GUID_XAxis.Data1,   (long)0xA36D02E0);
    CHECK_EQ((long)GUID_YAxis.Data1,   (long)0xA36D02E1);
    CHECK_EQ((long)GUID_ZAxis.Data1,   (long)0xA36D02E2);
    CHECK_EQ((long)GUID_RxAxis.Data1,  (long)0xA36D02E3);
    CHECK_EQ((long)GUID_RyAxis.Data1,  (long)0xA36D02E4);
    CHECK_EQ((long)GUID_RzAxis.Data1,  (long)0xA36D02E5);
    CHECK_EQ((long)GUID_Slider.Data1,  (long)0xA36D02E6);
    CHECK_EQ((long)GUID_POV.Data1,     (long)0xA36D02F2);
    CHECK_EQ((long)GUID_Button.Data1,  (long)0xA36D02F0);
    CHECK_EQ((long)GUID_Key.Data1,     (long)0x55728220);
    CHECK_EQ((long)GUID_SysKeyboard.Data1, (long)0x6F1D2B61);
    CHECK_EQ((long)GUID_SysMouse.Data1,    (long)0x6F1D2B60);

    // Device-class GUIDs must not alias each other — the game switches on
    // guidType during object enumeration.
    CHECK(memcmp(&GUID_XAxis, &GUID_YAxis, sizeof(GUID)) != 0);
    CHECK(memcmp(&GUID_Button, &GUID_POV, sizeof(GUID)) != 0);
    CHECK(memcmp(&GUID_Key, &GUID_Button, sizeof(GUID)) != 0);
    CHECK(memcmp(&GUID_SysKeyboard, &GUID_SysMouse, sizeof(GUID)) != 0);
}
