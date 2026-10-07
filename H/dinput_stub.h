// dinput.h - Linux stub for DirectInput (DX7)
// This is a non-functional placeholder to allow compilation on non-Windows.
// All interfaces are empty; all functions return failure codes.

#pragma once

#include "WIN32_COMPAT.H"
#include <deque>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DI_OK
#define DI_OK 0x00000000
#endif
#ifndef DI_NOEFFECT
#define DI_NOEFFECT 0x00000001  // effect params empty — still "success" (S_FALSE)
#endif
#ifndef DI_PROPNOEFFECT
#define DI_PROPNOEFFECT 0x00000001
#endif
#ifndef DI_BUFFEROVERFLOW
#define DI_BUFFEROVERFLOW 0x00000001  // non-fatal: some buffered data was lost
#endif
#ifndef DI_DOWNLOADSKIPPED
#define DI_DOWNLOADSKIPPED 0x00000001
#endif
#ifndef DI_EFFECTRESTARTED
#define DI_EFFECTRESTARTED 0x00000001
#endif
#ifndef DI_TRUNCATED
#define DI_TRUNCATED 0x00000001
#endif
#ifndef DI_TRUNCATEDANDRESTARTED
#define DI_TRUNCATEDANDRESTARTED 0x00000001
#endif
#ifndef DIGDD_PEEK
#define DIGDD_PEEK 0x00000001
#endif

#ifndef DI_NOTATTACHED
#define DI_NOTATTACHED 0x8007000A
#endif

#define DIERR_GENERIC   -1

/* Minimal COM IUnknown stub for non‑Windows builds */
#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

typedef struct IUnknown IUnknown;

/* Define a dummy vtable with the three standard COM methods */
typedef struct IUnknownVtbl {
    HRESULT (*QueryInterface)(IUnknown* This, const void* riid, void** ppvObject);
    ULONG   (*AddRef)(IUnknown* This);
    ULONG   (*Release)(IUnknown* This);
} IUnknownVtbl;

/* The IUnknown struct itself just holds a vtable pointer */
struct IUnknown {
    const IUnknownVtbl* lpVtbl;
};

typedef IUnknown *LPUNKNOWN;

#endif /* __IUnknown_INTERFACE_DEFINED__ */

// Object type flags
#define DIDFT_AXIS      0x00000001
#define DIDFT_BUTTON    0x0000000C
#define DIDFT_POV       0x00000010
#define DIDFT_RELAXIS   0x00000020L

// Device object instance flags
#define DIDOI_FFACTUATOR    0x00000001L

// Enumeration return values
#define DIENUM_STOP         0
#define DIENUM_CONTINUE     1

#ifndef DIDFT_GETTYPE
#define DIDFT_GETTYPE(n)    ((n) & 0xFF)
#endif
#ifndef DIDFT_MAKEINSTANCE
#define DIDFT_MAKEINSTANCE(n) ((DWORD)(n) << 8)
#define DIDFT_GETINSTANCE(n)  (((n) >> 8) & 0xFFFF)
#endif
// Either-axis class used in DIDATAFORMAT/DIEnumObjects masks
#ifndef DIDFT_ABSAXIS
#define DIDFT_ABSAXIS   0x00000001
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
typedef GUID* LPGUID;
#endif

// Forward declarations of opaque types
typedef struct IDirectInputA      IDirectInputA;
typedef struct IDirectInputDeviceA IDirectInputDeviceA;
// Add the pointer typedefs
typedef IDirectInputA* LPDIRECTINPUTA;
typedef IDirectInputA* LPDIRECTINPUT;   // ANSI version is default
typedef IDirectInputDeviceA* LPDIRECTINPUTDEVICEA;
typedef IDirectInputDeviceA* LPDIRECTINPUTDEVICE;

// Dummy interface structs
// Callback type stub
// Forward declare
struct DIDEVICEINSTANCE;
typedef const DIDEVICEINSTANCE* LPCDIDEVICEINSTANCE;
typedef BOOL (*LPDIENUMDEVICESCALLBACK)(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

// Device type constants
#define DIDEVTYPE_MOUSE 0x00000002
#define DIDEVTYPE_KEYBOARD 0x00000004
#define DIDEVTYPE_JOYSTICK 0x00000008

// EnumDevices flags
#define DIEDFL_ALLDEVICES   0x00000000
#define DIEDFL_ATTACHEDONLY 0x00000001

// DIDATAFORMAT flags
#define DIDF_ABSAXIS 0x00000001
#define DIDF_RELAXIS 0x00000002

#ifndef DIERR_NOTINITIALIZED
#define DIERR_NOTINITIALIZED 0x80070015
#endif

#ifndef DIERR_NOTFOUND
#define DIERR_NOTFOUND  0x80070002L
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

//////////////////////////////////////////////////////////////////////////////////////
/* special property GUIDs */
#define MAKEDIPROP(prop) GUID{}
#define DIPROP_BUFFERSIZE	MAKEDIPROP(1)
#define DIPROP_AXISMODE		MAKEDIPROP(2)

#define DIPROPAXISMODE_ABS	0
#define DIPROPAXISMODE_REL	1

#define DIPROP_GRANULARITY	MAKEDIPROP(3)
#define DIPROP_RANGE		MAKEDIPROP(4)
#define DIPROP_DEADZONE		MAKEDIPROP(5)
#define DIPROP_SATURATION	MAKEDIPROP(6)
#define DIPROP_FFGAIN		MAKEDIPROP(7)
#define DIPROP_FFLOAD		MAKEDIPROP(8)
#define DIPROP_AUTOCENTER	MAKEDIPROP(9)

#define DIPROPAUTOCENTER_OFF	0
#define DIPROPAUTOCENTER_ON	1

#define DIPROP_CALIBRATIONMODE	MAKEDIPROP(10)

#define DIPROPCALIBRATIONMODE_COOKED	0
#define DIPROPCALIBRATIONMODE_RAW	1

#define DIPROP_CALIBRATION	MAKEDIPROP(11)
#define DIPROP_GUIDANDPATH	MAKEDIPROP(12)

#define DIPROP_INSTANCENAME     MAKEDIPROP(13)
#define DIPROP_PRODUCTNAME      MAKEDIPROP(14)
#define DIPROP_JOYSTICKID       MAKEDIPROP(15)

#define DIPROP_PHYSICALRANGE    MAKEDIPROP(18)
#define DIPROP_LOGICALRANGE     MAKEDIPROP(19)

#define DIPROP_KEYNAME     MAKEDIPROP(20)
#define DIPROP_CPOINTS     MAKEDIPROP(21)
#define DIPROP_APPDATA     MAKEDIPROP(22)
#define DIPROP_SCANCODE    MAKEDIPROP(23)
#define DIPROP_VIDPID      MAKEDIPROP(24)
#define DIPROP_USERNAME    MAKEDIPROP(25)
#define DIPROP_TYPENAME    MAKEDIPROP(26)

#define DI8DEVTYPE_KEYBOARD 0x13

#define IDirectInputDevice_GetDeviceData(p,cb,obj,pcnt,flags) \
    (p)->GetDeviceData(cb,obj,pcnt,flags)


typedef struct DIDEVCAPS_DX3 {
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD	dwDevType;
    DWORD	dwAxes;
    DWORD	dwButtons;
    DWORD	dwPOVs;
} DIDEVCAPS_DX3, *LPDIDEVCAPS_DX3;

typedef struct DIDEVCAPS {
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD	dwDevType;
    DWORD	dwAxes;
    DWORD	dwButtons;
    DWORD	dwPOVs;
    DWORD	dwFFSamplePeriod;
    DWORD	dwFFMinTimeResolution;
    DWORD	dwFirmwareRevision;
    DWORD	dwHardwareRevision;
    DWORD	dwFFDriverVersion;
} DIDEVCAPS,*LPDIDEVCAPS;

#define DIDC_ATTACHED		0x00000001
#define DIDC_POLLEDDEVICE	0x00000002
#define DIDC_EMULATED		0x00000004
#define DIDC_POLLEDDATAFORMAT	0x00000008
#define DIDC_FORCEFEEDBACK	0x00000100
#define DIDC_FFATTACK		0x00000200
#define DIDC_FFFADE		0x00000400
#define DIDC_SATURATION		0x00000800
#define DIDC_POSNEGCOEFFICIENTS	0x00001000
#define DIDC_POSNEGSATURATION	0x00002000
#define DIDC_DEADBAND		0x00004000
#define DIDC_STARTDELAY		0x00008000
#define DIDC_ALIAS		0x00010000
#define DIDC_PHANTOM		0x00020000
#define DIDC_HIDDEN		0x00040000


/* SetCooperativeLevel dwFlags */
#define DISCL_EXCLUSIVE		0x00000001
#define DISCL_NONEXCLUSIVE	0x00000002
#define DISCL_FOREGROUND	0x00000004
#define DISCL_BACKGROUND	0x00000008
#define DISCL_NOWINKEY          0x00000010

/* Device FF flags */
#define DISFFC_RESET            0x00000001
#define DISFFC_STOPALL          0x00000002
#define DISFFC_PAUSE            0x00000004
#define DISFFC_CONTINUE         0x00000008
#define DISFFC_SETACTUATORSON   0x00000010
#define DISFFC_SETACTUATORSOFF  0x00000020
  
#define DIGFFS_EMPTY            0x00000001
#define DIGFFS_STOPPED          0x00000002
#define DIGFFS_PAUSED           0x00000004
#define DIGFFS_ACTUATORSON      0x00000010
#define DIGFFS_ACTUATORSOFF     0x00000020
#define DIGFFS_POWERON          0x00000040
#define DIGFFS_POWEROFF         0x00000080
#define DIGFFS_SAFETYSWITCHON   0x00000100
#define DIGFFS_SAFETYSWITCHOFF  0x00000200
#define DIGFFS_USERFFSWITCHON   0x00000400
#define DIGFFS_USERFFSWITCHOFF  0x00000800
#define DIGFFS_DEVICELOST       0x80000000

/* Effect flags */
#define DIEFT_ALL		0x00000000
                                                                                
#define DIEFT_CONSTANTFORCE	0x00000001
#define DIEFT_RAMPFORCE		0x00000002
#define DIEFT_PERIODIC		0x00000003
#define DIEFT_CONDITION		0x00000004
#define DIEFT_CUSTOMFORCE	0x00000005
#define DIEFT_HARDWARE		0x000000FF
#define DIEFT_FFATTACK		0x00000200
#define DIEFT_FFFADE		0x00000400
#define DIEFT_SATURATION	0x00000800
#define DIEFT_POSNEGCOEFFICIENTS 0x00001000
#define DIEFT_POSNEGSATURATION	0x00002000
#define DIEFT_DEADBAND		0x00004000
#define DIEFT_STARTDELAY	0x00008000
#define DIEFT_GETTYPE(n)	LOBYTE(n)
                                                                                
#define DIEFF_OBJECTIDS         0x00000001
#define DIEFF_OBJECTOFFSETS     0x00000002
#define DIEFF_CARTESIAN         0x00000010
#define DIEFF_POLAR             0x00000020
#define DIEFF_SPHERICAL         0x00000040

#define DIEP_DURATION           0x00000001
#define DIEP_SAMPLEPERIOD       0x00000002
#define DIEP_GAIN               0x00000004
#define DIEP_TRIGGERBUTTON      0x00000008
#define DIEP_TRIGGERREPEATINTERVAL 0x00000010
#define DIEP_AXES               0x00000020
#define DIEP_DIRECTION          0x00000040
#define DIEP_ENVELOPE           0x00000080
#define DIEP_TYPESPECIFICPARAMS 0x00000100
#define DIEP_STARTDELAY         0x00000200
#define DIEP_ALLPARAMS_DX5      0x000001FF
#define DIEP_ALLPARAMS          0x000003FF
#define DIEP_START              0x20000000
#define DIEP_NORESTART          0x40000000
#define DIEP_NODOWNLOAD         0x80000000
#define DIEB_NOTRIGGER          0xFFFFFFFF

#define DIES_SOLO               0x00000001
#define DIES_NODOWNLOAD         0x80000000

#define DIEGES_PLAYING          0x00000001
#define DIEGES_EMULATED         0x00000002

#define DI_DEGREES		100
#define DI_FFNOMINALMAX		10000
#define DI_SECONDS		1000000

// DirectInput error codes
#define DIERR_INPUTLOST    0x8007001E   // device input lost, must reacquire
#define DIERR_NOTACQUIRED  0x8007000C   // device not acquired
#define DIERR_INVALIDPARAM 0x80070057   // invalid parameter

typedef struct DICONSTANTFORCE {
	LONG			lMagnitude;
} DICONSTANTFORCE, *LPDICONSTANTFORCE;
typedef const DICONSTANTFORCE *LPCDICONSTANTFORCE;

typedef struct DIRAMPFORCE {
	LONG			lStart;
	LONG			lEnd;
} DIRAMPFORCE, *LPDIRAMPFORCE;
typedef const DIRAMPFORCE *LPCDIRAMPFORCE;

typedef struct DIPERIODIC {
	DWORD			dwMagnitude;
	LONG			lOffset;
	DWORD			dwPhase;
	DWORD			dwPeriod;
} DIPERIODIC, *LPDIPERIODIC;
typedef const DIPERIODIC *LPCDIPERIODIC;

typedef struct DICONDITION {
	LONG			lOffset;
	LONG			lPositiveCoefficient;
	LONG			lNegativeCoefficient;
	DWORD			dwPositiveSaturation;
	DWORD			dwNegativeSaturation;
	LONG			lDeadBand;
} DICONDITION, *LPDICONDITION;
typedef const DICONDITION *LPCDICONDITION;

typedef struct DICUSTOMFORCE {
	DWORD			cChannels;
	DWORD			dwSamplePeriod;
	DWORD			cSamples;
	LPLONG			rglForceData;
} DICUSTOMFORCE, *LPDICUSTOMFORCE;
typedef const DICUSTOMFORCE *LPCDICUSTOMFORCE;

typedef struct DIENVELOPE {
	DWORD			dwSize;
	DWORD			dwAttackLevel;
	DWORD			dwAttackTime;
	DWORD			dwFadeLevel;
	DWORD			dwFadeTime;
} DIENVELOPE, *LPDIENVELOPE;
typedef const DIENVELOPE *LPCDIENVELOPE;

typedef struct DIEFFECT_DX5 {
	DWORD			dwSize;
	DWORD			dwFlags;
	DWORD			dwDuration;
	DWORD			dwSamplePeriod;
	DWORD			dwGain;
	DWORD			dwTriggerButton;
	DWORD			dwTriggerRepeatInterval;
	DWORD			cAxes;
	LPDWORD			rgdwAxes;
	LPLONG			rglDirection;
	LPDIENVELOPE		lpEnvelope;
	DWORD			cbTypeSpecificParams;
	LPVOID			lpvTypeSpecificParams;
} DIEFFECT_DX5, *LPDIEFFECT_DX5;
typedef const DIEFFECT_DX5 *LPCDIEFFECT_DX5;

typedef struct DIEFFECT {
	DWORD			dwSize;
	DWORD			dwFlags;
	DWORD			dwDuration;
	DWORD			dwSamplePeriod;
	DWORD			dwGain;
	DWORD			dwTriggerButton;
	DWORD			dwTriggerRepeatInterval;
	DWORD			cAxes;
	LPDWORD			rgdwAxes;
	LPLONG			rglDirection;
	LPDIENVELOPE		lpEnvelope;
	DWORD			cbTypeSpecificParams;
	LPVOID			lpvTypeSpecificParams;
	DWORD			dwStartDelay;
} DIEFFECT, *LPDIEFFECT;
typedef const DIEFFECT *LPCDIEFFECT;
typedef DIEFFECT DIEFFECT_DX6;
typedef LPDIEFFECT LPDIEFFECT_DX6;

typedef struct DIEFFECTINFOA {
	DWORD			dwSize;
	GUID			guid;
	DWORD			dwEffType;
	DWORD			dwStaticParams;
	DWORD			dwDynamicParams;
	CHAR			tszName[MAX_PATH];
} DIEFFECTINFOA, *LPDIEFFECTINFOA;
typedef const DIEFFECTINFOA *LPCDIEFFECTINFOA;

//////////////////////////////////////////////////////////////////////////////////////

// X Axis
static const GUID GUID_XAxis =
{ 0xA36D02E0, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

static const GUID GUID_YAxis =
{ 0xA36D02E1, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

static const GUID GUID_ZAxis =
{ 0xA36D02E2, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

static const GUID GUID_RxAxis =
{ 0xA36D02E3, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

// Y Rotation (Ry)
static const GUID GUID_RyAxis =
{ 0xA36D02E4, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

// Z Rotation (Rz)
static const GUID GUID_RzAxis =
{ 0xA36D02E5, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };


// Slider (extra axis, e.g. throttle slider)
static const GUID GUID_Slider =
{ 0xA36D02E6, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

// Real DirectInput object GUIDs — games compare guidType against these,
// so they must be distinct (all-zero placeholders alias every class).
static const GUID GUID_POV =
{ 0xA36D02F2, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
static const GUID GUID_Button =
{ 0xA36D02F0, 0xC9F3, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
static const GUID GUID_Key =
{ 0x55728220, 0xD33C, 0x11CF,
  { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

typedef struct DIDEVICEOBJECTINSTANCE {
    uint32_t dwSize;
    GUID     guidType;
    uint32_t dwOfs;
    uint32_t dwType;
    uint32_t dwFlags;
    char     tszName[260]; // MAX_PATH
    // You can omit the rest if unused
} DIDEVICEOBJECTINSTANCE, *LPDIDEVICEOBJECTINSTANCE;

typedef const DIDEVICEOBJECTINSTANCE* LPCDIDEVICEOBJECTINSTANCE;

typedef const void* LPCDIPROPHEADER;
typedef const void* LPCDIDATAFORMAT;

// Callback type stub
typedef BOOL (*LPDIENUMDEVICEOBJECTSCALLBACK)(LPCDIDEVICEOBJECTINSTANCE, LPVOID);

typedef struct DIDEVICEINSTANCE {
    DWORD dwSize;
    GUID  guidInstance;
    GUID  guidProduct;
    DWORD dwDevType;
    char  tszInstanceName[MAX_PATH];
    char  tszProductName[MAX_PATH];
    GUID  guidFFDriver;
    WORD  wUsagePage;
    WORD  wUsage;
} DIDEVICEINSTANCE, *LPDIDEVICEINSTANCE;

// Forward declaration of DIJOYSTATE (legacy struct)
typedef struct DIJOYSTATE {
    LONG lX, lY, lZ;
    LONG lRx, lRy, lRz;
    LONG rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE rgbButtons[32];
} DIJOYSTATE, *LPDIJOYSTATE;

// Represents one event in the device's data buffer
typedef struct DIDEVICEOBJECTDATA {
    DWORD   dwOfs;      // Offset into data format (which axis/button)
    DWORD   dwData;     // Data value (e.g. axis position, button state)
    DWORD   dwTimeStamp;// Timestamp of event
    DWORD   dwSequence; // Sequence number of event
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;

typedef struct {
    const void* pguid;
    uint32_t    dwOfs;
    uint32_t    dwType;
    uint32_t    dwFlags;
} DIOBJECTDATAFORMAT, *LPDIOBJECTDATAFORMAT;


// DIPH_* constants
#define DIPH_DEVICE   0
#define DIPH_BYOFFSET 1
#define DIPH_BYID     2
#define DIPH_BYUSAGE  3

// DIPROPAXISMODE values
#ifndef DIPROPAXISMODE_ABS
#define DIPROPAXISMODE_ABS 0
#define DIPROPAXISMODE_REL 1
#endif

// DIDATAFORMAT struct
typedef struct DIDATAFORMAT {
    DWORD dwSize;
    DWORD dwObjSize;
    DWORD dwFlags;
    DWORD dwDataSize;
    DWORD dwNumObjs;
    void* rgodf;   // pointer to array of DIOBJECTDATAFORMAT
} DIDATAFORMAT, *LPDIDATAFORMAT;

// Stub GUIDs for interface IDs
static const GUID IID_IDirectInputDeviceA = {
    0x5944e680, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};

static const GUID IID_IDirectInputDevice2A = {
    0x5944e682, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};

// Same IID as Device2A — some game code spells it without the 'A' suffix.
static const GUID IID_IDirectInputDevice2 = {
    0x5944e682, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};

// {89521360-AA8A-11CF-BFC7-444553540000}
static const GUID IID_IDirectInputA =
{ 0x89521360, 0xaa8a, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

// {6F1D2B60-D5A0-11CF-BFC7-444553540000}
static const GUID GUID_SysMouse =
{ 0x6f1d2b60, 0xd5a0, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

// {6F1D2B61-D5A0-11CF-BFC7-444553540000}
static const GUID GUID_SysKeyboard =
{ 0x6f1d2b61, 0xd5a0, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

// Buffered-keyboard plumbing (implemented in Hardware/dinput_stub.cpp):
// an SDL event watch captures KEYDOWN/KEYUP transitions into a queue —
// so fast taps are never missed — and signals the DI notification handle.
extern void  RowanDISetKeyNotify(HANDLE hEvent);
extern void  RowanDIKeyWatchEnsure();
extern DWORD RowanDIKeyDrain(DIDEVICEOBJECTDATA* out, DWORD want, bool peek);

struct IDirectInputDeviceA {
    // SDL handle
    SDL_Joystick* sdlJoy = nullptr;
    int deviceIndex = -1;
    bool isMouse = false;
    bool isKeyboard = false;

    // --- Custom data-format bookkeeping ---
    // Rowan games tag each device object with a caller-chosen dwOfs in
    // SetDataFormat (BoB uses flag-encoded 0x300+axis*4 / 0x380+axis*4 /
    // button bases / hat slots).  Buffered events must report the exact
    // dwOfs the game assigned, so we record the format and decode each
    // entry's dwType (class | instance<<8, as EnumObjects reports it)
    // back to a physical object.
    enum { MAXFMTAXES = 32, MAXFMTBTNS = 128, MAXFMTPOVS = 8 };
    long  fmtAxis[MAXFMTAXES] = {};  // reported dwOfs per axis instance, -1 = unmapped
    long  fmtBtn[MAXFMTBTNS]  = {};
    long  fmtPov[MAXFMTPOVS]  = {};
    bool  fmtSet = false;
    // Buffered event state
    std::deque<DIDEVICEOBJECTDATA> evq;
    long  lastAxis[MAXFMTAXES] = {}; // last emitted absolute axis value
    unsigned char lastBtn[MAXFMTBTNS] = {};
    unsigned long lastPov[MAXFMTPOVS] = {};
    bool  primed = false;            // initial-state burst emitted

    virtual ~IDirectInputDeviceA() {
        if (sdlJoy) {
            SDL_JoystickClose(sdlJoy);
            sdlJoy = nullptr;
        }
    }

    // --- IUnknown style methods ---
    HRESULT QueryInterface(const GUID& riid, void** ppvObject) {
        if (!ppvObject) return E_POINTER;
        // Every device we create is really an IDirectInputDevice2A; it adds
        // no data members and its methods are virtual, so handing back the
        // same pointer for either IID is safe (single inheritance → same
        // address, virtual dispatch lands correctly).
        if (memcmp(&riid, &IID_IDirectInputDeviceA, sizeof(GUID)) == 0 ||
            memcmp(&riid, &IID_IDirectInputDevice2A, sizeof(GUID)) == 0 ||
            memcmp(&riid, &IID_IDirectInputDevice2, sizeof(GUID)) == 0) {
            *ppvObject = this;
            return DI_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    ULONG AddRef() { return 1; }
    ULONG Release() { return 1; }

    // --- Device setup ---
    virtual HRESULT Acquire() {
        if (sdlJoy && !SDL_JoystickGetAttached(sdlJoy)) {
            SDL_JoystickClose(sdlJoy);
            sdlJoy = SDL_JoystickOpen(deviceIndex);
        }
        // Reprime so the next drain emits a fresh initial-state burst.
        primed = false;
        evq.clear();
        return DI_OK;
    }

    HRESULT Unacquire() {
        if (isMouse) {
            // SDL doesn’t acquire/release the mouse, so just pretend success
            return DI_OK;
        }
        if (sdlJoy) {
            // Close the SDL joystick handle to release it
            SDL_JoystickClose(sdlJoy);
            sdlJoy = nullptr;
        }
        return DI_OK;
    }

    virtual HRESULT Poll() {
        if (isMouse) {
            // For mouse, SDL_GetMouseState already queries current state.
            // You can pump events to be safe.
            SDL_PumpEvents();
            return DI_OK;
        }
        if (sdlJoy) {
            // Refresh joystick state
            SDL_JoystickUpdate();
            return DI_OK;
        }
        return DIERR_NOTACQUIRED; // no device acquired
    }


    HRESULT SetDataFormat(LPCDIDATAFORMAT lpdf) {
        // Record the caller's object map: each entry's dwType identifies the
        // physical object (class in low byte, instance in bits 8+, exactly as
        // EnumObjects reports them) and dwOfs is the tag the game wants back
        // in buffered DIDEVICEOBJECTDATA events.
        memset(fmtAxis, 0xFF, sizeof(fmtAxis));
        memset(fmtBtn, 0xFF, sizeof(fmtBtn));
        memset(fmtPov, 0xFF, sizeof(fmtPov));
        const DIDATAFORMAT* df = (const DIDATAFORMAT*)lpdf;
        if (df && df->rgodf && df->dwObjSize >= sizeof(DIOBJECTDATAFORMAT) &&
            df->dwNumObjs > 0 && df->dwNumObjs < 4096) {
            const char* base = (const char*)df->rgodf;
            for (DWORD i = 0; i < df->dwNumObjs; ++i) {
                const DIOBJECTDATAFORMAT* o =
                    (const DIOBJECTDATAFORMAT*)(base + i * df->dwObjSize);
                DWORD cls  = DIDFT_GETTYPE(o->dwType);
                DWORD inst = DIDFT_GETINSTANCE(o->dwType);
                if (cls == DIDFT_RELAXIS || cls == DIDFT_ABSAXIS || cls == DIDFT_AXIS) {
                    if (inst < MAXFMTAXES) fmtAxis[inst] = (long)o->dwOfs;
                } else if (cls == DIDFT_POV) {
                    if (inst < MAXFMTPOVS) fmtPov[inst] = (long)o->dwOfs;
                } else if (cls & DIDFT_BUTTON) {
                    if (inst < MAXFMTBTNS) fmtBtn[inst] = (long)o->dwOfs;
                }
            }
        }
        fmtSet = true;
        primed = false;
        evq.clear();
        return DI_OK;
    }

    HRESULT SetCooperativeLevel(HWND hwnd, DWORD dwFlags) {
        // No-op in SDL
        return DI_OK;
    }

    HRESULT SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) {
        // Stubbed, ignore
        return DI_OK;
    }

    // --- Enumeration of objects (axes/buttons) ---
    // guidType MUST carry the real object GUIDs — the game classifies axes
    // by it (GUID_XAxis=first-of-pair → aileron/elevator candidates,
    // GUID_POV → hat, GUID_Button → button).  dwType carries the class in
    // the low byte plus the per-class instance index in bits 8+, which the
    // game copies verbatim into its data format for us to decode later.
    static void FillObj(DIDEVICEOBJECTINSTANCE& o, const GUID& g,
                        DWORD cls, DWORD inst, const char* name)
    {
        memset(&o, 0, sizeof(o));
        o.dwSize   = sizeof(o);
        o.guidType = g;
        o.dwOfs    = inst;
        o.dwType   = cls | DIDFT_MAKEINSTANCE(inst);
        snprintf(o.tszName, sizeof(o.tszName), "%s", name);
    }

    HRESULT EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback,
                        LPVOID pvRef,
                        DWORD dwFlags)
    {
        if (!lpCallback) return DIERR_GENERIC;

        static const GUID* axisGuids[] = {
            &GUID_XAxis, &GUID_YAxis, &GUID_ZAxis,
            &GUID_RxAxis, &GUID_RyAxis, &GUID_RzAxis,
            &GUID_Slider, &GUID_Slider };
        static const char* axisNames[] = {
            "X Axis", "Y Axis", "Z Axis",
            "X Rotation", "Y Rotation", "Z Rotation",
            "Slider 0", "Slider 1" };
        const bool wantAxes = (dwFlags & DIDFT_AXIS) != 0;
        const bool wantBtns = (dwFlags & DIDFT_BUTTON) != 0;
        const bool wantPovs = (dwFlags & DIDFT_POV) != 0;

        if (isMouse) {
            // Mouse axes are RELATIVE — the game tests
            // DIDFT_GETTYPE(dwType)==DIDFT_RELAXIS to pick delta mode.
            int naxes = 3; // X, Y, wheel
            for (int i = 0; i < naxes && wantAxes; ++i) {
                DIDEVICEOBJECTINSTANCE o;
                FillObj(o, *axisGuids[i], DIDFT_RELAXIS, i, axisNames[i]);
                if (lpCallback(&o, pvRef) == DIENUM_STOP) return DI_OK;
            }
            for (int i = 0; i < 5 && wantBtns; ++i) {
                DIDEVICEOBJECTINSTANCE o;
                char nm[32]; snprintf(nm, sizeof(nm), "Button %d", i);
                FillObj(o, GUID_Button, DIDFT_BUTTON, i, nm);
                if (lpCallback(&o, pvRef) == DIENUM_STOP) return DI_OK;
            }
        } else if (sdlJoy && SDL_JoystickGetAttached(sdlJoy)) {
            int axes = SDL_JoystickNumAxes(sdlJoy);
            if (axes > MAXFMTAXES) axes = MAXFMTAXES;
            for (int i = 0; i < axes && wantAxes; ++i) {
                DIDEVICEOBJECTINSTANCE o;
                const GUID* g = (i < 8) ? axisGuids[i] : &GUID_Slider;
                char nm[32]; snprintf(nm, sizeof(nm), "Axis %d", i);
                FillObj(o, *g, DIDFT_ABSAXIS, i, (i < 8) ? axisNames[i] : nm);
                if (lpCallback(&o, pvRef) == DIENUM_STOP) return DI_OK;
            }
            int buttons = SDL_JoystickNumButtons(sdlJoy);
            if (buttons > MAXFMTBTNS) buttons = MAXFMTBTNS;
            for (int i = 0; i < buttons && wantBtns; ++i) {
                DIDEVICEOBJECTINSTANCE o;
                char nm[32]; snprintf(nm, sizeof(nm), "Button %d", i);
                FillObj(o, GUID_Button, DIDFT_BUTTON, i, nm);
                if (lpCallback(&o, pvRef) == DIENUM_STOP) return DI_OK;
            }
            int hats = SDL_JoystickNumHats(sdlJoy);
            if (hats > MAXFMTPOVS) hats = MAXFMTPOVS;
            for (int i = 0; i < hats && wantPovs; ++i) {
                DIDEVICEOBJECTINSTANCE o;
                char nm[32]; snprintf(nm, sizeof(nm), "POV %d", i);
                FillObj(o, GUID_POV, DIDFT_POV, i, nm);
                if (lpCallback(&o, pvRef) == DIENUM_STOP) return DI_OK;
            }
        }
        return DI_OK;
    }

    // --- Buffered events -------------------------------------------------
    // DirectInput reports state *changes* through GetDeviceData, tagged with
    // the dwOfs from the data format.  We diff the SDL state per drain and
    // queue events; the first drain after acquire/format emits an initial
    // snapshot so the game can seed its axis bases.
    void EmitEv(DWORD ofs, DWORD data)
    {
        if (evq.size() < 512) {
            DIDEVICEOBJECTDATA d = {};
            d.dwOfs       = ofs;
            d.dwData      = data;
            d.dwTimeStamp = SDL_GetTicks();
            evq.push_back(d);
        }
    }

    void RefreshEvents()
    {
        if (!fmtSet || isKeyboard)
            return;
        static const bool dbg = getenv("ROWAN_DEBUG_INPUT") != nullptr;

        if (isMouse) {
            // Relative deltas since last drain — SDL_GetRelativeMouseState
            // accumulates motion even when relative mode is off.
            int dx = 0, dy = 0;
            SDL_GetRelativeMouseState(&dx, &dy);
            long deltas[3] = { dx, dy, 0 };
            for (int i = 0; i < 2; ++i) {
                if (fmtAxis[i] < 0) continue;
                long d = deltas[i];
                if (d > 32767) d = 32767;
                if (d < -32768) d = -32768;
                if (d)
                    EmitEv((DWORD)fmtAxis[i], (DWORD)(long)d);
            }
            Uint32 bs = SDL_GetMouseState(NULL, NULL);
            static const int sdlbtn[5] = {
                SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT,
                SDL_BUTTON_X1, SDL_BUTTON_X2 };
            for (int i = 0; i < 5; ++i) {
                if (fmtBtn[i] < 0) continue;
                unsigned char v = (bs & SDL_BUTTON(sdlbtn[i])) ? 0x80 : 0;
                if (!primed || v != lastBtn[i]) {
                    EmitEv((DWORD)fmtBtn[i], v);
                    lastBtn[i] = v;
                }
            }
        } else if (sdlJoy && SDL_JoystickGetAttached(sdlJoy)) {
            SDL_JoystickUpdate();
            int axes = SDL_JoystickNumAxes(sdlJoy);
            if (axes > MAXFMTAXES) axes = MAXFMTAXES;
            for (int i = 0; i < axes; ++i) {
                if (fmtAxis[i] < 0) continue;
                // DirectInput absolute axes: 0..65535 unsigned.
                long v = (long)SDL_JoystickGetAxis(sdlJoy, i) + 32768;
                if (!primed || v != lastAxis[i]) {
                    EmitEv((DWORD)fmtAxis[i], (DWORD)v);
                    lastAxis[i] = v;
                }
            }
            int btns = SDL_JoystickNumButtons(sdlJoy);
            if (btns > MAXFMTBTNS) btns = MAXFMTBTNS;
            for (int i = 0; i < btns; ++i) {
                if (fmtBtn[i] < 0) continue;
                unsigned char v = SDL_JoystickGetButton(sdlJoy, i) ? 0x80 : 0;
                if (!primed || v != lastBtn[i]) {
                    EmitEv((DWORD)fmtBtn[i], v);
                    lastBtn[i] = v;
                }
            }
            int hats = SDL_JoystickNumHats(sdlJoy);
            if (hats > MAXFMTPOVS) hats = MAXFMTPOVS;
            for (int i = 0; i < hats; ++i) {
                if (fmtPov[i] < 0) continue;
                Uint8 h = SDL_JoystickGetHat(sdlJoy, i);
                DWORD v;
                switch (h) {
                case SDL_HAT_UP:        v = 0;     break;
                case SDL_HAT_RIGHTUP:   v = 4500;  break;
                case SDL_HAT_RIGHT:     v = 9000;  break;
                case SDL_HAT_RIGHTDOWN: v = 13500; break;
                case SDL_HAT_DOWN:      v = 18000; break;
                case SDL_HAT_LEFTDOWN:  v = 22500; break;
                case SDL_HAT_LEFT:      v = 27000; break;
                case SDL_HAT_LEFTUP:    v = 31500; break;
                default:                v = 0xffff; break; // centered
                }
                if (!primed || v != lastPov[i]) {
                    EmitEv((DWORD)fmtPov[i], v);
                    lastPov[i] = v;
                }
            }
        }
        if (dbg && !evq.empty())
            fprintf(stderr, "[di] %s queued=%zu\n",
                    isMouse ? "mouse" : "joy", evq.size());
        primed = true;
    }

    // --- Polling state ---
    // If the game installed a custom data format (BoB's analogue layer does),
    // fill its buffer layout: each object lands at the dwOfs the game chose.
    // Otherwise fall back to the canned DIJOYSTATE layout.
    virtual HRESULT GetDeviceState(DWORD cbData, LPVOID lpvData) {
        if (!lpvData) return DIERR_GENERIC;

        if (fmtSet && !isKeyboard) {
            memset(lpvData, 0, cbData);
            char* buf = (char*)lpvData;
            if (isMouse) {
                int x, y;
                Uint32 bs = SDL_GetMouseState(&x, &y);
                long axes[3] = { x, y, 0 };
                for (int i = 0; i < 3; ++i)
                    if (fmtAxis[i] >= 0 && (DWORD)fmtAxis[i] + 4 <= cbData)
                        *(long*)(buf + fmtAxis[i]) = axes[i];
                static const int sdlbtn[5] = {
                    SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT,
                    SDL_BUTTON_X1, SDL_BUTTON_X2 };
                for (int i = 0; i < 5; ++i)
                    if (fmtBtn[i] >= 0 && (DWORD)fmtBtn[i] < cbData)
                        buf[fmtBtn[i]] = (bs & SDL_BUTTON(sdlbtn[i])) ? (char)0x80 : 0;
            } else if (sdlJoy && SDL_JoystickGetAttached(sdlJoy)) {
                SDL_JoystickUpdate();
                int axes = SDL_JoystickNumAxes(sdlJoy);
                if (axes > MAXFMTAXES) axes = MAXFMTAXES;
                for (int i = 0; i < axes; ++i)
                    if (fmtAxis[i] >= 0 && (DWORD)fmtAxis[i] + 4 <= cbData)
                        *(long*)(buf + fmtAxis[i]) =
                            (long)SDL_JoystickGetAxis(sdlJoy, i) + 32768;
                int btns = SDL_JoystickNumButtons(sdlJoy);
                if (btns > MAXFMTBTNS) btns = MAXFMTBTNS;
                for (int i = 0; i < btns; ++i)
                    if (fmtBtn[i] >= 0 && (DWORD)fmtBtn[i] < cbData)
                        buf[fmtBtn[i]] = SDL_JoystickGetButton(sdlJoy, i) ? (char)0x80 : 0;
                int hats = SDL_JoystickNumHats(sdlJoy);
                if (hats > MAXFMTPOVS) hats = MAXFMTPOVS;
                for (int i = 0; i < hats; ++i)
                    if (fmtPov[i] >= 0 && (DWORD)fmtPov[i] + 4 <= cbData)
                        *(DWORD*)(buf + fmtPov[i]) = 0xffff;
            }
            return DI_OK;
        }

        auto* state = reinterpret_cast<DIJOYSTATE*>(lpvData);

        memset(state, 0, sizeof(DIJOYSTATE));

        if (isMouse) {
            int x, y;
            Uint32 buttons = SDL_GetMouseState(&x, &y);
            state->lX = x;
            state->lY = y;
            state->lZ = 0;
            for (int b = 0; b < 32; ++b) {
                state->rgbButtons[b] = (buttons & SDL_BUTTON(b+1)) ? 0x80 : 0x00;
            }
        } else if (sdlJoy) {
            SDL_JoystickUpdate();

            int numAxes = SDL_JoystickNumAxes(sdlJoy);
            if (numAxes > 0) state->lX = SDL_JoystickGetAxis(sdlJoy, 0);
            if (numAxes > 1) state->lY = SDL_JoystickGetAxis(sdlJoy, 1);
            if (numAxes > 2) state->lZ = SDL_JoystickGetAxis(sdlJoy, 2);
            if (numAxes > 3) state->lRx = SDL_JoystickGetAxis(sdlJoy, 3);
            if (numAxes > 4) state->lRy = SDL_JoystickGetAxis(sdlJoy, 4);
            if (numAxes > 5) state->lRz = SDL_JoystickGetAxis(sdlJoy, 5);

            int sliders = std::min(2, numAxes - 6);
            for (int i = 0; i < sliders; ++i)
                state->rglSlider[i] = SDL_JoystickGetAxis(sdlJoy, 6+i);

            int hats = SDL_JoystickNumHats(sdlJoy);
            for (int i = 0; i < 4; ++i) {
                if (i < hats) {
                    Uint8 hat = SDL_JoystickGetHat(sdlJoy, i);
                    switch (hat) {
                        case SDL_HAT_UP:    state->rgdwPOV[i] = 0; break;
                        case SDL_HAT_RIGHT: state->rgdwPOV[i] = 9000; break;
                        case SDL_HAT_DOWN:  state->rgdwPOV[i] = 18000; break;
                        case SDL_HAT_LEFT:  state->rgdwPOV[i] = 27000; break;
                        default:            state->rgdwPOV[i] = -1; break; // centered
                    }
                } else {
                    state->rgdwPOV[i] = -1;
                }
            }

            int buttons = SDL_JoystickNumButtons(sdlJoy);
            for (int b = 0; b < 32; ++b) {
                state->rgbButtons[b] = (b < buttons && SDL_JoystickGetButton(sdlJoy, b)) ? 0x80 : 0x00;
            }
        }
        return DI_OK;
    }

    virtual HRESULT GetDeviceData(DWORD cbObjectData,
                        LPDIDEVICEOBJECTDATA rgdod,
                        LPDWORD pdwInOut,
                        DWORD dwFlags)
    {
        if (!pdwInOut) return DIERR_GENERIC;
        DWORD want = *pdwInOut;
        *pdwInOut = 0;

        RefreshEvents(); // diff current SDL state into the queue

        if (!rgdod || !want)
            return DI_OK;

        DWORD n = (DWORD)((want < (DWORD)evq.size()) ? want : evq.size());
        for (DWORD i = 0; i < n; ++i)
            rgdod[i] = evq[i];
        if (!(dwFlags & DIGDD_PEEK))
            for (DWORD i = 0; i < n; ++i)
                evq.pop_front();
        *pdwInOut = n;
        return DI_OK;
    }

    HRESULT GetCapabilities(DIDEVCAPS* caps) {
        if (!caps) return E_POINTER;

        memset(caps, 0, sizeof(DIDEVCAPS));
        caps->dwSize = sizeof(DIDEVCAPS);
        caps->dwFlags = 0; // no force feedback
        if (isMouse) {
            caps->dwDevType = DIDEVTYPE_MOUSE;
            caps->dwAxes = 3;
            caps->dwButtons = 5;
            caps->dwPOVs = 0;
        } else if (sdlJoy && SDL_JoystickGetAttached(sdlJoy)) {
            caps->dwDevType = DIDEVTYPE_JOYSTICK;
            caps->dwAxes = SDL_JoystickNumAxes(sdlJoy);
            caps->dwButtons = SDL_JoystickNumButtons(sdlJoy);
            caps->dwPOVs = SDL_JoystickNumHats(sdlJoy);
        } else {
            caps->dwDevType = DIDEVTYPE_KEYBOARD;
            caps->dwAxes = 0;
            caps->dwButtons = 256;
            caps->dwPOVs = 0;
        }
        return DI_OK;
    }

    virtual HRESULT SetEventNotification(HANDLE hEvent) {
        // Game only checks FAILED() or not; no real event needed
        return DI_OK;
    }

    HRESULT RunControlPanel(HWND hwndOwner, DWORD dwFlags)
    {
        // DirectInput's control panel is a Windows-only feature.
        // Rowan only checks success/failure, so just pretend it worked.
        return DI_OK;
    }

};

// ----- DIDEVICEOBJECTDATA -----

// DIPROPHEADER
typedef struct DIPROPHEADER {
    DWORD dwSize;
    DWORD dwHeaderSize;
    DWORD dwObj;
    DWORD dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;

// DIPROPDWORD
typedef struct DIPROPDWORD {
    DIPROPHEADER diph;
    DWORD        dwData;
} DIPROPDWORD, *LPDIPROPDWORD;

// DIEFFECTINFO stub + pointer type
typedef struct DIEFFECTINFO {
    DWORD dwSize;
    char  tszName[260]; // optional stub
} DIEFFECTINFO, *LPDIEFFECTINFO;
typedef const DIEFFECTINFO* LPCDIEFFECTINFO;

typedef struct IDirectInputEffect *LPDIRECTINPUTEFFECT;

// ----- Effect enumeration callback -----
// In real DirectInput, this is called once per effect during EnumEffects.
// For a stub, just define the signature and ignore the body.
typedef BOOL (*LPDIENUMEFFECTSCALLBACK)(LPCDIEFFECTINFO pdei, LPVOID pvRef);

// ----- Escape structure -----
// Used for vendor‑specific extensions. You can stub with minimal fields.
typedef struct DIEFFESCAPE {
    DWORD   dwSize;     // size of this structure
    DWORD   dwCommand;  // vendor-specific command
    LPVOID  lpInBuffer; // input data
    DWORD   cbInBuffer; // size of input data
    LPVOID  lpOutBuffer;// output data
    DWORD   cbOutBuffer;// size of output data
} DIEFFESCAPE, *LPDIEFFESCAPE;

// SDL scancode -> DirectInput (set-1 make code) for the keys BoB's
// commonkeymaps can bind: alnum, F-keys, arrows, modifiers, keypad.
// 0 means unmapped (skipped in event diffs).
inline int RowanSdlToDik(int sc)
{
    switch (sc) {
    case SDL_SCANCODE_ESCAPE: return 0x01;
    case SDL_SCANCODE_1: return 0x02; case SDL_SCANCODE_2: return 0x03;
    case SDL_SCANCODE_3: return 0x04; case SDL_SCANCODE_4: return 0x05;
    case SDL_SCANCODE_5: return 0x06; case SDL_SCANCODE_6: return 0x07;
    case SDL_SCANCODE_7: return 0x08; case SDL_SCANCODE_8: return 0x09;
    case SDL_SCANCODE_9: return 0x0A; case SDL_SCANCODE_0: return 0x0B;
    case SDL_SCANCODE_MINUS: return 0x0C; case SDL_SCANCODE_EQUALS: return 0x0D;
    case SDL_SCANCODE_BACKSPACE: return 0x0E; case SDL_SCANCODE_TAB: return 0x0F;
    case SDL_SCANCODE_Q: return 0x10; case SDL_SCANCODE_W: return 0x11;
    case SDL_SCANCODE_E: return 0x12; case SDL_SCANCODE_R: return 0x13;
    case SDL_SCANCODE_T: return 0x14; case SDL_SCANCODE_Y: return 0x15;
    case SDL_SCANCODE_U: return 0x16; case SDL_SCANCODE_I: return 0x17;
    case SDL_SCANCODE_O: return 0x18; case SDL_SCANCODE_P: return 0x19;
    case SDL_SCANCODE_LEFTBRACKET: return 0x1A;
    case SDL_SCANCODE_RIGHTBRACKET: return 0x1B;
    case SDL_SCANCODE_RETURN: return 0x1C; case SDL_SCANCODE_LCTRL: return 0x1D;
    case SDL_SCANCODE_A: return 0x1E; case SDL_SCANCODE_S: return 0x1F;
    case SDL_SCANCODE_D: return 0x20; case SDL_SCANCODE_F: return 0x21;
    case SDL_SCANCODE_G: return 0x22; case SDL_SCANCODE_H: return 0x23;
    case SDL_SCANCODE_J: return 0x24; case SDL_SCANCODE_K: return 0x25;
    case SDL_SCANCODE_L: return 0x26; case SDL_SCANCODE_SEMICOLON: return 0x27;
    case SDL_SCANCODE_APOSTROPHE: return 0x28;
    case SDL_SCANCODE_GRAVE: return 0x29; case SDL_SCANCODE_LSHIFT: return 0x2A;
    case SDL_SCANCODE_BACKSLASH: return 0x2B;
    case SDL_SCANCODE_Z: return 0x2C; case SDL_SCANCODE_X: return 0x2D;
    case SDL_SCANCODE_C: return 0x2E; case SDL_SCANCODE_V: return 0x2F;
    case SDL_SCANCODE_B: return 0x30; case SDL_SCANCODE_N: return 0x31;
    case SDL_SCANCODE_M: return 0x32; case SDL_SCANCODE_COMMA: return 0x33;
    case SDL_SCANCODE_PERIOD: return 0x34; case SDL_SCANCODE_SLASH: return 0x35;
    case SDL_SCANCODE_RSHIFT: return 0x36;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x37;
    case SDL_SCANCODE_LALT: return 0x38; case SDL_SCANCODE_SPACE: return 0x39;
    case SDL_SCANCODE_CAPSLOCK: return 0x3A;
    case SDL_SCANCODE_F1: return 0x3B; case SDL_SCANCODE_F2: return 0x3C;
    case SDL_SCANCODE_F3: return 0x3D; case SDL_SCANCODE_F4: return 0x3E;
    case SDL_SCANCODE_F5: return 0x3F; case SDL_SCANCODE_F6: return 0x40;
    case SDL_SCANCODE_F7: return 0x41; case SDL_SCANCODE_F8: return 0x42;
    case SDL_SCANCODE_F9: return 0x43; case SDL_SCANCODE_F10: return 0x44;
    case SDL_SCANCODE_NUMLOCKCLEAR: return 0x45;
    case SDL_SCANCODE_SCROLLLOCK: return 0x46;
    case SDL_SCANCODE_KP_7: return 0x47; case SDL_SCANCODE_KP_8: return 0x48;
    case SDL_SCANCODE_KP_9: return 0x49; case SDL_SCANCODE_KP_MINUS: return 0x4A;
    case SDL_SCANCODE_KP_4: return 0x4B; case SDL_SCANCODE_KP_5: return 0x4C;
    case SDL_SCANCODE_KP_6: return 0x4D; case SDL_SCANCODE_KP_PLUS: return 0x4E;
    case SDL_SCANCODE_KP_1: return 0x4F; case SDL_SCANCODE_KP_2: return 0x50;
    case SDL_SCANCODE_KP_3: return 0x51; case SDL_SCANCODE_KP_0: return 0x52;
    case SDL_SCANCODE_KP_PERIOD: return 0x53;
    case SDL_SCANCODE_NONUSBACKSLASH: return 0x56;
    case SDL_SCANCODE_F11: return 0x57; case SDL_SCANCODE_F12: return 0x58;
    case SDL_SCANCODE_KP_ENTER: return 0x9C; case SDL_SCANCODE_RCTRL: return 0x9D;
    case SDL_SCANCODE_KP_DIVIDE: return 0xB5;
    case SDL_SCANCODE_HOME: return 0xC7; case SDL_SCANCODE_UP: return 0xC8;
    case SDL_SCANCODE_PAGEUP: return 0xC9;
    case SDL_SCANCODE_LEFT: return 0xCB; case SDL_SCANCODE_RIGHT: return 0xCD;
    case SDL_SCANCODE_END: return 0xCF; case SDL_SCANCODE_DOWN: return 0xD0;
    case SDL_SCANCODE_PAGEDOWN: return 0xD1;
    case SDL_SCANCODE_INSERT: return 0xD2; case SDL_SCANCODE_DELETE: return 0xD3;
    case SDL_SCANCODE_RALT: return 0xB8;
    }
    return 0;
}

struct IDirectInputDevice2A : public IDirectInputDeviceA {
    // Inherit everything from IDirectInputDeviceA

    // The SDL key watch runs as soon as the keyboard is acquired — the
    // queue fills even if the game never arms a notification handle.
    HRESULT Acquire() override {
        if (isKeyboard)
            RowanDIKeyWatchEnsure();
        return IDirectInputDeviceA::Acquire();
    }

    HRESULT SetEventNotification(HANDLE hEvent) override {
        if (isKeyboard)
            RowanDISetKeyNotify(hEvent);
        return DI_OK;
    }

    // Keyboards don't need Poll to do work — buffered data is generated
    // by the SDL event watch, not lazily at poll time.
    HRESULT Poll() override {
        if (isKeyboard)
            return DI_OK;
        return IDirectInputDeviceA::Poll();
    }

    // c_dfDIKeyboard immediate state: 256 bytes indexed by DIK code.
    HRESULT GetDeviceState(DWORD cbData, LPVOID lpvData) override {
        if (isKeyboard) {
            if (!lpvData) return DIERR_GENERIC;
            Uint8* kstate = reinterpret_cast<Uint8*>(lpvData);
            memset(kstate, 0, cbData);
            const Uint8* ks = SDL_GetKeyboardState(NULL);
            for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc) {
                int dik = RowanSdlToDik(sc) & 0xFF;
                if (dik && dik < (int)cbData && ks[sc])
                    kstate[dik] = 0x80;
            }
            return DI_OK;
        }
        return IDirectInputDeviceA::GetDeviceState(cbData, lpvData);
    }

    // --- Extra methods in Device2 ---
    HRESULT GetDeviceData(DWORD cbObjectData,
                          LPDIDEVICEOBJECTDATA rgdod,
                          LPDWORD pdwInOut,
                          DWORD dwFlags) override
    {
        if (!pdwInOut) return DIERR_GENERIC;
        if (!isKeyboard)
            return IDirectInputDeviceA::GetDeviceData(cbObjectData, rgdod,
                                                    pdwInOut, dwFlags);
        DWORD want = *pdwInOut;
        *pdwInOut = 0;
        if (!rgdod || !want)
            return DI_OK;
        *pdwInOut = RowanDIKeyDrain(rgdod, want, (dwFlags & DIGDD_PEEK) != 0);
        return DI_OK;
    }

    // Force feedback stubs
    HRESULT GetForceFeedbackState(DWORD* pdwOut) {
        if (pdwOut) *pdwOut = 0; // no FF supported
        return DI_OK;
    }

    HRESULT SendForceFeedbackCommand(DWORD dwCommand) {
        return DI_OK;
    }

    HRESULT CreateEffect(REFGUID rguid,
                         LPCDIEFFECT lpeff,
                         LPDIRECTINPUTEFFECT* ppdeff,
                         LPUNKNOWN punkOuter)
    {
        if (ppdeff) *ppdeff = nullptr;
        return DI_OK; // stubbed, no force feedback
    }

    HRESULT EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback,
                        LPVOID pvRef,
                        DWORD dwFlags)
    {
        // No effects supported
        return DI_OK;
    }

    // DX7: enumerate effect objects created via CreateEffect — none exist.
    typedef BOOL (*LPDIENUMCREATEDEFFECTOBJECTSCALLBACK)(LPDIRECTINPUTEFFECT, LPVOID);
    HRESULT EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,
                                     LPVOID, DWORD)
    {
        return DI_OK;
    }

    HRESULT GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid)
    {
        if (pdei) memset(pdei, 0, sizeof(DIEFFECTINFO));
        return DI_OK;
    }

    HRESULT Escape(LPDIEFFESCAPE pesc)
    {
        return DI_OK;
    }
};

// Axis configuration structure (stub)
typedef struct SWFFAxisConfig {
    DWORD dwAxis;     // which axis (X, Y, Z, etc.)
    DWORD dwFlags;    // configuration flags
    LONG  lMin;       // min value
    LONG  lMax;       // max value
} SWFFAxisConfig;

// Stub functions

// (interface/system GUIDs moved up — see above the device classes)


#if 0
#else
// Forward declare
struct IDirectInputA;
typedef struct IDirectInputA IDirectInputA;

typedef struct IDirectInputAVtbl {
    // IUnknown
    HRESULT (*QueryInterface)(IDirectInputA* This, const void* riid, void** ppvObject);
    ULONG   (*AddRef)(IDirectInputA* This);
    ULONG   (*Release)(IDirectInputA* This);

    // IDirectInputA methods
    HRESULT (*CreateDevice)(IDirectInputA* This, const GUID& rguid,
                            LPDIRECTINPUTDEVICEA* lplpDevice,
                            LPUNKNOWN pUnkOuter);

    HRESULT (*EnumDevices)(IDirectInputA* This, DWORD dwDevType,
                           LPDIENUMDEVICESCALLBACK lpCallback,
                           LPVOID pvRef, DWORD dwFlags);

    HRESULT (*GetDeviceStatus)(IDirectInputA* This, const GUID& rguid);

} IDirectInputAVtbl;

class IDirectInputA {
public:
    const IDirectInputAVtbl* lpVtbl;
    ULONG refCount = 1;

    HRESULT QueryInterface(const IID& iid, void** ppv);
    ULONG AddRef();
    ULONG Release();

    // your existing methods
    HRESULT CreateDevice(const GUID& rguid,
                         LPDIRECTINPUTDEVICEA* lplpDevice,
                         LPUNKNOWN pUnkOuter);

    HRESULT EnumDevices(DWORD dwDevType,
                        LPDIENUMDEVICESCALLBACK lpCallback,
                        LPVOID pvRef,
                        DWORD dwFlags);

    HRESULT GetDeviceStatus(const GUID& rguid);
    HRESULT RunControlPanel(HWND hwndOwner, DWORD dwFlags);
};

static HRESULT IDI_QueryInterface(IDirectInputA* This, const void* riid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (memcmp(riid, &IID_IDirectInputA, sizeof(GUID)) == 0) {
        *ppv = This;
        This->lpVtbl->AddRef(This);
        return DI_OK;
    }
    return E_NOINTERFACE;
}

static ULONG IDI_AddRef(IDirectInputA* This) {
    return ++This->refCount;
}

static ULONG IDI_Release(IDirectInputA* This) {
    ULONG r = --This->refCount;
    if (r == 0) {
        delete This;
    }
    return r;
}

static HRESULT IDI_CreateDevice(IDirectInputA* This, const GUID& rguid,
                                LPDIRECTINPUTDEVICEA* lplpDevice,
                                LPUNKNOWN pUnkOuter)
{
    if (!lplpDevice) return DIERR_GENERIC;

    // Decide which interface is requested — always build the Device2A
    // object; QueryInterface hands back the same pointer for either IID.
    if (memcmp(&rguid, &IID_IDirectInputDeviceA, sizeof(GUID)) == 0) {
        auto* dev = new IDirectInputDevice2A();
        *lplpDevice = dev;
        return DI_OK;
    }
    else if (memcmp(&rguid, &IID_IDirectInputDevice2A, sizeof(GUID)) == 0) {
        auto* dev2 = new IDirectInputDevice2A();
        *lplpDevice = dev2;
        return DI_OK;
    }
    else {
        *lplpDevice = nullptr;
        return DIERR_NOTINITIALIZED; // unknown interface
    }
}

static HRESULT IDI_EnumDevices(IDirectInputA* This, DWORD dwDevType,
                               LPDIENUMDEVICESCALLBACK lpCallback,
                               LPVOID pvRef, DWORD dwFlags)
{
    if (!lpCallback) return DIERR_GENERIC;
    return This->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

static HRESULT IDI_GetDeviceStatus(IDirectInputA* This, const GUID& rguid)
{
    // For now we only distinguish joystick vs mouse
    if (rguid.Data1 == DIDEVTYPE_MOUSE) {
        // SDL always exposes the system mouse
        return DI_OK;
    }
    else if (rguid.Data1 == DIDEVTYPE_JOYSTICK) {
        // Check if any joystick is still attached
        int numJoy = SDL_NumJoysticks();
        if (numJoy > 0) {
            // Optionally: loop through all and check SDL_JoystickGetAttached
            for (int i = 0; i < numJoy; ++i) {
                SDL_Joystick* joy = SDL_JoystickOpen(i);
                if (joy) {
                    bool attached = SDL_JoystickGetAttached(joy);
                    SDL_JoystickClose(joy);
                    if (attached) return DI_OK;
                }
            }
            return DI_NOTATTACHED;
        } else {
            return DI_NOTATTACHED;
        }
    }
    // Unknown GUID → pretend not attached
    return DI_NOTATTACHED;
}

#endif

// Function stubs
static inline HRESULT DirectInput8Create(void* hinst, unsigned long version,
                                         const GUID* riid, void** ppvOut, void* punkOuter)
{
    (void)hinst; (void)version; (void)riid; (void)ppvOut; (void)punkOuter;
    return DIERR_GENERIC;
}

// Older DirectInputCreate stub
typedef void* HINSTANCE;
HRESULT DirectInputCreate(HINSTANCE hinst,
                                 uint32_t dwVersion,
                                 LPDIRECTINPUT* ppDI,
                                 LPUNKNOWN punkOuter);

typedef IDirectInputDevice2A* LPDIRECTINPUTDEVICE2;

/* DirectInput effect-type GUIDs (dinput.h SDK) — used by BoB's SWFF code */
static const GUID GUID_ConstantForce = {
    0x13541c2b, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_RampForce = {
    0x13541c2c, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_Square = {
    0x13541c2d, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_Sine = {
    0x13541c2e, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_Triangle = {
    0x13541c2f, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_SawtoothUp = {
    0x13541c30, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_SawtoothDown = {
    0x13541c31, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_Damper = {
    0x13541c33, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_Inertia = {
    0x13541c34, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_Friction = {
    0x13541c35, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };
static const GUID GUID_CustomForce = {
    0x13541c36, 0x8e33, 0x11d0, {0x9a,0xd0,0x00,0xa0,0xc9,0xa0,0x6e,0x35} };

#ifndef X_AXIS
#define X_AXIS 1
#endif
#ifndef Y_AXIS
#define Y_AXIS 2
#endif

// DIEFFECT parameter flags
#define DIEP_DURATION            0x00000001
#define DIEP_SAMPLEPERIOD        0x00000002
#define DIEP_GAIN                0x00000004
#define DIEP_TRIGGERBUTTON       0x00000008
#define DIEP_TRIGGERREPEATINTERVAL 0x00000010

struct IDirectInputEffect {
    // IUnknown methods
    HRESULT QueryInterface(const GUID& riid, void** ppvObject) {
        if (ppvObject) *ppvObject = nullptr;
        return DI_OK;
    }
    ULONG AddRef() { return 1; }
    ULONG Release() { return 1; }

    // Effect methods
    HRESULT Initialize(void* hinst, DWORD dwVersion, const GUID& rguid) {
        return DI_OK;
    }

    HRESULT GetEffectGuid(GUID* pguid) {
        if (pguid) *pguid = GUID{}; // zero GUID
        return DI_OK;
    }

    HRESULT GetParameters(DIEFFECT* peff, DWORD dwFlags) {
        if (peff) peff->dwSize = sizeof(DIEFFECT);
        return DI_OK;
    }

    HRESULT SetParameters(const DIEFFECT* peff, DWORD dwFlags) {
        return DI_OK;
    }

    HRESULT Start(DWORD dwIterations, DWORD dwFlags) {
        return DI_OK;
    }

    HRESULT Stop() {
        return DI_OK;
    }

    HRESULT Unload() {
        return DI_OK;
    }

    HRESULT GetEffectStatus(DWORD* pdwFlags) {
        if (pdwFlags) *pdwFlags = 0;
        return DI_OK;
    }

    HRESULT Download() {
        return DI_OK;
    }

    HRESULT Escape(void* pesc) {
        return DI_OK;
    }
};

#ifndef SFERR_INVALID_PARAM
#define SFERR_INVALID_PARAM 0x80070057  // same as E_INVALIDARG
#endif

#ifndef E_INVALIDARG
#define E_INVALIDARG 0x80070057
#endif

// GUID_Spring (DirectInput predefined effect type)
static const GUID GUID_Spring = {
    0x13541C20, 0x8E33, 0x11D0,
    {0x9A, 0x7F, 0x00, 0xA0, 0xC9, 0x0C, 0xA9, 0x26}
};

// Define DIK_* constants (from dinput.h). 
enum DIK {
    DIK_ESCAPE       = 0x01,
    DIK_1            = 0x02, DIK_2 = 0x03, DIK_3 = 0x04, DIK_4 = 0x05,
    DIK_5            = 0x06, DIK_6 = 0x07, DIK_7 = 0x08, DIK_8 = 0x09,
    DIK_9            = 0x0A, DIK_0 = 0x0B,
    DIK_MINUS        = 0x0C, DIK_EQUALS = 0x0D, DIK_BACK = 0x0E,
    DIK_TAB          = 0x0F,

    DIK_Q            = 0x10, DIK_W = 0x11, DIK_E = 0x12, DIK_R = 0x13,
    DIK_T            = 0x14, DIK_Y = 0x15, DIK_U = 0x16, DIK_I = 0x17,
    DIK_O            = 0x18, DIK_P = 0x19,
    DIK_LBRACKET     = 0x1A, DIK_RBRACKET = 0x1B,
    DIK_RETURN       = 0x1C, DIK_LCONTROL = 0x1D,

    DIK_A            = 0x1E, DIK_S = 0x1F, DIK_D = 0x20, DIK_F = 0x21,
    DIK_G            = 0x22, DIK_H = 0x23, DIK_J = 0x24, DIK_K = 0x25,
    DIK_L            = 0x26, DIK_SEMICOLON = 0x27, DIK_APOSTROPHE = 0x28,
    DIK_GRAVE        = 0x29,
    DIK_LSHIFT       = 0x2A, DIK_BACKSLASH = 0x2B,
    DIK_Z            = 0x2C, DIK_X = 0x2D, DIK_C = 0x2E, DIK_V = 0x2F,
    DIK_B            = 0x30, DIK_N = 0x31, DIK_M = 0x32,
    DIK_COMMA        = 0x33, DIK_PERIOD = 0x34, DIK_SLASH = 0x35,
    DIK_RSHIFT       = 0x36, DIK_MULTIPLY = 0x37,
    DIK_LALT         = 0x38, DIK_SPACE = 0x39,
    DIK_CAPSLOCK     = 0x3A,

    DIK_F1           = 0x3B, DIK_F2 = 0x3C, DIK_F3 = 0x3D, DIK_F4 = 0x3E,
    DIK_F5           = 0x3F, DIK_F6 = 0x40, DIK_F7 = 0x41, DIK_F8 = 0x42,
    DIK_F9           = 0x43, DIK_F10 = 0x44,

    DIK_NUMLOCK      = 0x45, DIK_SCROLL = 0x46,
    DIK_PAUSE        = 0xC5, // Added Pause key
    DIK_NUMPAD7      = 0x47, DIK_NUMPAD8 = 0x48, DIK_NUMPAD9 = 0x49,
    DIK_SUBTRACT     = 0x4A, DIK_NUMPAD4 = 0x4B, DIK_NUMPAD5 = 0x4C,
    DIK_NUMPAD6      = 0x4D, DIK_ADD = 0x4E,
    DIK_NUMPAD1      = 0x4F, DIK_NUMPAD2 = 0x50, DIK_NUMPAD3 = 0x51,
    DIK_NUMPAD0      = 0x52, DIK_DECIMAL = 0x53,

    DIK_F11          = 0x57, DIK_F12 = 0x58,

    DIK_NUMPADENTER  = 0x9C, DIK_RCONTROL = 0x9D,
    DIK_DIVIDE       = 0xB5,
    DIK_SYSRQ        = 0xB7, DIK_RALT = 0xB8,
    DIK_HOME         = 0xC7, DIK_UP = 0xC8, DIK_PRIOR = 0xC9,
    DIK_LEFT         = 0xCB, DIK_RIGHT = 0xCD,
    DIK_END          = 0xCF, DIK_DOWN = 0xD0, DIK_NEXT = 0xD1,
    DIK_INSERT       = 0xD2, DIK_DELETE = 0xD3,
    DIK_LWIN         = 0xDB, DIK_RWIN = 0xDC, DIK_APPS = 0xDD
};

// DirectInput keyboard scan codes (subset)
#define DIK_LMENU        0x38    // Left Alt
#define DIK_RMENU        0xB8    // Right Alt
#define DIK_CAPITAL      0x3A    // Caps Lock

#define DIK_F13          0x64
#define DIK_F14          0x65
#define DIK_F15          0x66

#define DIK_KANA         0x70
#define DIK_CONVERT      0x79
#define DIK_NOCONVERT    0x7B
#define DIK_YEN          0x7D

#define DIK_NUMPADEQUALS 0x8D
#define DIK_CIRCUMFLEX   0x90
#define DIK_AT           0x91
#define DIK_COLON        0x92
#define DIK_UNDERLINE    0x93
#define DIK_KANJI        0x94
#define DIK_STOP         0x95
#define DIK_AX           0x96
#define DIK_UNLABELED    0x97
#define DIK_NUMPADCOMMA  0xB3

static DIOBJECTDATAFORMAT g_DIKeyboardObjects[256] = {};

const DIDATAFORMAT c_dfDIKeyboard = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_RELAXIS,
    256,
    256,
    g_DIKeyboardObjects
};


#ifdef __cplusplus
} // extern "C"
#endif
