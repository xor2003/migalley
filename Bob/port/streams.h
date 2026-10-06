// bob_port shim: streams.h -> DirectShow declaration surface (RERUN)
//
// BoB's fullpsys.cpp plays smacker videos through the DirectShow graph
// (CoCreateInstance(CLSID_FilterGraph) + RenderFile). No DirectShow exists
// on Linux: CoCreateInstance always fails, every consumer is null-guarded,
// and video is skipped while game flow continues. A native smacker/SDL
// player can later be wired behind this same call surface.
#ifndef BOB_PORT_STREAMS_H_
#define BOB_PORT_STREAMS_H_
#include "WIN32_COMPAT.H"

#ifndef CP_ACP
#define CP_ACP 0
#endif
#ifndef OAHWND
typedef long OAHWND;
#endif
#ifndef OATRUE
#define OATRUE  (-1)
#define OAFALSE (0)
#endif
#ifndef EC_COMPLETE
#define EC_COMPLETE 0x01
#endif
#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER 0x1
#endif
#ifndef WCHAR
typedef wchar_t WCHAR;
#endif

/* MultiByteToWideChar — CP_ACP only; BoB uses it solely for filenames. */
int MultiByteToWideChar(UINT codepage, DWORD flags, const char* src,
                        int srclen, WCHAR* dst, int dstlen);

/* DirectShow interfaces — declarations only; nothing is ever instantiated. */
struct IBasicAudio : public IUnknown
{
    virtual HRESULT put_Volume(long lVolume) = 0;
    virtual HRESULT get_Volume(long* plVolume) = 0;
    virtual HRESULT put_Balance(long lBalance) = 0;
    virtual HRESULT get_Balance(long* plBalance) = 0;
};

struct IMediaControl : public IUnknown
{
    virtual HRESULT Run() = 0;
    virtual HRESULT Pause() = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT StopWhenReady() = 0;
    virtual HRESULT GetState(long msTimeout, long* pfs) = 0;
    virtual HRESULT RenderFile(const WCHAR* strFilename) = 0;
    virtual HRESULT AddSourceFilter(const WCHAR* strFilename, IUnknown** ppUnk) = 0;
    virtual HRESULT get_FilterCollection(IUnknown** ppUnk) = 0;
    virtual HRESULT get_RegFilterCollection(IUnknown** ppUnk) = 0;
};

struct IGraphBuilder : public IUnknown
{
    virtual HRESULT RenderFile(const WCHAR* lpcwstrFile, const WCHAR* lpcwstrPlayList) = 0;
};

struct IMediaEventEx : public IUnknown
{
    virtual HRESULT GetEvent(long* lEventCode, long* lParam1, long* lParam2,
                             long msTimeout) = 0;
    virtual HRESULT WaitForCompletion(long msTimeout, long* pEvCode) = 0;
    virtual HRESULT CancelDefaultHandling(long lEvCode) = 0;
    virtual HRESULT RestoreDefaultHandling(long lEvCode) = 0;
    virtual HRESULT FreeEventParams(long lEvCode, long lParam1, long lParam2) = 0;
    virtual HRESULT SetNotifyWindow(OAHWND hwnd, long lMsg, long lInstanceData) = 0;
    virtual HRESULT SetNotifyFlags(long lNoNotifyFlags) = 0;
    virtual HRESULT GetNotifyFlags(long* lplNoNotifyFlags) = 0;
};

struct IVideoWindow : public IUnknown
{
    virtual HRESULT put_Caption(const WCHAR* strCaption) = 0;
    virtual HRESULT get_Caption(WCHAR** strCaption) = 0;
    virtual HRESULT put_WindowStyle(long WindowStyle) = 0;
    virtual HRESULT get_WindowStyle(long* WindowStyle) = 0;
    virtual HRESULT put_WindowStyleEx(long WindowStyleEx) = 0;
    virtual HRESULT get_WindowStyleEx(long* WindowStyleEx) = 0;
    virtual HRESULT put_AutoShow(long AutoShow) = 0;
    virtual HRESULT get_AutoShow(long* AutoShow) = 0;
    virtual HRESULT put_WindowState(long WindowState) = 0;
    virtual HRESULT get_WindowState(long* WindowState) = 0;
    virtual HRESULT put_BackgroundPalette(long BackgroundPalette) = 0;
    virtual HRESULT get_BackgroundPalette(long* pBackgroundPalette) = 0;
    virtual HRESULT put_Visible(long Visible) = 0;
    virtual HRESULT get_Visible(long* pVisible) = 0;
    virtual HRESULT put_Left(long Left) = 0;
    virtual HRESULT get_Left(long* pLeft) = 0;
    virtual HRESULT put_Width(long Width) = 0;
    virtual HRESULT get_Width(long* pWidth) = 0;
    virtual HRESULT put_Top(long Top) = 0;
    virtual HRESULT get_Top(long* pTop) = 0;
    virtual HRESULT put_Height(long Height) = 0;
    virtual HRESULT get_Height(long* pHeight) = 0;
    virtual HRESULT put_Owner(OAHWND Owner) = 0;
    virtual HRESULT get_Owner(OAHWND* Owner) = 0;
    virtual HRESULT put_MessageDrain(OAHWND Drain) = 0;
    virtual HRESULT get_MessageDrain(OAHWND* Drain) = 0;
    virtual HRESULT get_BorderColor(long* Color) = 0;
    virtual HRESULT put_BorderColor(long Color) = 0;
    virtual HRESULT get_FullScreenMode(long* FullScreenMode) = 0;
    virtual HRESULT put_FullScreenMode(long FullScreenMode) = 0;
    virtual HRESULT SetWindowForeground(long Focus) = 0;
    virtual HRESULT NotifyOwnerMessage(OAHWND hwnd, long uMsg, long wParam, long lParam) = 0;
    virtual HRESULT SetWindowPosition(long Left, long Top, long Width, long Height) = 0;
    virtual HRESULT GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) = 0;
    virtual HRESULT GetMinIdealImageSize(long* pWidth, long* pHeight) = 0;
    virtual HRESULT GetMaxIdealImageSize(long* pWidth, long* pHeight) = 0;
    virtual HRESULT GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) = 0;
    virtual HRESULT HideCursor(long HideCursor) = 0;
    virtual HRESULT IsCursorHidden(long* CursorHidden) = 0;
};

/* GUIDs — distinct placeholder IIDs; no real DirectShow object is ever
   created so exact bit patterns are irrelevant. */
extern const GUID IID_IGraphBuilder;
extern const GUID IID_IMediaControl;
extern const GUID IID_IMediaEventEx;
extern const GUID IID_IVideoWindow;
extern const GUID IID_IBasicAudio;
extern const GUID CLSID_FilterGraph;

HRESULT CoCreateInstance(const GUID& rclsid, IUnknown* pUnkOuter,
                         DWORD dwClsContext, const GUID& riid, void** ppv);

#endif // BOB_PORT_STREAMS_H_
