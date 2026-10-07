#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <regex>
#include <unistd.h>
#include <string>
#include <sstream>
#include <map>

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "dinput_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "RBUTTON.H"
#include "RSTATIC.H"
#include "RCOMBO.H"
#include "RLISTBOX.H"
#include "REDIT.H"
#include "REDTBT.H"
#include "RTABS.H"
#include "RRADIO.H"
#include "RSPINBUT.H"
#include "RJOYCFG.H"
#include "RESOURCE.H"
#include "MIGVIEW.H"
#include "FILEMAN.H" //RERUN for File_Man/RowanFileNumSafe

#include <SDL.h>
#include "SDL_syswm.h"
#include <SDL_ttf.h>
#include <nlohmann/json.hpp>

float g_dluX = 1.5; // 1.5 to 2 is typical for 8pt font
float g_dluY = 1.5;

// Forward declaration for composition logic
static void PaintChildrenRecursive(HWND parentHwnd, SDL_Renderer* renderer, int parentAbsX, int parentAbsY, int depth);

// WINSMACK.CPP: arms the skip flag while a smack video is playing.
// Weak default — mig's WINSMACK.CPP strong-def overrides at link; consumers
// with no smack player (bob) get the no-op.
extern void InterruptSmack();
__attribute__((weak)) void InterruptSmack() {}

// XX_ScreenFlip_Vulkan.cpp (mig): set when the shared SDL/X11 window is
// torn down so the Vulkan blit path stops presenting. Weak default —
// consumers without the Vulkan renderer (bob) still need the symbol.
__attribute__((weak)) volatile bool g_migVulkanWindowGone = false;

// Every OS window is created SDL_WINDOW_VULKAN-flagged so mig's
// SDL_Vulkan_* calls work — but that flag hard-fails on video drivers
// with no Vulkan (dummy/Xvfb-without-llvmpipe, and BoB which never
// renders Vulkan). Try flagged, retry plain once, then remember.
static int g_sdlVulkanCapable = -1;  // -1 unknown, 0 no, 1 yes
static SDL_Window* CreateOSWindow(const char* title, int x, int y,
                                  int w, int h, Uint32 flags)
{
    SDL_Window* win = nullptr;
    if (g_sdlVulkanCapable != 0) {
        win = SDL_CreateWindow(title, x, y, w, h, flags | SDL_WINDOW_VULKAN);
        if (win) {
            g_sdlVulkanCapable = 1;
            return win;
        }
        g_sdlVulkanCapable = 0;
        SDL_ClearError();
    }
    return SDL_CreateWindow(title, x, y, w, h, flags);
}

#ifndef GetRValue
#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

// --- Base Class Message Map Implementations ---

const AFX_MSGMAP* PASCAL CCmdTarget::_GetBaseMessageMap() { return nullptr; }
const AFX_MSGMAP_ENTRY* CCmdTarget::_GetMessageEntries()
{
    static const AFX_MSGMAP_ENTRY entries[] = { {0,0,0,0,0} };
    return entries;
}
const AFX_MSGMAP CCmdTarget::messageMap = { &CCmdTarget::_GetBaseMessageMap, CCmdTarget::_GetMessageEntries() };
const AFX_MSGMAP* CCmdTarget::GetMessageMap() const
{
	return &CCmdTarget::messageMap;
}
BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
    ON_WM_PAINT()
END_MESSAGE_MAP()
BEGIN_MESSAGE_MAP(CFrameWnd, CWnd)
    ON_WM_SIZE()
END_MESSAGE_MAP()
BEGIN_MESSAGE_MAP(CWinApp, CCmdTarget)
END_MESSAGE_MAP()
BEGIN_MESSAGE_MAP(CDocument, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CStatic, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CButton, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CListBox, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CDialog, CWnd)
    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CComboBox, CWnd)
END_MESSAGE_MAP()

static CWnd* g_pFocusWnd = nullptr;

CWnd wndTop;
CWnd wndBottom;
CWnd wndTopMost;
CWnd wndNoTopMost;

using json = nlohmann::json;

std::unordered_map<std::string, int> g_resIDMap;
std::map<int, std::string> g_resStringsMap;

struct ButtonMetadata {
    int icon_enum = 0;
    std::string icon_name;
    int length = 0; // RERUN: Store length from DLGINIT
};

std::map<int, std::map<int, ButtonMetadata>> g_buttonMetadataMap;

IMPLEMENT_DYNCREATE(CCmdTarget, CObject)
IMPLEMENT_DYNCREATE(CWnd, CCmdTarget)
//IMPLEMENT_DYNAMIC(CView, CWnd)
IMPLEMENT_DYNCREATE(COleDocument, CDocument)
IMPLEMENT_DYNCREATE(CDocument, CCmdTarget)
IMPLEMENT_DYNAMIC(CListBox, CWnd)
IMPLEMENT_DYNAMIC(CFont, CGdiObject)
IMPLEMENT_DYNAMIC(CGdiObject, CObject)
IMPLEMENT_DYNAMIC(CFrameWnd, CWnd)
IMPLEMENT_DYNAMIC(CDialog, CWnd)
IMPLEMENT_DYNAMIC(DlgItem, CWnd)

bool g_shouldQuit = false;

// RERUN: SDL destroys the game window during process teardown while
// queued X requests (e.g. XInput select) are still in flight; the async
// BadWindow/BadDrawable they produce would fire the default handler's
// exit(1) inside __run_exit_handlers, making static destructors race
// live worker threads. Swallow just those two unconditionally — they
// are teardown noise on this single-window app — real X errors still
// defer to the default handler (and flag teardown-unsafe first).
// (Xlib.h comes in transitively here; XX_ScreenFlip_Vulkan.cpp avoids it
// because of Bool/Status clashes, but this TU already has it.)
static XErrorHandler g_prevXErrorHandler = nullptr;

// Set when an X error is about to reach _XDefaultError -> exit(1):
// exit handlers then run destructors while driver threads are parked
// on the dead X connection, so graceful Vulkan teardown deadlocks.
// Vulkan_Shutdown checks this and skips.
volatile bool g_migFatalXError = false;

static int MigXErrorHandler(Display* display, XErrorEvent* ev)
{
    // BadWindow/BadDrawable on our window is never actionable — if the X
    // window is gone the app is finished anyway. Swallowing regardless of
    // quit state keeps an async error racing the close (dispatched inside
    // an SDL XSync before the quit path arms) from firing _XDefaultError
    // -> exit(1) mid-teardown. Other codes defer to the default.
    if (ev->error_code == BadWindow || ev->error_code == BadDrawable) {
        fprintf(stderr, "[MFC_stub] X error %u (req %u.%u res 0x%lx) ignored\n",
                ev->error_code, ev->request_code, ev->minor_code, ev->resourceid);
        return 0;
    }
    g_migFatalXError = true;
    if (g_prevXErrorHandler)
        return g_prevXErrorHandler(display, ev);
    return 0;
}

// SDL may have swapped the X error handler since init — reinstall ours
// at every point where quitting begins so teardown-only BadWindow /
// BadDrawable can't fire the default handler's exit(1) inside
// __run_exit_handlers.
static void MigArmQuitTeardown()
{
    g_shouldQuit = true;
    g_prevXErrorHandler = XSetErrorHandler(MigXErrorHandler);
}

// Helper to find a system font
static std::string GetSystemFontPath(const char* faceName = nullptr)
{
    std::string target = "DejaVuSans.ttf";
    if (faceName) {
        std::string face = faceName;
        if (face == "MS Sans Serif" || face == "Helv") target = "DejaVuSans.ttf";
        else if (face == "Arial") target = "Arial.TTF";
        else if (face == "Arial Italic") target = "Ariali.TTF";
        else if (face == "Times New Roman") target = "Times.TTF";
        else if (face == "Courier New") target = "cour.ttf";
        else if (face == "Intel") target = "Intel.ttf";
    }

    const char* fonts[] = {
        "/usr/share/fonts/TTF/cour.ttf",
        "/usr/share/fonts/TTF/Arial.TTF",
        "/usr/share/fonts/TTF/Ariali.TTF",
        "/usr/share/fonts/TTF/Times.TTF",
        "/usr/share/fonts/Intel/Intel.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/IosevkaNerdFont-Regular.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "./INTEL.TTF",
        "./Intel.ttf"
    };
    
    // First try to find the specific target
    for (const char* f : fonts) {
        if (strstr(f, target.c_str())) {
             FILE* fp = fopen(f, "rb");
             if (fp) {
                 fclose(fp);
                 std::cout << "[MFC_stub] Found system font (first try): " << f << std::endl;
                 return f;
             }
        }
    }

    for (const char* f : fonts) {
        FILE* fp = fopen(f, "rb");
        if (fp) {
            fclose(fp);
            std::cout << "[MFC_stub] Found system font: " << f << std::endl;
            return f;
        }
    }
    std::cout << "[MFC_stub] GetSystemFontPath: Failed to find any usable font file." << std::endl;
    return "";
}

CFont::CFont() {}
CFont::~CFont()
{
    if (m_hObject) {
        TTF_CloseFont((TTF_Font*)m_hObject);
        m_hObject = nullptr;
    }
}

BOOL CFont::CreateFontIndirect(const LOGFONT* lpLogFont)
{
    if (!lpLogFont) return FALSE;
    m_lf = *lpLogFont;

    if (m_hObject) {
        TTF_CloseFont((TTF_Font*)m_hObject);
        m_hObject = nullptr;
    }

    std::string path = GetSystemFontPath(m_lf.lfFaceName);
    if (path.empty()) {
        std::cout << "[MFC_stub] CFont::CreateFontIndirect: No system font found!" << std::endl;
        return FALSE;
    }

    int size = abs(m_lf.lfHeight);
    if (size == 0) size = 24;

    if (!TTF_WasInit()) {
        std::cout << "[MFC_stub] Warning: TTF_Init() has not been called yet!" << std::endl;
    }

    //std::cout << "[MFC_stub] CFont::CreateFontIndirect: Loading " << path << " size=" << size << " (requested height=" << m_lf.lfHeight << ")" << std::endl;
    m_hObject = TTF_OpenFont(path.c_str(), size);
    if (!m_hObject) {
        std::cout << "[MFC_stub] CFont::CreateFontIndirect: FAILED to load font '" << path << "' size=" << size << ": " << SDL_GetError() << std::endl;
        // Try fallback size
        if (size != 24) {
             std::cout << "[MFC_stub] Retrying with safe size 24..." << std::endl;
             m_hObject = TTF_OpenFont(path.c_str(), 24);
             if (m_hObject) {
                 std::cout << "[MFC_stub] Success with safe size." << std::endl;
             } else {
                 std::cout << "[MFC_stub] Failed fallback size 24: " << SDL_GetError() << std::endl;
             }
        }
    } else {
        //std::cout << "[MFC_stub] CFont::CreateFontIndirect: SUCCESS loading font." << std::endl;
    }
    return (m_hObject != nullptr);
}

BOOL CFont::CreateFont(int nHeight, int nWidth, int nEscapement,
                int nOrientation, int nWeight, BYTE bItalic,
                BYTE bUnderline, BYTE cStrikeOut, BYTE nCharSet,
                BYTE nOutPrecision, BYTE nClipPrecision,
                BYTE nQuality, BYTE nPitchAndFamily,
                const char* lpszFacename)
{
    LOGFONT lf{};
    lf.lfHeight = nHeight;
    lf.lfWidth = nWidth;
    lf.lfEscapement = nEscapement;
    lf.lfOrientation = nOrientation;
    lf.lfWeight = nWeight;
    lf.lfItalic = bItalic;
    lf.lfUnderline = bUnderline;
    lf.lfStrikeOut = cStrikeOut;
    lf.lfCharSet = nCharSet;
    lf.lfOutPrecision = nOutPrecision;
    lf.lfClipPrecision = nClipPrecision;
    lf.lfQuality = nQuality;
    lf.lfPitchAndFamily = nPitchAndFamily;
    if (lpszFacename) {
        strncpy(lf.lfFaceName, lpszFacename, LF_FACESIZE - 1);
    }
    return CreateFontIndirect(&lf);
}

CDC::CDC(CWnd* wnd) {
    // Walk up parent chain to find a window with a backend (renderer)
    CWnd* pTarget = wnd;
    while (pTarget && !pTarget->GetSafeHwnd()) {
        pTarget = pTarget->GetParent();
    }

    if (pTarget && pTarget->GetSafeHwnd()) {
        m_backend = &g_hwndRegistry[pTarget->GetSafeHwnd()];
        m_renderer = m_backend->renderer;
        m_hDC = (HDC)this; // Allow this CDC to be passed as an HDC
    }
}

CObject* CDC::SelectStockObject(int object)
{
    static CPen blackPen(PS_SOLID, 1, RGB(0,0,0));
    static CPen whitePen(PS_SOLID, 1, RGB(255,255,255));
    static CPen nullPen(PS_NULL, 0, 0);
    static CFont dummyFont;
    
    static CBrush blackBrush(GetStockObject(BLACK_BRUSH));
    static CBrush whiteBrush(GetStockObject(WHITE_BRUSH));
    static CBrush ltGrayBrush(GetStockObject(LTGRAY_BRUSH));
    static CBrush grayBrush(GetStockObject(GRAY_BRUSH));
    static CBrush dkGrayBrush(GetStockObject(DKGRAY_BRUSH));
    static CBrush nullBrush(GetStockObject(NULL_BRUSH));

    if (dummyFont.m_lf.lfHeight == 0) {
        // Initialize default font properties so GetLogFont returns sensible values
        dummyFont.CreateFont(12, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "System");
    }

    switch (object) {
    case BLACK_PEN:
        m_pCurrentPen = &blackPen;
        return m_pCurrentPen;
    case WHITE_PEN:
        m_pCurrentPen = &whitePen;
        return m_pCurrentPen;
    case NULL_PEN:
        m_pCurrentPen = &nullPen;
        return m_pCurrentPen;

    case BLACK_BRUSH:
        m_pCurrentBrush = &blackBrush;
        return m_pCurrentBrush;
    case WHITE_BRUSH:
        m_pCurrentBrush = &whiteBrush;
        return m_pCurrentBrush;
    case LTGRAY_BRUSH:
        m_pCurrentBrush = &ltGrayBrush;
        return m_pCurrentBrush;
    case GRAY_BRUSH:
        m_pCurrentBrush = &grayBrush;
        return m_pCurrentBrush;
    case DKGRAY_BRUSH:
        m_pCurrentBrush = &dkGrayBrush;
        return m_pCurrentBrush;
    case NULL_BRUSH:
        m_pCurrentBrush = &nullBrush;
        return m_pCurrentBrush;

    case SYSTEM_FONT:
    case ANSI_FIXED_FONT:
    case ANSI_VAR_FONT:
        return &dummyFont;

    default:
        return nullptr;
    }
}

std::unordered_map<HWND, WindowBackend> g_hwndRegistry;

// Set by the DirectDraw present bridge (Bob/port/dd_present.cpp, bob exe
// only) each time a primary-surface frame is uploaded.  While presents are
// recent the game owns the window (DX7 fullscreen-exclusive semantics), so
// the MFC dialog repaint must not clear/present over the 3D frame.
extern "C" volatile unsigned long RowanDDLastPresentMs __attribute__((weak));
extern "C" void RowanDDPumpPresent() __attribute__((weak));
static bool DDPrimaryActive()
{
    return &RowanDDLastPresentMs && RowanDDLastPresentMs &&
           (SDL_GetTicks() - RowanDDLastPresentMs) < 250;
}

HWND allocate_hwnd() { 
    return new HWND__(); 
}

WindowBackend* backend_from_hwnd(HWND h)
{
    auto it = g_hwndRegistry.find(h);
    return (it == g_hwndRegistry.end()) ? nullptr : &it->second;
}

CWnd* AfxGetWndFromSDLWindow(SDL_Window* sdlWin)
{
    if (!sdlWin) return nullptr;
    for (auto& pair : g_hwndRegistry)
    {
        if (pair.second.window == sdlWin && !pair.second.isChild)
            return pair.second.owner;
    }
    for (auto& pair : g_hwndRegistry)
    {
        if (pair.second.window == sdlWin)
            return pair.second.owner;
    }
    return nullptr;
}

// --- Message Queue & Pump Implementation ---

static std::deque<MSG> g_msgQueue;
static CWnd* g_pCaptureWnd = nullptr;

std::deque<SdlKeyEvent> g_sdl_key_events;
std::mutex g_sdl_key_events_mutex;

struct TimerInfo {
    UINT_PTR id;
    UINT interval;
    Uint32 nextTime;
    HWND hwnd;
    void* callback;
};
static std::vector<TimerInfo> g_timers;

void PumpTimers()
{
    Uint32 now = SDL_GetTicks();
    for (auto& t : g_timers) {
        if ((int32_t)(now - t.nextTime) >= 0) {
            // Avoid flooding the queue with timer messages for the same timer
            bool pending = false;
            for (const auto& m : g_msgQueue) {
                if (m.message == WM_TIMER && m.hwnd == t.hwnd && m.wParam == t.id) {
                    pending = true;
                    break;
                }
            }
            
            if (!pending) {
                MSG msg = {};
                msg.hwnd = t.hwnd;
                msg.message = WM_TIMER;
                msg.wParam = t.id;
                msg.lParam = (LPARAM)t.callback;
                msg.time = now;
                g_msgQueue.push_back(msg);
            }
            t.nextTime = now + std::max(t.interval, (UINT)10);
        }
    }
}

// Helper to find the deepest child window at (x, y) relative to parent
HWND FindChildWindow(HWND parent, int& x, int& y)
{
    WindowBackend* be = backend_from_hwnd(parent);
    if (!be) return parent;
    // std::cout << "[HitTest] Checking parent HWND=" << parent << " at (" << x << "," << y << ")" << std::endl;

    // Check children in reverse order (assuming last created is on top)
    static const bool dbgHit = getenv("ROWAN_DEBUG_INPUT") != nullptr;
    for (auto it = be->children.rbegin(); it != be->children.rend(); ++it) {
        HWND child = *it;
        WindowBackend* childBe = backend_from_hwnd(child);
        if (dbgHit)
            fprintf(stderr, "[HIT] parent=%p child=%p be=%p vis=%d owner=%p id=%d type=%s\n",
                    parent, child, childBe, childBe ? childBe->visible : 0,
                    (childBe && childBe->owner) ? (void*)childBe->owner : nullptr,
                    (childBe && childBe->owner) ? childBe->owner->m_nID : -1,
                    (childBe && childBe->owner) ? typeid(*childBe->owner).name() : "-");
        if (childBe && childBe->visible && childBe->owner) {
            // RERUN: Standard Win32 behavior: Static controls are transparent to mouse 
            // hits unless they have the SS_NOTIFY (0x0100) style.
            if (childBe->owner->IsKindOf(RUNTIME_CLASS(CStatic))) {
                if (!(childBe->style & SS_NOTIFY)) {
                    continue; 
                }
            }

            CRect& r = childBe->owner->m_rect;
            
            // RERUN: Special handling for CRRadio to allow "click-through"
            if (childBe->owner->IsKindOf(RUNTIME_CLASS(CRRadio))) {
                CRRadio* pRadio = (CRRadio*)childBe->owner;
                int h = 24; // approx item height
                int cols = pRadio->GetCols();
                // If columns wasn't set, it might be horizontal (see OnPaint fix)
                if (cols == 1 && r.Width() > r.Height() * 2) cols = 3; 
                int rows = (10 + cols - 1) / cols; 
                if ((y - r.top) > (rows * h)) {
                    continue; // Whitespace hit, try window behind
                }
            }

            if (dbgHit)
                fprintf(stderr, "      test child=%p id=%d type=%s rect=(%ld,%ld)-(%ld,%ld) pt=(%d,%d) %s\n",
                        child, childBe->owner->m_nID, typeid(*childBe->owner).name(),
                        (long)r.left, (long)r.top, (long)r.right, (long)r.bottom,
                        x, y,
                        (x >= r.left && x < r.right && y >= r.top && y < r.bottom) ? "HIT" : "miss");
            if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) {
                x -= r.left;
                y -= r.top;
                return FindChildWindow(child, x, y);
            }
        }
    }
    return parent;
}

static const std::unordered_map<SDL_Scancode, int> scancode_to_dik_map = {
    {SDL_SCANCODE_A, DIK_A}, {SDL_SCANCODE_B, DIK_B}, {SDL_SCANCODE_C, DIK_C}, {SDL_SCANCODE_D, DIK_D},
    {SDL_SCANCODE_E, DIK_E}, {SDL_SCANCODE_F, DIK_F}, {SDL_SCANCODE_G, DIK_G}, {SDL_SCANCODE_H, DIK_H},
    {SDL_SCANCODE_I, DIK_I}, {SDL_SCANCODE_J, DIK_J}, {SDL_SCANCODE_K, DIK_K}, {SDL_SCANCODE_L, DIK_L},
    {SDL_SCANCODE_M, DIK_M}, {SDL_SCANCODE_N, DIK_N}, {SDL_SCANCODE_O, DIK_O}, {SDL_SCANCODE_P, DIK_P},
    {SDL_SCANCODE_Q, DIK_Q}, {SDL_SCANCODE_R, DIK_R}, {SDL_SCANCODE_S, DIK_S}, {SDL_SCANCODE_T, DIK_T},
    {SDL_SCANCODE_U, DIK_U}, {SDL_SCANCODE_V, DIK_V}, {SDL_SCANCODE_W, DIK_W}, {SDL_SCANCODE_X, DIK_X},
    {SDL_SCANCODE_Y, DIK_Y}, {SDL_SCANCODE_Z, DIK_Z},
    {SDL_SCANCODE_1, DIK_1}, {SDL_SCANCODE_2, DIK_2}, {SDL_SCANCODE_3, DIK_3}, {SDL_SCANCODE_4, DIK_4},
    {SDL_SCANCODE_5, DIK_5}, {SDL_SCANCODE_6, DIK_6}, {SDL_SCANCODE_7, DIK_7}, {SDL_SCANCODE_8, DIK_8},
    {SDL_SCANCODE_9, DIK_9}, {SDL_SCANCODE_0, DIK_0},
    {SDL_SCANCODE_RETURN, DIK_RETURN},
    {SDL_SCANCODE_ESCAPE, DIK_ESCAPE},
    {SDL_SCANCODE_BACKSPACE, DIK_BACK},
    {SDL_SCANCODE_TAB, DIK_TAB},
    {SDL_SCANCODE_SPACE, DIK_SPACE},
    {SDL_SCANCODE_MINUS, DIK_MINUS},
    {SDL_SCANCODE_EQUALS, DIK_EQUALS},
    {SDL_SCANCODE_LEFTBRACKET, DIK_LBRACKET},
    {SDL_SCANCODE_RIGHTBRACKET, DIK_RBRACKET},
    {SDL_SCANCODE_BACKSLASH, DIK_BACKSLASH},
    {SDL_SCANCODE_SEMICOLON, DIK_SEMICOLON},
    {SDL_SCANCODE_APOSTROPHE, DIK_APOSTROPHE},
    {SDL_SCANCODE_GRAVE, DIK_GRAVE},
    {SDL_SCANCODE_COMMA, DIK_COMMA},
    {SDL_SCANCODE_PERIOD, DIK_PERIOD},
    {SDL_SCANCODE_SLASH, DIK_SLASH},
    {SDL_SCANCODE_CAPSLOCK, DIK_CAPITAL},
    {SDL_SCANCODE_F1, DIK_F1}, {SDL_SCANCODE_F2, DIK_F2}, {SDL_SCANCODE_F3, DIK_F3},
    {SDL_SCANCODE_F4, DIK_F4}, {SDL_SCANCODE_F5, DIK_F5}, {SDL_SCANCODE_F6, DIK_F6},
    {SDL_SCANCODE_F7, DIK_F7}, {SDL_SCANCODE_F8, DIK_F8}, {SDL_SCANCODE_F9, DIK_F9},
    {SDL_SCANCODE_F10, DIK_F10}, {SDL_SCANCODE_F11, DIK_F11}, {SDL_SCANCODE_F12, DIK_F12},
    {SDL_SCANCODE_PRINTSCREEN, DIK_SYSRQ},
    {SDL_SCANCODE_SCROLLLOCK, DIK_SCROLL},
    {SDL_SCANCODE_PAUSE, DIK_PAUSE},
    {SDL_SCANCODE_INSERT, DIK_INSERT},
    {SDL_SCANCODE_HOME, DIK_HOME},
    {SDL_SCANCODE_PAGEUP, DIK_PRIOR},
    {SDL_SCANCODE_DELETE, DIK_DELETE},
    {SDL_SCANCODE_END, DIK_END},
    {SDL_SCANCODE_PAGEDOWN, DIK_NEXT},
    {SDL_SCANCODE_RIGHT, DIK_RIGHT}, {SDL_SCANCODE_LEFT, DIK_LEFT}, {SDL_SCANCODE_DOWN, DIK_DOWN}, {SDL_SCANCODE_UP, DIK_UP},
    {SDL_SCANCODE_NUMLOCKCLEAR, DIK_NUMLOCK},
    {SDL_SCANCODE_KP_DIVIDE, DIK_DIVIDE}, {SDL_SCANCODE_KP_MULTIPLY, DIK_MULTIPLY}, {SDL_SCANCODE_KP_MINUS, DIK_SUBTRACT},
    {SDL_SCANCODE_KP_PLUS, DIK_ADD}, {SDL_SCANCODE_KP_ENTER, DIK_NUMPADENTER},
    {SDL_SCANCODE_KP_1, DIK_NUMPAD1}, {SDL_SCANCODE_KP_2, DIK_NUMPAD2}, {SDL_SCANCODE_KP_3, DIK_NUMPAD3},
    {SDL_SCANCODE_KP_4, DIK_NUMPAD4}, {SDL_SCANCODE_KP_5, DIK_NUMPAD5}, {SDL_SCANCODE_KP_6, DIK_NUMPAD6},
    {SDL_SCANCODE_KP_7, DIK_NUMPAD7}, {SDL_SCANCODE_KP_8, DIK_NUMPAD8}, {SDL_SCANCODE_KP_9, DIK_NUMPAD9},
    {SDL_SCANCODE_KP_0, DIK_NUMPAD0}, {SDL_SCANCODE_KP_PERIOD, DIK_DECIMAL},
    {SDL_SCANCODE_LCTRL, DIK_LCONTROL}, {SDL_SCANCODE_LSHIFT, DIK_LSHIFT}, {SDL_SCANCODE_LALT, DIK_LMENU},
    {SDL_SCANCODE_RCTRL, DIK_RCONTROL}, {SDL_SCANCODE_RSHIFT, DIK_RSHIFT}, {SDL_SCANCODE_RALT, DIK_RMENU},
};

static int SDLScancodeToDIK(SDL_Scancode sc) {
    auto it = scancode_to_dik_map.find(sc);
    if (it != scancode_to_dik_map.end()) {
        return it->second;
    }
    return 0; // Unmapped
}

// The one real OS window — set when the first non-child backend is
// created. The liveness watchdog probes this directly instead of the
// CWnd registry, which may have no window-bearing entry at menu stages.
static SDL_Window* g_migOSWindow = nullptr;

// Throttled window-liveness watchdog (called from PumpSDL): an X window
// destroyed outside the WM_DELETE handshake — hard-kill via
// XDestroyWindow/xkill, or a compositor failure on a real desktop —
// delivers no close event at all, so nothing posts the quit and the app
// keeps looping invisibly (menu spins idle, 3D spins rasterizing). A dead
// window is terminal, so post the quit ourselves. XGetWindowAttributes is
// a synchronous roundtrip, so the probe is rate-limited.
static void MigCheckMainWindowAlive()
{
    if (g_shouldQuit) return;                   // quit already in flight
    static auto nextCheck = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (now < nextCheck) return;
    nextCheck = now + std::chrono::milliseconds(250);

    static Display* dpy = nullptr;
    static ::Window xwin = 0;
    static bool probed = false;
    if (!probed) {
        // The one real OS window is captured at creation — m_pMainWnd can
        // be null or a backend-less CWnd at menu/dialog stages, and the
        // owning registry entry may already be gone.
        SDL_Window* sdlWin = g_migOSWindow;
        if (!sdlWin) return;                    // no OS window yet — retry next tick
        // NB: WIN32_COMPAT headers leave #pragma pack(1) active in this TU,
        // which shifts SDL_SysWMinfo.subsystem one byte early — a local
        // padded struct mirrors the ABI instead (same pattern as
        // MigX11WMInfo in XX_ScreenFlip_Vulkan.cpp).
#pragma pack(push, 4)
        struct {
            SDL_version version;
            int subsystem;
            union { struct { Display* display; ::Window window; } x11; char _pad[256]; } info;
        } wmi;
#pragma pack(pop)
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(sdlWin, (SDL_SysWMinfo*)&wmi) || wmi.subsystem != SDL_SYSWM_X11)
            return;                             // not X11 — nothing to probe
        dpy = wmi.info.x11.display;
        xwin = wmi.info.x11.window;
        probed = true;
        fprintf(stderr, "[MFC_stub] window watchdog armed on X window 0x%lx\n",
                (unsigned long)xwin);
    }
    if (dpy && xwin) {
        XWindowAttributes attr;
        if (XGetWindowAttributes(dpy, xwin, &attr) == 0) {
            fprintf(stderr, "[MFC_stub] OS window 0x%lx dead outside close handshake — quitting\n",
                    (unsigned long)xwin);
            AfxPostQuitMessage(0);
        }
    }
}

// Helper to convert SDL events to Win32 MSG
void PumpSDL()
{
    MigCheckMainWindowAlive();
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        MSG msg = {};
        msg.time = SDL_GetTicks();

        switch (e.type)
        {
            case SDL_QUIT:
                msg.message = WM_QUIT;
                g_msgQueue.push_back(msg);
                MigArmQuitTeardown();
                break;

            case SDL_USEREVENT:
                // Queued DirectDraw primary frame — the 3D worker must not
                // touch SDL video, so it posts here and we present on the
                // main thread (Bob/port/dd_present.cpp; absent in migalley).
                {
                    if (e.user.code == 0x44445052 && &RowanDDPumpPresent)
                        RowanDDPumpPresent();
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                SDL_Window* sdlWin = SDL_GetWindowFromID(e.button.windowID);
                CWnd* wnd = AfxGetWndFromSDLWindow(sdlWin);
                if (wnd)
                {
                    int x = e.button.x;
                    int y = e.button.y;
                    HWND targetHwnd = nullptr;

                    if (g_pCaptureWnd) {
                        targetHwnd = g_pCaptureWnd->GetSafeHwnd();
                        CWnd* p = g_pCaptureWnd;
                        int offX = 0, offY = 0;
                        
                        CWnd* root = wnd;
                        if (wnd == AfxGetMainWnd()) root = ((CFrameWnd*)wnd)->GetActiveView();

                        while (p && p != root) {
                            offX += p->m_rect.left;
                            offY += p->m_rect.top;
                            p = p->GetParent();
                        }
                        if (p == root) {
                            x -= offX;
                            y -= offY;
                        } else {
                            targetHwnd = FindChildWindow(wnd->GetSafeHwnd(), x, y);
                        }
                    }

                    if (!targetHwnd) {
                        targetHwnd = FindChildWindow(wnd->GetSafeHwnd(), x, y);
                    }

                    msg.hwnd = targetHwnd;
                    msg.pt_x = x;
                    msg.pt_y = y;
                    if (e.button.button == SDL_BUTTON_LEFT)
                        msg.message = (e.type == SDL_MOUSEBUTTONDOWN) ? WM_LBUTTONDOWN : WM_LBUTTONUP;
                    //if (msg.message == WM_LBUTTONDOWN) std::cout << "[Input] MouseDown HWND=" << targetHwnd << " at (" << x << "," << y << ")" << std::endl;
                    else if (e.button.button == SDL_BUTTON_RIGHT)
                        msg.message = (e.type == SDL_MOUSEBUTTONDOWN) ? 0x0204 /*WM_RBUTTONDOWN*/ : 0x0205 /*WM_RBUTTONUP*/;
                    static const bool dbgInput = getenv("ROWAN_DEBUG_INPUT") != nullptr;
                    if (dbgInput)
                        std::cout << "[IN] " << (e.type == SDL_MOUSEBUTTONDOWN ? "DOWN" : "UP")
                                  << " btn=" << (int)e.button.button
                                  << " hwnd=" << targetHwnd << " wnd=" << (void*)wnd
                                  << " sdlwin=" << (void*)sdlWin << " at " << x << "," << y
                                  << " cap=" << (void*)g_pCaptureWnd << std::endl;
                    // Pack coordinates into lParam
                    msg.lParam = MAKELPARAM(x, y);
                    msg.wParam = (e.type == SDL_MOUSEBUTTONDOWN) ? 1 : 0; // MK_LBUTTON approx

                    g_msgQueue.push_back(msg);
                }
                break;
            }

            case SDL_MOUSEMOTION:
            {
                SDL_Window* sdlWin = SDL_GetWindowFromID(e.motion.windowID);
                CWnd* wnd = AfxGetWndFromSDLWindow(sdlWin);
                if (wnd)
                {
                    int x = e.motion.x;
                    int y = e.motion.y;
                    HWND targetHwnd = nullptr;

                    if (g_pCaptureWnd) {
                        targetHwnd = g_pCaptureWnd->GetSafeHwnd();
                        CWnd* p = g_pCaptureWnd;
                        int offX = 0, offY = 0;

                        CWnd* root = wnd;
                        if (wnd == AfxGetMainWnd()) root = ((CFrameWnd*)wnd)->GetActiveView();

                        // Walk up from the captured window to the main view, accumulating offsets.
                        // The coordinates from the SDL event are relative to the main view's container (the frame).
                        while (p && p != root) {
                            offX += p->m_rect.left;
                            offY += p->m_rect.top;
                            p = p->GetParent();
                        }

                        if (p == root) { // If we successfully walked up to the view
                            x -= offX;
                            y -= offY;
                        } else {
                            // Something is wrong with the hierarchy, fallback to hit-testing
                            targetHwnd = FindChildWindow(wnd->GetSafeHwnd(), x, y);
                        }
                    }

                    if (!targetHwnd) {
                        targetHwnd = FindChildWindow(wnd->GetSafeHwnd(), x, y);
                    }

                    msg.hwnd = targetHwnd;
                    msg.message = WM_MOUSEMOVE;
                    msg.pt_x = x;
                    msg.pt_y = y;
                    msg.wParam = 0;
                    if (e.motion.state & SDL_BUTTON_LMASK) msg.wParam |= MK_LBUTTON;
                    if (e.motion.state & SDL_BUTTON_RMASK) msg.wParam |= MK_RBUTTON;
                    msg.lParam = MAKELPARAM(x, y);
                    g_msgQueue.push_back(msg);
                }
                break;
            }

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                // Any key press skips a playing intro/smack video
                if (e.type == SDL_KEYDOWN)
                    InterruptSmack();

                // Handle special keys for focused controls (e.g. Backspace for CREdit)
                if (e.type == SDL_KEYDOWN && g_pFocusWnd) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE) {
                        msg.hwnd = g_pFocusWnd->GetSafeHwnd();
                        msg.message = WM_CHAR;
                        msg.wParam = 0x08; // VK_BACK
                        msg.time = SDL_GetTicks();
                        g_msgQueue.push_back(msg);
                    }
                    else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                        msg.hwnd = g_pFocusWnd->GetSafeHwnd();
                        msg.message = WM_CHAR;
                        msg.wParam = 0x0D; // VK_RETURN
                        msg.time = SDL_GetTicks();
                        g_msgQueue.push_back(msg);
                    }
                }

                // For 3D engine
                if (e.key.repeat == 0) { // Process only single key presses, not repeats
                    int dik_code = SDLScancodeToDIK(e.key.keysym.scancode);
                    if (dik_code != 0) {
                        std::lock_guard<std::mutex> lock(g_sdl_key_events_mutex);
                        g_sdl_key_events.push_back({dik_code, e.type == SDL_KEYDOWN});
                    }
                }

                // For UI
                CWnd* target = g_pFocusWnd ? g_pFocusWnd : AfxGetMainWnd();
                if (target)
                {
                    msg.hwnd = target->GetSafeHwnd();
                    msg.message = (e.type == SDL_KEYDOWN) ? WM_KEYDOWN : WM_KEYUP;
                    msg.wParam = e.key.keysym.sym; 
                    g_msgQueue.push_back(msg);
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                if (g_pFocusWnd) {
                    for (int i = 0; e.text.text[i] != '\0'; ++i) {
                        msg.hwnd = g_pFocusWnd->GetSafeHwnd();
                        msg.message = WM_CHAR;
                        msg.wParam = (unsigned char)e.text.text[i];
                        msg.time = SDL_GetTicks();
                        g_msgQueue.push_back(msg);
                    }
                }
                break;
            }

            case SDL_WINDOWEVENT:
            {
                SDL_Window* sdlWin = SDL_GetWindowFromID(e.window.windowID);
                CWnd* wnd = AfxGetWndFromSDLWindow(sdlWin);
                if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    // The X button on the application's real OS window must
                    // reach the main frame: many virtual CWnds can share one
                    // SDL window and AfxGetWndFromSDLWindow picks an
                    // arbitrary owner, so WM_CLOSE could otherwise die on a
                    // child/dialog and leave the app running windowless.
                    CWnd* main = AfxGetMainWnd();
                    WindowBackend* mbe = main ? backend_from_hwnd(main->GetSafeHwnd()) : nullptr;
                    CWnd* target = (mbe && mbe->window == sdlWin) ? main : wnd;
                    fprintf(stderr, "[MFC_stub] SDL_WINDOWEVENT_CLOSE sdlWin=%p wnd=%p main=%p mbe->window=%p target=%p\n",
                            (void*)sdlWin, (void*)wnd, (void*)main,
                            mbe ? (void*)mbe->window : nullptr, (void*)target);
                    if (target)
                    {
                        msg.hwnd = target->GetSafeHwnd();
                        msg.message = WM_CLOSE;
                        g_msgQueue.push_back(msg);
                    }
                }
                else if (wnd)
                {
                    msg.hwnd = wnd->GetSafeHwnd();
                    if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    {
                        // RERUN: If the main window gets focus while on the campaign map,
                        // push it to the bottom so dialogs stay visible.
                        if (wnd == AfxGetMainWnd())
                        {
                            CView* pView = ((CFrameWnd*)wnd)->GetActiveView();
                            // Use runtime class check to safely cast to CMIGView
                            if (pView && pView->IsKindOf(RUNTIME_CLASS(CMIGView)))
                            {
                                if (((CMIGView*)pView)->m_currentpage == 0)
                                {
                                    wnd->SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                                }
                            }
                        }
                    }
                    else if (e.window.event == SDL_WINDOWEVENT_EXPOSED ||
                             e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        {
                            MSG sizeMsg = {};
                            sizeMsg.hwnd = wnd->GetSafeHwnd();
                            sizeMsg.message = WM_SIZE;
                            sizeMsg.wParam = SIZE_RESTORED;
                            sizeMsg.lParam = MAKELPARAM(e.window.data1, e.window.data2);
                            sizeMsg.time = msg.time;
                            g_msgQueue.push_back(sizeMsg);
                        }
                        msg.message = WM_PAINT;
                        g_msgQueue.push_back(msg);
                    }
                }
                break;
            }

            case SDL_MOUSEWHEEL:
            {
                SDL_Window* sdlWin = SDL_GetWindowFromID(e.wheel.windowID);
                CWnd* wnd = AfxGetWndFromSDLWindow(sdlWin);
                if (wnd)
                {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    
                    HWND targetHwnd = nullptr;
                    if (g_pCaptureWnd) {
                        CWnd* p = g_pCaptureWnd;
                        int offX = 0, offY = 0;
                        bool found = false;
                        while (p) {
                            if (p == wnd) {
                                found = true;
                                break;
                            }
                            offX += p->m_rect.left;
                            offY += p->m_rect.top;
                            p = p->GetParent();
                        }
                        if (found) {
                            targetHwnd = g_pCaptureWnd->GetSafeHwnd();
                            x -= offX;
                            y -= offY;
                        }
                    }

                    if (!targetHwnd) {
                        targetHwnd = FindChildWindow(wnd->GetSafeHwnd(), x, y);
                    }

                    msg.hwnd = targetHwnd;
                    msg.message = WM_MOUSEWHEEL;
                    
                    int delta = e.wheel.y;
                    if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) delta *= -1;

                    msg.wParam = MAKELPARAM(0, (short)(delta * 120)); 
                    msg.lParam = MAKELPARAM(x, y);
                    msg.pt_x = x;
                    msg.pt_y = y;

                    g_msgQueue.push_back(msg);
                }
                break;
            }
        }
    }
}

BOOL PeekMessage(MSG* msg, HWND hwnd, UINT min, UINT max, UINT remove)
{
    PumpSDL();
    PumpTimers();

    if (g_msgQueue.empty())
    {
        // Check for dirty windows and synthesize WM_PAINT
        for (auto& pair : g_hwndRegistry) {
            WindowBackend& be = pair.second;
            if (be.needsRepaint && be.owner) {
                if (be.visible) {
                    MSG paintMsg = {};
                    paintMsg.hwnd = pair.first;
                    paintMsg.message = WM_PAINT;
                    paintMsg.time = SDL_GetTicks();
                    g_msgQueue.push_back(paintMsg);
                    break; // Only generate one WM_PAINT at a time to respect priority
                } else {
                    be.needsRepaint = false;
                }
            }
        }
    }

    if (g_msgQueue.empty())
    {
        // RERUN: Win32 keeps the quit state once PostQuitMessage fires —
        // every later GetMessage/PeekMessage returns WM_QUIT until the
        // thread exits. Nested pumps (CDialog::DoModal) can consume the
        // queued WM_QUIT and swallow PumpMessage's FALSE; re-synthesizing
        // here lets the unwind reach CMIGApp::Run instead of leaving the
        // app running with the main window already gone.
        if (g_shouldQuit && !hwnd && min <= WM_QUIT && (max == 0 || WM_QUIT <= max))
        {
            msg->hwnd = NULL;
            msg->message = WM_QUIT;
            msg->wParam = 0;
            msg->lParam = 0;
            msg->time = SDL_GetTicks();
            return TRUE;
        }
        //std::cout << "PeekMessage: No messages in queue." << std::endl;
        return FALSE;
    }
    
    *msg = g_msgQueue.front();
    
    if (remove == PM_REMOVE)
    {
        g_msgQueue.pop_front();
    }

    return TRUE;
}

BOOL DispatchMessage(const MSG* msg)
{
    if (!msg->hwnd) return FALSE;

    WindowBackend* backend = backend_from_hwnd((HWND)msg->hwnd);
    if (msg->message == WM_LBUTTONDOWN || msg->message == WM_LBUTTONUP) {
        static const bool dbgInput = getenv("ROWAN_DEBUG_INPUT") != nullptr;
        if (dbgInput)
            std::cout << "[DISP] " << (msg->message == WM_LBUTTONDOWN ? "DOWN" : "UP")
                      << " hwnd=" << msg->hwnd << " be=" << (void*)backend
                      << " owner=" << (backend ? (void*)backend->owner : nullptr)
                      << " type=" << ((backend && backend->owner) ? typeid(*backend->owner).name() : "?")
                      << std::endl;
    }
    if (backend && backend->owner)
    {
        LRESULT r = backend->owner->WindowProc(msg->message, msg->wParam, msg->lParam);
        // Win32 completes a paint with EndPaint — but legacy code paths
        // (BoB's CMainFrame::OnPaint) call BeginPaint without EndPaint.
        // After any WM_PAINT dispatch, walk to the root window and
        // present whatever was drawn, so those paths still reach screen.
        if (msg->message == WM_PAINT) {
            WindowBackend* root = backend;
            while (root && root->isChild && root->parent)
                root = backend_from_hwnd(root->parent->GetSafeHwnd());
            if (root && root->renderer && !DDPrimaryActive()) {
                PaintChildrenRecursive((HWND)msg->hwnd, root->renderer, 0, 0, 0);
                SDL_RenderPresent(root->renderer);
                root->needsRepaint = false;
                root->fullDirty = false;
                root->dirtyRegions.clear();
            }
        }
        return r;
    }
    return FALSE;
}

void AfxPostQuitMessage(int /*nExitCode*/)
{
    fprintf(stderr, "[MFC_stub] AfxPostQuitMessage -> WM_QUIT queued\n");
    MigArmQuitTeardown();
    MSG msg = {};
    msg.message = WM_QUIT;
    msg.time = SDL_GetTicks();
    g_msgQueue.push_back(msg);
}

BOOL PumpMessage()
{
    MSG msg;
    if (!PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        return TRUE;

    if (msg.message == WM_QUIT)
    {
        MigArmQuitTeardown();
        return FALSE;
    }

    DispatchMessage(&msg);
    return TRUE;
}

BOOL IsIdleMessage(MSG* pMsg)
{
    // Return TRUE if the message does NOT reset the idle timer.
    // Standard MFC implementation returns TRUE for WM_TIMER and WM_MOUSEMOVE (without buttons)
    if (pMsg->message == WM_TIMER || pMsg->message == 0x0113 /*WM_SYSTIMER*/)
        return TRUE;
    if (pMsg->message == WM_MOUSEMOVE) // && (pMsg->wParam == 0) check buttons?
        return TRUE;
    return FALSE;
}

BOOL OnIdle(LONG lCount)
{
    // Stub implementation
    return FALSE; // No more idle processing needed
}

// Win32 treats a dirty update region as a pending QS_PAINT input for
// MsgWaitForMultipleObjects — the wait must return so the caller reaches
// PeekMessage, where the stub synthesizes WM_PAINT.  Without this a wait
// that only saw queued messages would sleep forever while repaints piled
// up (BoB froze on the last cockpit frame after exiting the 3D view: the
// debrief's dirty marks never woke the loop).
static bool AnyDirtyVisibleWindow()
{
    for (auto& pair : g_hwndRegistry)
        if (pair.second.needsRepaint && pair.second.owner &&
            pair.second.visible)
            return true;
    return false;
}

// ROWAN_DEBUG_REPAINT dump: which backends carry a dirty mark and why
// each one fails the repaint gate (missing owner, hidden).  Also lists
// every non-child backend (the SDL window owners) since repaints only
// reach the screen through them.
static void DumpDirtyWindows()
{
    static const bool dbg = getenv("ROWAN_DEBUG_REPAINT") != nullptr;
    if (!dbg) return;
    for (auto& pair : g_hwndRegistry)
        if (pair.second.needsRepaint || !pair.second.isChild)
            printf("[REPAINT] %s hwnd=%p win=%p vis=%d owner=%p child=%d dirty=%d tid=%d\n",
                   pair.second.needsRepaint ? "DIRTY" : "root ",
                   pair.first, (void*)pair.second.window,
                   (int)pair.second.visible,
                   (void*)pair.second.owner, (int)pair.second.isChild,
                   (int)pair.second.needsRepaint,
                   (int)pair.second.templateID);
    fflush(stdout);
}

DWORD MsgWaitForMultipleObjects(
    DWORD nCount,
    const HANDLE* pHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask)
{
    // 1. Check for input if requested
    if (dwWakeMask & QS_ALLINPUT)
    {
        PumpSDL();
        PumpTimers();
        // RERUN: a pending quit must also wake the caller — WM_QUIT is
        // re-synthesized by PeekMessage once the queue has drained.
        if (!g_msgQueue.empty() || g_shouldQuit ||
            AnyDirtyVisibleWindow())
        {
            // Return index equal to nCount to indicate input is available
            return WAIT_OBJECT_0 + nCount;
        }
    }

    // 2. Check handles
    // This is a simplified poll. Real implementation would need to wait on condition variables.
    // Given the game loop structure, a poll with short sleep is acceptable for now.
    
    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        // Check handles
        for (DWORD i = 0; i < nCount; ++i)
        {
            PosixEventObject* ev = (PosixEventObject*)pHandles[i];
            if (ev)
            {
                // Try lock to check state safely
                if (pthread_mutex_trylock(&ev->mutex) == 0)
                {
                    // Win32 auto-reset semantics: a satisfied wait consumes
                    // the signal unless the event was created manual-reset.
                    if (ev->signaled)
                    {
                        if (!ev->manualReset)
                            ev->signaled = false;
                        pthread_mutex_unlock(&ev->mutex);
                        return WAIT_OBJECT_0 + i;
                    }
                    pthread_mutex_unlock(&ev->mutex);
                }
            }
        }

        // Check input again if we are waiting
        if (dwWakeMask & QS_ALLINPUT)
        {
            PumpSDL();
            PumpTimers();
            if (!g_msgQueue.empty() || g_shouldQuit ||
                AnyDirtyVisibleWindow())
                return WAIT_OBJECT_0 + nCount;
            static int dumpDiv = 0;
            if (++dumpDiv >= 2000) { dumpDiv = 0; DumpDirtyWindows(); }
        }

        // Check timeout
        if (dwMilliseconds != INFINITE)
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            if ((DWORD)elapsed >= dwMilliseconds)
                return WAIT_TIMEOUT;
        }
        else if (dwMilliseconds == 0)
        {
            return WAIT_TIMEOUT;
        }

        // Sleep a tiny bit to avoid 100% CPU
        SDL_Delay(1);
    }
}

void backend_mark_dirty(HWND hwnd, const SDL_Rect* rect)
{
    auto it = g_hwndRegistry.find(hwnd);
    if (it == g_hwndRegistry.end())
        return;

    WindowBackend* pBe = &it->second;

    // If this is a child window, we must mark the top-level parent dirty
    // because only the parent calls SDL_RenderPresent.
    while (pBe->isChild && pBe->parent && pBe->parent->m_hWnd) {
        auto parentIt = g_hwndRegistry.find(pBe->parent->m_hWnd);
        if (parentIt == g_hwndRegistry.end()) break;
        pBe = &parentIt->second;
    }

    // Mark the window as needing repaint
    pBe->needsRepaint = true;
    //std::cout << "backend_mark_dirty: Marked " << pBe->owner << " (HWND " << hwnd << ") dirty. Parent chain walked." << std::endl;

    if (rect) {
        pBe->dirtyRegions.push_back(*rect);
    } else {
        // Mark whole window dirty
        pBe->fullDirty = true;
    }
}

BOOL CWnd::Create(
    LPCTSTR lpszClassName,
    LPCTSTR lpszWindowName,
    DWORD dwStyle,
    const RECT& rect,
    CWnd* pParentWnd,
    UINT nID,
    CCreateContext* pContext)
{
    //m_hWnd = allocate_hwnd();
    HWND h = allocate_hwnd();
    WindowBackend& backend = g_hwndRegistry[h];

    m_pParent = pParentWnd;
    m_nID = nID;

    backend.owner = this;
    backend.parent = pParentWnd;
    backend.templateID = nID;
    m_dwStyle = dwStyle; // RERUN: Store the style in the CWnd object
    backend.style = dwStyle;
    backend.visible = (dwStyle & WS_VISIBLE) != 0;
    backend.isChild = false;
    //SDL_Window* win = SDL_CreateWindow(
    m_windowText = lpszWindowName ? lpszWindowName : ""; // RERUN: Store initial text
    // Check if this should be a child window sharing the backend
    bool isChild = (pParentWnd != nullptr) && (dwStyle & WS_CHILD);

    if (isChild && pParentWnd->m_hWnd) {
        WindowBackend* parentBe = backend_from_hwnd(pParentWnd->m_hWnd);
        if (parentBe) {
            backend.window = parentBe->window;
            backend.renderer = parentBe->renderer;
            backend.isChild = true;
        }
    }

    // RERUN: route top-level dialogs into the existing game window once it
    // exists. Giving each dialog its own SDL window scatters UI pieces
    // across the desktop under a real window manager.
    CWnd* host = pParentWnd;
    if (!isChild) {
        CWnd* main = AfxGetMainWnd();
        if (main && main != this && main->GetSafeHwnd()) {
            WindowBackend* mainBe = backend_from_hwnd(main->GetSafeHwnd());
            if (mainBe && mainBe->window && mainBe->renderer) {
                backend.window = mainBe->window;
                backend.renderer = mainBe->renderer;
                backend.isChild = true;
                backend.parent = pParentWnd ? pParentWnd : main;
                isChild = true;
                host = main;
            }
        }
        if (!isChild) {
            // No CFrameWnd exists in this UI (top sheet is an RDEmptyD), so
            // fall back to the first real window backend in the registry.
            for (auto& kv : g_hwndRegistry) {
                WindowBackend& be = kv.second;
                if (be.window && !be.isChild && be.owner && be.owner != this) {
                    backend.window = be.window;
                    backend.renderer = be.renderer;
                    backend.isChild = true;
                    backend.parent = pParentWnd ? pParentWnd : be.owner;
                    isChild = true;
                    host = be.owner;
                    break;
                }
            }
        }
    }

    if (!backend.isChild) {
    backend.window = CreateOSWindow(
        lpszWindowName ? lpszWindowName : "",
            rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS
    );
    if (backend.window && !g_migOSWindow)
        g_migOSWindow = backend.window;

    //SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
    //WindowBackend backend =  WindowBackend{win, ren, this};
    backend.renderer = SDL_CreateRenderer(backend.window, -1, SDL_RENDERER_ACCELERATED); 
        if (!backend.renderer) {
             backend.renderer = SDL_CreateRenderer(backend.window, -1, 0);
        }
    }

//////////
    // Generate HWND key
    //HWND key = reinterpret_cast<HWND>(backend.window); 
    // or reinterpret_cast<HWND>(this) — whichever convention you use

    // Register
    //g_hwndRegistry[key] = backend;

    // Store in CWnd
    //m_hWnd = key;
    m_hWnd = h;

    // Initialize m_rect from the SDL window so GetWindowRect works
    if (backend.window && !backend.isChild) {
        int x, y, w, h;
        SDL_GetWindowPosition(backend.window, &x, &y);
        SDL_GetWindowSize(backend.window, &w, &h);
        m_rect = CRect(x, y, x + w, y + h);
    } else {
        m_rect = rect;
    }

    // Attach to parent for hit-testing. m_rect is always stored relative to
    // the CWnd parent (pParentWnd), so the children list must follow CWnd
    // parentage — not the SDL host, which only decides which OS window we
    // render into. Routed top-level dialogs (isChild forced) therefore still
    // land on their dialog parent's child list.
    CWnd* hittestParent = (pParentWnd && pParentWnd->m_hWnd) ? pParentWnd : host;
    if (hittestParent) {
        if (hittestParent->m_hWnd) {
             g_hwndRegistry[hittestParent->m_hWnd].children.push_back(h);
        }
        if (pParentWnd && nID != 0) {
            pParentWnd->m_children[nID] = this;
        }
    }
//////////

    return TRUE;
}

CWnd* CWnd::FromHandle(HWND hWnd)
{
    if (!hWnd) return nullptr;
    auto it = g_hwndRegistry.find(hWnd);
    if (it != g_hwndRegistry.end()) {
        return it->second.owner;
    }
    return nullptr;
}

void CWnd::SetFocus()
{
    if (g_pFocusWnd != this) {
        if (g_pFocusWnd) g_pFocusWnd->Invalidate();
        g_pFocusWnd = this;
        Invalidate();

        if (IsKindOf(RUNTIME_CLASS(CREdit))) {
            SDL_StartTextInput();
        } else {
            SDL_StopTextInput();
        }
    }
}

CWnd* CWnd::GetFocus()
{
    return g_pFocusWnd;
}

BOOL CWnd::InvalidateRect(const CRect* lpRect, BOOL bErase)
{
    if (!m_hWnd)
        return FALSE;

    SDL_Rect r{};
    SDL_Rect* pr = nullptr;

    if (lpRect) {
        r.x = lpRect->left;
        r.y = lpRect->top;
        r.w = lpRect->Width();
        r.h = lpRect->Height();
        pr = &r;
    }

    backend_mark_dirty(m_hWnd, pr);
    return TRUE;
}

BOOL CWnd::InvalidateRect(const CRect& rect, BOOL bErase)
{
    return InvalidateRect(&rect, bErase);
}

CWnd* CWnd::GetDlgItem(int id) const
{
    auto it = m_children.find(id);
    return it != m_children.end() ? it->second : nullptr;
}

void CWnd::GetDlgItem(int id, HWND* phWnd) const
{
    if (!phWnd) return;

    // Look up child window in your registry
    for (auto& [hwnd, be] : g_hwndRegistry)
    {
        if (be.owner && be.owner->m_nID == id && be.owner->m_pParent == this)
        {
            *phWnd = hwnd;
            return;
        }
    }

    *phWnd = nullptr;
}

// CEvent
CEvent::CEvent(bool manualReset, bool initialState, const char* lpName, void* lpEventAttributes) {
    hEvent = CreateEvent(lpEventAttributes, manualReset, initialState, lpName);
}
CEvent::~CEvent() {
    PosixEventObject* ev = (PosixEventObject*)hEvent;
    pthread_mutex_destroy(&ev->mutex);
    pthread_cond_destroy(&ev->cond);
    delete ev;
}
void CEvent::SetEvent() { ::SetEvent(hEvent); }
void CEvent::ResetEvent() { ::ResetEvent(hEvent); }
int CEvent::WaitForSingleObject(int timeout) { return ::WaitForSingleObject(hEvent, timeout); }
bool CEvent::Lock(int timeout) { return (::WaitForSingleObject(hEvent, timeout) == WAIT_OBJECT_0); }
CEvent::operator HANDLE() const { return hEvent; }

// CDC
void CDC::MoveTo(int x, int y)
{
    m_currentPos.x = x;
    m_currentPos.y = y;
}
void CDC::MoveTo(const CPoint& pt) {MoveTo(pt.x, pt.y);}
void CDC::MoveTo(const POINT& pt) {MoveTo(pt.x, pt.y);}
void CDC::LineTo(int x, int y)
{
    if (!m_renderer) return;

    COLORREF color = RGB(150, 150, 150); // Default to gray if no pen
    if (m_pCurrentPen) {
        color = m_pCurrentPen->m_color;
    }

    SDL_SetRenderDrawColor(m_renderer, GetRValue(color), GetGValue(color), GetBValue(color), 255);
    SDL_RenderDrawLine(m_renderer, m_currentPos.x, m_currentPos.y, x, y);

    // Update current position
    m_currentPos.x = x;
    m_currentPos.y = y;
}
void CDC::LineTo(const CPoint& pt) {LineTo(pt.x, pt.y);}
void CDC::LineTo(const POINT& pt) {LineTo(pt.x, pt.y);}
void CDC::Rectangle(int x1, int y1, int x2, int y2)
{
    if (!m_renderer) return;

    // Fill
    if (m_pCurrentBrush) {
        HBRUSH hBrush = (HBRUSH)*m_pCurrentBrush;
        _BRUSH* pBrush = (_BRUSH*)hBrush;
        if (pBrush && pBrush->type == _BRUSH::SOLID) {
             SDL_SetRenderDrawColor(m_renderer, pBrush->solid.r, pBrush->solid.g, pBrush->solid.b, 255);
             SDL_Rect r = { x1, y1, x2 - x1, y2 - y1 };
             SDL_RenderFillRect(m_renderer, &r);
        }
    }

    // Border
    if (m_pCurrentPen && m_pCurrentPen->m_style != PS_NULL) {
        COLORREF color = m_pCurrentPen->m_color;
        SDL_SetRenderDrawColor(m_renderer, GetRValue(color), GetGValue(color), GetBValue(color), 255);
        SDL_Rect r = { x1, y1, x2 - x1, y2 - y1 };
        SDL_RenderDrawRect(m_renderer, &r);
    }
}
void CDC::Ellipse(int x1, int y1, int x2, int y2)
{
    if (!m_renderer) return;

    int w = x2 - x1;
    int h = y2 - y1;
    if (w <= 0 || h <= 0) return;

    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    int a = w / 2;
    int b = h / 2;

    // Fill (Scanline approximation)
    if (m_pCurrentBrush) {
        HBRUSH hBrush = (HBRUSH)*m_pCurrentBrush;
        _BRUSH* pBrush = (_BRUSH*)hBrush;
        if (pBrush && pBrush->type == _BRUSH::SOLID) {
             SDL_SetRenderDrawColor(m_renderer, pBrush->solid.r, pBrush->solid.g, pBrush->solid.b, 255);
             for (int dy = -b; dy <= b; dy++) {
                 int dx = (int)(a * sqrt(1.0 - (double)(dy*dy)/(b*b)));
                 SDL_RenderDrawLine(m_renderer, cx - dx, cy + dy, cx + dx, cy + dy);
             }
        }
    }

    // Outline (Simple angular step approximation)
    if (m_pCurrentPen && m_pCurrentPen->m_style != PS_NULL) {
        COLORREF color = m_pCurrentPen->m_color;
        SDL_SetRenderDrawColor(m_renderer, GetRValue(color), GetGValue(color), GetBValue(color), 255);
        const int points = 64;
        SDL_Point pts[points + 1];
        for (int i = 0; i <= points; ++i) {
            float theta = 2.0f * 3.14159f * i / points;
            pts[i].x = cx + (int)(a * cos(theta));
            pts[i].y = cy + (int)(b * sin(theta));
        }
        SDL_RenderDrawLines(m_renderer, pts, points + 1);
    }
}
CBitmap* CDC::SelectObject(CBitmap* pBitmap)
{
    CBitmap* old = m_selectedBitmap;
    m_selectedBitmap = pBitmap;
    return old;
}
void CDC::SetTextColor(unsigned int color) { m_textColor = color; }
int CDC::SetBkMode(int mode)
{
    int old = m_bkMode;
    m_bkMode = mode;
    return old;
}
void CDC::TextOut(int x, int y, const CString& str) { TextOut(x, y, str.c_str(), str.GetLength()); }
int CDC::DrawText(const CString& str, const CRect& rect, UINT nFormat)
{
    // RERUN: Handle DT_WORDBREAK using SDL_ttf wrapping
    if (nFormat & DT_WORDBREAK) {
        if (!m_renderer) return 0;

        TTF_Font* font = nullptr;
        if (m_pCurrentFont && m_pCurrentFont->m_hObject) {
            font = (TTF_Font*)m_pCurrentFont->m_hObject;
        }
        if (!font) {
             static TTF_Font* s_defaultFont = nullptr;
             if (!s_defaultFont) {
                 std::string path = GetSystemFontPath();
                 if (!path.empty()) s_defaultFont = TTF_OpenFont(path.c_str(), 16);
             }
             font = s_defaultFont;
        }
        if (!font) return 0;

        SDL_Color color = { (Uint8)GetRValue(m_textColor), (Uint8)GetGValue(m_textColor), (Uint8)GetBValue(m_textColor), 255 };
        
        // Use rect width for wrapping limit. 
        // Note: TTF_RenderText_Blended_Wrapped takes width in pixels.
        Uint32 wrapLength = rect.Width();
        if (wrapLength <= 0) wrapLength = 100; 

        SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, str.c_str(), color, wrapLength);
        if (!surf) {
             surf = TTF_RenderText_Solid(font, str.c_str(), color);
        }
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surf);
            if (tex) {
                int x = rect.left;
                int y = rect.top;
                // Simple alignment support
                if (nFormat & DT_CENTER) x += (rect.Width() - surf->w) / 2;
                else if (nFormat & DT_RIGHT) x += (rect.Width() - surf->w);
                
                if (nFormat & DT_VCENTER) y += (rect.Height() - surf->h) / 2;
                else if (nFormat & DT_BOTTOM) y += (rect.Height() - surf->h);

                SDL_Rect dst = {x, y, surf->w, surf->h};
                SDL_RenderCopy(m_renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            int h = surf->h;
            SDL_FreeSurface(surf);
            return h;
        }
        return 0;
    }

    CSize sz = GetTextExtent(str);
    int x = rect.left;
    int y = rect.top;

    if (nFormat & DT_CENTER)
        x = rect.left + (rect.Width() - sz.cx) / 2;
    else if (nFormat & DT_RIGHT)
        x = rect.right - sz.cx;

    if (nFormat & DT_VCENTER)
        y = rect.top + (rect.Height() - sz.cy) / 2;
    else if (nFormat & DT_BOTTOM)
        y = rect.bottom - sz.cy;

    TextOut(x, y, str);
    return sz.cy;
}
void CDC::FillSolidRect(const CRect& rect, unsigned int color)
{
    FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), color);
}
void CDC::FillSolidRect(int x, int y, int cx, int cy, unsigned int color)
{
    if (!m_renderer) return;
    SDL_Rect r = { x, y, cx, cy };
    SDL_SetRenderDrawColor(m_renderer, GetRValue(color), GetGValue(color), GetBValue(color), 255);
    SDL_RenderFillRect(m_renderer, &r);
}
BOOL CDC::FillRect(const CRect& rect, HBRUSH hBrush)
{
    if (!hBrush) return FALSE;
    _BRUSH* pBrush = (_BRUSH*)hBrush;
    if (pBrush->type == _BRUSH::SOLID) {
        FillSolidRect(rect, RGB(pBrush->solid.r, pBrush->solid.g, pBrush->solid.b));
    }
    return TRUE;
}
void CDC::Draw3dRect(int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight)
{
    FillSolidRect(x, y, cx - 1, 1, clrTopLeft);
    FillSolidRect(x, y, 1, cy - 1, clrTopLeft);
    FillSolidRect(x + cx - 1, y, 1, cy, clrBottomRight);
    FillSolidRect(x, y + cy - 1, cx, 1, clrBottomRight);
}
void CDC::Draw3dRect(const CRect& rect, COLORREF clrTopLeft, COLORREF clrBottomRight)
{
    Draw3dRect(rect.left, rect.top, rect.Width(), rect.Height(), clrTopLeft, clrBottomRight);
}
BOOL CDC::CreateCompatibleDC(CDC* /*pDC*/)
{
    // Memory DC: no target surface
    m_targetSurface = nullptr;
    m_selectedBitmap = nullptr;
    return TRUE;
}
void CDC::SetMapMode(int /*mode*/) {}
CPen* CDC::SelectObject(CPen* pen)
{
    CPen* oldPen = m_pCurrentPen;
    m_pCurrentPen = pen;
    return oldPen;
}
CBrush* CDC::SelectObject(CBrush* pBrush)
{
    CBrush* old = m_pCurrentBrush;
    m_pCurrentBrush = pBrush;
    return old;
}
int CDC::GetBoundsRect(RECT* /*pRect*/, unsigned int /*flags*/) {return 0;}
HDC CDC::GetSafeHdc() const { return m_hDC; }
COLORREF CDC::SetBkColor(COLORREF color) { COLORREF old = m_bkColor; m_bkColor = color; return old; }
CSize CDC::GetTextExtent(const CString& str) const 
{ 
    if (str.IsEmpty()) {
        return CSize(0, 0);
    }

    TTF_Font* font = nullptr;
    if (m_pCurrentFont && m_pCurrentFont->m_hObject) {
        font = (TTF_Font*)m_pCurrentFont->m_hObject;
    }

    if (!font) {
        static bool s_fallbackWarned = false;
        if (!s_fallbackWarned) {
             std::cout << "[CDC::GetTextExtent] Font is NULL (load failed?), falling back to default 24pt font." << std::endl;
             s_fallbackWarned = true;
        }
        static TTF_Font* s_defaultFont = nullptr;
        if (!s_defaultFont) {
            std::string path = GetSystemFontPath();
            if (!path.empty()) {
                s_defaultFont = TTF_OpenFont(path.c_str(), 24);
            }
        }
        font = s_defaultFont;
    }

    if (font) {
        int w, h;
        if (TTF_SizeText(font, str.c_str(), &w, &h) == 0) {
            return CSize(w, h);
        }
    }

    // Fallback
    return CSize(str.GetLength() * 8, 16);
}
CSize CDC::GetTextExtent(LPCTSTR text) const { return GetTextExtent(CString(text)); }
UINT CDC::SetTextAlign(UINT align) 
{ 
    UINT old = m_textAlign; 
    m_textAlign = align; 
    return old; 
}
CFont* CDC::SelectObject(CFont* pFont)
{
    CFont* old = m_pCurrentFont;
    m_pCurrentFont = pFont;
    return old;
}
CFont* CDC::GetCurrentFont() const
{
    if (!m_pCurrentFont)
        return (CFont*)const_cast<CDC*>(this)->SelectStockObject(SYSTEM_FONT);
    return m_pCurrentFont;
}
BOOL CDC::BitBlt(int x, int y, int cx, int cy,
                 CDC* pSrcDC, int srcX, int srcY, DWORD /*rop*/)
{
    if (!pSrcDC || !pSrcDC->m_selectedBitmap)
        return FALSE;

    // If we have a renderer, use hardware acceleration
    if (m_renderer) {
        SDL_Texture* tex = pSrcDC->m_selectedBitmap->GetTexture(m_renderer);
        if (!tex) {
            std::cout << "CDC::BitBlt: Failed to create texture from bitmap: " << SDL_GetError() << std::endl;
            return FALSE;
        }
        
        SDL_Rect srcRect{ srcX, srcY, cx, cy };
        SDL_Rect dstRect{ x, y, cx, cy };
        
        SDL_RenderCopy(m_renderer, tex, &srcRect, &dstRect);
        return TRUE;
    }

    // Fallback to software blit
    SDL_Surface* src = (SDL_Surface*)pSrcDC->m_selectedBitmap->m_hObject;
    if (!src)
        return FALSE;

    SDL_Surface* dst = m_targetSurface;
    if (!dst)
        return FALSE;

    SDL_Rect srcRect{ srcX, srcY, cx, cy };
    SDL_Rect dstRect{ x, y, cx, cy };

    SDL_BlitSurface(src, &srcRect, dst, &dstRect);
    return TRUE;
}

int CDC::SetDIBitsToDevice(int xDest, int yDest, DWORD w, DWORD h, int xSrc, int ySrc, UINT uStartScan, UINT cScanLines, const void *lpvBits, const BITMAPINFO *lpbmi, UINT fuColorUse)
{
    if (!m_renderer || !lpvBits || !lpbmi) return 0;

    int width = lpbmi->bmiHeader.biWidth;
    int height = lpbmi->bmiHeader.biHeight; // Can be negative for top-down
    bool bottomUp = (height > 0);
    height = abs(height);

    SDL_Surface* surface = nullptr;

    if (lpbmi->bmiHeader.biBitCount == 8) {
        // 8-bit palettized
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpvBits, width, height, 8, 
                                                     (width + 3) & ~3, // 4-byte aligned pitch
                                                     SDL_PIXELFORMAT_INDEX8);
        if (surface) {
            SDL_Color colors[256];
            for (int i = 0; i < 256; ++i) {
                colors[i].r = lpbmi->bmiColors[i].rgbRed;
                colors[i].g = lpbmi->bmiColors[i].rgbGreen;
                colors[i].b = lpbmi->bmiColors[i].rgbBlue;
                colors[i].a = 255;
            }
            SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
        }
    } else if (lpbmi->bmiHeader.biBitCount == 16) {
        // 16-bit RGB565 (common in this era)
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpvBits, width, height, 16,
                                                     ((width * 2) + 3) & ~3,
                                                     SDL_PIXELFORMAT_RGB565);
    } else if (lpbmi->bmiHeader.biBitCount == 24) {
        // 24-bit BGR
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpvBits, width, height, 24,
                                                     ((width * 3) + 3) & ~3,
                                                     SDL_PIXELFORMAT_BGR24);
    } else if (lpbmi->bmiHeader.biBitCount == 32) {
        // 32-bit BGRA (or BGRX)
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpvBits, width, height, 32,
                                                     width * 4,
                                                     SDL_PIXELFORMAT_ARGB8888); // Often matches Windows 32-bit DIB
    }

    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
        if (texture) {
            // Handle source rect (Rowan usually passes 0,0 for src)
            SDL_Rect srcRect = { xSrc, ySrc, (int)w, (int)h };
            // Handle dest rect
            SDL_Rect dstRect = { xDest, yDest, (int)w, (int)h };

            // If bottom-up, we need to flip vertically.
            // SDL_RenderCopyEx flips the image inside the destination rect.
            if (bottomUp) {
                SDL_RenderCopyEx(m_renderer, texture, &srcRect, &dstRect, 0, NULL, SDL_FLIP_VERTICAL);
            } else {
                SDL_RenderCopy(m_renderer, texture, &srcRect, &dstRect);
            }
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    return cScanLines;
}

int StretchDIBits(HDC hdc, int xDest, int yDest, int nDestWidth, int nDestHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, const VOID *lpBits, const BITMAPINFO *lpbmi, UINT iUsage, DWORD dwRop)
{
    CDC* pDC = (CDC*)hdc;
    //std::cout << "StretchDIBits: Dest(" << xDest << "," << yDest << " " << nDestWidth << "x" << nDestHeight << ") "
    //          << "Src(" << xSrc << "," << ySrc << " " << nSrcWidth << "x" << nSrcHeight << ")" << std::endl;

    if (!pDC || !pDC->m_renderer || !lpBits || !lpbmi) {
        if (!pDC) std::cout << "StretchDIBits: pDC is NULL" << std::endl;
        else if (!pDC->m_renderer) std::cout << "StretchDIBits: pDC->m_renderer is NULL" << std::endl;
        else std::cout << "StretchDIBits: lpBits or lpbmi is NULL" << std::endl;
        return 0;
    }

    int width = lpbmi->bmiHeader.biWidth;
    int height = lpbmi->bmiHeader.biHeight; // Can be negative for top-down
    bool bottomUp = (height > 0);
    height = abs(height);

    SDL_Surface* surface = nullptr;

    if (lpbmi->bmiHeader.biBitCount == 8) {
        // 8-bit palettized
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpBits, width, height, 8, 
                                                     (width + 3) & ~3, // 4-byte aligned pitch
                                                     SDL_PIXELFORMAT_INDEX8);
        if (surface) {
            SDL_Color colors[256];
            int magentaIndex = -1;
            for (int i = 0; i < 256; ++i) {
                colors[i].r = lpbmi->bmiColors[i].rgbRed;
                colors[i].g = lpbmi->bmiColors[i].rgbGreen;
                colors[i].b = lpbmi->bmiColors[i].rgbBlue;
                colors[i].a = 255;
                // Check for Magic Pink/Magenta (Transparency Key)
                if (colors[i].r == 255 && colors[i].g == 0 && colors[i].b == 255)
                    magentaIndex = i;
            }
            SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
            // RERUN: Use Magenta index if found, otherwise default to index 0
            SDL_SetColorKey(surface, SDL_TRUE, (magentaIndex != -1) ? magentaIndex : 0);
        }
    } else if (lpbmi->bmiHeader.biBitCount == 16) {
        // 16-bit RGB565 (common in this era)
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpBits, width, height, 16,
                                                     ((width * 2) + 3) & ~3,
                                                     SDL_PIXELFORMAT_RGB565);
        if (surface) {
             Uint32 key = SDL_MapRGB(surface->format, 255, 0, 255);
             SDL_SetColorKey(surface, SDL_TRUE, key);
        }
    } else if (lpbmi->bmiHeader.biBitCount == 24) {
        // 24-bit BGR
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpBits, width, height, 24,
                                                     ((width * 3) + 3) & ~3,
                                                     SDL_PIXELFORMAT_BGR24);
        if (surface) {
             Uint32 key = SDL_MapRGB(surface->format, 255, 0, 255);
             SDL_SetColorKey(surface, SDL_TRUE, key);
        }
    } else if (lpbmi->bmiHeader.biBitCount == 32) {
        // 32-bit BGRA (or BGRX)
        surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)lpBits, width, height, 32,
                                                     width * 4,
                                                     SDL_PIXELFORMAT_ARGB8888); // Often matches Windows 32-bit DIB
        if (surface) {
             Uint32 key = SDL_MapRGB(surface->format, 255, 0, 255);
             SDL_SetColorKey(surface, SDL_TRUE, key);
        }
    }

    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(pDC->m_renderer, surface);
        if (texture) {
            SDL_Rect srcRect = { xSrc, ySrc, nSrcWidth, nSrcHeight };
            SDL_Rect dstRect = { xDest, yDest, nDestWidth, nDestHeight };

            if (bottomUp) {
                SDL_RenderCopyEx(pDC->m_renderer, texture, &srcRect, &dstRect, 0.0, NULL, SDL_FLIP_VERTICAL);
            } else {
                SDL_RenderCopy(pDC->m_renderer, texture, &srcRect, &dstRect);
            }
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
        return nDestHeight;
    }
    
    std::cout << "StretchDIBits: Failed to create surface. BitCount=" << lpbmi->bmiHeader.biBitCount << " Width=" << width << " Height=" << height << " SDL_Error: " << SDL_GetError() << std::endl;
    return 0;
}

void CDC::TextOut(int x, int y, const char* str, int len)
{
    if (!m_renderer || !str || len <= 0) return;

    TTF_Font* font = nullptr;
    if (m_pCurrentFont && m_pCurrentFont->m_hObject) {
        font = (TTF_Font*)m_pCurrentFont->m_hObject;
    }

    if (!font) {
        static bool s_fallbackWarned = false;
        if (!s_fallbackWarned) {
             std::cout << "[CDC::TextOut] Font is NULL (load failed?), falling back to default 16pt font." << std::endl;
             s_fallbackWarned = true;
        }
        static TTF_Font* s_defaultFont = nullptr;
        if (!s_defaultFont) {
            std::string path = GetSystemFontPath();
            if (!path.empty()) {
                s_defaultFont = TTF_OpenFont(path.c_str(), 16);
            }
        }
        font = s_defaultFont;
    }

    if (!font) {
        static bool s_warned = false;
        if (!s_warned) {
            std::cout << "[CDC::TextOut] Failed to load font. Text will not be visible." << std::endl;
            s_warned = true;
        }
        return;
    }

    SDL_Color color = { (Uint8)GetRValue(m_textColor), (Uint8)GetGValue(m_textColor), (Uint8)GetBValue(m_textColor), 255 };
    std::string s(str, len);
    SDL_Surface* surf = TTF_RenderText_Solid(font, s.c_str(), color);
    if (surf) {
        if (m_bkMode == OPAQUE) {
            SDL_SetRenderDrawColor(m_renderer, GetRValue(m_bkColor), GetGValue(m_bkColor), GetBValue(m_bkColor), 255);
            SDL_Rect bgRect = { x, y, surf->w, surf->h };
            SDL_RenderFillRect(m_renderer, &bgRect);
        }
        SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surf);
        if (tex) {
            SDL_Rect dst = {x, y, surf->w, surf->h};
            SDL_RenderCopy(m_renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_FreeSurface(surf);
    }
}

void UpdateSDLVisibility(WindowBackend& backend)
{
    if (!backend.window)
        return;

    if (!backend.isChild) {
        if (backend.visible)
            SDL_ShowWindow(backend.window);
        else
            SDL_HideWindow(backend.window);
    }

    if (backend.owner)
        backend_mark_dirty(backend.owner->m_hWnd, nullptr);
}

SDL_Window* CreateSDLDialogWindow(int templateID)
{
    // For now, create a simple SDL window.
    return CreateOSWindow(
        "Dialog",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        100, 100,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS
    );
}


// CWnd
BOOL CWnd::EnableWindow(BOOL /*bEnable*/) { return TRUE; }
BOOL CWnd::IsWindowEnabled() const { return TRUE; }
BOOL CWnd::IsWindowVisible() const 
{ 
    if (!m_hWnd) return FALSE;
    auto it = g_hwndRegistry.find(m_hWnd);
    if (it != g_hwndRegistry.end())
        return it->second.visible;
    return FALSE; 
}
void CWnd::RedrawWindow(const RECT* lpRectUpdate, void* /*pRgnUpdate*/, unsigned int flags)
{
    if (flags & RDW_INVALIDATE)
    {
        CRect rect;
        const CRect* pRect = nullptr;
        if (lpRectUpdate)
        {
            rect.left = lpRectUpdate->left;
            rect.top = lpRectUpdate->top;
            rect.right = lpRectUpdate->right;
            rect.bottom = lpRectUpdate->bottom;
            pRect = &rect;
        }
        BOOL bErase = (flags & RDW_ERASE) != 0;
        InvalidateRect(pRect, bErase);
    }

    if (flags & (RDW_UPDATENOW | RDW_ERASENOW))
    {
        UpdateWindow();
    }
}
BOOL CWnd::ShowWindow(int cmd)
{
    if (!m_hWnd) return FALSE;

    auto& backend = g_hwndRegistry[m_hWnd];
    BOOL old = backend.visible;

    if (cmd == SW_HIDE)
        backend.visible = false;
    else
        backend.visible = true;

    UpdateSDLVisibility(backend);
    return old;
}

BOOL CWnd::DestroyWindow() 
{ 
    if (m_hWnd) {
        // RERUN: 1. Notify derived classes (standard MFC behavior)
        OnDestroy();

        WindowBackend* be = backend_from_hwnd(m_hWnd);
        if (be) {
            // RERUN: 2. Recursively destroy child windows first
            std::vector<HWND> children = be->children; // Copy list as it will be modified
            for (HWND hChild : children) {
                CWnd* pChild = CWnd::FromHandle(hChild);
                if (pChild && pChild->m_hWnd) {
                    pChild->DestroyWindow();
                }
            }

            if (be->window && !be->isChild) {
                // RERUN: never destroy the shared SDL window mid-app.
                // Dozens of virtual CWnd backends reference this one OS
                // window — destroying it here orphans them all (the old
                // black-window-keeps-running bug) and its async BadWindow
                // errors fire _XDefaultError -> exit(1) while static
                // destructors race live worker threads. SDL_Quit reaps the
                // window at process exit.
                //
                // A dedicated SDL window (non-child backend) is different:
                // with no other backend referencing it, leaving it mapped
                // keeps the dead dialog's last frame on top of the windows
                // beneath — the BoB "Take-off" window froze the mission's
                // last cockpit frame over the debrief screen. Unmap it
                // instead of destroying, which has none of the async
                // BadWindow/destructor-race hazards above.
                bool still_used = false;
                for (auto& kv : g_hwndRegistry)
                    if (kv.first != m_hWnd &&
                        kv.second.window == be->window) {
                        still_used = true;
                        break;
                    }
                if (!still_used)
                    SDL_HideWindow(be->window);
                if (AfxGetMainWnd() == this) {
                    extern volatile bool g_migVulkanWindowGone;
                    g_migVulkanWindowGone = true;
                }
            }
            g_hwndRegistry.erase(m_hWnd);
        }
        
        // Remove any timers associated with this window
        auto it = std::remove_if(g_timers.begin(), g_timers.end(), 
            [this](const TimerInfo& t) {
                return t.hwnd == m_hWnd;
            });
        if (it != g_timers.end()) {
            g_timers.erase(it, g_timers.end());
        }

        if (g_pCaptureWnd == this)
            ReleaseCapture();
        
        if (g_pFocusWnd == this)
            g_pFocusWnd = nullptr;

        if (AfxGetMainWnd() == this) {
            AfxPostQuitMessage(0);
            AfxGetApp()->m_pMainWnd = nullptr;
        }

        // RERUN: Clean up the handle itself (allocated in Create/allocate_hwnd)
        delete m_hWnd;
        m_hWnd = nullptr;

        // RERUN: 3. Final cleanup (allows self-deletion via 'delete this')
        PostNcDestroy();
    }
    return TRUE; 
}
void CWnd::SetWindowText(const char* text) {
    m_windowText = text ? text : ""; // RERUN: Store text
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (be && be->window && !be->isChild) {
        SDL_SetWindowTitle(be->window, text);
    }
    Invalidate(); // RERUN: Repaint when text changes
}
void CWnd::GetWindowText(char* buf, int max) const {
    if (max > 0) buf[0] = '\0';
    // RERUN: Return stored text
    if (!m_windowText.empty()) {
        snprintf(buf, max, "%s", m_windowText.c_str());
        return;
    }
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (be && be->window && !be->isChild) {
        const char* title = SDL_GetWindowTitle(be->window);
        if (title) snprintf(buf, max, "%s", title);
    }
}
void CWnd::GetWindowText(CString& s) const {
    s = m_windowText.c_str(); // RERUN: Return stored text
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (s.IsEmpty() && be && be->window && !be->isChild) {
        const char* title = SDL_GetWindowTitle(be->window);
        if (title) s = title;
    }
}
BOOL CWnd::ScrollWindow(int dx, int dy, const RECT* prcScroll, const RECT* prcClip) {
    if (dx == 0 && dy == 0) return TRUE;

    // RERUN: Move child windows to simulate scrolling
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (be) {
        // Iterate over a copy of children list since moving might theoretically alter order (though not typically)
        std::vector<HWND> children = be->children;
        for (HWND childHwnd : children) {
            CWnd* pChild = CWnd::FromHandle(childHwnd);
            if (pChild) {
                // Don't scroll the scrollbars themselves!
                int id = pChild->GetDlgCtrlID();
                if (id == 1003 /*IDJ_HORZ_SCROLLBAR*/ || id == 1004 /*IDJ_VERT_SCROLLBAR*/) 
                    continue;

                CRect r;
                pChild->GetWindowRect(&r); // Screen coords
                ScreenToClient(&r);        // Parent client coords
                r.OffsetRect(dx, dy);
                pChild->MoveWindow(r, TRUE);
            }
        }
    }

    Invalidate();
    return TRUE;
}
bool CWnd::CreateControl(const CLSID&, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CFile*, BOOL, BSTR) 
{ 
    // RERUN: Forward to Create to ensure we get an HWND. This handles wrappers that use CreateControl.
    // Use CWnd::Create explicitly to avoid infinite recursion if the derived class overrides Create to call CreateControl.
    return CWnd::Create(NULL, lpszWindowName, dwStyle, rect, pParentWnd, nID);
}
UINT_PTR CWnd::SetTimer(UINT id, UINT elapse, void* callback) 
{ 
    KillTimer(id); // Remove existing timer with same ID
    
    TimerInfo t;
    t.id = id;
    t.interval = elapse;
    t.nextTime = SDL_GetTicks() + elapse;
    t.hwnd = m_hWnd;
    t.callback = callback;
    g_timers.push_back(t);
    
    return id; 
}
void CWnd::KillTimer(UINT id) 
{
    auto it = std::remove_if(g_timers.begin(), g_timers.end(), 
        [this, id](const TimerInfo& t) {
            return t.hwnd == m_hWnd && t.id == id;
        });
    if (it != g_timers.end()) {
        g_timers.erase(it, g_timers.end());
    }
}
void CWnd::GetClientRect(CRect& r)
{
   // Delegate to the const LPRECT version
    GetClientRect(&r);
}
void CWnd::GetClientRect(LPRECT lpRect) const
{
    if (lpRect) {
        // RERUN: If this window is backed by a real SDL window (and is not a child control sharing it),
        // query SDL for the true size. This fixes views/frames reporting 0x0 or stale sizes.
        // Only the backend's owner reports the physical size: a CWnd sharing another
        // window's backend (e.g. a view holding the frame HWND) reports its own m_rect.
        if (m_hWnd) {
             WindowBackend* be = backend_from_hwnd(m_hWnd);
             if (be && be->window && !be->isChild && be->owner == this) {
                 int w, h;
                 SDL_GetWindowSize(be->window, &w, &h);
                 lpRect->left = 0;
                 lpRect->top = 0;
                 lpRect->right = w;
                 lpRect->bottom = h;
                 return;
             }
        }

        lpRect->left = 0;
        lpRect->top = 0;
        lpRect->right = m_rect.Width();
        lpRect->bottom = m_rect.Height();
    }
}
CWnd* CWnd::GetParent() const { return m_pParent; }
void CWnd::SetParent(CWnd* p) { m_pParent = p; }
void CWnd::BringWindowToTop() 
{
    if (!m_hWnd) return;
    WindowBackend* be = backend_from_hwnd(m_hWnd);

    // RERUN: For independent top-level windows, physically raise the window
    if (be && be->window && !be->isChild && be->owner == this) {
        SDL_RaiseWindow(be->window);
    }

    if (be && be->parent && be->parent->m_hWnd) {
        WindowBackend* parentBe = backend_from_hwnd(be->parent->m_hWnd);
        if (parentBe) {
            auto& children = parentBe->children;
            auto it = std::find(children.begin(), children.end(), m_hWnd);
            if (it != children.end()) {
                children.erase(it);
                children.push_back(m_hWnd);
                backend_mark_dirty(be->parent->m_hWnd, nullptr);
            }
        }
    }
}
void CWnd::Invalidate() { InvalidateRect(NULL, FALSE); }
BOOL CWnd::PreTranslateMessage(MSG* /*pMsg*/) { return FALSE; }
BOOL CWnd::OnCmdMsg(UINT /*nID*/, int /*nCode*/, void* /*pExtra*/, AFX_CMDHANDLERINFO* /*pHandlerInfo*/) { return FALSE; }
int CWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    return 0;
};
BOOL CWnd::PreCreateWindow(CREATESTRUCT& cs) 
{ 
    return TRUE; 
}
void CWnd::GetWindowRect(LPRECT lpRect) const
{
    if (!lpRect) return;
    if (m_hWnd) {
        WindowBackend* be = backend_from_hwnd(m_hWnd);
        if (be && be->window && !be->isChild && be->owner == this) {
			// This is a top-level window, get rect directly from SDL.
            int x, y, w, h;
            SDL_GetWindowPosition(be->window, &x, &y);
            SDL_GetWindowSize(be->window, &w, &h);
            lpRect->left = x;
            lpRect->top = y;
            lpRect->right = x + w;
            lpRect->bottom = y + h;
            return;
        }
    }
	// For child windows, get client rect and convert to screen coordinates.
    GetClientRect(lpRect);
    ClientToScreen(lpRect);
}
void CWnd::MoveWindow(int x, int y, int w, int h)
{
    SetWindowPos(NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}
void CWnd::MoveWindow(const CRect& r, BOOL repaint)
{
    SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), (repaint ? 0 : SWP_NOREDRAW) | SWP_NOZORDER | SWP_NOACTIVATE);
}
void CWnd::ScreenToClient(POINT* pt) const
{
    if (!pt) return;
    pt->x -= m_rect.left; // Correct: ScreenToClient SUBTRACTs origin
    pt->y -= m_rect.top;

    if (m_hWnd) { // Recurse to parent view/frame
        WindowBackend* be = backend_from_hwnd(m_hWnd);
        if (be && be->isChild && be->parent)
            be->parent->ScreenToClient(pt);
    }
}
void CWnd::ScreenToClient(LPRECT r) const
{
    if (!r) return;
    ScreenToClient((POINT*)&r->left);
    ScreenToClient((POINT*)&r->right);
}
void CWnd::ClientToScreen(LPRECT r) const
{
    if (!r) return;
    ClientToScreen((POINT*)&r->left);
    ClientToScreen((POINT*)&r->right);
}
void CWnd::ClientToScreen(POINT* pt) const
{
    if (!pt) return;
    pt->x += m_rect.left; // Correct: ClientToScreen ADDs origin
    pt->y += m_rect.top;

    if (m_hWnd) { // Recurse to parent view/frame
        WindowBackend* be = backend_from_hwnd(m_hWnd);
        if (be && be->isChild && be->parent)
            be->parent->ClientToScreen(pt);
    }
}
LRESULT CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    return WindowProc(message, wParam, lParam);
}
LRESULT CWnd::SendMessageToDescendants(UINT, WPARAM, LPARAM) { return 0; }
CWnd* CWnd::GetTopWindow() const 
{ 
    if (!m_hWnd) return nullptr;
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (be && !be->children.empty()) {
        // In Win32, GetTopWindow gets the first child in the Z-order.
        // We maintain children in Z-order (last is top), so return the last one.
        return CWnd::FromHandle(be->children.back());
    }
    return nullptr; 
}
CWnd* CWnd::GetNextWindow(unsigned int nDirection) const 
{ 
    if (!m_hWnd) return nullptr;
    CWnd* parent = GetParent();
    if (!parent || !parent->m_hWnd) return nullptr;

    WindowBackend* parentBe = backend_from_hwnd(parent->m_hWnd);
    if (!parentBe) return nullptr;

    auto& children = parentBe->children;
    auto it = std::find(children.begin(), children.end(), m_hWnd);
    if (it == children.end()) return nullptr;

    // Children are stored bottom->top in Z-order (back() = topmost).
    // GW_HWNDNEXT steps DOWN the Z-order (toward begin), GW_HWNDPREV up (toward end).
    if (nDirection == GW_HWNDNEXT) {
        if (it != children.begin())
            return CWnd::FromHandle(*(--it));
    } else if (nDirection == GW_HWNDPREV) {
        if (++it != children.end())
            return CWnd::FromHandle(*it);
    }
    return nullptr; 
}

BOOL CWnd::SubclassWindow(HWND hWnd)
{
    if (m_hWnd != NULL || hWnd == NULL)
        return FALSE; // Already attached or invalid HWND

    WindowBackend* be = backend_from_hwnd(hWnd);
    if (be)
    {
        // The placeholder CWnd/DlgItem object is now orphaned. This is a known
        // simplification in this stub to avoid complex memory management.
        if (be->owner)
        {
            m_rect = be->owner->m_rect; // RERUN FIX: Copy rect from placeholder
            m_dwStyle = be->owner->GetStyle(); // RERUN FIX: Copy style from placeholder
            m_windowText = be->owner->m_windowText; // RERUN FIX: Preserve text from placeholder (e.g. from RC file)

            // RERUN: Transfer specific Rowan control properties from placeholder
            if (IsKindOf(RUNTIME_CLASS(CRStatic)) && be->owner->IsKindOf(RUNTIME_CLASS(CRStatic))) {
                ((CRStatic*)this)->SetFontNum(((CRStatic*)be->owner)->GetFontNum());
                ((CRStatic*)this)->SetResourceNumber(((CRStatic*)be->owner)->GetResourceNumber());
            }

            be->owner->m_hWnd = nullptr; // Prevent placeholder from destroying window
        }

        // Wire up the new control object (e.g., RButton) to the HWND
        be->owner = this;
        m_hWnd = hWnd;
        m_pParent = be->parent; // RERUN FIX: Restore parent pointer from backend

        // RERUN FIX: Restore ID from the backend (template ID) so message routing works
        if (m_nID == 0 && be->templateID != 0) {
            m_nID = be->templateID;
            // std::cout << "[MFC_stub] SubclassWindow: Restored m_nID=" << m_nID << " from backend." << std::endl;
        }

        // RERUN FIX: Update parent's child map to point to this new object instead of the placeholder
        if (m_pParent && m_nID != 0) {
             m_pParent->AddChild(this, m_nID);
             // std::cout << "[MFC_stub] SubclassWindow: Updated parent's child map for ID " << m_nID << " to object " << this << " HWND=" << hWnd << std::endl;
        }
        PreSubclassWindow();

        return TRUE;
    }
    return FALSE;
}

int CWnd::GetDlgCtrlID() const { return m_nID; }
LRESULT CWnd::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
    if (lParam != 0)
        WinHelp((DWORD)lParam, HELP_CONTEXT);
    else if (wParam != 0)
        WinHelp((DWORD)wParam, HELP_CONTEXT);
    else
        WinHelp(0, HELP_FINDER);
    return TRUE;
}
void CWnd::GetProperty(DISPID dispid, unsigned short vt, void* pvResult) {}
void CWnd::SetProperty(DISPID dispid, unsigned short vt, long value) {}
void CWnd::SetProperty(DISPID dispid, unsigned short vt, const void* pvValue) {}
CWnd* CWnd::SetCapture() 
{ 
    CWnd* pOld = g_pCaptureWnd;
    g_pCaptureWnd = this;
    SDL_CaptureMouse(SDL_TRUE);
    return pOld; 
}
void CWnd::ReleaseCapture() 
{
    g_pCaptureWnd = nullptr;
    SDL_CaptureMouse(SDL_FALSE);
}
void CWnd::PostNcDestroy() {}
void CWnd::OnGetMinMaxInfo(MINMAXINFO* /*lpMMI*/) {}
void CWnd::MapDialogRect(LPRECT lpRect) const 
{
    if (!lpRect) return;
    
    // In this stub, we use a fixed scaling factor consistent with CDialog::Create.
    lpRect->left   *= g_dluX;
    lpRect->right  *= g_dluX;
    lpRect->top    *= g_dluY;
    lpRect->bottom *= g_dluY;
}
CDC* CWnd::BeginPaint(PAINTSTRUCT* /*ps*/)
{
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    // Win32 BeginPaint validates the update region — the window stops
    // being dirty here, not in EndPaint.
    // Exception: while DirectDraw presents own the window, EndPaint and
    // the WM_PAINT present path both refuse to present — clearing the
    // dirty marks here would drop the repaint on the floor with nothing
    // left to retry it (BoB froze on the last 3D frame after mission
    // exit: the debrief's first paint landed inside the 250ms DD-active
    // window and was silently consumed).  Keep the marks so the dirty
    // scan re-synthesizes WM_PAINT once presents stop.
    if (be && !DDPrimaryActive()) {
        be->needsRepaint = false;
        be->fullDirty = false;
        be->dirtyRegions.clear();
    }
    // If this window owns its own renderer, clear it to start a new frame
    if (be && be->renderer && !be->isChild && !DDPrimaryActive()) {
        SDL_SetRenderDrawColor(be->renderer, 192, 192, 192, 255); // Standard dialog gray
        SDL_RenderClear(be->renderer);
    }

    static CDC dummyDC;
    dummyDC = CDC(this);           // Re-initialize with current window's renderer
    dummyDC.m_hDC = (HDC)&dummyDC; // Ensure HDC handle points to the static instance, not the temp
    return &dummyDC;
}

// Global ::BeginPaint/::EndPaint — delegate to the window's member
// versions so BoB code using the raw Win32 calls gets the same DC and
// the same paint/present semantics.
CDC* BeginPaint(void* hwnd, PAINTSTRUCT* ps)
{
    WindowBackend* be = backend_from_hwnd((HWND)hwnd);
    if (be && be->owner) return be->owner->BeginPaint(ps);
    static CDC dummy;
    return &dummy;
}

void EndPaint(void* hwnd, PAINTSTRUCT* ps)
{
    WindowBackend* be = backend_from_hwnd((HWND)hwnd);
    if (be && be->owner) be->owner->EndPaint(ps);
}

void CWnd::EndPaint(PAINTSTRUCT* /*ps*/) 
{
    if (!m_hWnd) return;
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    
    // Only the root window of an SDL window hierarchy should finish composition and present.
    if (be && be->renderer && !be->isChild && !DDPrimaryActive())
    {
        // Compose the entire child window tree onto the root renderer
        PaintChildrenRecursive(m_hWnd, be->renderer, 0, 0, 0);

        // Finish rendering and swap buffers
        SDL_RenderPresent(be->renderer);

        // Reset the dirty state for this window hierarchy
        be->needsRepaint = false;
        be->fullDirty = false;
        be->dirtyRegions.clear();
    }
}

void CWnd::OnPaint()
{
    // Default: do nothing
    // std::cout << "CWnd::OnPaint() called" << std::endl;
}

LRESULT CWnd::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Message map dispatch
    for (const AFX_MSGMAP* pMap = GetMessageMap(); pMap != nullptr; pMap = (*pMap->pfnGetBaseMap)())
    {
        if (!pMap->lpEntries) continue;

        for (const AFX_MSGMAP_ENTRY* pEntry = pMap->lpEntries; pEntry->pfn != 0; pEntry++)
        {
            // Check for a match
            if (pEntry->nMessage == msg && pEntry->nID <= pEntry->nLastID)
            {
                // Command messages have a specific ID
                if (msg == WM_COMMAND) {
                    if (pEntry->nID == LOWORD(wParam)) {
                        if (pEntry->nCode == HIWORD(wParam)) {
                            (this->*pEntry->pfn)();
                            return 0; // Handled
                        }
                    }
                } else if (pEntry->nCode == 0) { // It's a window message
                    union {
                        AFX_PMSG pfn;
                        int(CWnd::*pfn_i_CREATE)(LPCREATESTRUCT);
                        void(CWnd::*pfn_v_PAINT)();
                        void(CWnd::*pfn_v_SIZE)(UINT, int, int);
                        void(CWnd::*pfn_v_MOVE)(int, int);
                        void(CWnd::*pfn_v_CLOSE)();
                        BOOL(CWnd::*pfn_B_HELPINFO)(HELPINFO*);
                        BOOL(CWnd::*pfn_B_ERASEBKGND)(CDC*);
                        void(CWnd::*pfn_v_HSCROLL)(UINT, UINT, CScrollBar*);
                        void(CWnd::*pfn_v_VSCROLL)(UINT, UINT, CScrollBar*);
                        void(CWnd::*pfn_v_GETMINMAXINFO)(MINMAXINFO*);
                        void(CWnd::*pfn_v_ACTIVATE)(UINT, CWnd*, BOOL);
                        void(CWnd::*pfn_v_ACTIVATEAPP)(BOOL, DWORD);
                        LRESULT(CWnd::*pfn_L_COMMANDHELP)(WPARAM, LPARAM);
                        void(CWnd::*pfn_v_ENABLE)(BOOL);
                        void(CWnd::*pfn_v_TIMER)(UINT);
                        void(CWnd::*pfn_v_LBUTTONDOWN)(UINT, CPoint);
                        void(CWnd::*pfn_v_LBUTTONUP)(UINT, CPoint);
                        void(CWnd::*pfn_v_MOUSEMOVE)(UINT, CPoint);
                        void(CWnd::*pfn_v_CONTEXTMENU)(CWnd*, CPoint);
                        BOOL(CWnd::*pfn_B_MOUSEWHEEL)(UINT, short, CPoint);
                        void(CWnd::*pfn_v_SHOWWINDOW)(BOOL, UINT);
                        BOOL(CWnd::*pfn_B_SETCURSOR)(CWnd*, UINT, UINT);
                        void(CWnd::*pfn_v_NCMOUSEMOVE)(UINT, CPoint);
                        void(CWnd::*pfn_v_NCLBUTTONDOWN)(UINT, CPoint);
                        void(CWnd::*pfn_v_NCLBUTTONUP)(UINT, CPoint);
                        void(CWnd::*pfn_v_KILLFOCUS)(CWnd*);
                        void(CWnd::*pfn_v_CANCELMODE)();
                        void(CWnd::*pfn_v_INITMENU)(CMenu*);
                        void(CWnd::*pfn_v_CAPTURECHANGED)(CWnd*);
                        void(CWnd::*pfn_v_CHAR)(UINT, UINT, UINT);
                        void(CWnd::*pfn_v_RBUTTONDOWN)(UINT, CPoint);
                        void(CWnd::*pfn_v_RBUTTONUP)(UINT, CPoint);
                        int(CWnd::*pfn_i_CHARTOITEM)(UINT, CListBox*, UINT);
                    } u;
                    u.pfn = pEntry->pfn;

                    switch (msg) {
                        case WM_PAINT:          (this->*u.pfn_v_PAINT)(); return 0;
                        case WM_CREATE:         return (this->*u.pfn_i_CREATE)((LPCREATESTRUCT)lParam);
                        case WM_SIZE:           (this->*u.pfn_v_SIZE)((UINT)wParam, LOWORD(lParam), HIWORD(lParam)); return 0;
                        case WM_MOVE:           (this->*u.pfn_v_MOVE)(LOWORD(lParam), HIWORD(lParam)); return 0;
                        case WM_CLOSE:          (this->*u.pfn_v_CLOSE)(); return 0;
                        case WM_HELP:           return (this->*u.pfn_B_HELPINFO)((HELPINFO*)lParam);
                        case WM_DESTROY:        (this->*u.pfn_v_CLOSE)(); return 0; // OnDestroy signature matches OnClose (void/void)
                        case WM_ERASEBKGND:     return (this->*u.pfn_B_ERASEBKGND)((CDC*)wParam);
                        case WM_HSCROLL:        (this->*u.pfn_v_HSCROLL)((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (CScrollBar*)lParam); return 0;
                        case WM_VSCROLL:        (this->*u.pfn_v_VSCROLL)((UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (CScrollBar*)lParam); return 0;
                        case WM_GETMINMAXINFO:  (this->*u.pfn_v_GETMINMAXINFO)((MINMAXINFO*)lParam); return 0;
                        case WM_ACTIVATE:       (this->*u.pfn_v_ACTIVATE)(LOWORD(wParam), CWnd::FromHandle((HWND)lParam), HIWORD(wParam)); return 0;
                        case WM_ACTIVATEAPP:    (this->*u.pfn_v_ACTIVATEAPP)((BOOL)wParam, (DWORD)lParam); return 0;
                        case WM_ENABLE:         (this->*u.pfn_v_ENABLE)((BOOL)wParam); return 0;
                        case WM_COMMANDHELP:    return (this->*u.pfn_L_COMMANDHELP)(wParam, lParam);
                        case WM_TIMER:          (this->*u.pfn_v_TIMER)((UINT)wParam); return 0;
                        case WM_LBUTTONDOWN:    (this->*u.pfn_v_LBUTTONDOWN)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_LBUTTONUP:      (this->*u.pfn_v_LBUTTONUP)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_MOUSEMOVE:      (this->*u.pfn_v_MOUSEMOVE)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_CONTEXTMENU:    (this->*u.pfn_v_CONTEXTMENU)(CWnd::FromHandle((HWND)wParam), CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_MOUSEWHEEL:     return (this->*u.pfn_B_MOUSEWHEEL)(LOWORD(wParam), (short)HIWORD(wParam), CPoint(LOWORD(lParam), HIWORD(lParam)));
                        case WM_SHOWWINDOW:     (this->*u.pfn_v_SHOWWINDOW)((BOOL)wParam, (UINT)lParam); return 0;
                        case WM_SETCURSOR:      return (this->*u.pfn_B_SETCURSOR)(CWnd::FromHandle((HWND)wParam), LOWORD(lParam), HIWORD(lParam));
                        case WM_NCMOUSEMOVE:    (this->*u.pfn_v_NCMOUSEMOVE)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_NCLBUTTONDOWN:  (this->*u.pfn_v_NCLBUTTONDOWN)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_NCLBUTTONUP:    (this->*u.pfn_v_NCLBUTTONUP)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_KILLFOCUS:      (this->*u.pfn_v_KILLFOCUS)(CWnd::FromHandle((HWND)wParam)); return 0;
                        case WM_CANCELMODE:     (this->*u.pfn_v_CANCELMODE)(); return 0; // RERUN FIX: Route to OnCancelMode, not OnClose
                        case WM_INITMENU:       { CMenu menu; (this->*u.pfn_v_INITMENU)(&menu); return 0; }
                        case WM_CAPTURECHANGED: (this->*u.pfn_v_CAPTURECHANGED)(CWnd::FromHandle((HWND)lParam)); return 0;
                        case WM_CHAR:           (this->*u.pfn_v_CHAR)((UINT)wParam, LOWORD(lParam), HIWORD(lParam)); return 0;
                        case WM_RBUTTONDOWN:    (this->*u.pfn_v_RBUTTONDOWN)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_RBUTTONUP:      (this->*u.pfn_v_RBUTTONUP)((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam))); return 0;
                        case WM_CHARTOITEM:     return (this->*u.pfn_i_CHARTOITEM)((UINT)LOWORD(wParam), (CListBox*)CWnd::FromHandle((HWND)lParam), (UINT)HIWORD(wParam));
                        // Default to generic ON_MESSAGE handler
                        default: return (this->*u.pfn_L_COMMANDHELP)(wParam, lParam);
                    }
                }
            }
        }
    }

    // If no handler was found, call the default implementation
    return DefWindowProc(msg, wParam, lParam);
}
void CWnd::ModifyStyle(DWORD remove, DWORD add) 
{ 
    m_dwStyle = (m_dwStyle & ~remove) | add; 
}
BOOL CWnd::SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    // RERUN: For independent top-level windows, synchronize with the physical SDL window.
    // Only the backend's owner may move/resize it: a CWnd sharing another window's
    // backend (e.g. a view holding the frame HWND) is a logical child and must only
    // update m_rect, never touch the OS window.
    if (cx > 0 || cy > 0) {
        if (be && be->window && !be->isChild && be->owner == this) {
            if (!(nFlags & SWP_NOMOVE)) {
                SDL_SetWindowPosition(be->window, x, y);
            }
            if (!(nFlags & SWP_NOSIZE)) {
                SDL_SetWindowSize(be->window, cx, cy);
            }
        }
    
        if (!(nFlags & SWP_NOMOVE)) {
            m_rect.left   = x;
            m_rect.top    = y;
            SendMessage(WM_MOVE, 0, MAKELPARAM(x, y));
        }
        if (!(nFlags & SWP_NOSIZE)) {
            m_rect.right  = m_rect.left + cx;
            m_rect.bottom = m_rect.top + cy;
            SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(cx, cy));
        }
    }
    else
    {
        std::cout << "[MFC_stub] WARNING WARNING SetWindowPos called with zero size, ignoring size change." << std::endl;
    }

    if (be && (nFlags & (SWP_SHOWWINDOW | SWP_HIDEWINDOW))) {
        be->visible = (nFlags & SWP_SHOWWINDOW) != 0;
        UpdateSDLVisibility(*be);
    }

    if (!(nFlags & SWP_NOZORDER)) {
        if (pWndInsertAfter == &wndTop) {
            BringWindowToTop();
        }
        else if (pWndInsertAfter == &wndBottom) {
            if (be && be->window && !be->isChild && be->owner == this) {
                // To move to bottom in SDL, we raise all other top-level windows
                for (auto& pair : g_hwndRegistry) {
                    WindowBackend& otherBe = pair.second;
                    if (otherBe.window && !otherBe.isChild && otherBe.window != be->window) {
                        SDL_RaiseWindow(otherBe.window);
                    }
                }
            }
        }
    }
    
    backend_mark_dirty(m_hWnd, nullptr);
    return TRUE;
}
void CWnd::SetMenu(HMENU /*hMenu*/) {}
CDC* CWnd::GetDC()
{
    return new CDC(this);
}
void CWnd::ReleaseDC(CDC* pDC) { delete pDC; }
void CWnd::UpdateWindow() 
{
    if (!m_hWnd) return;
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (!be) return;

    // If child, delegate to parent (since parent owns the renderer/presentation)
    if (be->isChild && be->parent) {
        be->parent->UpdateWindow();
        return;
    }

    // RERUN: If this CWnd is not the owner of the backend (e.g. it's a copy or view sharing HWND),
    // delegate to the owner.
    if (be->owner && be->owner != this) {
        be->owner->UpdateWindow();
        return;
    }

    if (be->needsRepaint) {
        SendMessage(WM_PAINT, 0, 0);
    }
}
void CWnd::DragAcceptFiles() {}
BOOL CWnd::PostMessage(UINT msg, WPARAM wParam, LPARAM lParam) 
{
    if (!m_hWnd) return FALSE;

    MSG message;
    message.hwnd = m_hWnd;
    message.message = msg;
    message.wParam = wParam;
    message.lParam = lParam;
    message.time = SDL_GetTicks();
    message.pt_x = 0; // Not available here
    message.pt_y = 0;

    g_msgQueue.push_back(message);

    return TRUE;
}
BOOL CWnd::PostMessage(UINT msg) {return PostMessage(msg, 0, 0);}
void CWnd::OnKillFocus(CWnd* pNewWnd) {}
BOOL CWnd::IsZoomed() const { return FALSE; }
void CWnd::OnCancelMode() {}
void CWnd::OnInitMenu(CMenu* pMenu) {}
LRESULT CWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if (message == WM_MOUSEWHEEL)
    {
        if (m_pParent)
            return m_pParent->SendMessage(message, wParam, lParam);
    }

    switch (message)
    {
    case WM_LBUTTONDOWN:
        OnLButtonDown((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam)));
        break;
    case WM_LBUTTONUP:
        OnLButtonUp((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam)));
        break;
    case WM_MOUSEMOVE:
        OnMouseMove((UINT)wParam, CPoint(LOWORD(lParam), HIWORD(lParam)));
        break;
    }
 
    return 0;
}
CScrollBar* CWnd::GetScrollBarCtrl(int nBar) const {return nullptr;}
BOOL CWnd::OnAmbientProperty(COleControlSite* pSite, DISPID dispid, VARIANT* pvar) {return FALSE;}
BOOL CWnd::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) { return FALSE; }
BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam) { return FALSE; }
void CWnd::OnFinalRelease() {}
BOOL CWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) { return FALSE; }
void CWnd::OnSetFont(CFont* pFont) {
    m_pFont = pFont;
    Invalidate();
}
void CWnd::PreSubclassWindow() {}
void CWnd::WinHelp(DWORD dwData, UINT nCmd)
{
    // Resolve the help file path (absolute path recommended)
    const char* helpFile = "English/text/mig.hlp";

    // Build argument list based on nCmd
    std::vector<const char*> args;
    args.push_back("wine");
    args.push_back("winhelp.exe");
    std::string ctxStr, idStr; // keep alive until execvp below

    switch (nCmd)
    {
        case HELP_CONTEXT:
        case HELP_CONTEXTPOPUP:
        {
            // /c <contextID>
            args.push_back(helpFile);

            std::ostringstream ctx;
            ctx << "/c";
            ctxStr = ctx.str();
            args.push_back(ctxStr.c_str());

            std::ostringstream id;
            id << dwData;
            idStr = id.str();
            args.push_back(idStr.c_str());
            break;
        }

        case HELP_FINDER:
        case HELP_CONTENTS:
        {
            args.push_back(helpFile);
            break;
        }

        case HELP_QUIT:
        {
            args.push_back("/x");
            break;
        }

        default:
        {
            // Fallback: open contents
            args.push_back(helpFile);
            break;
        }
    }

    args.push_back(nullptr);

    // Fork + exec to avoid blocking
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child
        execvp("wine", const_cast<char* const*>(args.data()));
        _exit(1); // Only reached if exec fails
    }
}
void CWnd::OnCaptureChanged(CWnd* pWnd) {}
void CWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {}
int CWnd::OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex) { return -1; }
void CWnd::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags) { m_dwExStyle &= ~dwRemove; m_dwExStyle |= dwAdd; }
void CWnd::SetLimitText(int) {}
void CWnd::SetReadOnly(BOOL) {}
void CWnd::OnClose() { DestroyWindow(); }
void CWnd::OnDestroy()
{
    // Remove from parent's logical child map (MFC structure)
    if (m_pParent && m_nID != 0) {
        auto it = m_pParent->m_children.find(m_nID);
        if (it != m_pParent->m_children.end() && it->second == this) {
            m_pParent->m_children.erase(it);
        }
    }

    // Remove from parent's backend children list (SDL/Windowing linkage)
    if (m_pParent && m_pParent->m_hWnd) {
        WindowBackend* parentBe = backend_from_hwnd(m_pParent->m_hWnd);
        if (parentBe) {
            auto& kids = parentBe->children;
            auto it = std::find(kids.begin(), kids.end(), m_hWnd);
            if (it != kids.end()) {
                kids.erase(it);
                // Mark parent dirty since a child was removed/uncovered
                backend_mark_dirty(m_pParent->m_hWnd, nullptr);
            }
        }
    }
}
void CWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
    SetCapture();

    // RERUN: Check for scrollbar hits on generic CWnds
    if (GetStyle() & WS_VSCROLL) {
        CRect r;
        GetClientRect(r);
        if (point.x >= r.right - 16) {
            SCROLLINFO& si = m_vertScrollInfo;
            int range = si.nMax - si.nMin + 1;
            int pageSize = (si.nPage > 0) ? si.nPage : 1;
            int h = r.Height();
            int thumbHeight = std::max(20, (h * pageSize) / range);
            int trackSpace = h - thumbHeight;
            int thumbY = (trackSpace * (si.nPos - si.nMin)) / (range - pageSize);

            if (point.y < thumbY)
                SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
            else if (point.y > thumbY + thumbHeight)
                SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
            
            return; 
        }
    }
}
void CWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
    ReleaseCapture();
}

int CWnd::SetScrollPos(int nBar, int nPos, BOOL bRedraw) { return 0; }
int CWnd::GetScrollPos(int nBar) const { return 0; }

int CWnd::SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw)
{
    if (nBar == SB_VERT && lpScrollInfo) {
        m_vertScrollInfo = *lpScrollInfo;
        if (bRedraw) Invalidate();
        return m_vertScrollInfo.nPos;
    }
    return 0;
}

BOOL CWnd::GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask)
{
    if (nBar == SB_VERT && lpScrollInfo) {
        *lpScrollInfo = m_vertScrollInfo;
        return TRUE;
    }
    return FALSE;
}

// CListBox
CWnd* CWnd::GetEventParent()
{
    return m_pParent;
}

void CWnd::FireEvent(int eventID, long l1, long l2, LPCTSTR text, int index)
{
    CWnd* pParent = GetEventParent();
    if (!pParent) return;

    const AFX_EVENTSINKMAP* pMap = pParent->GetSinkMap();
    if (!pMap) return;

    for (; pMap->idFirst != 0; ++pMap) {
        if (m_nID >= pMap->idFirst && m_nID <= pMap->idLast && pMap->eventid == eventID) {
            // Found the handler. Dispatch based on param string from ON_EVENT macro.
            if (strcmp(pMap->params, VTS_NONE) == 0) {
                typedef void (CCmdTarget::*Func)();
                Func f = (Func)pMap->pfn;
                (pParent->*f)();
            } else if (strcmp(pMap->params, VTS_BSTR) == 0) {
                typedef void (CCmdTarget::*Func)(LPCTSTR);
                Func f = (Func)pMap->pfn;
                (pParent->*f)(text);
            } else if (strcmp(pMap->params, VTS_I4) == 0) {
                typedef void (CCmdTarget::*Func)(long);
                Func f = (Func)pMap->pfn;
                (pParent->*f)(l1);
            } else if (strcmp(pMap->params, VTS_I4 VTS_I4) == 0) {
                typedef void (CCmdTarget::*Func)(long, long);
                Func f = (Func)pMap->pfn;
                (pParent->*f)(l1, l2);
            } else if (strcmp(pMap->params, VTS_BSTR VTS_I2) == 0) {
                typedef void (CCmdTarget::*Func)(LPCTSTR, short);
                Func f = (Func)pMap->pfn;
                (pParent->*f)(text, (short)index);
            } else if (strcmp(pMap->params, VTS_I4 VTS_BSTR VTS_I2) == 0) {
                typedef void (CCmdTarget::*Func)(long, LPCTSTR, short);
                Func f = (Func)pMap->pfn;
                (pParent->*f)(l1, text, (short)index);
            } else {
                std::cout << "[FireEvent] unhandled params '" << pMap->params
                          << "' for ctrl " << m_nID << " event " << eventID << std::endl;
            }
            break; // Assume one handler per event
        }
    }
}

int CListBox::AddString(LPCTSTR lpszItem) {
    if (m_pComboOwner) {
        std::cout << "[CListBox] Populating Dropdown Item: " << (lpszItem ? lpszItem : "NULL") << std::endl;
    }
    m_items.push_back(lpszItem);
    return m_items.size() - 1;
}
int CListBox::InsertString(int nIndex, LPCTSTR lpszItem) {
    if (nIndex < 0) nIndex = 0;
    if (nIndex > (int)m_items.size()) nIndex = m_items.size();
    m_items.insert(m_items.begin() + nIndex, lpszItem);
    return nIndex;
}
void CListBox::ResetContent() {
    m_items.clear();
    m_curSel = -1;
    m_topIndex = 0;
}
int CListBox::GetCount() const { return m_items.size(); }
int CListBox::GetCurSel() const { return m_curSel; }
int CListBox::SetCurSel(int nSelect) {
    if (nSelect >= -1 && nSelect < (int)m_items.size()) {
        m_curSel = nSelect;
        // RERUN: Scroll to show selected item at the top (clamping will handle bounds in OnPaint)
        if (m_curSel != -1) m_topIndex = m_curSel;
        return m_curSel;
    }
    return LB_ERR;
}
int CListBox::GetText(int nIndex, LPTSTR lpszBuffer) const {
    if (nIndex >= 0 && nIndex < (int)m_items.size()) {
        strcpy(lpszBuffer, m_items[nIndex].c_str());
        return m_items[nIndex].length();
    }
    return LB_ERR;
}
int CListBox::GetTextLen(int nIndex) const {
    if (nIndex >= 0 && nIndex < (int)m_items.size()) {
        return m_items[nIndex].length();
    }
    return LB_ERR;
}
int CListBox::DeleteString(int nIndex) {
    if (nIndex >= 0 && nIndex < (int)m_items.size()) {
        m_items.erase(m_items.begin() + nIndex);
        if (m_curSel >= nIndex) m_curSel--;
        return m_items.size();
    }
    return LB_ERR;
}

void CListBox::OnClose()
{
    if (m_pComboOwner) {
        // We are a dropdown, so just hide instead of destroying.
        ShowWindow(SW_HIDE);
        if (g_pCaptureWnd == this) { // Check if we still have capture
            ReleaseCapture();
        }
    } else {
        // Normal listbox, default behavior.
        CWnd::OnClose();
    }
}

void CListBox::OnKillFocus(CWnd* pNewWnd)
{
    if (m_pComboOwner) {
        // If we are a dropdown and we've lost focus, hide.
        ShowWindow(SW_HIDE);
    }
    CWnd::OnKillFocus(pNewWnd);
}

void CListBox::OnSetFont(CFont* pFont)
{
    CWnd::OnSetFont(pFont);
    m_itemHeight = -1; // RERUN: Invalidate cache when font changes
}

int CListBox::GetItemHeight() const
{
    if (m_itemHeight > 0) return m_itemHeight; // RERUN: Return cached value if valid

    // RERUN: Centralized item height calculation, but without dependency on RListBoxData.
    CDC dc(const_cast<CListBox*>(this));
    
    CFont* pFont = m_pFont; // Check explicitly set font first
    CWnd* pParent = GetParent();
    
    if (!pFont) {
        int fontNum = 2; // Default font

        if (m_pComboOwner) {
            // If this is a dropdown for a CRCombo, get the font from the owner.
            fontNum = m_pComboOwner->GetFontNum();
        }

        if (pParent) {
            pFont = (CFont*)pParent->SendMessage(WM_GETGLOBALFONT, fontNum);
        }
    }

    CFont* pOldFont = nullptr;
    if (pFont) {
        pOldFont = dc.SelectObject(pFont);
    }

    CSize sz = dc.GetTextExtent("Wg");
    int height = 20; // Fallback
	if (sz.cy > 0) {
        height = sz.cy + 4; // Add some padding
    }

    if (pOldFont) {
        dc.SelectObject(pOldFont);
    }
    if (height < 24) height = 24; // RERUN: Enforce minimum row height to match CRListBox
    m_itemHeight = height; // RERUN: Cache the result
    return m_itemHeight;
}

int CListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const {
    for (int i = nStartAfter + 1; i < (int)m_items.size(); ++i) {
        if (strncasecmp(m_items[i].c_str(), lpszItem, strlen(lpszItem)) == 0) return i;
    }
    return LB_ERR;
}
int CListBox::FindStringExact(int nStartAfter, LPCTSTR lpszItem) const {
    for (int i = nStartAfter + 1; i < (int)m_items.size(); ++i) {
        if (strcasecmp(m_items[i].c_str(), lpszItem) == 0) return i;
    }
    return LB_ERR;
}
int CListBox::OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex) { return -1; }
void CListBox::OnLButtonDown(UINT nFlags, CPoint point) {
    // RERUN: Do not call base CWnd::OnLButtonDown, as it calls SetCapture(),
    // which we want to avoid for this new two-click behavior.

    int itemHeight = GetItemHeight();
    CRect rect;
    GetClientRect(&rect);
    int visibleCount = rect.Height() / itemHeight;
    bool hasScrollbar = (int)m_items.size() > visibleCount;
    int scrollWidth = 16;

    if (hasScrollbar && point.x >= rect.right - scrollWidth) {
        // Handle scrollbar click (simple absolute positioning)
        int maxTop = std::max(0, (int)m_items.size() - visibleCount);
        int trackHeight = rect.Height();
        int thumbHeight = std::max(20, (trackHeight * visibleCount) / (int)m_items.size());
        int maxThumbY = trackHeight - thumbHeight;
        
        int clickY = point.y - (thumbHeight / 2); // Center thumb on click
        if (clickY < 0) clickY = 0;
        if (clickY > maxThumbY) clickY = maxThumbY;
        
        if (maxThumbY > 0) {
            m_topIndex = (clickY * maxTop) / maxThumbY;
            Invalidate();
        }
        return;
    }

    int index = (point.y / itemHeight) + m_topIndex; // RERUN: Account for scroll
    if (index >= 0 && index < (int)m_items.size()) {
        m_curSel = index;
        if (m_pComboOwner) { // This is a dropdown list for a CRCombo
            // RERUN: Selection change via UI interaction. 
            // Capture the string first to prevent Use-After-Free if the handler clears the list.
            std::string selectedText = m_items[m_curSel];
            
            m_pComboOwner->SetCurSel(m_curSel);

            // Fire the TextChanged event (1) and send selection change notification.
            // l1 carries the control id so ON_EVENT_RANGE (VTS_I4 VTS_BSTR VTS_I2)
            // handlers learn which combo fired; plain VTS_BSTR VTS_I2 handlers
            // ignore it and take (text, index).
            m_pComboOwner->FireEvent(1, (long)m_pComboOwner->GetDlgCtrlID(), 0,
                                     selectedText.c_str(), (int)m_curSel);
            if (m_pComboOwner->GetParent()) {
                m_pComboOwner->GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_pComboOwner->GetDlgCtrlID(), 1 /*CBN_SELCHANGE*/), (LPARAM)m_pComboOwner->m_hWnd);
            }
            
            m_hotSel = -1; // Reset hover highlight
            ShowWindow(SW_HIDE);
            ReleaseCapture(); // RERUN: Release mouse capture after selection.
        } else { // This is a normal standalone listbox
            Invalidate(); // Redraw to show new selection highlight
            if (GetParent()) {
                GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
            }
        }
    }
    // RERUN: If the click is outside any valid item but still within the listbox,
    // or if the click is on an item but the listbox is not part of a combo,
    // we need to handle capture release correctly.
    // For a combo dropdown, any click should close it.
    else if (m_pComboOwner) {
        ShowWindow(SW_HIDE);
        ReleaseCapture();
    }
}
void CListBox::OnLButtonUp(UINT nFlags, CPoint point) {
    // RERUN: Logic moved to OnLButtonDown. We don't call base CWnd::OnLButtonUp
    // because we didn't call the base OnLButtonDown which would have set capture.
    // There is no capture to release.
}

void CListBox::OnMouseMove(UINT nFlags, CPoint point)
{
    int itemHeight = GetItemHeight();
    int index = (point.y / itemHeight) + m_topIndex; // RERUN: Account for scroll

    if (index < 0 || index >= (int)m_items.size()) {
        index = -1; // Mouse is outside the item area
    }

    if (index != m_hotSel) {
        m_hotSel = index;
        Invalidate(); // Redraw to show new hot item
    }
    CWnd::OnMouseMove(nFlags, point);
}

BOOL CListBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (zDelta > 0) {
        m_topIndex--; // Scroll up
    } else if (zDelta < 0) {
        m_topIndex++; // Scroll down
    }
    // Clamping is done in OnPaint, but we do it here too to trigger redraw only if needed
    int maxTop = std::max(0, (int)m_items.size() - 1);
    if (m_topIndex < 0) m_topIndex = 0;
    if (m_topIndex > maxTop) m_topIndex = maxTop;

    Invalidate();
    return TRUE;
}

void CListBox::OnSelChange() {}
void CListBox::OnDblClk() {
    if (GetParent()) {
        GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_DBLCLK), (LPARAM)m_hWnd);
    }
}

void CListBox::OnPaint() {
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    // RERUN: Fill the entire background once to handle areas with no items.
    // Only fill for dropdowns (which need opaque background); main menus use transparent lists over artwork.
    if (m_pComboOwner) {
        dc.FillSolidRect(rect, RGB(255, 255, 255)); 
    }
    
    int itemHeight = GetItemHeight(); // This gets the height correctly based on the font
    if (itemHeight <= 0) itemHeight = 24; // RERUN: Safety check to prevent divide by zero
    if (m_pComboOwner) std::cout << "[CRCombo] Painting dropdown with " << m_items.size() << " items. Height=" << itemHeight << std::endl;
    
    // RERUN: Safety check for vector corruption (unreasonably large size)
    if (m_items.size() > 10000) {
        return;
    }

    // RERUN: Calculate visible items and clamp scroll position
    int visibleCount = rect.Height() / itemHeight;
    int maxTop = std::max(0, (int)m_items.size() - visibleCount);
    
    if (m_topIndex > maxTop) m_topIndex = maxTop;
    if (m_topIndex < 0) m_topIndex = 0;

    // RERUN: Check for scrollbar
    bool hasScrollbar = (int)m_items.size() > visibleCount;
    int scrollWidth = 16;
    if (hasScrollbar) {
        // Reduce drawing area width to avoid drawing text over scrollbar
        rect.right -= scrollWidth;
    }

    // RERUN: We must select the same font here for drawing to match the height calculation.
    CFont* pFont = m_pFont;
    CWnd* pParent = GetParent();
    if (!pFont) {
        int fontNum = 2; // Default font
        if (m_pComboOwner) {
            fontNum = m_pComboOwner->GetFontNum();
        }
        if (pParent) {
            pFont = (CFont*)pParent->SendMessage(WM_GETGLOBALFONT, fontNum);
        }
    }

    CFont* pOldFont = nullptr;
    if (pFont) {
        pOldFont = dc.SelectObject(pFont);
    }

    int y = 0;
    for (int i = m_topIndex; i < (int)m_items.size(); ++i) {
        if (y >= rect.Height()) break; // Stop drawing if we run out of space

        CRect itemRect(0, y, rect.Width(), y + itemHeight); // RERUN FIX: Corrected bottom coordinate
        if (i == m_curSel) {
            dc.FillSolidRect(itemRect, RGB(0, 0, 128)); // Blue for selected
            dc.SetTextColor(RGB(255, 255, 255));
        } else if (i == m_hotSel) {
            // RERUN: Add visual marker for hot-tracked item
            dc.FillSolidRect(itemRect, RGB(230, 230, 250)); // Lavender for hot-track
            dc.SetTextColor(RGB(0, 0, 0));
        } else if (m_pComboOwner) {
            // Dropdown items need opaque background
            dc.FillSolidRect(itemRect, RGB(255, 255, 255));
            dc.SetTextColor(RGB(0, 0, 0));
        } else {
            // Transparent listbox (main menu): Don't fill background.
            // But ensure we set a visible text color (Rowan usually uses yellow/white for these).
            // Let's default to black, but TitleBar sets it differently maybe?
            dc.SetTextColor(RGB(255, 255, 0)); // RERUN: Default to Yellow for visibility on dark artwork
        }
        dc.SetBkMode(TRANSPARENT); // Background is now handled by FillSolidRect
        
        if (i < (int)m_items.size()) {
            dc.DrawText(m_items[i].c_str(), itemRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        y += itemHeight;
    }

    // RERUN: Draw a simple scrollbar track if needed
    if (hasScrollbar) {
        // Restore full rect for scrollbar drawing
        GetClientRect(&rect);
        CRect scrollRect(rect.right - scrollWidth, 0, rect.right, rect.Height());
        dc.FillSolidRect(scrollRect, RGB(212, 208, 200)); // Standard gray track
        dc.Draw3dRect(scrollRect, RGB(64, 64, 64), RGB(255, 255, 255)); // Inset look
        
        int thumbHeight = std::max(20, (rect.Height() * visibleCount) / (int)m_items.size());
        int trackHeight = rect.Height();
        int maxThumbY = trackHeight - thumbHeight;
        int thumbY = 0;
        if (maxTop > 0)
             thumbY = (m_topIndex * maxThumbY) / maxTop;
             
        CRect thumbRect(rect.right - scrollWidth, thumbY, rect.right, thumbY + thumbHeight);
        // Draw thumb button look
        dc.FillSolidRect(thumbRect, RGB(212, 208, 200));
        dc.Draw3dRect(thumbRect, RGB(255, 255, 255), RGB(64, 64, 64)); // Raised button
    }

    if (pOldFont) {
        dc.SelectObject(pOldFont);
    }
}

void CListBox::OnCaptureChanged(CWnd* pWnd)
{
    // If we are a dropdown and we've lost capture to something that isn't ourselves
    if (m_pComboOwner && pWnd != this) {
        m_hotSel = -1;
        ShowWindow(SW_HIDE);
    }
    CWnd::OnCaptureChanged(pWnd); // Call base
}

IMPLEMENT_DYNAMIC(CButton, CWnd)
IMPLEMENT_DYNAMIC(CStatic, CWnd)
IMPLEMENT_DYNAMIC(CComboBox, CWnd)

BOOL CButton::Create(LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    return CWnd::Create("BUTTON", lpszCaption, dwStyle, rect, pParentWnd, nID);
}
void CButton::SetCheck(int nCheck) { m_nCheck = nCheck; }
int CButton::GetCheck() const { return m_nCheck; }
void CButton::SetButtonStyle(UINT nStyle, BOOL bRedraw) { m_nStyle = nStyle; }
int CButton::GetState() const { return m_nState; }
void CButton::SetState(BOOL bHighlight) { m_nState = bHighlight ? 1 : 0; }
void CButton::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    
    // Simple button look
    dc.FillSolidRect(rect, RGB(192, 192, 0));
    dc.Rectangle(rect.left, rect.top, rect.right, rect.bottom);
    
    CString text;
    GetWindowText(text);
    if (!text.IsEmpty())
    {
        dc.SetBkMode(TRANSPARENT);
        dc.DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

BOOL CStatic::Create(LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    return CWnd::Create("STATIC", lpszText, dwStyle, rect, pParentWnd, nID);
}
HICON CStatic::SetIcon(HICON hIcon) { return nullptr; }
HICON CStatic::GetIcon() const { return nullptr; }
void* CStatic::SetBitmap(void* hBitmap) { return nullptr; }
void* CStatic::GetBitmap() const { return nullptr; }
HCURSOR CStatic::SetCursor(HCURSOR hCursor) { return nullptr; }
HCURSOR CStatic::GetCursor() const { return nullptr; }
void CStatic::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    
    CString text;
    GetWindowText(text);
    
    std::cout << "[DEBUG] CStatic::OnPaint ID=" << GetDlgCtrlID() << " Text='" << text << "' Rect=" << rect.left << "," << rect.top << " " << rect.Width() << "x" << rect.Height() << std::endl;
    
    // RERUN: Ensure we have a font
    CFont* pFont = m_pFont;
    if (!pFont && GetParent()) {
        // Try to get default font (index 2) from parent.
        pFont = (CFont*)GetParent()->SendMessage(WM_GETGLOBALFONT, 2);
    }
    CFont* pOldFont = nullptr;
    if (pFont) {
        pOldFont = dc.SelectObject(pFont);
    }

    if (!text.IsEmpty())
    {
        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(RGB(255, 255, 0)); // RERUN: Revert to UI-standard yellow

        UINT format = DT_WORDBREAK;
        DWORD style = GetStyle();
        if (style & SS_CENTER) format |= DT_CENTER;
        else if (style & SS_RIGHT) format |= DT_RIGHT;
        else format |= DT_LEFT;
        
        format |= DT_VCENTER; // Vertical centering

        dc.DrawText(text, rect, format);
    }
    
    if (pOldFont) {
        dc.SelectObject(pOldFont);
    }
}

BOOL CComboBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    return CWnd::Create("COMBOBOX", nullptr, dwStyle, rect, pParentWnd, nID);
}
int CComboBox::AddString(LPCTSTR lpszString) { return 0; }
int CComboBox::GetCurSel() const { return -1; }
int CComboBox::SetCurSel(int nSelect) { return CB_ERR; }
void CComboBox::ResetContent() {}
// --- Dialog Template Support ---
std::map<int, DlgTemplate> g_dlgTemplates;
// RERUN: Store DLGINIT strings for controls (DialogID -> ControlID -> Text)
std::map<int, std::map<int, std::string>> g_dlgInitStrings;
// RERUN: Store dialog captions (DialogID -> Caption)
std::map<int, std::string> g_dlgCaptions;


// DDX/DDV support
// This is a simplified CDataExchange, matching what CDialog::OnInitDialog expects.
CDataExchange::CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate)
    : m_pDlgWnd(pDlgWnd), m_bSaveAndValidate(bSaveAndValidate) {}
// This function connects a dialog's member variable (like an RButton) to a control
// that was created from the RC template. This is the standard MFC mechanism.
void AFXAPI DDX_Control(CDataExchange* pDX, int nIDC, CWnd& rControl)
{
    if (pDX->m_bSaveAndValidate)
        return; // Data saving is not implemented in this stub.
    // If the C++ object isn't attached to a window yet, subclass it.
    if (rControl.m_hWnd == NULL)
    {
        CWnd* pOldCtrl = pDX->m_pDlgWnd ? pDX->m_pDlgWnd->GetDlgItem(nIDC) : nullptr;
        if (pOldCtrl)
        {
            HWND hWnd = pOldCtrl->GetSafeHwnd();
            if (hWnd)
            {
                // Re-wire the HWND to be owned by the new control object (e.g., an RButton)
                rControl.SubclassWindow(hWnd);

                // RERUN: After subclassing, check for and apply custom button metadata
                int dlgID = 0;
                {
                    WindowBackend* be = backend_from_hwnd(pDX->m_pDlgWnd->GetSafeHwnd());
                    if (be) dlgID = be->templateID;
                }

                if (g_buttonMetadataMap.count(dlgID) && g_buttonMetadataMap[dlgID].count(nIDC)) {
                    const ButtonMetadata& meta = g_buttonMetadataMap[dlgID][nIDC];
                    CRButton* pButton = dynamic_cast<CRButton*>(&rControl);
                    CRRadio* pRadio = dynamic_cast<CRRadio*>(&rControl);
                    CRStatic* pStatic = dynamic_cast<CRStatic*>(&rControl);
                    CREdit* pEdit = dynamic_cast<CREdit*>(&rControl);

                    if (pButton) {
                        pButton->SetButtonMetadata(meta.icon_enum, meta.icon_name.c_str());
                    } else if (pRadio) {
                        if (meta.icon_enum == 0)
                        {
                            // RERUN force radioup.bmp — a MiG Alley filenum;
                            // BoB metadata never supplies radio artwork and
                            // BoB icon indices are not FileNums.
                            if (!g_rowanGameIsBoB)
                                pRadio->SetFileNum(27330);
                        }
                        else
                            pRadio->SetFileNum(meta.icon_enum);
                        // RERUN: Apply the length as ColumnWidth if present in metadata
                        if (meta.length > 0) {
                            pRadio->SetColumnWidth(meta.length);
                        }
                    } else if (pStatic) {
                        // RERUN: Apply font number from metadata if present
                        if (meta.icon_enum != 0) {
                            pStatic->SetFontNum(meta.icon_enum);
                        }
                    } else if (pEdit) {
                        if (meta.icon_enum != 0) pEdit->SetLimitText(meta.icon_enum);
                    } else if (rControl.IsKindOf(RUNTIME_CLASS(CRCombo))) {
                        // Apply initial string from DLGINIT if it exists
                        if (!g_dlgInitStrings[dlgID][nIDC].empty()) {
                            rControl.SetWindowText(g_dlgInitStrings[dlgID][nIDC].c_str());
                        }
                    } else {
                        // RERUN: Improved warning message for unhandled control types with metadata.
                        const std::type_info& ti = typeid(rControl);
                        std::cout << "[DDX_Control] Warning: Metadata found for control ID " 
                                  << nIDC << " in Dialog " << dlgID 
                                  << " but its type (" << ti.name() 
                                  << ") is not a CRButton or CRRadio." << std::endl;
                    }
                }
            }
        }
    }
}

void ParseRCFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[RCParser] Could not open " << filename << std::endl;
        return;
    }

    json data;
    try {
        file >> data;
    } catch (json::parse_error& e) {
        std::cerr << "[RCParser] JSON parse error: " << e.what() << std::endl;
        return;
    }

    // RERUN: Load ID mappings (Name -> ID) from header defines
    if (data.contains("header_defines") && data["header_defines"].is_array()) {
        for (const auto& define : data["header_defines"]) {
            if (define.contains("name") && define.contains("id") && !define["id"].is_null()) {
                g_resIDMap[define["name"].get<std::string>()] = define["id"].get<int>();
            }
        }
    }

    // RERUN: Load all resource strings (ID -> String) from string tables
    if (data.contains("stringtables") && data["stringtables"].is_array()) {
        for (const auto& table : data["stringtables"]) {
            if (table.contains("entries") && table["entries"].is_array()) {
                for (const auto& entry : table["entries"]) {
                    if (entry.contains("id") && !entry["id"].is_null() && entry.contains("text")) {
                        int id = entry["id"].get<int>();
                        g_resStringsMap[id] = entry["text"].get<std::string>();
                    }
                }
            }
        }
    }

    if (!data.contains("dialogs") || !data["dialogs"].is_array()) {
        std::cout << "[RCParser] JSON does not contain a 'dialogs' array." << std::endl;
        return;
    }

    for (const auto& dialog_json : data["dialogs"]) {
        if (!dialog_json.contains("numeric_id") || dialog_json["numeric_id"].is_null()) {
            continue;
        }

        DlgTemplate currentDlg = {}; // RERUN: Zero initialize template
        currentDlg.id = dialog_json["numeric_id"].get<int>();

        if (dialog_json.contains("rect") && dialog_json["rect"].is_array() && dialog_json["rect"].size() == 4 && !dialog_json["rect"][0].is_null()) {
            currentDlg.x = dialog_json["rect"][0].get<int>();
            currentDlg.y = dialog_json["rect"][1].get<int>();
            currentDlg.cx = dialog_json["rect"][2].get<int>();
            currentDlg.cy = dialog_json["rect"][3].get<int>();
        } else {
            currentDlg.x = 0; currentDlg.y = 0; 
            // RERUN: Use -1 to signal that coordinates were missing/null in JSON
            currentDlg.cx = -1; currentDlg.cy = -1;
        }

        if (dialog_json.contains("controls") && dialog_json["controls"].is_array()) {
            for (const auto& control_json : dialog_json["controls"]) {
                DlgControlTemplate ctl = {}; // RERUN: Zero initialize control

                if (control_json.contains("id") && !control_json["id"].is_null()) {
                    ctl.id = control_json["id"].get<int>();
                } else {
                    ctl.id = 0;
                }

                if (control_json.contains("text") && !control_json["text"].is_null()) {
                    ctl.text = control_json["text"].get<std::string>();
                }

                if (control_json.contains("rect") && control_json["rect"].is_array() && control_json["rect"].size() == 4 && !control_json["rect"][0].is_null()) {
                    ctl.x = control_json["rect"][0].get<short>();
                    ctl.y = control_json["rect"][1].get<short>();
                    ctl.cx = control_json["rect"][2].get<short>();
                    ctl.cy = control_json["rect"][3].get<short>();
                }

                std::string keyword;
                if (control_json.contains("keyword") && !control_json["keyword"].is_null()) {
                    keyword = control_json["keyword"].get<std::string>();
                }

                ctl.style = 0; // Initialize style

                // RERUN: Attempt to parse style string from JSON if available
                if (control_json.contains("style") && control_json["style"].is_string()) {
                    std::string s = control_json["style"].get<std::string>();
                    if (s.find("SS_CENTER") != std::string::npos) ctl.style |= SS_CENTER;
                    else if (s.find("SS_RIGHT") != std::string::npos) ctl.style |= SS_RIGHT;
                    else if (s.find("SS_LEFT") != std::string::npos) ctl.style |= SS_LEFT;
                    if (s.find("WS_VISIBLE") != std::string::npos) ctl.style |= WS_VISIBLE;
                    if (s.find("WS_DISABLED") != std::string::npos) ctl.style |= WS_DISABLED;
                }

                if (keyword == "CONTROL") {
                    if (control_json.contains("class") && !control_json["class"].is_null()) {
                        ctl.className = control_json["class"].get<std::string>();
                    }
                } else if (keyword == "PUSHBUTTON" || keyword == "DEFPUSHBUTTON" || keyword == "GROUPBOX") {
                    ctl.className = "BUTTON";
                } else if (keyword == "LTEXT" || keyword == "RTEXT" || keyword == "CTEXT") {
                    ctl.className = "STATIC";
                    if (keyword == "CTEXT") ctl.style |= SS_CENTER;
                    else if (keyword == "RTEXT") ctl.style |= SS_RIGHT;
                    else ctl.style |= SS_LEFT;
                } else if (keyword == "EDITTEXT") {
                    ctl.className = "EDIT";
                } else if (keyword == "COMBOBOX") {
                    ctl.className = "COMBOBOX";
                } else if (keyword == "LISTBOX") {
                    ctl.className = "LISTBOX";
                } else {
                    ctl.className = keyword; // Fallback
                }

                currentDlg.controls.push_back(ctl);
            }
        }

        // RERUN: Capture caption from properties if available
        if (dialog_json.contains("properties") && dialog_json["properties"].is_object()) {
            if (dialog_json["properties"].contains("CAPTION") && dialog_json["properties"]["CAPTION"].is_string()) {
                g_dlgCaptions[currentDlg.id] = dialog_json["properties"]["CAPTION"].get<std::string>();
                std::cout << "[ParseRCFile] Loaded Caption for " << currentDlg.id << ": '" << g_dlgCaptions[currentDlg.id] << "'" << std::endl;
            }
        }

        if (currentDlg.id != 0) {
            // RERUN: Better template selection. A template with controls is always 
            // preferred over an empty one. Never let an empty/junk stub overwrite 
            // a valid template that was already loaded.
            if (g_dlgTemplates.count(currentDlg.id)) {
                if (currentDlg.controls.empty() && !g_dlgTemplates[currentDlg.id].controls.empty()) {
                    continue;
                }
            }
            // Valid full template found; it may overwrite an existing stub if necessary.
            g_dlgTemplates[currentDlg.id] = currentDlg;
        }
    }

    // RERUN: Parse DLGINIT blocks to get custom button metadata
    if (data.contains("dlginits") && data["dlginits"].is_array()) {
        for (const auto& dlginit_json : data["dlginits"]) {
            int dlgID = 0;
            if (dlginit_json.contains("numeric_id") && !dlginit_json["numeric_id"].is_null()) {
                dlgID = dlginit_json["numeric_id"].get<int>();
            }

            if (dlginit_json.contains("entries") && dlginit_json["entries"].is_array()) {
                for (const auto& entry_json : dlginit_json["entries"]) {
                    // Resolve Control ID
                    if (entry_json.contains("control") && entry_json["control"].is_string()) {
                        std::string control_name = entry_json["control"].get<std::string>();
                        int control_id = 0;
                        
                        auto res_it = g_resIDMap.find(control_name);
                        if (res_it != g_resIDMap.end()) {
                            control_id = res_it->second;
                        } else {
                            // Try parsing as integer if name lookup failed
                            try { control_id = std::stoi(control_name); } catch (...) {}
                        }

                        if (control_id != 0) {
                            // 1. Button Metadata
                            if (entry_json.contains("button_metadata") && entry_json["button_metadata"].is_object()) {
                                const auto& meta = entry_json["button_metadata"];
                                ButtonMetadata btnMeta;
                                if (meta.contains("icon_enum") && !meta["icon_enum"].is_null()) {
                                    btnMeta.icon_enum = meta["icon_enum"].get<int>();
                                }
                                if (meta.contains("icon_name") && !meta["icon_name"].is_null()) {
                                    btnMeta.icon_name = meta["icon_name"].get<std::string>();
                                }
                                if (btnMeta.icon_enum != 0) {
                                    g_buttonMetadataMap[dlgID][control_id] = btnMeta;
                                }
                            }

                            // RERUN: Capture 'length' field from the DLGINIT entry
                            if (entry_json.contains("length") && entry_json["length"].is_number()) {
                                g_buttonMetadataMap[dlgID][control_id].length = entry_json["length"].get<int>();
                            }

                            // 2. Initialization Strings (for CRStatic labels etc.)
                            // Rowan resources often put the display text at index 1 of the strings array 
                            // (index 0 is often copyright/empty).
                            if (dlgID != 0 && entry_json.contains("strings") && entry_json["strings"].is_array()) {
                                const auto& strings = entry_json["strings"];
                                if (strings.size() >= 2 && strings[1].is_string()) {
                                    std::string val = strings[1].get<std::string>();
                                    if (!val.empty()) {
                                        g_dlgInitStrings[dlgID][control_id] = val;
                                        //std::cout << "[DEBUG] ParseRCFile: Loaded init string for Dialog " << dlgID << " Control " << control_id << ": '" << val << "'" << std::endl;
                                    }
                                }
                            }

                            // RERUN: 3. Label ID (Resource String ID)
                            // This overrides the raw strings if a valid resource ID is found.
                            if (entry_json.contains("label_id") && entry_json["label_id"].is_string()) {
                                std::string labelID = entry_json["label_id"].get<std::string>();
                                int resID = 0;
                                
                                auto itID = g_resIDMap.find(labelID);
                                if (itID != g_resIDMap.end()) {
                                    resID = itID->second;
                                }
                                
                                if (resID != 0) {
                                    auto itStr = g_resStringsMap.find(resID);
                                    if (itStr != g_resStringsMap.end()) {
                                        const std::string& resStr = itStr->second;
                                        // RERUN FIX: Only use the resource string if it's not a known placeholder.
                                        // This prevents "Xx" or "-" from overwriting good scavenged text.
                                        if (resStr != "Xx" && resStr != "xx" && resStr != "-" && !resStr.empty()) {
                                            g_dlgInitStrings[dlgID][control_id] = resStr;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //std::cout << "[RCParser] Loaded " << g_buttonMetadataMap.size() << " custom button metadata entries from JSON." << std::endl;
    //std::cout << "[RCParser] Loaded " << g_dlgTemplates.size() << " dialog templates from JSON." << std::endl;
    std::cout << "[RCParser] Loaded " << g_dlgCaptions.size() << " captions from JSON." << std::endl;
}

// True when the active game is Battle of Britain — set by which RC json
// loaded below. Lets shared UI code skip MiG Alley specific artwork hacks.
bool g_rowanGameIsBoB = false;

// Filter for fields that mix FileNums and BoB icon-table indices: fileman
// treats an out-of-range FileNum as fatal, so paint code validates first.
// namenumberedfilelessfail returns "//", NULL, or a path with an empty
// filename (trailing '/') for anything that cannot resolve.
bool RowanFileNumSafe(long fnum)
{
    string nm = File_Man.namenumberedfilelessfail((FileNum)fnum);
    size_t nl = nm ? strlen(nm) : 0;
    return nl && nm[nl - 1] != '/';
}

void LoadDialogTemplates()
{
    // Try to find the game's RC json — each install tree ships its own
    // (MIG_RC.json for MiG Alley, BOB_RC.json for BoB); first match wins.
    const char* paths[] = {
        "MIG_RC.json", "./MIG_RC.json", "./MigAlley/MIG_RC.json",
        "BOB_RC.json", "./BOB_RC.json", "./Bob/BOB_RC.json"
    };
    for (const char* p : paths) {
        std::ifstream f(p);
        if (f.good()) {
            std::cout << "[MFC_stub] Loading dialog templates from " << p << std::endl;
            if (strstr(p, "BOB_RC.json"))
                g_rowanGameIsBoB = true;
            ParseRCFile(p);
            return;
        }
    }
    std::cerr << "[MFC_stub] ERROR: RC json NOT FOUND in expected paths. Dialogs will be missing layout and captions." << std::endl;
}

BOOL CDialog::GetControlInfo(int id, CRect& rect, DWORD& style)
{
    if (!m_pTemplate) return FALSE;
    
    for (const auto& ctl : m_pTemplate->controls) {
        if (ctl.id == id) {
            // Convert DLU to Pixels (Approximate for now: 1 DLU x = 2 px, 1 DLU y = 2 px)
            // Real conversion depends on font.
            // Standard: 1 horz DLU = 1/4 avg char width. 1 vert DLU = 1/8 char height.
            // Assuming 8x16 font for simplicity: 1x = 2px, 1y = 2px.
            rect.left = ctl.x * g_dluX;
            rect.top = ctl.y * g_dluY;
            rect.right = rect.left + ctl.cx * g_dluX;
            rect.bottom = rect.top + ctl.cy * g_dluY;

            // If we have a valid style parsed, we could return it, but for now keep default
            return TRUE;
        }
    }
    return FALSE;
}

// CDialog
CDialog::CDialog(int iid) : CWnd()
{
    m_nIDHelp = iid;
}
CDialog::CDialog(int iid, CWnd* pParent) : CWnd()
{
    m_nIDHelp = iid;
    m_pParent = pParent;
}

int CDialog::DoModal()
{
    m_forceTopLevel = true;
    if (!m_hWnd)
        Create(m_nIDHelp, m_pParent);

    ShowWindow(SW_SHOW);

    OnInitDialog();

    m_modalRunning = true;
    m_isModal = true;
    m_modalResult = IDCANCEL;

    // Modal message loop
    while (m_modalRunning && !g_shouldQuit)
    {
        PumpMessage();
        SDL_Delay(1);
    }

    DestroyWindow();
    return m_modalResult;
}

void CDialog::EndDialog(int result)
{
    m_modalResult = result;
    m_modalRunning = false;
}
void CDialog::OnDestroy()
{
    CWnd::OnDestroy();
}


void CDialog::OnOK() { EndDialog(IDOK); }
void CDialog::OnCancel() { EndDialog(IDCANCEL); }
void CDialog::OnClose()
{
    if (m_isModal)
        OnCancel();      // calls EndDialog()
    else
        DestroyWindow(); // modeless behavior
}
BOOL CDialog::OnInitDialog() 
{ 
    // The default CDialog::OnInitDialog calls UpdateData(FALSE), which
    // in turn calls the virtual DoDataExchange. This is where DDX_Control
    // is used to subclass controls.
    CDataExchange dx(this, FALSE);
    DoDataExchange(&dx);
    return TRUE;
}
void CDialog::DoDataExchange(CDataExchange*) {}

BOOL CDialog::Create(int id, CWnd* pParent)
{
    m_pParent = pParent;
    m_isModal = false;
    
    // RERUN: If the window already exists (e.g. CToolBar created via CWnd::Create then LoadToolBar),
    // we just need to load the template and create children.
    if (m_hWnd) {
        // Look up template
        auto it = g_dlgTemplates.find(id);
        if (it != g_dlgTemplates.end()) {
            m_pTemplate = &it->second;
        }
        // Fall through to control creation
    }
    else
    {

    // Allocate backend
    WindowBackend backend;
    backend.owner = this;
    backend.parent = pParent;
    backend.templateID = id;
    backend.style = 0;
    backend.visible = false;
    backend.isChild = false;

    // Look up template
    auto it = g_dlgTemplates.find(id);
    if (it != g_dlgTemplates.end()) {
        m_pTemplate = &it->second;
    }

    bool createChild = (pParent != nullptr) && !m_forceTopLevel;
    CWnd* host = pParent;

    if (createChild && pParent->m_hWnd) {
        WindowBackend* parentBe = backend_from_hwnd(pParent->m_hWnd);
        if (parentBe) {
            backend.window = parentBe->window;
            backend.renderer = parentBe->renderer;
            backend.isChild = true;
            // m_rect will be initialized to 0,0,0,0 or similar, 
            // caller (e.g. CMainFrame::RecalcLayout) is expected to MoveWindow it.
            if (m_pTemplate) {
                // Use template size (approx DLU conversion)
                m_rect = CRect(m_pTemplate->x * g_dluX, m_pTemplate->y * g_dluY, (m_pTemplate->x + m_pTemplate->cx) * g_dluX, (m_pTemplate->y + m_pTemplate->cy) * g_dluY);
            } else {
                m_rect = CRect(0, 0, 300, 200);
            }
        }
    }

    // RERUN: a parentless dialog must not get its own SDL window once a real
    // game window exists - under a WM each one lands as a stray window and
    // eats clicks. Share the first real window backend instead.
    if (!backend.isChild) {
        for (auto& kv : g_hwndRegistry) {
            WindowBackend& be = kv.second;
            if (be.window && !be.isChild && be.owner && be.owner != this) {
                backend.window = be.window;
                backend.renderer = be.renderer;
                backend.isChild = true;
                backend.parent = be.owner;
                host = be.owner;
                break;
            }
        }
    }

    // RERUN: logical children need a real m_rect before OnInitDialog queries
    // GetClientRect - the routed-dialog path above never initializes it, which
    // left modal dialogs (RMdlDlg) zero-sized and unreachable by hit-testing.
    if (backend.isChild && m_rect.IsRectEmpty()) {
        if (m_pTemplate) {
            m_rect = CRect(m_pTemplate->x * g_dluX, m_pTemplate->y * g_dluY, (m_pTemplate->x + m_pTemplate->cx) * g_dluX, (m_pTemplate->y + m_pTemplate->cy) * g_dluY);
        } else {
            m_rect = CRect(0, 0, 300, 200);
        }
    }

    if (!backend.isChild) {
        // Create SDL window (or UI node)
        backend.window = CreateSDLDialogWindow(id);
        if (!backend.window) {
            std::cerr << "Failed to create SDL window for dialog ID " << id
                    << ": " << SDL_GetError() << std::endl;
            return FALSE;
        }

        int w = 199, h = 238;
        int x = 0, y = 0;
        if (m_pTemplate) {
            w = m_pTemplate->cx * g_dluX;
            h = m_pTemplate->cy * g_dluY;
            x = m_pTemplate->x * g_dluX;
            y = m_pTemplate->y * g_dluY;
        }

        // --- Dialog window tweaks (MFC‑style) ---
        SDL_SetWindowPosition(
            backend.window,
            x,
            y
        );

        SDL_SetWindowResizable(backend.window, SDL_FALSE);
        SDL_SetWindowSize(backend.window, w, h);
        std::string title = "Dialog ";
        title += std::to_string(id);
        SDL_SetWindowTitle(backend.window, title.c_str());
        // ---------------------------------------

        backend.renderer = SDL_CreateRenderer(backend.window, -1, SDL_RENDERER_ACCELERATED);
    }

    // Generate HWND key
    HWND key = allocate_hwnd();
    // or reinterpret_cast<HWND>(this) — whichever convention you use

    // Register
    g_hwndRegistry[key] = backend;

    // Store in CWnd
    m_hWnd = key;

    // Initialize m_rect from the SDL window so GetWindowRect works
    if (backend.window && !backend.isChild) {
        int x, y, w, h;
        SDL_GetWindowPosition(backend.window, &x, &y);
        SDL_GetWindowSize(backend.window, &w, &h);
        m_rect = CRect(x, y, x + w, y + h);
    }

    // Attach to parent (or to the host window owner for routed dialogs)
    if (host) {
        if (host->m_hWnd) {
             g_hwndRegistry[host->m_hWnd].children.push_back(key);
        }
    }
    
    } // End of if (!m_hWnd)

    // RERUN: deliver WM_CREATE the way ::CreateDialogIndirect does — before
    // child controls exist and before WM_INITDIALOG. The stub never sent it,
    // so RDialog::OnCreate never ran and homesize stayed (0,0,0,0); nested
    // dials then got zero-height rects and were unreachable by hit-testing.
    {
        CREATESTRUCT cs = {};
        cs.hInstance = nullptr;
        cs.hMenu = nullptr;
        cs.hwndParent = (m_pParent) ? m_pParent->m_hWnd : nullptr;
        SendMessage(WM_CREATE, 0, (LPARAM)&cs);
    }

    // Create controls from template
    if (m_pTemplate) {
        // RERUN: Set dialog caption if available
        auto itCap = g_dlgCaptions.find(id);
        if (itCap != g_dlgCaptions.end()) {
            SetWindowText(itCap->second.c_str());
            std::cout << "[CDialog::Create] ID=" << id << " Set Caption='" << itCap->second << "'" << std::endl;
        }
        else {
            // RERUN: Fallback captions for known dialogs if RC/JSON is missing them
            const char* fallback = nullptr;
            switch (id) {
                case 276: fallback = "Player Log"; break;
                case 287: fallback = "Quick Mission"; break;
                case 917: fallback = "Ready Room"; break;
                case 289: fallback = "Campaign Select"; break;
                case 823: fallback = "System"; break;
                case 280: fallback = "Replay"; break;
                case 291: fallback = "Mission Results"; break;
                case 279: fallback = "Overview"; break;
                case 267: fallback = "Squadron List"; break;
                case 263: fallback = "Intelligence"; break;
                case 265: fallback = "Weather"; break;
                case 221: fallback = "Directives"; break;
                case 222: fallback = "Authorisation"; break;
                case 220: fallback = "Bases"; break;
                case 200: fallback = "Mission Folder"; break;
                case 266: fallback = "3D Detail"; break; // RERUN: Fix for Preferences Tab
                case 998: fallback = "Credits"; break;
            }
            if (fallback) {
                SetWindowText(fallback);
                std::cout << "[CDialog::Create] ID=" << id << " Set Fallback Caption='" << fallback << "'" << std::endl;
            } else 
            {
                std::cout << "[CDialog::Create] ID=" << id << " No caption found in g_dlgCaptions." << std::endl;
            }
        }

        for (const auto& ctl : m_pTemplate->controls) {
            CWnd* pChild = nullptr;
            // RERUN: Force CStatic for campaign text (IDC_SDETAIL1=2151) to ensure it renders.
            // IDs: 948=CampaignBack, 1014=QuickText, 977=CampaignBackEntireWar
            if (ctl.id == 2151 && (id == 948 || id == 1014 || id == 977)) {
                pChild = new CRStatic();
                ((CRStatic*)pChild)->SetFontNum(8); // RERUN: Use smaller font (8) for campaign details
            }
            else if (ctl.id == IDC_RLISTBOX) pChild = new CRListBox(); // RERUN: Use CRListBox for main menu
            else if (ctl.className == "LISTBOX") pChild = new CRListBox(); // RERUN: Revert to CRListBox for other listboxes
            else if (ctl.className == "EDIT" || ctl.className == "CREdit" || ctl.className == "6CREdit") pChild = new CREdit();
            else if (ctl.className == "BUTTON") pChild = new CRButton();
            else if (ctl.className == "STATIC") {
                pChild = new CRStatic(); // RERUN: Revert to CRStatic
                ((CRStatic*)pChild)->SetFontNum(2); // RERUN: Use Font 2 for standard static text labels (e.g. UI descriptions)
            }
            else if (ctl.className == "COMBOBOX") pChild = new CRCombo();
            else {
                // RERUN: Rowan OCX CLSID dispatch. The GUID's first field
                // identifies the control family; the third field varies
                // (11D1 = MiG Alley typelib, 11D6 = BoB typelib) and BoB's
                // rc mixes hex case, so match the normalized prefix.
                std::string clsid = ctl.className;
                for (auto& ch : clsid) ch = (char)toupper((unsigned char)ch);
                if (clsid.rfind("{78918646-", 0) == 0) {
                    pChild = new CRButton();
                    // RERUN: Automatically enable Close/Help buttons for Title Bars (IDJ_TITLE=1001)
                    if (ctl.id == 1001) {
                        ((CRButton*)pChild)->SetCloseButton(TRUE);
                        ((CRButton*)pChild)->SetHelpButton(TRUE);
                    }
                }
                else if (clsid.rfind("{C42BAC3D-", 0) == 0) {
                    pChild = new CRStatic();
                    // RERUN: Use Font 2 for standard static text labels in UI menus (Preferences, Quick Mission),
                    // but keep default (0) for others (Campaign details etc) which may need smaller text.
                    if (id == 257 || id == 266 || id == 274 || id == 271 || id == 273 || id == 261 || id == 958 || id == 287 || id == 297 || id == 298)
                        ((CRStatic*)pChild)->SetFontNum(8);
                    else
                        ((CRStatic*)pChild)->SetFontNum(0);
                }
                else if (clsid.rfind("{737CB0C9-", 0) == 0) pChild = new CRCombo();
                else if (clsid.rfind("{461A1FE3-", 0) == 0 || ctl.className == "7CREdtBt") pChild = new CREdtBt();
                else if (clsid.rfind("{48814009-", 0) == 0) pChild = new CRListBox();
                else if (clsid.rfind("{4A1E1986-", 0) == 0 || ctl.className.find("Tab") != std::string::npos) pChild = new CRTabs();
                else if (clsid.rfind("{5363BA22-", 0) == 0) pChild = new CRRadio();
                else if (clsid.rfind("{C3270E66-", 0) == 0) pChild = new CRSpinBut();
                else if (clsid.rfind("{1B499F6B-", 0) == 0) pChild = new CRJoyCfg();
                else if (clsid.rfind("{499E2BE6-", 0) == 0) pChild = new CREdit();
                else pChild = new DlgItem();
            }

            if (pChild) {
                CRect r(ctl.x * g_dluX, ctl.y * g_dluY, (ctl.x + ctl.cx) * g_dluX, (ctl.y + ctl.cy) * g_dluY);
                // RERUN: Apply style parsed from RC file (e.g. SS_CENTER)
                pChild->Create(ctl.className.c_str(), ctl.text.c_str(), WS_CHILD | WS_VISIBLE | ctl.style, r, this, ctl.id);

                // RERUN: Apply DLGINIT text if available for this control
                auto itDlg = g_dlgInitStrings.find(id);
                if (itDlg != g_dlgInitStrings.end()) {
                    auto itCtl = itDlg->second.find(ctl.id);
                    if (itCtl != itDlg->second.end()) {
                        pChild->SetWindowText(itCtl->second.c_str());
                        //std::cout << "[DEBUG] CDialog::Create: Applied text '" << itCtl->second << "' to Control " << ctl.id << " in Dialog " << id << std::endl;
                    }
                }
            } else {
            }
        }
    } else {
    }

    // Call initialization
    if (!OnInitDialog()) {
        return FALSE;
    }

    return TRUE;
}

void CDialog::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
    if (!lpClientRect) return;

    DWORD dwStyle = GetStyle();

    // Approximate Win32 metrics
    const int cxBorder = 1;
    const int cyBorder = 1;
    const int cxFrame = 4;
    const int cyFrame = 4;
    const int cyCaption = 20;

    int dx = 0;
    int dy = 0;

    if (dwStyle & WS_THICKFRAME) {
        dx = cxFrame;
        dy = cyFrame;
    } else if (dwStyle & WS_DLGFRAME) {
        dx = cxFrame;
        dy = cyFrame;
    } else if (dwStyle & WS_BORDER) {
        dx = cxBorder;
        dy = cyBorder;
    }

    lpClientRect->left   -= dx;
    lpClientRect->right  += dx;
    lpClientRect->top    -= dy;
    lpClientRect->bottom += dy;

    if ((dwStyle & WS_CAPTION) == WS_CAPTION) {
        lpClientRect->top -= cyCaption;
    }
}

void CDialog::OnPaint()
{
    CPaintDC dc(this);
    if (!SendMessage(WM_ERASEBKGND, (WPARAM)&dc, 0))
    {
        CRect r;
        GetClientRect(r);
        dc.FillSolidRect(r, RGB(192,192,192));
    }
}

void CDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    CWnd::OnLButtonDown(nFlags, point);
}
void CDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
    CWnd::OnLButtonUp(nFlags, point);
}

// CEdit
void CEdit::SetWindowText(LPCTSTR) {}
void CEdit::GetWindowText(LPTSTR buf, int max) const { if (max > 0) buf[0] = '\0'; }
void CEdit::GetWindowText(CString& s) const { s = ""; }
void CEdit::SetLimitText(int) {}
void CEdit::ReplaceSel(LPCTSTR) {}
void CEdit::SetSel(int, int) {}

// CCmdUI
void CCmdUI::Enable(BOOL /*bOn*/) {}
void CCmdUI::SetCheck(int /*nCheck*/) {}
void CCmdUI::SetRadio(BOOL /*bOn*/) {}
void CCmdUI::SetText(const char* /*pszText*/) {}

// CToolBar
BOOL CToolBar::Create(CWnd* parentWnd, DWORD style, UINT id) { m_pParent = parentWnd; return TRUE; }
BOOL CToolBar::CreateEx(CWnd* parentWnd, DWORD dwCtrlStyle, DWORD dwStyle, const RECT* rect, UINT nID) { m_pParent = parentWnd; return TRUE; }
BOOL CToolBar::LoadToolBar(UINT resourceID) { return CDialog::Create(resourceID, m_pParent); }
void CToolBar::EnableDocking(DWORD /*dockStyle*/) {}
BOOL CToolBar::SetButtons(const UINT* /*buttons*/, int /*count*/) { return TRUE; }
CSize CToolBar::CalcFixedLayout(BOOL /*stretch*/, BOOL /*horz*/) { return CSize(0, 0); }
CSize CToolBar::CalcDynamicLayout(int /*length*/, DWORD /*mode*/) { return CSize(0, 0); }
BOOL CToolBar::SetButtonInfo(int /*index*/, UINT /*id*/, UINT /*style*/, int /*image*/) { return TRUE; }
int CToolBar::CommandToIndex(UINT /*id*/) const { return -1; }
BOOL CToolBar::OnAmbientProperty(COleControlSite* /*pSite*/, DISPID /*dispid*/, VARIANT* /*pvar*/) { return FALSE; }
BOOL CToolBar::OnCmdMsg(UINT /*nID*/, int /*nCode*/, void* /*pExtra*/, AFX_CMDHANDLERINFO* /*pHandlerInfo*/) { return FALSE; }

// CMenu
BOOL CMenu::CreateMenu() { return TRUE; }
BOOL CMenu::CreatePopupMenu() { return TRUE; }
BOOL CMenu::LoadMenu(UINT /*id*/) { return TRUE; }
BOOL CMenu::EnableMenuItem(UINT /*id*/, UINT /*flags*/) { return TRUE; }
BOOL CMenu::Attach(HMENU /*hMenu*/) { return TRUE; }
HMENU CMenu::Detach() { return nullptr; }
BOOL CMenu::AppendMenu(UINT /*flags*/, UINT /*id*/, const char* /*text*/) { return TRUE; }
BOOL CMenu::InsertMenu(UINT /*pos*/, UINT /*flags*/, UINT /*id*/, const char* /*text*/) { return TRUE; }
BOOL CMenu::DeleteMenu(UINT /*pos*/, UINT /*flags*/) { return TRUE; }
BOOL CMenu::RemoveMenu(UINT /*pos*/, UINT /*flags*/) { return TRUE; }
BOOL CMenu::ModifyMenu(UINT /*pos*/, UINT /*flags*/, UINT /*newID*/, const char* /*text*/) { return TRUE; }
CMenu* CMenu::GetSubMenu(int /*pos*/) const { return nullptr; }
int CMenu::GetMenuItemCount() const { return 0; }
UINT CMenu::GetMenuItemID(int /*pos*/) const { return 0; }
void CMenu::MeasureItem(LPMEASUREITEMSTRUCT /*lpMIS*/) {}
void CMenu::DrawItem(LPDRAWITEMSTRUCT /*lpDIS*/) {}
BOOL CMenu::TrackPopupMenu(UINT /*flags*/, int /*x*/, int /*y*/, CWnd* /*pWnd*/, const RECT* /*rect*/) { return TRUE; }
HMENU CMenu::GetSafeHmenu() const { return nullptr; }

// CFrameWnd
BOOL CFrameWnd::Create(const char* className, const char* windowName, DWORD style, const RECT& rect, CWnd* pParentWnd, const char* /*menuName*/, DWORD /*exStyle*/, void* /*lpCreateParam*/) 
{
	// In our stub, we forward this to the base CWnd::Create to handle SDL window creation.
	return CWnd::Create(className, windowName, style, rect, pParentWnd, 0, nullptr);
}
BOOL CFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, void* pContext)
{
    // Allocate an HWND for this window
    HWND h = allocate_hwnd();
    m_hWnd = h;

    // Get backend entry
    WindowBackend& backend = g_hwndRegistry[h];

    // Create SDL window
    backend.window = CreateOSWindow(
        "MFC Stub Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 960,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (backend.window && !g_migOSWindow)
        g_migOSWindow = backend.window;

    if (!backend.window)
        return FALSE;

    // RERUN: Enforce a minimum window size of 1024x768 for the main menu and campaign map.
    SDL_SetWindowMinimumSize(backend.window, 1024, 768);

    // Try hardware acceleration first
    backend.renderer = SDL_CreateRenderer(backend.window, -1, SDL_RENDERER_ACCELERATED);
    if (!backend.renderer) {
        std::cout << "Warning: Hardware acceleration failed (" << SDL_GetError() << "), falling back to software renderer." << std::endl;
        backend.renderer = SDL_CreateRenderer(backend.window, -1, 0);
        if (!backend.renderer) {
            std::cout << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
            return FALSE;           
        }
    }

    backend.owner = this;
    backend.parent = pParentWnd;
    backend.style = dwDefaultStyle;
    backend.visible = true;

    // Mark this as the main window if none exists yet
    if (AfxGetMainWnd() == nullptr || AfxGetMainWnd()->GetSafeHwnd() == nullptr)
        AfxGetApp()->m_pMainWnd = this;

    // Initialize m_rect from the SDL window so GetWindowRect works
    if (backend.window) {
        int x, y, w, h;
        SDL_GetWindowPosition(backend.window, &x, &y);
        SDL_GetWindowSize(backend.window, &w, &h);
        m_rect = CRect(x, y, x + w, y + h);
    }

    return TRUE;
}
CMenu* CFrameWnd::GetMenu() const { return nullptr; }
BOOL CFrameWnd::SetMenu(CMenu* /*pMenu*/) { return TRUE; }
void CFrameWnd::RecalcLayout(BOOL /*bNotify*/) {}
BOOL CFrameWnd::OnCmdMsg(UINT /*nID*/, int /*nCode*/, void* /*pExtra*/, AFX_CMDHANDLERINFO* /*pHandlerInfo*/) { return FALSE; }
BOOL CFrameWnd::PreTranslateMessage(MSG* /*pMsg*/) { return FALSE; }
BOOL CFrameWnd::DockControlBar(CWnd* /*pBar*/, UINT /*nDockBarID*/, const RECT* /*lpRect*/) { return TRUE; }
void CFrameWnd::SetActiveView(CView* pView) 
{
    m_pActiveView = pView;
}
CView* CFrameWnd::GetActiveView() const 
{
    return m_pActiveView;
}
void CFrameWnd::AttachView(CView* pView)
{
    m_pActiveView = pView;
    pView->m_pParent = this;
    pView->m_hWnd = m_hWnd;
    pView->m_rect = CRect(0, 0, m_rect.Width(), m_rect.Height()); // RERUN: View is at 0,0 in client coords
}

static void PaintChildrenRecursive(HWND parentHwnd, SDL_Renderer* renderer, int parentAbsX, int parentAbsY, int depth = 0)
{
    static const bool dumpLayout = getenv("ROWAN_DUMP_LAYOUT") != nullptr;
    WindowBackend* parentBe = backend_from_hwnd(parentHwnd);
    if (!parentBe) return;

    for (HWND childHwnd : parentBe->children) {
        WindowBackend* childBe = backend_from_hwnd(childHwnd);
        int childAbsX = 0, childAbsY = 0;
        if (childBe && childBe->owner) {
            childAbsX = parentAbsX + childBe->owner->m_rect.left;
            childAbsY = parentAbsY + childBe->owner->m_rect.top;
            if (dumpLayout)
                printf("[LAYOUT] %*shwnd=%p vis=%d tid=%d id=%d type=%s par=%p ptype=%s abs=(%d,%d) rect=(%ld,%ld)-(%ld,%ld)\n",
                    depth * 2, "",
                    childHwnd, (int)childBe->visible, childBe->templateID, childBe->owner->m_nID,
                    typeid(*childBe->owner).name(), (void*)childBe->parent,
                    childBe->parent ? typeid(*childBe->parent).name() : "-", childAbsX, childAbsY,
                    (long)childBe->owner->m_rect.left, (long)childBe->owner->m_rect.top,
                    (long)childBe->owner->m_rect.right, (long)childBe->owner->m_rect.bottom);
        }
        if (childBe && childBe->visible && childBe->owner) {

            SDL_Rect viewport;
            viewport.x = childAbsX;
            viewport.y = childAbsY;
            viewport.w = childBe->owner->m_rect.Width();
            viewport.h = childBe->owner->m_rect.Height();
            
            SDL_Rect oldViewport;
            SDL_RenderGetViewport(renderer, &oldViewport);

            SDL_RenderSetViewport(renderer, &viewport);
            childBe->owner->SendMessage(WM_PAINT, 0, 0); // Trigger standard paint path
            
            // RERUN: Draw standard Vertical Scrollbar if style is set
            if (childBe->owner->GetStyle() & WS_VSCROLL) {
                SCROLLINFO& si = childBe->owner->m_vertScrollInfo;
                int sw = 16; // Scrollbar width
                int h = childBe->owner->m_rect.Height();
                int w = childBe->owner->m_rect.Width();
                
                if (h > 0 && w > sw) {
                    SDL_Rect track = { w - sw, 0, sw, h };
                    // Track
                    SDL_SetRenderDrawColor(renderer, 212, 208, 200, 255);
                    SDL_RenderFillRect(renderer, &track);
                    // Border
                    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
                    SDL_RenderDrawLine(renderer, track.x, track.y, track.x, track.y + track.h);

                    // Thumb
                    if (si.nMax > si.nMin) { // Avoid divide by zero
                        int range = si.nMax - si.nMin + 1; // +1 because max is inclusive
                        int pageSize = (si.nPage > 0) ? si.nPage : 1;
                        int thumbHeight = (h * pageSize) / range;
                        if (thumbHeight < 20) thumbHeight = 20;
                        int trackSpace = h - thumbHeight;
                        int thumbY = (trackSpace * (si.nPos - si.nMin)) / (range - pageSize);
                        
                        SDL_Rect thumb = { w - sw + 2, thumbY, sw - 4, thumbHeight };
                        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
                        SDL_RenderFillRect(renderer, &thumb);
                    }
                }
            }

            PaintChildrenRecursive(childHwnd, renderer, childAbsX, childAbsY, depth + 1);

            SDL_RenderSetViewport(renderer, &oldViewport);

            // Mark as clean since we just painted it
            childBe->needsRepaint = false;
        }
    }
}

void CFrameWnd::OnPaint()
{
    WindowBackend* backend = backend_from_hwnd(m_hWnd);
    if (backend && backend->renderer && !DDPrimaryActive()) {
        // Clear background
        SDL_SetRenderDrawColor(backend->renderer, 240, 240, 240, 255);
        SDL_RenderClear(backend->renderer);
        
        // 1. Draw the main view (background)
        if (m_pActiveView)
            m_pActiveView->OnPaint();

        // Paint children recursively (toolbars, dialogs, panels)
        PaintChildrenRecursive(m_hWnd, backend->renderer, 0, 0, 0);

        // Reset viewport
        SDL_RenderSetViewport(backend->renderer, NULL);

        // Present the single, fully composed frame
        SDL_RenderPresent(backend->renderer);

        // RERUN: Clear dirty flags now that painting is complete.
        // This is the correct place to do this to avoid race conditions.
        backend->needsRepaint = false;
        backend->fullDirty = false;
        backend->dirtyRegions.clear();
    }
}

void CFrameWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    // RERUN: Check for scrollbar hits on the active view
    if (m_pActiveView && (m_pActiveView->GetStyle() & WS_VSCROLL)) {
        CRect r;
        m_pActiveView->GetClientRect(r);
        if (point.x >= r.right - 16) {
            // Scrollbar click logic
            SCROLLINFO& si = m_pActiveView->m_vertScrollInfo;
            int range = si.nMax - si.nMin + 1;
            int pageSize = (si.nPage > 0) ? si.nPage : 1;
            int h = r.Height();
            int thumbHeight = (h * pageSize) / range;
            if (thumbHeight < 20) thumbHeight = 20;
            int trackSpace = h - thumbHeight;
            int thumbY = (trackSpace * (si.nPos - si.nMin)) / (range - pageSize);

            int clickY = point.y;
            if (clickY < thumbY) {
                m_pActiveView->SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
            } else if (clickY > thumbY + thumbHeight) {
                m_pActiveView->SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
            } else {
                // Dragging logic would go here, for now jump
            }
            return;
        }
    }

    std::cout << "CFrameWnd::OnLButtonDown(" << point.x << "," << point.y << ")" << std::endl;
    if (m_pActiveView)
        m_pActiveView->OnLButtonDown(nFlags, point);
    else
        CWnd::OnLButtonDown(nFlags, point);
}

void CFrameWnd::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    WindowBackend* be = backend_from_hwnd(m_hWnd);
    if (be && be->renderer)
    {
        // Update the logical size to match the new window dimensions.
        // This ensures mouse coordinates in SDL events map 1:1 to the window client area.
        SDL_RenderSetLogicalSize(be->renderer, cx, cy);
    }
    // RERUN: Real MFC calls RecalcLayout on frame resize; the derived class
    // repositions docked toolbars and resizes the view around them. Sizing the
    // view to the full client here would overlay docked controls.
    RecalcLayout();
}

void CFrameWnd::OnHelp()
{
    std::cout << "CFrameWnd::OnHelp called" << std::endl;
}

void CFrameWnd::OnContextHelp()
{
    std::cout << "CFrameWnd::OnContextHelp called" << std::endl;
}

// COleClientItem
void COleClientItem::OnChange(int /*nCode*/, DWORD /*dwParam*/) {}
void COleClientItem::OnActivate() {}
void COleClientItem::OnDeactivate() {}
void COleClientItem::OnGetItemPosition(RECT* /*pRect*/) {}
BOOL COleClientItem::OnChangeItemPosition(const CRect& /*rectPos*/) {return TRUE;}
void COleClientItem::OnUpdate() {}
BOOL COleClientItem::OnDraw(CDC* /*pDC*/, CSize& /*rSize*/) { return FALSE; }
void COleClientItem::DoVerb(LONG /*iVerb*/, CWnd* /*pWnd*/, LPCRECT /*lpRect*/) {}
void COleClientItem::DoVerb(LONG iVerb, CWnd* pWnd) {DoVerb(iVerb, pWnd, nullptr);}
BOOL COleClientItem::CreateFromFile(LPCTSTR /*lpszFileName*/) { return FALSE; }
BOOL COleClientItem::CreateLinkFromFile(LPCTSTR /*lpszFileName*/) { return FALSE; }
BOOL COleClientItem::CreateFromClipboard() { return FALSE; }
BOOL COleClientItem::CreateFromData(COleDataObject* /*pDataObject*/) { return FALSE; }
BOOL COleClientItem::CopyToClipboard() { return FALSE; }
void COleClientItem::Serialize(CArchive& /*ar*/) {}
CView* COleClientItem::GetActiveView() { return nullptr; }
void COleClientItem::Close() {}
void COleClientItem::OnDeactivateUI(BOOL /*bUndoable*/) {}
DVASPECT COleClientItem::GetDrawAspect() const {return DVASPECT_CONTENT;}

// CWinApp
static CWinApp* g_pApp = nullptr;
CWinApp::CWinApp()
{
    g_pApp = this;
}
CWinApp::~CWinApp() {}
BOOL CWinApp::InitApplication()
{
    return TRUE;
}

BOOL GetWindowRect(HWND hWnd, LPRECT lpRect)
{
    if (!hWnd || !lpRect) return FALSE;

    CWnd* pWnd = CWnd::FromHandle(hWnd);
    if (pWnd) {
        pWnd->GetWindowRect(lpRect);
        return TRUE;
    }
    return FALSE;
}

int CWinApp::Run()
{
    while (!g_shouldQuit)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            
            DispatchMessage(&msg);
        }
        else
        {
            if (!OnIdle(0))
            {
                SDL_Delay(1);
            }
        }
    }

    return 0;
}
BOOL CWinApp::InitInstance() 
{
    LoadDialogTemplates(); // RERUN: Load RC file

	if (SDL_Init(SDL_INIT_VIDEO| SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0){
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return FALSE;
	}
	if (TTF_Init() == -1) {
		std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
		return FALSE;
	}

	// Swallow teardown-only BadWindow/BadDrawable (see MigXErrorHandler).
	// atexit is LIFO: this runs before SDL_Quit's own teardown, re-arming
	// the handler after anything (SDL, Mesa) has swapped it out.
	g_prevXErrorHandler = XSetErrorHandler(MigXErrorHandler);
	atexit(+[]{
		g_shouldQuit = true;
		g_prevXErrorHandler = XSetErrorHandler(MigXErrorHandler);
	});

    // After SDL_Init and TTF_Init
    if (!m_docTemplates.empty())
    {
        // SDI app → use the first template
        CSingleDocTemplate* pTemplate = m_docTemplates[0];

        // This creates the document, frame, view, SDL window, etc.
        CDocument* pDoc = pTemplate->OpenDocumentFile(nullptr);

        // Set main window pointer (MFC does this)
        m_pMainWnd = pTemplate->m_pFrame;
    }

    return TRUE; 
}
int CWinApp::ExitInstance() 
{ 
    return 0; 
}
void CWinApp::SetRegistryKey(const char*) {}
void CWinApp::AddDocTemplate(CSingleDocTemplate* pTemplate)
{
    m_docTemplates.push_back(pTemplate);
}
void CWinApp::RegisterShellFileTypes(BOOL /*bCompat*/) {}
void CWinApp::EnableShellOpen() {}
void CWinApp::ParseCommandLine(CCommandLineInfo& /*cmdInfo*/) {}
BOOL CWinApp::ProcessShellCommand(CCommandLineInfo&)
{
    if (m_docTemplates.empty())
        return FALSE;

    // Single-document interface: always use the first template
    CSingleDocTemplate* pTemplate = m_docTemplates[0];

    return pTemplate->OpenDocumentFile(nullptr) != nullptr;
}


/// Global functions Afx
CWinApp* AfxGetApp()
{
    return g_pApp;
}
CWnd* AfxGetMainWnd()
{
    return AfxGetApp()->m_pMainWnd;
}

// CDocument
void CDocument::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint) {}
void CDocument::AddView(CView* pView) {
    m_viewList.push_back(pView);
    pView->m_pDocument = this;
}
void CDocument::RemoveView(CView* pView) {
    m_viewList.erase(std::remove(m_viewList.begin(), m_viewList.end(), pView),
                     m_viewList.end());
    pView->m_pDocument = nullptr;
}


//CObject
CRuntimeClass CObject::classCObject
( 
    "CObject", 
    nullptr, // base class 
    nullptr // no dynamic creation 
);
CRuntimeClass* CObject::GetRuntimeClass() const
{
    return &classCObject;
}
BOOL CObject::IsKindOf(const CRuntimeClass* pClass) const
{
    const CRuntimeClass* pThis = GetRuntimeClass();
    while (pThis)
    {
        if (pThis == pClass)
            return TRUE;
        pThis = pThis->m_pBaseClass;
    }
    return FALSE;
}
// MFC diagnostic stubs 
void CObject::AssertValid() const {}
void CObject::Dump(CDumpContext&) const {}

// CSingleDocTemplate
CSingleDocTemplate::CSingleDocTemplate(UINT nIDResource,
                                       CRuntimeClass* pDocClass,
                                       CRuntimeClass* pFrameClass,
                                       CRuntimeClass* pViewClass)
    : m_nIDResource(nIDResource),
      m_pDocClass(pDocClass),
      m_pFrameClass(pFrameClass),
      m_pViewClass(pViewClass),
      m_pFrame(nullptr)
{
}
// Called by InitInstance() to register the template
void CSingleDocTemplate::SetContainerInfo(UINT /*nIDResource*/)
{
    // No OLE container support — intentionally empty
}
// In real MFC this registers the template with the app
void CSingleDocTemplate::AddDocTemplate()
{
    // No-op
}
// Some MFC apps call this to create the initial document
CDocument* CSingleDocTemplate::OpenDocumentFile(LPCTSTR /*lpszPathName*/)
{
    // 1. Create document
    CDocument* pDoc = (CDocument*)m_pDocClass->m_pfnCreateObject();
    if (!pDoc)
        return nullptr;

    // 2. Create frame window
    CFrameWnd* pFrame = (CFrameWnd*)m_pFrameClass->m_pfnCreateObject();
    if (!pFrame)
        return nullptr;

    // 3. Create the actual SDL window
    if (!pFrame->LoadFrame(m_nIDResource))
    {
        std::cout << "OpenDocumentFile: LoadFrame failed: " << SDL_GetError() << std::endl;
        delete pFrame;
        delete pDoc;
        return nullptr;
    }

    // 4. Create the view
    CView* pView = (CView*)m_pViewClass->m_pfnCreateObject();
    if (!pView)
        return nullptr;

    pView->m_pDocument = pDoc;
    pFrame->AttachView(pView);

    // Call the MFC initialization hook 
    pView->OnInitialUpdate();

    // 5. Set main window
    AfxGetApp()->m_pMainWnd = pFrame; //RERUN I have doubts
    m_pFrame = pFrame; //RERUN test

    return pDoc;
}

// Used by MFC to retrieve strings like "Document", "File", etc.
BOOL CSingleDocTemplate::GetDocumentString(CString& /*rString*/, enum DocStringIndex /*index*/) const
{
    return FALSE; // nothing to return
}


//CView
IMPLEMENT_DYNCREATE(CView, CWnd)
void CView::OnInitialUpdate()
{
    // Default implementation does nothing.
    // Mig Alley will override this in CMIGView.
}
void CView::OnDraw(CDC* /*pDC*/)
{
    // Default implementation does nothing.
    // CMIGView will override this to perform rendering.
}
void CView::HandleSDLEvent(const SDL_Event& /*e*/)
{
    // Default implementation does nothing.
    // CMIGView will override this for input handling.
}
void CView::OnPaint()
{
    CDC dc(this);   // your stub CDC
    OnDraw(&dc);
}


//CBitmap
BOOL CBitmap::LoadBitmap(UINT resourceID)
{
    // Convert resource ID → filename
    char filename[64];
    snprintf(filename, sizeof(filename), "%u.bmp", resourceID);

    std::cout << "[DEBUG] Loading bitmap: " << filename << "... ";

    SDL_Surface* surface = SDL_LoadBMP(filename);
    
    if (!surface) {
        std::cout << "FAILED. SDL Error: " << SDL_GetError() << std::endl;
        return FALSE;
    }

    std::cout << "SUCCESS (" << surface->w << "x" << surface->h << ")" << std::endl;
    // Store SDL surface pointer in m_hObject
    m_hObject = surface;

    // Fill BITMAP info
    info.bmWidth  = surface->w;
    info.bmHeight = surface->h;
    info.bmBits   = surface->pixels;
    info.bmPlanes = 1;
    info.bmBitsPixel = surface->format->BitsPerPixel;

    return TRUE;
}
int CBitmap::GetObject(int cbBuffer, void* lpvObject) const
{
    if (lpvObject && cbBuffer >= sizeof(BITMAP))
        *(BITMAP*)lpvObject = info;

    return sizeof(BITMAP);
}
BOOL CBitmap::GetBitmap(BITMAP* pBM) const
{
    if (!pBM) return FALSE;
    *pBM = info;
    return TRUE;
}
BOOL CBitmap::DeleteObject()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
        m_pRenderer = nullptr;
    }
    if (m_hObject)
    {
        SDL_FreeSurface((SDL_Surface*)m_hObject);
        m_hObject = nullptr;
    }
    return TRUE;
}

SDL_Texture* CBitmap::GetTexture(SDL_Renderer* renderer)
{
    if (!m_hObject) return nullptr;
    if (m_texture && m_pRenderer == renderer) return m_texture;
    
    if (m_texture) SDL_DestroyTexture(m_texture);
    
    m_texture = SDL_CreateTextureFromSurface(renderer, (SDL_Surface*)m_hObject);
    m_pRenderer = renderer;
    return m_texture;
}


// -----------------------------------------------------------------------------
// Polygon regions — artwork hit-testing (BoB polylist path, mapdlg).
// Regions are plain heap objects; the magic tag lets global DeleteObject
// verify the handle without colliding with other GDI object kinds.
// -----------------------------------------------------------------------------
namespace {
struct WinRgnImpl {
    uint32_t magic;
    int fillMode;
    std::vector<POINT> pts;
};
constexpr uint32_t kRgnMagic = 0x4E475257; // 'WRGN'

inline long is_left(const POINT& a, const POINT& b, int x, int y)
{
    return (long)(b.x - a.x) * (y - a.y) - (long)(x - a.x) * (b.y - a.y);
}

bool rgn_contains(const WinRgnImpl* r, int x, int y)
{
    const std::vector<POINT>& p = r->pts;
    const size_t n = p.size();
    if (n < 3) return false;

    if (r->fillMode == WINDING) {
        int wn = 0;
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            if (p[j].y <= y) {
                if (p[i].y > y && is_left(p[j], p[i], x, y) > 0) ++wn;
            } else {
                if (p[i].y <= y && is_left(p[j], p[i], x, y) < 0) --wn;
            }
        }
        return wn != 0;
    }
    // ALTERNATE — even-odd raycast
    bool inside = false;
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        if (((p[i].y > y) != (p[j].y > y)) &&
            (x < (double)(p[j].x - p[i].x) * (y - p[i].y) / (p[j].y - p[i].y) + p[i].x))
            inside = !inside;
    }
    return inside;
}
} // namespace

HRGN CreatePolygonRgn(const POINT* pts, int count, int fillMode)
{
    if (!pts || count < 3) return nullptr;
    WinRgnImpl* r = new WinRgnImpl;
    r->magic = kRgnMagic;
    r->fillMode = fillMode;
    r->pts.assign(pts, pts + count);
    return (HRGN)r;
}

HRGN CreateRectRgn(int x1, int y1, int x2, int y2)
{
    POINT pts[4] = { {x1,y1}, {x2,y1}, {x2,y2}, {x1,y2} };
    return CreatePolygonRgn(pts, 4, ALTERNATE);
}

BOOL PtInRegion(HRGN hrgn, int x, int y)
{
    if (!hrgn) return FALSE;
    const WinRgnImpl* r = (const WinRgnImpl*)hrgn;
    if (r->magic != kRgnMagic) return FALSE;
    return rgn_contains(r, x, y) ? TRUE : FALSE;
}

BOOL DeleteObject(HGDIOBJ obj)
{
    if (!obj) return FALSE;
    WinRgnImpl* r = (WinRgnImpl*)obj;
    if (r->magic != kRgnMagic) return FALSE;
    r->magic = 0;
    delete r;
    return TRUE;
}

BOOL CRgn::CreatePolygonRgn(const POINT* pts, int count, int fillMode)
{
    DeleteObject();
    m_hObject = ::CreatePolygonRgn(pts, count, fillMode);
    return m_hObject != nullptr;
}

BOOL CRgn::CreateRectRgn(int x1, int y1, int x2, int y2)
{
    DeleteObject();
    m_hObject = ::CreateRectRgn(x1, y1, x2, y2);
    return m_hObject != nullptr;
}

BOOL CRgn::PtInRegion(int x, int y) const
{
    return ::PtInRegion((HRGN)m_hObject, x, y);
}

BOOL CRgn::DeleteObject()
{
    if (!m_hObject) return FALSE;
    ::DeleteObject((HGDIOBJ)m_hObject);
    m_hObject = nullptr;
    m_bOwner = false;   // keep ~CGdiObject from touching the freed handle
    return TRUE;
}

int AfxLoadString(unsigned int id, char* buffer, unsigned int maxLen)
{
    if (!buffer || maxLen == 0) return 0;
    auto it = g_resStringsMap.find((int)id);
    if (it != g_resStringsMap.end()) {
        snprintf(buffer, maxLen, "%s", it->second.c_str());
        return (int)it->second.length();
    }
    buffer[0] = '\0';
    return 0;
}

void InvokeHelper(
    int dispID,
    int wFlags,
    int vtRet,
    void* pvRet,
    const BYTE* pszArgTypes,
    ...)
{
    std::cout << "InvokeHelper called with arguments: "
              << "dispID=" << dispID
              << ", wFlags=" << wFlags
              << ", vtRet=" << vtRet
              << std::endl;

    va_list args;
    va_start(args, pszArgTypes);

    const char* types = reinterpret_cast<const char*>(pszArgTypes);

    if (!types) {
        std::cout << "  (no type string)" << std::endl;
        va_end(args);
        return;
    }

    if (strcmp(types, "BSTR") == 0) {
        const char* s = va_arg(args, const char*);
        std::cout << "  BSTR argument: \"" << (s ? s : "(null)") << "\"" << std::endl;
    }
    else if (strcmp(types, "I4") == 0) {
        int v = va_arg(args, int);
        std::cout << "  I4 argument (int): " << v << std::endl;
    }
    else if (strcmp(types, "R4") == 0) {
        float v = static_cast<float>(va_arg(args, double)); // float promoted to double
        std::cout << "  R4 argument (float): " << v << std::endl;
    }
    else if (strcmp(types, "R8") == 0) {
        double v = va_arg(args, double);
        std::cout << "  R8 argument (double): " << v << std::endl;
    }
    else if (strcmp(types, "BOOL") == 0) {
        int v = va_arg(args, int); // promoted
        std::cout << "  BOOL argument: " << (v ? "true" : "false") << std::endl;
    }
    else if (strcmp(types, "VOID") == 0 || strcmp(types, "NONE") == 0) {
        std::cout << "  (no arguments)" << std::endl;
    }
    else {
        std::cout << "  Unknown arg type string: \"" << types << "\"" << std::endl;
    }

    va_end(args);
}

/////////////////////////////////////////////////////////////////////////////
// Display settings — BoB's twodpref enumerates real display modes to offer
// map resolutions. Backed by SDL: iModeNum walks SDL mode list,
// ENUM_CURRENT_SETTINGS returns the desktop mode. ChangeDisplaySettings is a
// no-op — the port runs windowed and never performs a real mode switch.

static void FillDevMode(DEVMODE* dm, const SDL_DisplayMode& sm)
{
    memset(dm, 0, sizeof(*dm));
    dm->dmSize = sizeof(*dm);
    dm->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    dm->dmBitsPerPel = (DWORD)SDL_BITSPERPIXEL(sm.format);
    dm->dmPelsWidth = (DWORD)sm.w;
    dm->dmPelsHeight = (DWORD)sm.h;
    dm->dmDisplayFrequency = (DWORD)sm.refresh_rate;
}

BOOL EnumDisplaySettings(const char* /*lpszDeviceName*/, DWORD iModeNum, DEVMODE* lpDevMode)
{
    if (!lpDevMode)
        return FALSE;
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        SDL_InitSubSystem(SDL_INIT_VIDEO);

    SDL_DisplayMode sm;
    if (iModeNum == ENUM_CURRENT_SETTINGS || iModeNum == ENUM_REGISTRY_SETTINGS)
    {
        if (SDL_GetDesktopDisplayMode(0, &sm) != 0)
            return FALSE;
        FillDevMode(lpDevMode, sm);
        return TRUE;
    }
    int n = SDL_GetNumDisplayModes(0);
    if (n <= 0 || (int)iModeNum >= n)
        return FALSE;
    if (SDL_GetDisplayMode(0, (int)iModeNum, &sm) != 0)
        return FALSE;
    FillDevMode(lpDevMode, sm);
    return TRUE;
}

LONG ChangeDisplaySettings(DEVMODE* lpDevMode, DWORD dwflags)
{
    // The game runs windowed under SDL: a real mode switch is never performed.
    // CDS_TEST validates the request — report success for any sane mode.
    if (!lpDevMode)
        return DISP_CHANGE_SUCCESSFUL; // nullptr = restore; nothing to restore
    if (lpDevMode->dmPelsWidth == 0 || lpDevMode->dmPelsHeight == 0)
        return DISP_CHANGE_FAILED;
    return DISP_CHANGE_SUCCESSFUL;
}

unsigned int SHAppBarMessage(DWORD /*dwMessage*/, PAPPBARDATA /*pData*/)
{
    return 0; // no auto-hide appbars in the SDL environment
}

char* _itoa(int value, char* str, int base)
{
    if (base == 10)
        sprintf(str, "%d", value);
    else if (base == 16)
        sprintf(str, "%x", value);
    else
    {
        char tmp[66];
        char* p = tmp + sizeof(tmp) - 1;
        unsigned int u = (unsigned int)value; // non-decimal bases are unsigned
        *p = '\0';
        if (u == 0)
            *--p = '0';
        while (u)
        {
            int d = u % base;
            *--p = (char)(d < 10 ? '0' + d : 'a' + d - 10);
            u /= base;
        }
        strcpy(str, p);
    }
    return str;
}

/////////////////////////////////////////////////////////////////////////////
// T118 wave-3 — BoB frontend surface impls

int GetDeviceCaps(HDC /*hdc*/, int nIndex)
{
    switch (nIndex)
    {
    case LOGPIXELSX:
    case LOGPIXELSY:
        return 96;
    case BITSPIXEL:
        return 32; // native surfaces are always >= High Color
    }
    return 0;
}

// Registry — hard requirement: no registry. Always fail so callers keep
// compiled-in defaults.
LONG RegOpenKeyEx(HKEY, const char*, DWORD, REGSAM, HKEY* out)
{
    if (out) *out = nullptr;
    return ERROR_FILE_NOT_FOUND;
}
LONG RegQueryValueEx(HKEY, const char*, DWORD*, DWORD*, unsigned char*, DWORD*)
{
    return ERROR_FILE_NOT_FOUND;
}
LONG RegSetValueEx(HKEY, const char*, DWORD, DWORD, const unsigned char*, DWORD)
{
    return ERROR_FILE_NOT_FOUND;
}
LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }

// Version resources — the T119 RC loader owns real resource data.
HRSRC  FindResource(HMODULE, const char*, const char*) { return nullptr; }
HGLOBAL LoadResource(HMODULE, HRSRC)                   { return nullptr; }
void*  LockResource(HGLOBAL)                           { return nullptr; }
DWORD  GlobalSize(HGLOBAL)                             { return 0; }
BOOL   FreeResource(HGLOBAL)                           { return TRUE; }

// Language-resource DLL — T119 maps this to the RC blob; NULL = not loaded.
HMODULE LoadLibraryA(const char*) { return nullptr; }
BOOL    FreeLibrary(HMODULE)      { return TRUE; }

static HINSTANCE g_resHandle = nullptr;
HINSTANCE AfxGetResourceHandle()          { return g_resHandle; }
void      AfxSetResourceHandle(HINSTANCE h) { g_resHandle = h; }

// Font enumeration — report success without invoking the callback; callers
// fall back to their default font paths.
int EnumFontFamiliesEx(HDC, LOGFONT* lpLogFont, FONTENUMPROC proc, long lParam, DWORD)
{
    // Report only the faces GetSystemFontPath resolves to an explicit target —
    // the games probe fallback lists ("Fusion Bold\n...\nArial") and must see
    // misses for names we don't have, else the first name "wins" wrongly.
    if (!proc) return 0;
    const char* face = lpLogFont ? lpLogFont->lfFaceName : nullptr;
    static const char* known[] = { "MS Sans Serif", "Helv", "Arial",
        "Arial Italic", "Times New Roman", "Courier New", "Intel" };
    bool found = !face || !*face;
    for (const char* k : known)
        if (face && strcmp(face, k) == 0) { found = true; break; }
    if (!found) return 1;
    ENUMLOGFONTEX elf{};
    if (lpLogFont) {
        elf.elfLogFont = *lpLogFont;
        strncpy(elf.elfFullName, face, sizeof(elf.elfFullName) - 1);
    }
    return proc(&elf, nullptr, TRUETYPE_FONTTYPE, lParam);
}

// Raw-handle GDI entry points used by BoB's map view. The SDL backend draws
// through WindowBackend surfaces; until the T120 artwork path lands these are
// safe no-ops.
int SetDIBitsToDevice(HDC, int, int, DWORD, DWORD, int, int, UINT, UINT,
                      const void*, const BITMAPINFO*, UINT) { return 0; }
HDC GetDC(HWND)              { return nullptr; }
int ReleaseDC(HWND, HDC)     { return 0; }

// winmm surface (timeGetTime/timeSetEvent/timeKillEvent/...) lives in
// H/timeapi.h — SDL-backed static-inline impls shared by mig and BoB.

UINT SetSystemPaletteUse(HDC, UINT) { return SYSPAL_NOSTATIC; }

/* weak: BoB's lib3d carries its own _i64toa — the strong game definition
   wins where both are linked; this is the fallback for everyone else. */
__attribute__((weak))
char* _i64toa(long long value, char* str, int base)
{
    if (!str) return str;
    char* p = str;
    bool neg = value < 0 && base == 10;
    unsigned long long v = neg ? (unsigned long long)(-value)
                               : (unsigned long long)value;
    char tmp[66]; int i = 0;
    do { unsigned d = (unsigned)(v % (unsigned)base);
         tmp[i++] = d < 10 ? char('0' + d) : char('a' + d - 10);
         v /= (unsigned)base; } while (v);
    if (neg) *p++ = '-';
    while (i) *p++ = tmp[--i];
    *p = 0;
    return str;
}

// DirectShow-era COM surface — nothing is ever instantiated.
HRESULT CoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID* ppv)
{
    if (ppv) *ppv = nullptr;
    return E_NOINTERFACE;
}

int MultiByteToWideChar(UINT codepage, DWORD, const char* src,
                        int srclen, wchar_t* dst, int dstlen)
{
    // CP_ACP only: narrow bytes widen 1:1 (BoB uses it for ASCII filenames).
    if (!src || !dst || dstlen <= 0) return 0;
    int n = srclen < 0 ? (int)strlen(src) + 1 : srclen;
    if (n > dstlen) n = dstlen;
    for (int i = 0; i < n; i++) dst[i] = (wchar_t)(unsigned char)src[i];
    (void)codepage;
    return n;
}

const GUID IID_IGraphBuilder = { 0x56a868a9, 0x0ad4, 0x11ce, {0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70} };
const GUID IID_IMediaControl = { 0x56a868b1, 0x0ad4, 0x11ce, {0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70} };
const GUID IID_IMediaEventEx = { 0x56a868c0, 0x0ad4, 0x11ce, {0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70} };
const GUID IID_IVideoWindow  = { 0x56a868b4, 0x0ad4, 0x11ce, {0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70} };
const GUID IID_IBasicAudio   = { 0x56a868b3, 0x0ad4, 0x11ce, {0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70} };
const GUID CLSID_FilterGraph = { 0xe436ebb3, 0x524f, 0x11ce, {0x9f,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70} };

BOOL CDC::ExtTextOut(int x, int y, UINT /*nOptions*/, LPCRECT /*lpRect*/,
                     LPCTSTR str, LPINT /*lpDxWidths*/)
{
    // Character-spacing variant of TextOut; spacing is ignored for now —
    // the frontend legibility cost is cosmetic.
    TextOut(x, y, str ? str : "", str ? (int)strlen(str) : 0);
    return TRUE;
}

// Frame/help machinery for BoB's CMainFrame
CWnd* CWnd::GetTopLevelParent()
{
    CWnd* p = this;
    while (p && p->GetParent())
        p = p->GetParent();
    return p;
}

void CWnd::SendMessageToDescendants(UINT message, WPARAM wParam, LPARAM lParam,
                                    BOOL bDeep, BOOL /*bOnlyPerm*/)
{
    for (auto& kv : m_children)
    {
        if (kv.second)
        {
            kv.second->SendMessage(message, wParam, lParam);
            if (bDeep)
                kv.second->SendMessageToDescendants(message, wParam, lParam, TRUE, FALSE);
        }
    }
}

// Mouse capture — single static owner, mirroring Win32's one-capture rule.
static HWND g_capture = nullptr;
HWND GetCapture()            { return g_capture; }
HWND SetCapture(HWND h)      { HWND old = g_capture; g_capture = h; return old; }
BOOL ReleaseCapture()        { g_capture = nullptr; return TRUE; }

// DIB upload — the T120 artwork pipeline owns real bitmap ingestion.
HBITMAP CreateDIBitmap(HDC, const BITMAPINFOHEADER*, DWORD, const void*,
                       const BITMAPINFO*, UINT)
{
    return nullptr;
}

int SetStretchBltMode(HDC, int) { return 0; }

BOOL CDC::Polygon(const POINT* lpPoints, int nCount)
{
    // Map-view arrowheads — draw the outline through the backend path.
    if (!lpPoints || nCount < 2 || !m_renderer)
        return FALSE;
    SDL_Point pts[64];
    int n = nCount < 63 ? nCount : 63;
    for (int i = 0; i < n; i++)
    {
        pts[i].x = (int)lpPoints[i].x;
        pts[i].y = (int)lpPoints[i].y;
    }
    pts[n] = pts[0];
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(m_renderer,
        (Uint8)(m_textColor & 0xFF), (Uint8)((m_textColor >> 8) & 0xFF),
        (Uint8)((m_textColor >> 16) & 0xFF), a);
    SDL_RenderDrawLines(m_renderer, pts, n + 1);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    return TRUE;
}
