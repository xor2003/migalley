#pragma once
#include <algorithm>
#include <deque>
#include <mutex>
#include <vector>
#include <string>

#include "WIN32_COMPAT.H"

#define SW_HIDE 0
#define SW_SHOW 5
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWNORMAL 1
#define SW_NORMAL     SW_SHOWNORMAL

#ifndef TRANSPARENT
#define TRANSPARENT 1
#endif
#ifndef OPAQUE
#define OPAQUE 2
#endif

#ifndef HICON
typedef void* HICON;
#endif
#ifndef HCURSOR
typedef void* HCURSOR;
#endif
#ifndef CB_ERR
#define CB_ERR (-1)
#endif


#define BN_CLICKED 0
#define LBN_SELCHANGE 1
#define LBN_DBLCLK 2
#define EN_UPDATE 0x0400

struct SdlKeyEvent {
    int dik_code;
    bool down;
};
extern std::deque<SdlKeyEvent> g_sdl_key_events;
extern std::mutex g_sdl_key_events_mutex;


///SDL
class CWnd;
struct WindowBackend {
    SDL_Window*   window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Surface* surface = nullptr;

    CWnd* owner = nullptr;       // The CWnd/CDialog that owns this backend
    CWnd* parent = nullptr;      // Parent CWnd (if any)

    uint32_t style = 0;          // WS_VISIBLE, WS_CHILD, etc.
    bool visible = false;        // Current visibility state

    int templateID = 0;          // Dialog template ID (optional but useful)

    std::vector<HWND> children;  // Child HWNDs
    bool isChild = false;        // Is this a child window sharing the SDL window?

    bool fullDirty = false;
    bool needsRepaint = false;
    std::vector<SDL_Rect> dirtyRegions;
};

extern std::unordered_map<HWND, WindowBackend> g_hwndRegistry;
HWND allocate_hwnd(); // declaration only
WindowBackend* backend_from_hwnd(HWND h);
///SDL

// MFC message‑map annotation compatibility
#define afx_msg
#ifndef AFXAPI
#define AFXAPI
#endif
// MFC event sink compatibility (no-op in SDL2 port)

class CCmdTarget;
typedef void (CCmdTarget::*AFX_PMSG)(void);

struct AFX_EVENTSINKMAP
{
    int         idFirst;
    int         idLast;
    int         eventid;
    AFX_PMSG    pfn;
    const char* params;   // we keep this only to match the shape; it's unused
};

#define DECLARE_EVENTSINK_MAP() \
    static const AFX_EVENTSINKMAP* PASCAL _GetSinkMap(); \
    virtual const AFX_EVENTSINKMAP* GetSinkMap() const;

#define BEGIN_EVENTSINK_MAP(theClass, baseClass) \
    const AFX_EVENTSINKMAP* theClass::GetSinkMap() const \
        { return theClass::_GetSinkMap(); } \
    const AFX_EVENTSINKMAP* PASCAL theClass::_GetSinkMap() \
        { static AFX_EVENTSINKMAP map[] = {

#define END_EVENTSINK_MAP() \
            { 0, 0, 0, nullptr } }; \
          return map; }

#define ON_EVENT(classname, id, eventid, handler, params) \
    { id, id, eventid, (AFX_PMSG)&classname::handler, params },

#define ON_EVENT_RANGE(classname, idFirst, idLast, eventid, handler, params) \
    { idFirst, idLast, eventid, (AFX_PMSG)&classname::handler, params },

struct AFX_EVENT
{
    enum { event = 0 };   // Mig Alley uses AFX_EVENT::event

    int m_eventType;
    int m_eventID;
    void* m_p1;
    void* m_p2;
    void* m_p3;

    AFX_EVENT(int eventType, int eventID, void* p1, void* p2, void* p3)
        : m_eventType(eventType),
          m_eventID(eventID),
          m_p1(p1),
          m_p2(p2),
          m_p3(p3)
    {}
};

#define CN_COMMAND           0
#define CN_UPDATE_COMMAND_UI 1
#define CN_EVENT             2


// --- Real Message Map Implementation ---

// Message map entry structure
struct AFX_MSGMAP_ENTRY {
    UINT nMessage;
    UINT nCode;
    UINT nID;
    UINT nLastID;
    AFX_PMSG pfn;
};

// Message map structure
struct AFX_MSGMAP {
    const AFX_MSGMAP* (PASCAL* pfnGetBaseMap)();
    const AFX_MSGMAP_ENTRY* lpEntries;
};

#define DECLARE_MESSAGE_MAP() \
private: \
    static const AFX_MSGMAP_ENTRY* _GetMessageEntries(); \
protected: \
    static const AFX_MSGMAP* PASCAL _GetBaseMessageMap(); \
    static const AFX_MSGMAP messageMap; \
    virtual const AFX_MSGMAP* GetMessageMap() const;

// Entries are built inside _GetMessageEntries() so that &OnPaint-style
// member addresses resolve in member-function scope. Clang rejects the
// unqualified form in out-of-class member-initializer context; a member
// function body is legal everywhere (MSVC-era sources rely on it).
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
    const AFX_MSGMAP* PASCAL theClass::_GetBaseMessageMap() \
        { return &baseClass::messageMap; } \
    const AFX_MSGMAP* theClass::GetMessageMap() const \
        { return &theClass::messageMap; } \
    const AFX_MSGMAP theClass::messageMap = \
        { &theClass::_GetBaseMessageMap, theClass::_GetMessageEntries() }; \
    const AFX_MSGMAP_ENTRY* theClass::_GetMessageEntries() { \
        typedef theClass thisClass; \
        static const AFX_MSGMAP_ENTRY entries[] = {

#define END_MESSAGE_MAP() \
            {0, 0, 0, 0, (AFX_PMSG)0 } \
        }; \
        return entries; \
    }

#define ON_COMMAND(id, memberFxn) \
    { WM_COMMAND, 0, (WORD)id, (WORD)id, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::memberFxn },

#define ON_UPDATE_COMMAND_UI(id, memberFxn) \
    { WM_COMMAND, CN_UPDATE_COMMAND_UI, (WORD)id, (WORD)id, (AFX_PMSG)(void (CCmdTarget::*)())(void (CCmdTarget::*)(CCmdUI*))&thisClass::memberFxn },

#define ON_MESSAGE(message, memberFxn) \
    { message, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(LRESULT (CWnd::*)(WPARAM, LPARAM))&thisClass::memberFxn },

#define ON_WM_PAINT() \
    { WM_PAINT, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::OnPaint },

#define ON_WM_CREATE() \
    { WM_CREATE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(int (CWnd::*)(LPCREATESTRUCT))&thisClass::OnCreate },

#define ON_WM_SIZE() \
    { WM_SIZE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, int, int))&thisClass::OnSize },

#define ON_WM_MOVE() \
    { WM_MOVE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(int, int))&thisClass::OnMove },

#define ON_WM_CLOSE() \
    { WM_CLOSE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::OnClose },

#define ON_WM_DESTROY() \
    { WM_DESTROY, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::OnDestroy },

#define ON_WM_ERASEBKGND() \
    { WM_ERASEBKGND, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(BOOL (CWnd::*)(CDC*))&thisClass::OnEraseBkgnd },

#define ON_WM_HSCROLL() \
    { WM_HSCROLL, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, UINT, CScrollBar*))&thisClass::OnHScroll },

#define ON_WM_VSCROLL() \
    { WM_VSCROLL, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, UINT, CScrollBar*))&thisClass::OnVScroll },

#define ON_WM_GETMINMAXINFO() \
    { WM_GETMINMAXINFO, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(MINMAXINFO*))&thisClass::OnGetMinMaxInfo },

#define ON_WM_ACTIVATE() \
    { WM_ACTIVATE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CWnd*, BOOL))&thisClass::OnActivate },

#define ON_WM_ACTIVATEAPP() \
    { WM_ACTIVATEAPP, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(BOOL, DWORD))&thisClass::OnActivateApp },

#define ON_WM_ENABLE() \
    { WM_ENABLE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(BOOL))&thisClass::OnEnable },

// --- ADDED ---
#define ON_WM_TIMER() \
    { WM_TIMER, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT))&thisClass::OnTimer },

#define ON_WM_LBUTTONDOWN() \
    { WM_LBUTTONDOWN, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnLButtonDown },

#define ON_WM_LBUTTONUP() \
    { WM_LBUTTONUP, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnLButtonUp },

#define ON_WM_MOUSEMOVE() \
    { WM_MOUSEMOVE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnMouseMove },

#define ON_WM_CONTEXTMENU() \
    { WM_CONTEXTMENU, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(CWnd*, CPoint))&thisClass::OnContextMenu },

#define ON_WM_MOUSEWHEEL() \
    { WM_MOUSEWHEEL, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(BOOL (CWnd::*)(UINT, short, CPoint))&thisClass::OnMouseWheel },

#define ON_WM_SHOWWINDOW() \
    { WM_SHOWWINDOW, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(BOOL, UINT))&thisClass::OnShowWindow },

#define ON_WM_SETCURSOR() \
    { WM_SETCURSOR, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(BOOL (CWnd::*)(CWnd*, UINT, UINT))&thisClass::OnSetCursor },

#define ON_WM_NCMOUSEMOVE() \
    { WM_NCMOUSEMOVE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnNcMouseMove },

#define ON_WM_NCLBUTTONDOWN() \
    { WM_NCLBUTTONDOWN, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnNcLButtonDown },

#define ON_WM_NCLBUTTONUP() \
    { WM_NCLBUTTONUP, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnNcLButtonUp },

#define ON_WM_KILLFOCUS() \
    { WM_KILLFOCUS, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(CWnd*))&thisClass::OnKillFocus },

#define ON_WM_INITMENU() \
    { WM_INITMENU, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(CMenu*))&thisClass::OnInitMenu },

#define ON_WM_CANCELMODE() \
    { WM_CANCELMODE, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::OnCancelMode },

#define ON_BN_CLICKED(id, memberFxn) \
    { WM_COMMAND, BN_CLICKED, (WORD)id, (WORD)id, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::memberFxn },

#define ON_WM_CAPTURECHANGED() \
    { WM_CAPTURECHANGED, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(CWnd*))&thisClass::OnCaptureChanged },

#define ON_WM_CHAR() \
    { WM_CHAR, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, UINT, UINT))&thisClass::OnChar },

#define ON_WM_RBUTTONDOWN() \
    { WM_RBUTTONDOWN, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnRButtonDown },

#define ON_WM_RBUTTONUP() \
    { WM_RBUTTONUP, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)(UINT, CPoint))&thisClass::OnRButtonUp },

#define ON_WM_CHARTOITEM() \
    { WM_CHARTOITEM, 0, 0, 0, (AFX_PMSG)(void (CCmdTarget::*)())(int (CWnd::*)(UINT, CListBox*, UINT))&thisClass::OnCharToItem },

#define ON_EN_UPDATE(id, memberFxn) \
    { WM_COMMAND, EN_UPDATE, (WORD)id, (WORD)id, (AFX_PMSG)(void (CCmdTarget::*)())(void (CWnd::*)())&thisClass::memberFxn },
// --- End Message Map ---

#define AFX_IDW_DOCKBAR_TOP        0xE81B
#define AFX_IDW_DOCKBAR_BOTTOM     0xE81C
#define AFX_IDW_DOCKBAR_LEFT       0xE81D
#define AFX_IDW_DOCKBAR_RIGHT      0xE81E

#define CBRS_ALIGN_LEFT     0x1000L
#define CBRS_ALIGN_TOP      0x2000L
#define CBRS_ALIGN_RIGHT    0x4000L
#define CBRS_ALIGN_BOTTOM   0x8000L
#define CBRS_ALIGN_ANY      (CBRS_ALIGN_LEFT | CBRS_ALIGN_TOP | CBRS_ALIGN_RIGHT | CBRS_ALIGN_BOTTOM)

#ifndef HELP_CONTEXT
#define HELP_CONTEXT        0x0001
#define HELP_QUIT           0x0002
#define HELP_CONTENTS       0x0003
#define HELP_CONTEXTPOPUP   0x0008
#define HELP_FINDER         0x000B
#endif

// --- Minimal COM VARIANT type codes ---
#define VT_EMPTY    0
#define VT_NULL     1
#define VT_I2       2
#define VT_I4       3
#define VT_R4       4
#define VT_R8       5
#define VT_CY       6
#define VT_DATE     7
#define VT_BSTR     8
#define VT_DISPATCH 9
#define VT_ERROR    10
#define VT_BOOL     11
#define VT_VARIANT  12
#define VT_UNKNOWN  13
#define VT_UI1      17
#define VT_UI2      18
#define VT_UI4      19

#define VTS_I2      "I2"
#define VTS_I4      "I4"
#define VTS_R4      "R4"
#define VTS_R8      "R8"
#define VTS_CY      "CY"
#define VTS_DATE    "DATE"
#define VTS_BSTR    "BSTR"
#define VTS_DISPATCH "DISPATCH"
#define VTS_ERROR   "ERROR"
#define VTS_BOOL    "BOOL"
#define VTS_VARIANT "VARIANT"
#define VTS_UNKNOWN "UNKNOWN"
#define VTS_UI1     "UI1"
#define VTS_UI2     "UI2"
#define VTS_UI4     "UI4"
#define VTS_INT     "INT"
#define VTS_UINT    "UINT"
#define VTS_PVARIANT "PVARIANT"
#define VTS_PDISPATCH "PDISPATCH"
#define VTS_PBSTR   "PBSTR"
#define VTS_PI2     "PI2"
#define VTS_PI4     "PI4"
#define VTS_PR4     "PR4"
#define VTS_PR8     "PR8"
#define VTS_PCY     "PCY"
#define VTS_PDATE   "PDATE"
#define VTS_PBOOL   "PBOOL"
#define VTS_PUI1    "PUI1"
#define VTS_PUI2    "PUI2"
#define VTS_PUI4    "PUI4"
#define VTS_PINT    "PINT"
#define VTS_PUINT   "PUINT"
#define VTS_NONE ""

#define DT_TOP              0x00000000
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_WORDBREAK        0x00000010
#define DT_SINGLELINE       0x00000020
#define DT_EXPANDTABS       0x00000040
#define DT_TABSTOP          0x00000080
#define DT_NOCLIP           0x00000100
#define DT_EXTERNALLEADING  0x00000200
#define DT_CALCRECT         0x00000400
#define DT_NOPREFIX         0x00000800
#define DT_INTERNAL         0x00001000

#define DISPID_BACKCOLOR        (-501)
#define DISPID_FORECOLOR        (-513)
#define DISPID_FONT             (-512)
#define DISPID_ENABLED          (-514)
#define DISPID_TABSTOP          (-516)
#define DISPID_CAPTION          (-518)
#define DISPID_TEXT             (-517)
#define DISPID_VISIBLE          (-522)
#define DISPID_READYSTATE       (-525)
#define DISPID_REFRESH          (-550)
#define DISPID_MOUSEPOINTER     (-521)
#define DISPID_MOUSEICON        (-522)   // sometimes -522 is shared
#define DISPID_BORDERSTYLE      (-504)
#define DISPID_APPEARANCE       (-520)
#define DISPID_BORDERWIDTH      (-503)
#define DISPID_HWND             (-515)

#define DISPATCH_METHOD 1

#ifdef _DEBUG
    #define TRACE0(sz)               printf("%s\n", sz)
    #define TRACE1(sz, p1)           printf(sz "\n", p1)
    #define TRACE2(sz, p1, p2)       printf(sz "\n", p1, p2)
    #define TRACE3(sz, p1, p2, p3)   printf(sz "\n", p1, p2, p3)
#else
    #define TRACE0(sz)
    #define TRACE1(sz, p1)
    #define TRACE2(sz, p1, p2)
    #define TRACE3(sz, p1, p2, p3)
#endif

#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200
#define SWP_NOSENDCHANGING  0x0400

#define RDW_INVALIDATE          0x0001
#define RDW_INTERNALPAINT       0x0002
#define RDW_ERASE               0x0004
#define RDW_VALIDATE            0x0008
#define RDW_NOINTERNALPAINT     0x0010
#define RDW_NOERASE             0x0020
#define RDW_NOCHILDREN          0x0040
#define RDW_ALLCHILDREN         0x0080
#define RDW_UPDATENOW           0x0100
#define RDW_ERASENOW            0x0200
#define RDW_FRAME               0x0400
#define RDW_NOFRAME             0x0800

// ---------------------------------------------------------------------------
// Minimal COM interface stubs (enough for MFC automation wrappers)
// ---------------------------------------------------------------------------
struct IDispatch : public IUnknown
{
    virtual ~IDispatch() {}
};
typedef IDispatch* LPDISPATCH;

enum {
    adjustBorder = 0,
    adjustOutside = 1
};

// MFC stubs
class CCreateContext {};
class CFile {};
struct AFX_CMDHANDLERINFO {};

// COM stubs
using BSTR = wchar_t*;

// OLE color is just a DWORD in MFC
using OLE_COLOR = unsigned long;

struct CREATESTRUCT {
    void* lpCreateParams;
    void* hInstance;
    void* hMenu;
    void* hwndParent;
    int   cy;
    int   cx;
    int   y;
    int   x;
    long  style;
    const char* lpszName;
    const char* lpszClass;
    unsigned long dwExStyle;
};

using LPCREATESTRUCT = CREATESTRUCT*;

typedef long DISPID;

struct VARIANT {
    unsigned short vt;   // VARTYPE
    unsigned short wReserved1;
    unsigned short wReserved2;
    unsigned short wReserved3;
    void*          pvRecord; // unused
};

// Posix version of the CEvent class.
class CEvent {
public:
    // Constructor: matches MFC style
    CEvent(bool manualReset = false, bool initialState = false, const char* lpName = nullptr, void* lpEventAttributes = nullptr);
    ~CEvent();

    // Delegate to global SetEvent
    void SetEvent();

    // Delegate to global ResetEvent
    void ResetEvent();

    // Delegate to global WaitForSingleObject
    int WaitForSingleObject(int timeout = INFINITE);

    // MFC-style Lock() wrapper
    bool Lock(int timeout = INFINITE);

    // Allow implicit conversion to HANDLE (like MFC)
    operator HANDLE() const;

private:
    HANDLE hEvent;
};


class CDumpContext
{
public:
    void operator<<(const char*) {}
};

// --- Minimal runtime class stub ---
class CObject;
class CRuntimeClass
{
public:
    const char* m_lpszClassName;
    const CRuntimeClass* m_pBaseClass;
    CObject* (*m_pfnCreateObject)();

    CRuntimeClass(const char* name,
                  CRuntimeClass* base,
                  CObject* (*createFn)())
        : m_lpszClassName(name),
          m_pBaseClass(base),
          m_pfnCreateObject(createFn)
    {}
};

class CObject
{
public:
    static CRuntimeClass classCObject;

    virtual CRuntimeClass* GetRuntimeClass() const;
    virtual BOOL IsKindOf(const CRuntimeClass* pClass) const;

    virtual void AssertValid() const;
    virtual void Dump(CDumpContext&) const;
};

// -----------------------------------------------------------------------------
// CPoint — minimal MFC-compatible point type
// -----------------------------------------------------------------------------
class CPoint : public POINT {
public:
    CPoint() : POINT{0, 0} {}
    CPoint(LONG X, LONG Y) : POINT{X, Y} {}
    CPoint(const POINT& p) : POINT{p.x, p.y} {}

    // MFC-compatible arithmetic
    CPoint operator+(const CPoint& other) const {
        return CPoint(x + other.x, y + other.y);
    }

    CPoint operator-(const CPoint& other) const {
        return CPoint(x - other.x, y - other.y);
    }

    CPoint& operator+=(const CPoint& other) {
        x += other.x; y += other.y; return *this;
    }

    CPoint& operator-=(const CPoint& other) {
        x -= other.x; y -= other.y; return *this;
    }

    void Offset(LONG dx, LONG dy) {
        x += dx;
        y += dy;
    }
};


// -----------------------------------------------------------------------------
// CSize — minimal MFC-compatible size type
// -----------------------------------------------------------------------------
class CSize {
public:
    int cx;
    int cy;

    CSize() : cx(0), cy(0) {}
    CSize(int x, int y) : cx(x), cy(y) {}

    CSize(const CPoint& p) : cx(p.x), cy(p.y) {}

    // Arithmetic helpers (MFC-compatible)
    CSize& operator+=(const CSize& s) {
        cx += s.cx;
        cy += s.cy;
        return *this;
    }

    CSize& operator-=(const CSize& s) {
        cx -= s.cx;
        cy -= s.cy;
        return *this;
    }
};

// -----------------------------------------------------------------------------
// CRect — minimal MFC-compatible rectangle type
// -----------------------------------------------------------------------------
class CRect : public tagRECT {
public:

    CRect()
        : tagRECT{0,0,0,0} {}

    CRect(int l, int t, int r, int b)
        : tagRECT{l, t, r, b} {}

    CRect(const CPoint& tl, const CPoint& br)
        : tagRECT{tl.x, tl.y, br.x, br.y} {}

    CRect(const RECT& src)
        : tagRECT{src.left, src.top, src.right, src.bottom} {}

    CRect& operator=(const RECT& src)
    {
        left = src.left; top = src.top; right = src.right; bottom = src.bottom;
        return *this;
    }

    // Overloads
    CRect& operator+=(const CPoint& p)
    {
        left   += p.x;
        right  += p.x;
        top    += p.y;
        bottom += p.y;
        return *this;
    }

    CRect& operator-=(const CPoint& p)
    {
        left   -= p.x;
        right  -= p.x;
        top    -= p.y;
        bottom -= p.y;
        return *this;
    }

    CRect& operator+=(const CSize& s)
    {
        left   += s.cx;
        right  += s.cx;
        top    += s.cy;
        bottom += s.cy;
        return *this;
    }

    CRect& operator-=(const CSize& s)
    {
        left   -= s.cx;
        right  -= s.cx;
        top    -= s.cy;
        bottom -= s.cy;
        return *this;
    }

    bool operator==(const CRect& other) const
    {
        return left   == other.left &&
            top    == other.top  &&
            right  == other.right &&
            bottom == other.bottom;
    }

    bool operator!=(const CRect& other) const
    {
        return !(*this == other);
    }

    CRect operator+(const CPoint& p) const
    {
        return CRect(left + p.x, top + p.y,
                    right + p.x, bottom + p.y);
    }

    CRect operator-(const CPoint& p) const
    {
        return CRect(left - p.x, top - p.y,
                    right - p.x, bottom - p.y);
    }


    // --- Basic geometry ---
    int Width() const  { return right - left; }
    int Height() const { return bottom - top; }

    void SetRect(int l, int t, int r, int b) {
        left = l; top = t; right = r; bottom = b;
    }

    void OffsetRect(int dx, int dy) {
        left += dx; right += dx;
        top += dy; bottom += dy;
    }

    bool PtInRect(const CPoint& p) const {
        return (p.x >= left && p.x < right &&
                p.y >= top  && p.y < bottom);
    }

    void InflateRect(int dx, int dy) {
        left -= dx; right += dx;
        top -= dy; bottom += dy;
    }

    void DeflateRect(int dx, int dy) {
        left += dx; right -= dx;
        top += dy; bottom -= dy;
    }

    // --- MFC convenience helpers Rowan code expects ---
    CPoint TopLeft() const     { return CPoint(left, top); }
    CPoint BottomRight() const { return CPoint(right, bottom); }

    CPoint CenterPoint() const
    {
        return CPoint((left + right) / 2, (top + bottom) / 2);
    }

    void SetRectEmpty() {
        left = top = right = bottom = 0;
    }

    bool IsRectEmpty() const {
        return (left == right) || (top == bottom);
    }

    bool IsRectNull() const {
        return left == 0 && top == 0 && right == 0 && bottom == 0;
    }

    void NormalizeRect() {
        if (left > right)   std::swap(left, right);
        if (top > bottom)   std::swap(top, bottom);
    }

    BOOL IntersectRect(const RECT* src1, const RECT* src2)
    {
        if (!src1 || !src2)
            return FALSE;

        left   = std::max(src1->left,   src2->left);
        top    = std::max(src1->top,    src2->top);
        right  = std::min(src1->right,  src2->right);
        bottom = std::min(src1->bottom, src2->bottom);

        return (right > left) && (bottom > top);
    }

    BOOL IntersectRect(const CRect& r1, const CRect& r2)
    {
        return IntersectRect((const RECT*)&r1, (const RECT*)&r2);
    }
};


#define DECLARE_DYNCREATE(class_name) \
public: \
    static CObject* CreateObject(); \
    static CRuntimeClass class##class_name; \
    virtual CRuntimeClass* GetRuntimeClass() const override;


#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
    CObject* class_name::CreateObject() { return new class_name; } \
    CRuntimeClass class_name::class##class_name( \
        #class_name, \
        RUNTIME_CLASS(base_class_name), \
        &class_name::CreateObject \
    ); \
    CRuntimeClass* class_name::GetRuntimeClass() const { \
        return &class_name::class##class_name; \
    }


#define RUNTIME_CLASS(class_name) \
    (&class_name::class##class_name)

#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
    CRuntimeClass class_name::class##class_name( \
        #class_name, \
        &base_class_name::class##base_class_name, \
        reinterpret_cast<CObject*(*)()>(pfnNew) \
    ); \
    CRuntimeClass* class_name::GetRuntimeClass() const { \
        return &class_name::class##class_name; \
    }


#define DECLARE_DYNAMIC(class_name) \
public: \
    static CRuntimeClass class##class_name; \
    virtual CRuntimeClass* GetRuntimeClass() const override;


#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
    CRuntimeClass class_name::class##class_name( \
        #class_name, \
        &base_class_name::class##base_class_name, \
        nullptr \
    ); \
    CRuntimeClass* class_name::GetRuntimeClass() const { \
        return &class_name::class##class_name; \
    }


class CGdiObject : public CObject
{
    DECLARE_DYNAMIC(CGdiObject)
public:
    void* m_hObject = nullptr;
    bool  m_bOwner  = false;

    virtual ~CGdiObject()
    {
        if (m_bOwner)
            DeleteObject();
    }

    bool DeleteObject()
    {
        if (!m_hObject)
            return false;

        _BRUSH* brush = static_cast<_BRUSH*>(m_hObject);

        if (!brush->isStock)
            delete brush;

        m_hObject = nullptr;
        return true;
    }
};

class CFont : public CGdiObject
{
    DECLARE_DYNAMIC(CFont)

public:
    LOGFONT m_lf{}; // store something minimal

    CFont();
    virtual ~CFont();

    // MFC API surface — all no-ops
    BOOL CreateFontIndirect(const LOGFONT* lpLogFont);
    BOOL CreateFont(int nHeight, int nWidth, int nEscapement,
                    int nOrientation, int nWeight, BYTE bItalic,
                    BYTE bUnderline, BYTE cStrikeOut, BYTE nCharSet,
                    BYTE nOutPrecision, BYTE nClipPrecision,
                    BYTE nQuality, BYTE nPitchAndFamily,
                    const char* lpszFacename);

    // In MFC this returns an HFONT; here it's just a placeholder
    void* GetSafeHandle() const { return m_hObject; }

    // MFC sometimes calls this when selecting fonts into a DC
    operator void*() const { return m_hObject; }

    virtual BOOL GetLogFont(LOGFONT* pLogFont) const 
    { 
        if (!pLogFont) 
            return FALSE; 
            
        *pLogFont = m_lf; 
        // copy stored values 
        return TRUE; 
    }
};

// CCmdTarget — placeholder for MFC command target class
class CCmdTarget : public CObject
{
public:
    DECLARE_DYNCREATE(CCmdTarget)DECLARE_MESSAGE_MAP() public:
    // MFC has a virtual destructor here; optional but harmless
    virtual ~CCmdTarget() {}
    virtual const AFX_EVENTSINKMAP* GetSinkMap() const { return nullptr; }
};

// -----------------------------------------------------------------------------
// CPen — minimal MFC‑compatible stub
// -----------------------------------------------------------------------------
class CPen : public CGdiObject
{
public:
    int     m_style   = 0;
    int     m_width   = 0;
    COLORREF m_color  = 0;

    CPen() = default;

    CPen(int style, int width, COLORREF color)
        : m_style(style), m_width(width), m_color(color)
    {}

    // MFC exposes GetSafeHandle(), but Rowan never uses it
    void* GetSafeHandle() const { return nullptr; }

    BOOL CreatePen(int style, int width, COLORREF color)
    {
        m_style = style;
        m_width = width;
        m_color = color;
        return TRUE;
    }
};

// -----------------------------------------------------------------------------
// CDC — minimal MFC compatibility stub
// -----------------------------------------------------------------------------
class CPen;
class CBitmap;
class CBrush;
class CDC {
private:
    COLORREF m_bkColor = RGB(255,255,255); // or any default
    COLORREF m_textColor = RGB(0,0,0);
    UINT m_textAlign = TA_LEFT | TA_TOP;
    CPoint m_currentPos;
    CPen*  m_pCurrentPen = nullptr;
    CBrush* m_pCurrentBrush = nullptr;
    CFont* m_pCurrentFont = nullptr;
    WindowBackend* m_backend = nullptr;
    int m_bkMode = OPAQUE;
public:
    HDC m_hDC = nullptr;
    CBitmap* m_selectedBitmap = nullptr;
     SDL_Surface* m_targetSurface = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    CDC() = default; 
    explicit CDC(HDC hdc) : m_hDC(hdc) {}
    CDC(CWnd* wnd);

    // No-op drawing methods for compatibility
    void MoveTo(int, int);
    void MoveTo(const CPoint& pt);
    void MoveTo(const POINT& pt);
    void LineTo(int, int);
    void LineTo(const CPoint& pt);
    void LineTo(const POINT& pt);
    void Rectangle(int, int, int, int);
    void Ellipse(int, int, int, int);
    CBitmap* SelectObject(CBitmap* pBitmap);
    void SetTextColor(unsigned int);
    int SetBkMode(int);
    void TextOut(int, int, const char*, int);
    void TextOut(int /*x*/, int /*y*/, const CString& /*str*/);
    int DrawText(const CString& str, const CRect& rect, UINT nFormat);
    void FillSolidRect(const CRect& /*rect*/, unsigned int /*color*/);
    void FillSolidRect(int /*x*/, int /*y*/, int /*cx*/, int /*cy*/, unsigned int /*color*/);
    BOOL FillRect(const CRect& rect, HBRUSH hBrush);
    void Draw3dRect(int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight);
    void Draw3dRect(const CRect& rect, COLORREF clrTopLeft, COLORREF clrBottomRight);
    BOOL CreateCompatibleDC(CDC* pDC);
    void SetMapMode(int /*mode*/);
    CPen* SelectObject(CPen* pen);
    CBrush* SelectObject(CBrush* pBrush);
    int GetBoundsRect(RECT* /*pRect*/, unsigned int /*flags*/);
    HDC GetSafeHdc() const;
    COLORREF SetBkColor(COLORREF color);
    CSize GetTextExtent(const CString& str) const;
    CSize GetTextExtent(LPCTSTR text) const;

    CObject* SelectStockObject(int object); //defined in cpp
    virtual UINT SetTextAlign(UINT align);
    virtual CFont* SelectObject(CFont* pFont);
    virtual CFont* GetCurrentFont() const;
    BOOL BitBlt(int x, int y, int cx, int cy, CDC* srcDC, int srcX, int srcY, DWORD /*rop*/);
    int SetDIBitsToDevice(int xDest, int yDest, DWORD w, DWORD h, int xSrc, int ySrc, UINT uStartScan, UINT cScanLines, const void *lpvBits, const BITMAPINFO *lpbmi, UINT fuColorUse);
};

class CScrollBar {};
class COleControlSite {};

// -----------------------------------------------------------------------------
// CWnd — placeholder for MFC window class
// -----------------------------------------------------------------------------
#include <unordered_map>
class CMenu;
class CListBox;
class CWnd : public CCmdTarget{
protected:
    std::unordered_map<int, CWnd*> m_children;
    UINT m_nIDHelp = 0;
    DWORD m_dwStyle = 0;
    DWORD m_dwExStyle = 0; // extended window style storage
    COleControlSite* m_pCtrlSite = nullptr;
    std::string m_windowText; // RERUN: Store window text for child controls
    CFont* m_pFont = nullptr; // RERUN: Store current font
public:
    DECLARE_DYNCREATE(CWnd)DECLARE_MESSAGE_MAP() public: CRect m_rect; CWnd* m_pParent = nullptr; HWND m_hWnd = nullptr; int m_nID = 0; // control ID, like in real MFC 
public:
    CWnd() = default;
    virtual ~CWnd() {}

    // Non-client mouse handlers (MFC stubs)
    virtual void OnNcMouseMove(UINT /*nHitTest*/, CPoint /*point*/) {}
    virtual void OnNcLButtonDown(UINT /*nHitTest*/, CPoint /*point*/) {}
    virtual void OnNcLButtonUp(UINT /*nHitTest*/, CPoint /*point*/) {}
    virtual void OnNcRButtonDown(UINT /*nHitTest*/, CPoint /*point*/) {}
    virtual void OnNcRButtonUp(UINT /*nHitTest*/, CPoint /*point*/) {}
    virtual void OnLButtonDown(UINT, CPoint);
    virtual void OnLButtonUp(UINT, CPoint);
    virtual void OnLButtonDblClk(UINT, CPoint) {}
    virtual BOOL GetControlInfo(int id, CRect& rect, DWORD& style) { return FALSE; }

    virtual void OnRButtonDown(UINT, CPoint) {}
    virtual void OnRButtonUp(UINT, CPoint) {}

    virtual void OnMButtonDown(UINT, CPoint) {}
    virtual void OnMButtonUp(UINT, CPoint) {}

    virtual void OnMouseMove(UINT, CPoint) {}
    virtual void OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/) {}
    
    virtual BOOL EnableWindow(BOOL /*bEnable*/ = TRUE);
    virtual BOOL IsWindowEnabled() const;
    virtual BOOL IsWindowVisible() const;

    virtual void RedrawWindow(const RECT* lpRectUpdate = nullptr, void* pRgnUpdate = nullptr, unsigned int flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

    virtual void OnShowWindow(BOOL bShow, UINT nStatus) {}
    virtual BOOL OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
    {
        // MFC default returns FALSE to allow cursor to be set by parent or system
        return FALSE;
    }

    virtual void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
    {
        // No-op in Linux port
    }
    virtual void OnMove(int /*x*/, int /*y*/)
    {
        // No-op in Linux port
    }

    // These exist only so code compiles; they do nothing.
    virtual BOOL ShowWindow(int);
    virtual BOOL DestroyWindow();
    virtual void SetWindowText(const char*);
    virtual void GetWindowText(char*, int) const;
    virtual void GetWindowText(CString&) const;
    virtual BOOL ScrollWindow(int dx, int dy, const RECT* prcScroll = nullptr, const RECT* prcClip = nullptr);
    bool CreateControl(const CLSID&, LPCTSTR, DWORD, const RECT&, CWnd*, UINT, CFile* = nullptr, BOOL = FALSE, BSTR = nullptr);
    // Stub timer: return a fake timer ID
    UINT_PTR SetTimer(UINT id, UINT elapse, void* callback = nullptr);
    void KillTimer(UINT id);

    void GetClientRect(CRect& r);
    void GetClientRect(LPRECT /*lpRect*/) const;

    CWnd* GetParent() const;
    void SetParent(CWnd* p);
    virtual void BringWindowToTop();
    virtual void Invalidate();

    virtual BOOL PreTranslateMessage(MSG* /*pMsg*/);
    virtual BOOL OnCmdMsg(UINT /*nID*/, int /*nCode*/, void* /*pExtra*/, AFX_CMDHANDLERINFO* /*pHandlerInfo*/);

    virtual void OnEnable(BOOL) {}
    virtual void OnClose();
    virtual void OnDestroy();

    virtual int OnCreate(LPCREATESTRUCT /*lpCreateStruct*/);
    virtual BOOL PreCreateWindow(CREATESTRUCT& /*cs*/);
    void GetWindowRect(LPRECT lpRect) const;
    virtual void MoveWindow(int x, int y, int w, int h);
    virtual void MoveWindow(const CRect& r, BOOL repaint = TRUE);
    virtual void ScreenToClient(POINT* pt) const;
    virtual void ScreenToClient(LPRECT r) const;
    virtual void ClientToScreen(LPRECT r) const;
    virtual void ClientToScreen(POINT* pt) const;
    virtual LRESULT SendMessage(UINT, WPARAM = 0, LPARAM = 0);
    virtual LRESULT SendMessageToDescendants(UINT, WPARAM = 0, LPARAM = 0);

    virtual CWnd* GetTopWindow() const;
    virtual CWnd* GetNextWindow(unsigned int nDirection = 0) const;
    virtual int GetDlgCtrlID() const;
    virtual LRESULT OnCommandHelp(WPARAM /*wParam*/, LPARAM /*lParam*/);
    virtual LRESULT MSG2_OnCommandHelp(int a, int b) { return OnCommandHelp(a, b); }
    BOOL SubclassWindow(HWND hWnd);

    // --- ActiveX-style property stubs ---
    virtual void GetProperty(DISPID dispid, unsigned short vt, void* pvResult);
    virtual void SetProperty(DISPID dispid, unsigned short vt, long value);
    virtual void SetProperty(DISPID dispid, unsigned short vt, const void* pvValue);
    virtual CWnd* SetCapture();
    virtual void ReleaseCapture();
    virtual void PostNcDestroy();
    virtual void OnGetMinMaxInfo(MINMAXINFO* /*lpMMI*/);

    void AddChild(CWnd* child, int id)
    {
        if (!child) return;
        child->m_nID = id;
        m_children[id] = child;
    }

    virtual CWnd* GetDlgItem(int id) const;
    virtual void GetDlgItem(int id, HWND* phWnd) const;

    HWND GetHwnd() const { return m_hWnd; } 
    HWND GetSafeHwnd() const { return m_hWnd; }
    static CWnd* FromHandle(HWND hWnd);

    virtual void MapDialogRect(LPRECT /*lpRect*/) const;
    virtual CDC* BeginPaint(PAINTSTRUCT* /*ps*/);
    virtual void EndPaint(PAINTSTRUCT* /*ps*/);
    virtual LRESULT WindowProc(UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/);

    void ModifyStyle(DWORD remove, DWORD add);
    virtual BOOL SetWindowPos(const CWnd* /*pWndInsertAfter*/, int x, int y, int cx, int cy, UINT /*nFlags*/);

    virtual void OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/) {}
    virtual void OnVScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/) {}
    virtual BOOL OnMouseWheel(UINT /*nFlags*/, short /*zDelta*/, CPoint /*pt*/) { return FALSE; }
    virtual void OnActivate(UINT /*nState*/, CWnd* /*pWndOther*/, BOOL /*bMinimized*/) {}
    virtual void OnActivateApp(BOOL /*bActive*/, DWORD /*dwThreadID*/) {}

    DWORD GetStyle() const { return m_dwStyle; }
    void SetMenu(HMENU /*hMenu*/);
    CDC* GetDC();
    void ReleaseDC(CDC* /*pDC*/);
    void UpdateWindow();
    void DragAcceptFiles();
    virtual void SetFocus();
    static CWnd* GetFocus();

    //virtual HWND GetSafeHwnd() const { return nullptr; }
    BOOL InvalidateRect(const CRect* lpRect, BOOL bErase = TRUE);
    BOOL InvalidateRect(const CRect& rect, BOOL bErase = TRUE);
    BOOL IntersectRect(RECT* dst, const RECT* src1, const RECT* src2){return ::IntersectRect(dst, src1, src2);}
    virtual void OnTimer(UINT nIDEvent) {}
    BOOL PostMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL PostMessage(UINT msg);

    virtual void OnKillFocus(CWnd* pNewWnd);
    virtual BOOL IsZoomed() const;
    virtual void OnCancelMode();
    virtual void OnInitMenu(CMenu* pMenu);
    virtual BOOL Create(
        LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle,
        const RECT& rect,
        CWnd* pParentWnd,
        UINT nID,
        CCreateContext* pContext = nullptr
    );

    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
    virtual BOOL OnAmbientProperty(COleControlSite* pSite, DISPID dispid, VARIANT* pvar);
    virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnFinalRelease();
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual void OnSetFont(CFont* pFont);

    // RERUN: Add virtual function to get the target for events.
    virtual CWnd* GetEventParent();
    // RERUN: Generic event firing mechanism to replace ActiveX event sinks.
    void FireEvent(int eventID, ...);
    virtual void PreSubclassWindow();
    virtual void WinHelp(DWORD dwData, UINT nCmd);
    virtual void OnCaptureChanged(CWnd* pWnd);
    virtual void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    virtual int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
    virtual void ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0);
    void SetLimitText(int);
    void SetReadOnly(BOOL);
    virtual void OnPaint();

    virtual int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE);
    virtual int GetScrollPos(int nBar) const;
    virtual int SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE);
    virtual BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL);
    SCROLLINFO m_vertScrollInfo = {sizeof(SCROLLINFO), 0};
};

class CRCombo;
class CListBox : public CWnd
{
    DECLARE_DYNAMIC(CListBox)
    DECLARE_MESSAGE_MAP()
    std::vector<std::string> m_items;
    int m_curSel = -1;
    int m_hotSel = -1; // RERUN: Add hot-tracking index
    mutable int m_itemHeight = -1; // RERUN: Cache item height to ensure consistency
    int m_topIndex = 0; // RERUN: Scroll position
public:
    CRCombo* m_pComboOwner = nullptr; // RERUN: Add owner pointer for dropdowns

public:
    CListBox() = default;
    virtual ~CListBox() = default;

    // --- Core MFC API stubs ---

    virtual int AddString(LPCTSTR lpszItem);
    virtual int InsertString(int nIndex, LPCTSTR lpszItem);
    virtual void ResetContent();
    virtual int GetCount() const;
    virtual int GetCurSel() const;
    virtual int SetCurSel(int nSelect);
    virtual int GetText(int nIndex, LPTSTR lpszBuffer) const;
    virtual int GetTextLen(int nIndex) const;
    virtual int DeleteString(int nIndex);
    int GetItemHeight() const;
    virtual int FindString(int nStartAfter, LPCTSTR lpszItem) const;
    virtual int FindStringExact(int nStartAfter, LPCTSTR lpszItem) const;

    afx_msg void OnClose();
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    virtual void OnSetFont(CFont* pFont) override;
    // --- Message handlers used by RowanUI ---

    virtual int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
    virtual void OnLButtonDown(UINT nFlags, CPoint point);
    virtual void OnLButtonUp(UINT nFlags, CPoint point);
    virtual void OnMouseMove(UINT nFlags, CPoint point) override;
    virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) override;
    virtual void OnSelChange();
    virtual void OnDblClk();
    virtual void OnPaint() override;
    virtual void OnCaptureChanged(CWnd* pWnd) override;
};

class CButton : public CWnd
{
    DECLARE_DYNAMIC(CButton)
    DECLARE_MESSAGE_MAP()
public:
    CButton() = default;
    virtual ~CButton() = default;
    virtual BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
    
    void SetCheck(int nCheck);
    int GetCheck() const;
    void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);
    void SetState(BOOL bHighlight);
    int GetState() const;
    
    virtual void OnPaint() override;
    
protected:
    int m_nCheck = 0;
    int m_nState = 0;
    UINT m_nStyle = 0;
};

class CStatic : public CWnd
{
    DECLARE_DYNAMIC(CStatic)
    DECLARE_MESSAGE_MAP()
public:
    CStatic() = default;
    virtual ~CStatic() = default;
    virtual BOOL Create(LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);
    
    HICON SetIcon(HICON hIcon);
    HICON GetIcon() const;
    void* SetBitmap(void* hBitmap);
    void* GetBitmap() const;
    HCURSOR SetCursor(HCURSOR hCursor);
    HCURSOR GetCursor() const;
    
    virtual void OnPaint() override;
};

class CComboBox : public CWnd
{
    DECLARE_DYNAMIC(CComboBox)
    DECLARE_MESSAGE_MAP()
public:
    CComboBox() = default;
    virtual ~CComboBox() = default;
    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
    
    int AddString(LPCTSTR lpszString);
    int GetCurSel() const;
    int SetCurSel(int nSelect);
    void ResetContent();
};





// -----------------------------------------------------------------------------
// CDataExchange — minimal MFC compatibility stub
// -----------------------------------------------------------------------------
class CDataExchange {
public:
    // In MFC, this indicates whether data is being loaded or saved.
    // Rowan code never checks it, so we keep it for completeness.
    BOOL m_bSaveAndValidate = FALSE;
    CWnd* m_pDlgWnd = nullptr;

    CDataExchange(CWnd* pDlg, BOOL saveAndValidate);

    // Default constructor for convenience
    CDataExchange() = default;
};

// -----------------------------------------------------------------------------
// CDialog — placeholder for MFC dialog class (The one not inside a Rowan struct)
// -----------------------------------------------------------------------------
struct DlgControlTemplate {
    int id;
    std::string text;
    std::string className;
    DWORD style;
    short x, y, cx, cy;
};

struct DlgTemplate {
    int id;
    std::string title;
    short x, y, cx, cy;
    std::string fontName;
    int fontSize;
    std::vector<DlgControlTemplate> controls;
};

class CDialog : public CWnd {
protected:
    UINT m_nIDHelp = 0;
    bool m_modalRunning = false; 
    int m_modalResult = IDCANCEL;
    bool m_isModal = false;
    bool m_forceTopLevel = false;
    DECLARE_MESSAGE_MAP()
    const DlgTemplate* m_pTemplate = nullptr;
public:
    DECLARE_DYNCREATE(CDialog)
    CDialog() = default;
    CDialog(int iid);
    CDialog(int iid, CWnd* pParent);
    virtual ~CDialog() override = default;

    virtual int DoModal();
    virtual void EndDialog(int);

    // MFC signature compatibility
    virtual void OnOK();
    virtual void OnCancel();

    // MFC message handlers (no-op)
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange*);
    virtual void OnClose() override;
    virtual void OnDestroy();
    virtual BOOL Create(int /*id*/, CWnd* /*pParent*/ = nullptr);
    virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = 0);
    virtual void OnPaint() override;
    virtual void OnLButtonDown(UINT nFlags, CPoint point) override;
    virtual void OnLButtonUp(UINT nFlags, CPoint point) override;
    virtual BOOL GetControlInfo(int id, CRect& rect, DWORD& style) override;
};

// -----------------------------------------------------------------------------
// CEdit — minimal MFC compatibility stub
// -----------------------------------------------------------------------------
class CEdit : public CWnd
{
public:
    CEdit() = default;
    virtual ~CEdit() = default;

    void SetWindowText(LPCTSTR);
    void GetWindowText(LPTSTR buf, int max) const;
    void GetWindowText(CString& s) const;
    void SetLimitText(int);
    void ReplaceSel(LPCTSTR);
    void SetSel(int, int);
};


#ifndef ASSERT
    #include <cassert>
    #define ASSERT(x) assert(x)
#endif

class CCmdUI
{
public:
    CCmdUI() {}
    virtual ~CCmdUI() {}

    // MFC API surface — all no‑ops
    void Enable(BOOL /*bOn*/);
    void SetCheck(int /*nCheck*/);
    void SetRadio(BOOL /*bOn*/);
    void SetText(const char* /*pszText*/);

    // These exist in real MFC but are rarely used
    UINT m_nID = 0;        // command ID
    UINT m_nIndex = 0;     // index in menu/toolbar
    void* m_pOther = nullptr; // menu/toolbar pointer
    void* m_pMenu = nullptr;  // menu pointer
};

class CDocument; //Forward, declared later
// ------------------------------------------------------------
// Minimal MFC-compatible CView stub for Mig Alley (Linux/SDL)
// ------------------------------------------------------------
class CView : public CWnd
{
public:
    CDocument* m_pDocument = nullptr;

    // Allow CDocument to set m_pDocument
    friend class CDocument;

    DECLARE_DYNCREATE(CView)

    CView() = default;
    virtual ~CView() = default;

    // Called after the view is attached to the frame and the frame window exists.
    virtual void OnInitialUpdate();

    // Called when the view needs to render.
    virtual void OnDraw(CDC* pDC);

    // SDL event routing entry point (called from CFrameWnd)
    virtual void HandleSDLEvent(const SDL_Event& e);

    // Access to the document
    CDocument* GetDocument() const { return m_pDocument; }

    // Printing stubs (unused in Mig Alley)
    virtual BOOL DoPreparePrinting(void* pInfo) { return TRUE; }
    virtual void OnPrepareDC(CDC* pDC, void* pInfo = nullptr) {}
    virtual void OnBeginPrinting(CDC* pDC, void* pInfo) {}
    virtual void OnEndPrinting(CDC* pDC, void* pInfo) {}
    virtual void OnFilePrint() {}
    virtual void OnFilePrintPreview() {}

    // Scrolling stubs (unused)
    virtual CScrollBar* GetScrollBarCtrl(int nBar) const { return nullptr; }
    virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) { return FALSE; }

    virtual void OnPaint();
};


//class CDocument : public CWnd
class CDocument : public CCmdTarget
{
protected: 
    std::vector<CView*> m_viewList;
public:
    DECLARE_DYNCREATE(CDocument)
    virtual ~CDocument() {}

    virtual void UpdateAllViews(CView* /*pSender*/, LPARAM /*lHint*/ = 0, CObject* /*pHint*/ = nullptr);
    void AddView(CView* pView); void 
    RemoveView(CView* pView);
    DECLARE_MESSAGE_MAP()
};


// --- MFC serialization stub ---
class CArchive
{
public:
    bool IsStoring() const { return false; }
    bool IsLoading() const { return false; }
};

// --- MFC printing stub ---
class CPrintInfo
{
public:
    CPrintInfo() {}
};

class COleClientItem; //RERUN
class COleDocument : public CDocument
{
public:
    DECLARE_DYNCREATE(COleDocument)
    virtual ~COleDocument() {}
    
    // --- Enable OLE compound-file storage ---
    virtual void EnableCompoundFile() {}

    // --- OLE UI update handlers ---
    virtual void OnUpdatePasteMenu(CCmdUI* /*pCmdUI*/) {}
    virtual void OnUpdateObjectVerbMenu(CCmdUI* /*pCmdUI*/) {}
    virtual void OnUpdatePasteLinkMenu(CCmdUI* /*pCmdUI*/) {}
    virtual void OnUpdateEditLinksMenu(CCmdUI* /*pCmdUI*/) {}

    // --- OLE command handlers ---
    virtual void OnEditConvert() {}
    virtual void OnEditLinks() {}
    virtual BOOL OnNewDocument() { return TRUE; }

    virtual void Serialize(CArchive& /*ar*/) {}

    void SetCheck(int) {} 
    void SetText(const char*) {}

    COleClientItem* GetInPlaceActiveItem(CWnd* /*pWnd*/) const { return nullptr;}
};


class CCommandLineInfo
{
public:
    enum { FileNew, FileOpen, FilePrint, FilePrintTo, FileDDE, FileNothing };

    CCommandLineInfo()
        : m_nShellCommand(FileNew)
        , m_strFileName("")
        , m_strPrinterName("")
        , m_strDriverName("")
        , m_strPortName("")
    {}

    virtual ~CCommandLineInfo() {}

    // Called for each token on the command line
    virtual void ParseParam(const char* /*pszParam*/, BOOL /*bFlag*/, BOOL /*bLast*/) {}

    // MFC uses this to decide what to do at startup
    int m_nShellCommand;

    // File to open (if any)
    std::string m_strFileName;

    // Used only for /pt (print to) commands
    std::string m_strPrinterName;
    std::string m_strDriverName;
    std::string m_strPortName;

    // MFC calls these helpers during ParseParam
    virtual void ParseParamFlag(const char* /*pszParam*/) {}
    virtual void ParseParamNotFlag(const char* /*pszParam*/) {}
    virtual void ParseLast(bool /*bLast*/) {}
};


// -----------------------------------------------------------------------------
// CToolBar — minimal MFC-compatible toolbar stub
// -----------------------------------------------------------------------------
class CToolBar : public CDialog
{
public:
    CToolBar() = default;
    virtual ~CToolBar() = default;

    // MFC-style creation API (Rowan calls this)
    virtual BOOL Create(CWnd* parentWnd, DWORD style = 0, UINT id = 0);

    // Some MFC toolbars use this overload
    virtual BOOL CreateEx(CWnd* parentWnd, DWORD dwCtrlStyle = 0, DWORD dwStyle = 0, const RECT* rect = nullptr, UINT nID = 0);

    // Rowan code may call this when loading button layouts
    virtual BOOL LoadToolBar(UINT /*resourceID*/);

    // MFC docking API — Rowan never uses real docking, so no-op
    virtual void EnableDocking(DWORD /*dockStyle*/);
    virtual BOOL SetButtons(const UINT* /*buttons*/, int /*count*/);

    // Layout helpers — Rowan sometimes calls these indirectly
    virtual CSize CalcFixedLayout(BOOL /*stretch*/, BOOL /*horz*/);
    virtual CSize CalcDynamicLayout(int /*length*/, DWORD /*mode*/);

    // MFC command routing — safe no-op
    virtual BOOL SetButtonInfo(int /*index*/, UINT /*id*/, UINT /*style*/, int /*image*/);
    virtual int CommandToIndex(UINT /*id*/) const;
    virtual BOOL OnAmbientProperty(COleControlSite* /*pSite*/, DISPID /*dispid*/, VARIANT* /*pvar*/);
    virtual BOOL OnCmdMsg(UINT /*nID*/, int /*nCode*/, void* /*pExtra*/, AFX_CMDHANDLERINFO* /*pHandlerInfo*/);

};

// -----------------------------------------------------------------------------
// CBitmap — minimal MFC-compatible bitmap stub
// -----------------------------------------------------------------------------
class CBitmap : public CGdiObject {
public:
    BITMAP info{};   // width, height, etc.
    SDL_Texture* m_texture = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;

    CBitmap() = default;
    ~CBitmap() { DeleteObject(); }

    BOOL LoadBitmap(UINT resourceID);
    int GetObject(int cbBuffer, void* lpvObject) const;
    BOOL GetBitmap(BITMAP* pBM) const;
    BOOL DeleteObject();
    SDL_Texture* GetTexture(SDL_Renderer* renderer);
};


// -----------------------------------------------------------------------------
// CList — minimal MFC-compatible doubly-linked list template
// -----------------------------------------------------------------------------
typedef void* POSITION;
template <class TYPE, class ARG_TYPE = TYPE>
class CList
{
private:
    struct Node {
        TYPE data;
        Node* next;
        Node* prev;
        Node(const ARG_TYPE& d) : data(d), next(nullptr), prev(nullptr) {}
    };

    Node* m_head = nullptr;
    Node* m_tail = nullptr;
    int   m_count = 0;

public:
    CList() = default;
    ~CList() { RemoveAll(); }

    // Add element at end
    void AddTail(const ARG_TYPE& value)
    {
        Node* n = new Node(value);
        if (!m_tail) {
            m_head = m_tail = n;
        } else {
            m_tail->next = n;
            n->prev = m_tail;
            m_tail = n;
        }
        ++m_count;
    }

    // Remove all elements
    void RemoveAll()
    {
        Node* n = m_head;
        while (n) {
            Node* next = n->next;
            delete n;
            n = next;
        }
        m_head = m_tail = nullptr;
        m_count = 0;
    }

    // MFC-style iteration
    POSITION GetHeadPosition() const
    {
        return (POSITION)m_head;
    }

    TYPE& GetNext(POSITION& pos)
    {
        Node* n = (Node*)pos;
        pos = (POSITION)(n ? n->next : nullptr);
        return n->data;
    }

    // Helpers
    BOOL IsEmpty() const { return m_count == 0; }
    int  GetCount() const { return m_count; }
    POSITION FindIndex(int index) const
    {
        Node* n = m_head;
        int i = 0;
        while (n && i < index)
        {
            n = n->next;
            ++i;
        }
        return (POSITION)n;
    }

    POSITION InsertAfter(POSITION pos, const ARG_TYPE& value)
    {
        Node* p = (Node*)pos;

        if (!p) {
            AddTail(value);
            return (POSITION)m_tail;
        }

        Node* n = new Node(value);

        n->prev = p;
        n->next = p->next;

        if (p->next)
            p->next->prev = n;
        else
            m_tail = n;

        p->next = n;

        ++m_count;
        return (POSITION)n;
    }

    POSITION AddHead(const ARG_TYPE& value)
    {
        Node* n = new Node(value);

        n->next = m_head;
        n->prev = nullptr;

        if (m_head)
            m_head->prev = n;
        else
            m_tail = n;   // list was empty

        m_head = n;
        ++m_count;

        return (POSITION)n;
    }

    void RemoveAt(POSITION pos)
    {
        Node* p = (Node*)pos;
        if (!p)
            return;

        if (p->prev)
            p->prev->next = p->next;
        else
            m_head = p->next;

        if (p->next)
            p->next->prev = p->prev;
        else
            m_tail = p->prev;

        delete p;
        --m_count;
    }

    void SetAt(POSITION pos, const ARG_TYPE& value)
    {
        Node* p = (Node*)pos;
        if (p)
            p->data = value;
    }

};

// -----------------------------------------------------------------------------
// CMenu — minimal MFC-compatible menu stub
// -----------------------------------------------------------------------------
class CMenu
{
public:
    CMenu() = default;
    virtual ~CMenu() = default;

    // Creation API — always succeed
    BOOL CreateMenu();
    BOOL CreatePopupMenu();
    
    BOOL LoadMenu(UINT /*id*/);
    BOOL EnableMenuItem(UINT /*id*/, UINT /*flags*/);

    BOOL Attach(HMENU /*hMenu*/);
    HMENU Detach();

    BOOL AppendMenu(UINT /*flags*/, UINT /*id*/, const char* /*text*/ = nullptr);
    BOOL InsertMenu(UINT /*pos*/, UINT /*flags*/, UINT /*id*/, const char* /*text*/ = nullptr);
    BOOL DeleteMenu(UINT /*pos*/, UINT /*flags*/);
    BOOL RemoveMenu(UINT /*pos*/, UINT /*flags*/);
    BOOL ModifyMenu(UINT /*pos*/, UINT /*flags*/, UINT /*newID*/, const char* /*text*/ = nullptr);

    CMenu* GetSubMenu(int /*pos*/) const;

    int GetMenuItemCount() const;
    UINT GetMenuItemID(int /*pos*/) const;

    void MeasureItem(LPMEASUREITEMSTRUCT /*lpMIS*/);
    void DrawItem(LPDRAWITEMSTRUCT /*lpDIS*/);
    BOOL TrackPopupMenu(UINT /*flags*/, int /*x*/, int /*y*/, CWnd* /*pWnd*/, const RECT* /*rect*/ = nullptr);

    HMENU GetSafeHmenu() const;
};

// -----------------------------------------------------------------------------
// CFrameWnd — minimal MFC-compatible frame window stub
// -----------------------------------------------------------------------------
class CFrameWnd : public CWnd
{
public:
    DECLARE_MESSAGE_MAP() CView* m_pActiveView = nullptr;

    DECLARE_DYNCREATE(CFrameWnd)
    CFrameWnd() = default;
    virtual ~CFrameWnd() = default;

    // Creation API — always succeed
    virtual BOOL Create(
        const char* /*className*/, const char* /*windowName*/, DWORD /*style*/ = 0, const RECT& /*rect*/ = RECT{0,0,0,0},
        CWnd* m_pParent = nullptr, const char* /*menuName*/ = nullptr, DWORD /*exStyle*/ = 0, void* /*lpCreateParam*/ = nullptr);

    // LoadFrame — Rowan sometimes calls this
    virtual BOOL LoadFrame(UINT /*nIDResource*/, DWORD /*dwDefaultStyle*/ = 0, CWnd* /*pParentWnd*/ = nullptr, void* /*pContext*/ = nullptr);

    // Menu access — no-op
    virtual CMenu* GetMenu() const;
    virtual BOOL SetMenu(CMenu* /*pMenu*/);

    // Client area adjustments — no-op
    virtual void RecalcLayout(BOOL /*bNotify*/ = TRUE);

    // Command routing — always return FALSE
    virtual BOOL OnCmdMsg(UINT /*nID*/, int /*nCode*/, void* /*pExtra*/, AFX_CMDHANDLERINFO* /*pHandlerInfo*/) override;

    // Message pre-translation — default behavior
    virtual BOOL PreTranslateMessage(MSG* /*pMsg*/) override;

    // Status bar / toolbar docking — never used in MiG Alley
    virtual BOOL DockControlBar(CWnd* /*pBar*/, UINT /*nDockBarID*/ = 0, const RECT* /*lpRect*/ = nullptr);

    virtual void SetActiveView(CView* pView);
    virtual CView* GetActiveView() const;

    virtual void AttachView(CView* pView);
    virtual void OnPaint();
    virtual void OnLButtonDown(UINT nFlags, CPoint point) override;
    virtual void OnSize(UINT nType, int cx, int cy) override;
    virtual void OnHelp();
    virtual void OnContextHelp();
};

int AfxLoadString(unsigned int, char* buffer, unsigned int maxLen);

inline void AfxMessageBox(const char* /*message*/) {}
inline void AfxMessageBox(int /*message*/) {}

inline void AfxEnableControlContainer() {}

inline BOOL AfxOleInit() { return TRUE; }

void AFXAPI DDX_Control(CDataExchange* pDX, int id, CWnd& member);

inline CWnd* GetDesktopWindow()
{
    static CWnd desktop;
    SDL_Rect displayBounds;
    SDL_GetDisplayUsableBounds(0, &displayBounds);
    desktop.SetWindowPos(&desktop, displayBounds.x, displayBounds.y, displayBounds.w, displayBounds.h, SWP_NOACTIVATE);
    return &desktop;
}

// MFC status bar indicator IDs (dummy values for Linux port)
#define ID_SEPARATOR        0xE000
#define ID_INDICATOR_CAPS   0xE001
#define ID_INDICATOR_NUM    0xE002
#define ID_INDICATOR_SCRL   0xE003

// Standard MFC Command IDs
#define ID_FILE_NEW             0xE100
#define ID_FILE_OPEN            0xE101
#define ID_FILE_CLOSE           0xE102
#define ID_FILE_SAVE            0xE103
#define ID_FILE_SAVE_AS         0xE104
#define ID_FILE_PAGE_SETUP      0xE105
#define ID_FILE_PRINT_SETUP     0xE106
#define ID_FILE_PRINT           0xE107
#define ID_FILE_PRINT_DIRECT    0xE108
#define ID_FILE_PRINT_PREVIEW   0xE109
#define ID_FILE_UPDATE          0xE10A
#define ID_FILE_SAVE_COPY_AS    0xE10B
#define ID_FILE_SEND_MAIL       0xE10C
#define ID_APP_ABOUT            0xE114
#define ID_APP_EXIT             0xE111
#define ID_HELP                 0xE146
#define ID_HELP_INDEX           0xE142
#define ID_HELP_FINDER          0xE143
#define ID_HELP_USING           0xE144
#define ID_CONTEXT_HELP         0xE145
#define ID_DEFAULT_HELP         0xE147
#define ID_NEXT_PANE            0xE150
#define ID_PREV_PANE            0xE151
#define ID_EDIT_CLEAR           0xE120
#define ID_EDIT_CLEAR_ALL       0xE121
#define ID_EDIT_COPY            0xE122
#define ID_EDIT_CUT             0xE123
#define ID_EDIT_FIND            0xE124
#define ID_EDIT_PASTE           0xE125
#define ID_EDIT_PASTE_LINK      0xE126
#define ID_EDIT_PASTE_SPECIAL   0xE127
#define ID_EDIT_REPEAT          0xE128
#define ID_EDIT_REPLACE         0xE129
#define ID_EDIT_SELECT_ALL      0xE12A
#define ID_EDIT_UNDO            0xE12B
#define ID_EDIT_REDO            0xE12C
#define ID_VIEW_TOOLBAR         0xE800
#define ID_VIEW_STATUS_BAR      0xE801

inline CDC* BeginPaint(void* /*hwnd*/, PAINTSTRUCT* /*ps*/)
{
    // Return a dummy DC, just like CWnd::BeginPaint does
    static CDC dummy;
    return &dummy;
}

inline void EndPaint(void* /*hwnd*/, PAINTSTRUCT* /*ps*/)
{
    // No-op
}

// Win32/MFC window-positioning pseudo-handles
extern CWnd wndTop;
extern CWnd wndBottom;
extern CWnd wndTopMost;
extern CWnd wndNoTopMost;

#define HWND_TOP        ((HWND)nullptr)
#define HWND_BOTTOM     ((HWND)nullptr)
#define HWND_TOPMOST    ((HWND)nullptr)
#define HWND_NOTOPMOST  ((HWND)nullptr)

// -----------------------------------------------------------------------------
// Global Win32-compatible SetWindowPos stub
// -----------------------------------------------------------------------------
inline BOOL SetWindowPos(
    HWND hWnd,               // SDL_Window*
    HWND hWndInsertAfter,    // SDL_Window* (ignored)
    int x, int y,
    int cx, int cy,
    UINT flags)
{
#if 0
    // Forward to the CWnd version if possible
    if (hWnd)
    {
        CWnd* wnd = static_cast<CWnd*>(hWnd);
        return wnd->SetWindowPos(nullptr, x, y, cx, cy, flags);
    }
#endif
    return TRUE;
}

// ---------------------------------------------------------------------------
// Minimal COleDataObject stub 
// ---------------------------------------------------------------------------

class COleDataObject
{
public:
    COleDataObject() {}
    virtual ~COleDataObject() {}

    // -----------------------------------------------------------------------
    // Initialization / attachment — no-op
    // -----------------------------------------------------------------------
    BOOL AttachClipboard() { return FALSE; }
    BOOL AttachDataObject(void* /*pDataObject*/, BOOL /*bAutoRelease*/ = TRUE)
    {
        return FALSE;
    }

    // -----------------------------------------------------------------------
    // Data query — always return FALSE or nullptr
    // -----------------------------------------------------------------------
    BOOL IsDataAvailable(CLIPFORMAT /*cfFormat*/) const { return FALSE; }

    BOOL GetData(CLIPFORMAT /*cfFormat*/, void* /*pStgMedium*/) const
    {
        return FALSE;
    }

    BOOL GetGlobalData(CLIPFORMAT /*cfFormat*/, HGLOBAL* /*phGlobal*/) const
    {
        return FALSE;
    }

    // -----------------------------------------------------------------------
    // Drag-and-drop helpers — unused in Mig Alley
    // -----------------------------------------------------------------------
    BOOL BeginDrag(CWnd* /*pWnd*/, DWORD /*dwEffects*/ = DROPEFFECT_COPY)
    {
        return FALSE;
    }

    // -----------------------------------------------------------------------
    // OLE IDataObject access — unused
    // -----------------------------------------------------------------------
    void* GetIDataObject(BOOL /*bAddRef*/ = FALSE) const
    {
        return nullptr;
    }

    // -----------------------------------------------------------------------
    // Reset — no-op
    // -----------------------------------------------------------------------
    void Release() {}
};

class CDocItem : public CCmdTarget
{
public:
    virtual ~CDocItem() {}
};

// ---------------------------------------------------------------------------
// Minimal COleClientItem stub
// ---------------------------------------------------------------------------
enum DVASPECT {
    DVASPECT_CONTENT = 1
};
struct _DummyOleObject {
    HRESULT GetMiscStatus(DWORD /*aspect*/, DWORD* /*pdwStatus*/) {
        return S_OK;
    }
};
class COleClientItem : public CDocItem
{
protected:
    COleDocument* m_pDocument;
    _DummyOleObject* m_lpObject; 
    _DummyOleObject m_dummyObject;
public:
    COleClientItem(COleDocument* pContainer = nullptr)
        : m_pDocument(pContainer),
        m_lpObject(&m_dummyObject)
    {
    }

    virtual ~COleClientItem() {}

    // -----------------------------------------------------------------------
    // Basic MFC-style API surface (no-op implementations)
    // -----------------------------------------------------------------------

    virtual void OnChange(int /*nCode*/, DWORD /*dwParam*/ = 0);
    virtual void OnActivate();
    virtual void OnDeactivate();
    virtual void OnGetItemPosition(RECT* /*pRect*/);
    virtual BOOL OnChangeItemPosition(const CRect& /*rectPos*/);

    virtual void OnUpdate();

    virtual BOOL OnDraw(CDC* /*pDC*/, CSize& /*rSize*/);

    virtual void DoVerb(LONG /*iVerb*/, CWnd* /*pWnd*/, LPCRECT /*lpRect*/);
    virtual void DoVerb(LONG iVerb, CWnd* pWnd);

    // Link/embedding stubs
    virtual BOOL CreateFromFile(LPCTSTR /*lpszFileName*/);
    virtual BOOL CreateLinkFromFile(LPCTSTR /*lpszFileName*/);
    virtual BOOL CreateFromClipboard();
    virtual BOOL CreateFromData(COleDataObject* /*pDataObject*/);

    // Clipboard support
    virtual BOOL CopyToClipboard();

    // Serialization stub
    virtual void Serialize(CArchive& /*ar*/);

    // Container access
    COleDocument* GetDocument() const { return m_pDocument; }

    virtual CView* GetActiveView();
    virtual void Close();
    virtual void OnDeactivateUI(BOOL /*bUndoable*/);
    virtual DVASPECT GetDrawAspect() const;

};

#define DECLARE_SERIAL(class_name) \
public: \
    static const char* _GetClassName() { return #class_name; }

#define IMPLEMENT_SERIAL(class_name, base_class, version)

// ---------------------------------------------------------------------------
// Minimal OLE_NOTIFICATION stub for Linux/SDL2 port of Mig Alley
// ---------------------------------------------------------------------------

typedef int OLE_NOTIFICATION;

// Optional symbolic constants (not required unless referenced)
#define OLE_CHANGED        1
#define OLE_SAVED          2
#define OLE_CLOSED         3
#define OLE_RENAMED        4
#define OLE_CHANGED_STATE  5

#define ID_EDIT_PASTE_LINK     0xE126
#define ID_OLE_EDIT_CONVERT    0xE12D
#define ID_OLE_EDIT_LINKS      0xE12E
#define ID_OLE_VERB_FIRST      0xE210

inline void Enable3dControlsStatic() {}

enum DocStringIndex
{
    windowTitle,        // Title for the main window
    docName,            // Base document name
    fileNewName,        // Name used in File > New
    filterName,         // Display name for file filter
    filterExt,          // File extension (e.g. ".mig")
    regFileTypeId,      // Registry file type ID (unused in your port)
    regFileTypeName     // Registry file type name (unused)
};

class CSingleDocTemplate
{
public:
    UINT m_nIDResource;
    CRuntimeClass* m_pDocClass; 
    CRuntimeClass* m_pFrameClass; 
    CRuntimeClass* m_pViewClass;
    CFrameWnd* m_pFrame = nullptr;
public:
    CSingleDocTemplate(UINT nIDResource,
                       CRuntimeClass* pDocClass,
                       CRuntimeClass* pFrameClass,
                       CRuntimeClass* pViewClass);
    
    void SetContainerInfo(UINT /*nIDResource*/);
    void AddDocTemplate();
    CDocument* OpenDocumentFile(const char* /*lpszPathName*/ = nullptr);
    BOOL GetDocumentString(CString& /*rString*/, enum DocStringIndex /*index*/) const;
};


typedef void* HINSTANCE;

class CWinApp : public CCmdTarget {
public:

    HINSTANCE m_hInstance = nullptr;
    char* m_pszHelpFilePath = nullptr;
    // MFC apps usually expose the main window
    CWnd* m_pMainWnd = nullptr;

    std::vector<CSingleDocTemplate*> m_docTemplates;

    CWinApp();
    virtual ~CWinApp();

    // MFC compatibility
    virtual BOOL InitApplication();
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    // Rowan code sometimes checks this
    void SetRegistryKey(const char*);

    int Run();

    void AddDocTemplate(CSingleDocTemplate* pTemplate);
    void RegisterShellFileTypes(BOOL /*bCompat*/);
    void EnableShellOpen();
    void ParseCommandLine(CCommandLineInfo& /*cmdInfo*/);
    BOOL ProcessShellCommand(CCommandLineInfo& /*cmdInfo*/);
    afx_msg void OnFileNew() {}
    afx_msg void OnFileOpen() {}
    DECLARE_MESSAGE_MAP()

};

CWinApp* AfxGetApp();
/* Set m_pMainWnd when your main window is created
   In your main window creation code:

   AfxGetApp()->m_pMainWnd = this;
*/
CWnd* AfxGetMainWnd();

extern bool g_shouldQuit;
//RERUN: real MFC PostQuitMessage queues WM_QUIT; just setting the flag leaves
//CMIGApp::Run's message pump looping forever (it only exits on PumpMessage()==FALSE).
void AfxPostQuitMessage(int nExitCode);

inline BOOL AfxOleGetUserCtrl()
{
    return TRUE;   // Always behave like a normal app
}

#define ASSERT_VALID(pObj) ((void)0)
#define QS_ALLINPUT 0xFFFF

DWORD MsgWaitForMultipleObjects(
    DWORD nCount,
    const HANDLE* pHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask);
//{
//    return WAIT_TIMEOUT; // Always 'no wait' / immediate return
//}

#define PM_NOREMOVE 0

BOOL PeekMessage(MSG* msg, HWND hwnd, UINT min, UINT max, UINT remove);

BOOL PumpMessage(); 
//{ return TRUE;}

BOOL IsIdleMessage(MSG* /*pMsg*/);
// { return FALSE; }

BOOL OnIdle(LONG lCount);

struct _AFX_THREAD_STATE { MSG m_msgCur; };
static _AFX_THREAD_STATE _afxThreadState;
inline _AFX_THREAD_STATE* AfxGetThreadState() { return &_afxThreadState; }

void InvokeHelper(
    int dispID,
    int wFlags,
    int vtRet,
    void* pvRet,
    const BYTE* pszArgTypes,
    ...);

// ---------------------------------------------------------------------------
// Minimal COleDispatchDriver stub
// ---------------------------------------------------------------------------
class COleDispatchDriver
{
public:
    // MFC stores the COM interface pointer here.
    // In your port, it is always null.
    IDispatch* m_lpDispatch;

public:
    // Constructors -----------------------------------------------------------
    COleDispatchDriver()
        : m_lpDispatch(nullptr)
    {
    }

    COleDispatchDriver(IDispatch* lpDispatch, BOOL /*bAutoRelease*/ = TRUE)
        : m_lpDispatch(lpDispatch)
    {
    }

    COleDispatchDriver(const COleDispatchDriver& other)
        : m_lpDispatch(other.m_lpDispatch)
    {
    }

    // Destructor -------------------------------------------------------------
    virtual ~COleDispatchDriver()
    {
        // No COM, so nothing to release
        m_lpDispatch = nullptr;
    }

    // Assignment -------------------------------------------------------------
    COleDispatchDriver& operator=(const COleDispatchDriver& other)
    {
        if (this != &other)
            m_lpDispatch = other.m_lpDispatch;
        return *this;
    }

    // Attach / Detach --------------------------------------------------------
    void AttachDispatch(IDispatch* lpDispatch, BOOL /*bAutoRelease*/ = TRUE)
    {
        m_lpDispatch = lpDispatch;
    }

    void ReleaseDispatch()
    {
        m_lpDispatch = nullptr;
    }

    // Property helpers -------------------------------------------------------
    void GetProperty(DISPID /*dwDispID*/, VARTYPE /*vtProp*/, void* /*pvProp*/) const
    {
        // No-op
    }

    void SetProperty(DISPID /*dwDispID*/, VARTYPE /*vtProp*/, const void* /*pvProp*/)
    {
        // No-op
    }

    // Invoke helpers ---------------------------------------------------------
    void InvokeHelper(
        DISPID /*dwDispID*/,
        WORD /*wFlags*/,
        VARTYPE /*vtRet*/,
        void* /*pvRet*/,
        const BYTE* /*pbParamInfo*/,
        ...
    ) const
    {
        // No-op
    }

    void InvokeHelperV(
        DISPID /*dwDispID*/,
        WORD /*wFlags*/,
        VARTYPE /*vtRet*/,
        void* /*pvRet*/,
        const BYTE* /*pbParamInfo*/,
        va_list /*argList*/
    ) const
    {
        // No-op
    }

    // Operator overloads -----------------------------------------------------
    operator IDispatch*() const
    {
        return m_lpDispatch;
    }

    IDispatch* operator->() const
    {
        return m_lpDispatch;
    }
};

#ifndef VERIFY
    #ifdef _DEBUG
        #define VERIFY(expr) assert(expr)
    #else
        #define VERIFY(expr) (void)(expr)
    #endif
#endif

class CBrush : public CGdiObject
{
public:
    CBrush() {}

    explicit CBrush(COLORREF color)
    {
        m_hObject = CreateSolidBrush(color);
        m_bOwner  = true;
    }

    explicit CBrush(HBRUSH h)
    {
        m_hObject = h;
        m_bOwner  = false;   // does NOT own
    }

    static CBrush FromHandle(HBRUSH h)
    {
        return CBrush(h);    // non-owning wrapper
    }

    operator HBRUSH() const
    {
        return static_cast<HBRUSH>(m_hObject);
    }

private:
    static SDL_Color colorrefToSDL(COLORREF c)
    {
        return SDL_Color{
            static_cast<Uint8>( c        & 0xFF),
            static_cast<Uint8>((c >> 8)  & 0xFF),
            static_cast<Uint8>((c >> 16) & 0xFF),
            255
        };
    }

    static HBRUSH CreateSolidBrush(COLORREF color)
    {
        _BRUSH* brush = new _BRUSH;

        brush->type    = _BRUSH::SOLID;
        brush->solid   = colorrefToSDL(color);
        brush->isStock = false;

        return brush;
    }
};

// MFC-compatible thread proc type
typedef UINT (*AFX_THREADPROC)(LPVOID);

// Minimal stand‑in for MFC's CWinThread
struct CWinThread {
    HANDLE m_hThread = nullptr;
    int priority = THREAD_PRIORITY_NORMAL;

    BOOL SetThreadPriority(int p) {
        priority = p;
        // pthreads don't expose Windows priorities; store it only
        return TRUE;
    }
};

inline CWinThread* AfxBeginThread(
    AFX_THREADPROC pfnThreadProc,
    LPVOID pParam,
    int nPriority = THREAD_PRIORITY_NORMAL,
    UINT nStackSize = 0,
    DWORD dwCreateFlags = 0,
    LPSECURITY_ATTRIBUTES lpSecurityAttrs = nullptr)
{
    // Reuse your existing CreateThread stub
    HANDLE hThread = CreateThread(
        lpSecurityAttrs,
        nStackSize,
        (LPTHREAD_START_ROUTINE)pfnThreadProc,
        pParam,
        dwCreateFlags,
        nullptr);

    if (!hThread)
        return nullptr;

    // Optional: priority stub (pthread has no direct equivalent)
    SetThreadPriority(hThread, nPriority);

    CWinThread* pThread = new CWinThread;
    pThread->m_hThread = hThread;
    pThread->SetThreadPriority(nPriority);
    return pThread;
}

// Global instance representing the main thread
static CWinThread g_mainThread;

// MFC-style accessor
inline CWinThread* AfxGetThread() {
    return &g_mainThread;
}

class CPaintDC : public CDC
{
public:
    CPaintDC(CWnd* pWnd) : CDC(pWnd), m_pWnd(pWnd)
    {
        m_pWnd->BeginPaint(&m_ps);
    }

    ~CPaintDC()
    {
        m_pWnd->EndPaint(&m_ps);
    }

private:
    CWnd* m_pWnd;
    PAINTSTRUCT m_ps;   // optional, but harmless and keeps structure intact
};


inline CWnd* WindowFromPoint(const CPoint& pt)
{
    // For now, return nullptr so Rowan falls back to its own logic.
    return nullptr;
}

inline HINSTANCE AfxGetInstanceHandle()
{
    return reinterpret_cast<HINSTANCE>(1);
}
