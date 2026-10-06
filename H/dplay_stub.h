#ifndef __DPLAY_H__
#define __DPLAY_H__

#include <stdint.h>

/* Boolean values */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* HRESULT success/failure macros */
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

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

/* DirectPlay return codes */
#define DP_OK            ((HRESULT)0x00000000L)  /* same as S_OK */
#define DPERR_GENERIC    ((HRESULT)0x80004005L)  /* E_FAIL */
#define DPERR_NOMEMORY   ((HRESULT)0x8007000EL)  /* E_OUTOFMEMORY */
#define DPERR_INVALIDPARAM ((HRESULT)0x80070057L) /* E_INVALIDARG */
#define DPERR_UNSUPPORTED ((HRESULT)0x80004021L)  /* E_NOTIMPL */

/* DPOPEN flags */
#define DPOPEN_JOIN                   0x00000004

/* Receive flags */
#ifndef DPRECEIVE_TOPLAYER
#define DPRECEIVE_TOPLAYER     0x00000000  /* deliver to target player only */
#endif

#ifndef DPRECEIVE_FROMPLAYER
#define DPRECEIVE_FROMPLAYER   0x00000001  /* receive only messages sent by target player */
#endif

/* Special player IDs */
#define DPID_ALLPLAYERS               0x00000000  /* broadcast to all players */

/* Send flags */
#define DPSEND_ASYNC                  0x00000002
#define DPSEND_NOSENDCOMPLETEMSG      0x00000008

#ifndef DPSEND_GUARANTEED
#define DPSEND_GUARANTEED 0x00000001
#endif

/* ------------------------------------------------------------------
   COM context flags
   ------------------------------------------------------------------ */
#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER 0x1
#endif
#ifndef __cplusplus
#define REFIID const GUID*
#define REFCLSID const GUID*
#endif

/* Common DirectPlay structures (stubs only) */
typedef struct _DPCAPS {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMaxBufferSize;
    DWORD dwMaxQueueSize;
    DWORD dwMaxPlayers;
    DWORD dwHundredBaud;
    DWORD dwLatency;
    DWORD dwMaxLocalPlayers;
} DPCAPS, *LPDPCAPS;

/* DirectPlay interface stubs */
typedef struct IDirectPlay IDirectPlay;
typedef struct IDirectPlay2 IDirectPlay2;
typedef struct IDirectPlay3 IDirectPlay3;
typedef struct IDirectPlay4 IDirectPlay4;

/* Dummy vtable declarations (empty) */
struct IDirectPlay { void* lpVtbl; };
struct IDirectPlay2 { void* lpVtbl; };
struct IDirectPlay3 { void* lpVtbl; };


/* IID stubs (normally provided by dxguid.lib) */
extern const GUID IID_IDirectPlay;
extern const GUID IID_IDirectPlay2;
extern const GUID IID_IDirectPlay3;
//extern const GUID IID_IDirectPlay4;

typedef IDirectPlay*  LPDIRECTPLAY;
typedef IDirectPlay2* LPDIRECTPLAY2;
typedef IDirectPlay3* LPDIRECTPLAY3;
typedef IDirectPlay4* LPDIRECTPLAY4;

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

#endif /* __IUnknown_INTERFACE_DEFINED__ */

/* Function prototypes (no-op stubs) */
static inline HRESULT DirectPlayCreate(LPGUID lpGUID, IDirectPlay** lplpDP, IUnknown* pUnk) {
    (void)lpGUID; (void)lplpDP; (void)pUnk;
    return DPERR_UNSUPPORTED;
}

/* ------------------------------------------------------------------
   DirectPlay 6 session/lobby structures (complete stubs)
   ------------------------------------------------------------------ */

/* Player/group name */
#if 0
typedef struct _DPNAME {
    DWORD   dwSize;         /* Size of this structure */
    DWORD   dwFlags;        /* Flags (e.g. DPNA_* in DX8, unused here) */
    LPWSTR  lpszShortName;  /* Unicode short name */
    LPWSTR  lpszLongName;   /* Unicode long name */
} DPNAME, *LPDPNAME;
typedef const DPNAME* LPCDPNAME;
#endif
/* DPNAME with both ANSI and Unicode fields */
typedef struct _DPNAME {
    DWORD dwSize;
    DWORD dwFlags;
    union {
        LPWSTR lpszShortName;
        LPSTR  lpszShortNameA;
    };
    union {
        LPWSTR lpszLongName;
        LPSTR  lpszLongNameA;
    };
} DPNAME, *LPDPNAME;
typedef const DPNAME* LPCDPNAME;

/* Session description */
typedef struct _DPSESSIONDESC2 {
    DWORD   dwSize;            /* Size of this structure */
    DWORD   dwFlags;           /* Session flags */
    GUID    guidInstance;      /* Unique instance GUID */
    GUID    guidApplication;   /* Application GUID */
    DWORD   dwMaxPlayers;      /* Max players allowed */
    DWORD   dwCurrentPlayers;  /* Current number of players */

    /* Session name (ANSI/Unicode variants) */
    union {
        LPWSTR lpszSessionName;   /* Unicode */
        LPSTR  lpszSessionNameA;  /* ANSI */
    };

    /* Password (ANSI/Unicode variants) */
    union {
        LPWSTR lpszPassword;      /* Unicode */
        LPSTR  lpszPasswordA;     /* ANSI */
    };

    DWORD   dwReserved1;
    DWORD   dwReserved2;
    DWORD   dwUser1;
    DWORD   dwUser2;
    DWORD   dwUser3;
    DWORD   dwUser4;
} DPSESSIONDESC2, *LPDPSESSIONDESC2;
typedef const DPSESSIONDESC2* LPCDPSESSIONDESC2;


/* DirectPlay EnumSessions callback (DP3/DP4 style) */
typedef BOOL (WINAPI *LPDPENUMSESSIONSCALLBACK)(
    LPCDPSESSIONDESC2 lpSessionDesc,  /* const DPSESSIONDESC2* */
    LPDWORD           lpdwTimeout,    /* DWORD* */
    DWORD             dwFlags,
    LPVOID            lpContext
);

/* DirectPlay 6 introduced the “2” variant; many titles use this */
typedef BOOL (WINAPI *LPDPENUMSESSIONSCALLBACK2)(
    LPCDPSESSIONDESC2 lpSessionDesc,
    LPDWORD           lpdwTimeout,
    DWORD             dwFlags,
    LPVOID            lpContext
);

/* Callback typedef for EnumConnections */
typedef BOOL (WINAPI *LPDPENUMCONNECTIONSCALLBACK)(
    LPCGUID lpguidSP,
    LPVOID lpConnection,
    DWORD dwSize,
    LPCDPNAME lpName,
    DWORD dwFlags,
    LPVOID lpContext
);

typedef unsigned long DPID;
typedef DPID* LPDPID;

HRESULT GetConnectionSettings(
    DWORD   dwAppID,
    LPVOID  lpData,
    LPDWORD lpdwSize
);

HRESULT SetConnectionSettings(
    DWORD   dwFlags,
    DWORD   dwAppID,
    LPVOID  lpConn
);



/* ------------------------------------------------------------------
   Minimal but complete IDirectPlay4 stub
   ------------------------------------------------------------------ */
struct IDirectPlay4 {
    // IUnknown
    HRESULT QueryInterface(REFIID, void**) { return DPERR_UNSUPPORTED; }
    ULONG   AddRef() { return 1; }
    ULONG   Release() { return 1; }

    // Connection/session management
    HRESULT InitializeConnection(LPVOID, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT Open(LPCDPSESSIONDESC2, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT Close() { return DPERR_UNSUPPORTED; }

    // Enumeration
    HRESULT EnumSessions(LPCDPSESSIONDESC2, DWORD,
                         LPDPENUMSESSIONSCALLBACK2, LPVOID, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT EnumConnections(LPCGUID, LPDPENUMCONNECTIONSCALLBACK, LPVOID, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT EnumPlayers(LPGUID, LPVOID, LPVOID, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT EnumGroups(LPGUID, LPVOID, LPVOID, DWORD) { return DPERR_UNSUPPORTED; }

    // Player management
    HRESULT CreatePlayer(LPDPID lpidPlayer,
                         LPDPNAME lpName,
                         HANDLE hEvent,
                         LPVOID lpData,
                         DWORD dwDataSize,
                         DWORD dwFlags) {
        if (lpidPlayer) *lpidPlayer = 1; // dummy ID
        return DPERR_UNSUPPORTED;
    }
    HRESULT DestroyPlayer(DPID) { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerName(DPID, LPVOID, LPDWORD) { return DPERR_UNSUPPORTED; }
    HRESULT GetPlayerCaps(DPID, LPDPCAPS, DWORD) { return DPERR_UNSUPPORTED; }

    // Group management
    HRESULT CreateGroup(LPDPID lpidGroup,
                        LPDPNAME lpName,
                        LPVOID lpData,
                        DWORD dwDataSize,
                        DWORD dwFlags) {
        if (lpidGroup) *lpidGroup = 2; // dummy group ID
        return DPERR_UNSUPPORTED;
    }
    HRESULT DestroyGroup(DPID) { return DPERR_UNSUPPORTED; }
    HRESULT AddPlayerToGroup(DPID, DPID) { return DPERR_UNSUPPORTED; }
    HRESULT RemovePlayerFromGroup(DPID, DPID) { return DPERR_UNSUPPORTED; }
    // Legacy alias
    HRESULT DeletePlayerFromGroup(DPID dpIdGroup, DPID dpIdPlayer) {
        return RemovePlayerFromGroup(dpIdGroup, dpIdPlayer);
    }
    HRESULT GetGroupName(DPID, LPVOID, LPDWORD) { return DPERR_UNSUPPORTED; }
    HRESULT GetGroupData(DPID, LPVOID, LPDWORD, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT SetGroupData(DPID, LPVOID, DWORD, DWORD) { return DPERR_UNSUPPORTED; }

    // Messaging
    HRESULT Send(DPID, DPID, DWORD, LPVOID, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT Receive(LPDPID, LPDPID, DWORD, LPVOID, LPDWORD) { return DPERR_UNSUPPORTED; }
    HRESULT SendEx(DPID, DPID, DWORD,
                   LPVOID, DWORD,
                   DWORD, DWORD,
                   LPVOID, LPDWORD) { return DPERR_UNSUPPORTED; }

    // Session info
    HRESULT GetSessionDesc(LPVOID, LPDWORD) { return DPERR_UNSUPPORTED; }
    HRESULT SetSessionDesc(LPCDPSESSIONDESC2, DWORD) { return DPERR_UNSUPPORTED; }

    // Capabilities
    HRESULT GetCaps(LPDPCAPS, DWORD) { return DPERR_UNSUPPORTED; }

    // Misc
    HRESULT CancelMessage(DWORD, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT CancelPriority(DWORD, DWORD) { return DPERR_UNSUPPORTED; }
    HRESULT GetMessageQueue(DPID, DPID, DWORD, LPDWORD, LPDWORD) { return DPERR_UNSUPPORTED; }
};

//typedef IDirectPlay4* LPDIRECTPLAY4A;


/* ------------------------------------------------------------------
   DirectPlay Lobby stubs (normally in dplobby.h)
   ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
   Callback typedefs
   ------------------------------------------------------------------ */

/* EnumAddress callback */
typedef BOOL (*LPDPENUMADDRESSCALLBACK)(
    REFGUID guidDataType,
    LPCVOID lpData,
    DWORD   dwDataSize,
    LPVOID  lpContext
);

/* EnumAddressTypes callback */
typedef BOOL (*LPDPLENUMADDRESSTYPESCALLBACK)(
    REFGUID guidDataType,
    LPVOID  lpContext,
    DWORD   dwFlags
);

/* EnumLocalApplications callback */
typedef BOOL (*LPDPLENUMLOCALAPPLICATIONSCALLBACK)(
    LPCDPNAME lpName,
    DWORD     dwFlags,
    LPVOID    lpContext
);


/* Forward declarations of lobby interfaces */
typedef struct IDirectPlayLobby   IDirectPlayLobby;
typedef struct IDirectPlayLobby2  IDirectPlayLobby2;
typedef struct IDirectPlayLobby3  IDirectPlayLobby3;
typedef struct IDirectPlayLobbyA  IDirectPlayLobbyA;
typedef struct IDirectPlayLobby2A IDirectPlayLobby2A;
//typedef struct IDirectPlayLobby3A IDirectPlayLobby3A;

/* Dummy vtable holders */
struct IDirectPlayLobby   { void* lpVtbl; };
struct IDirectPlayLobby2  { void* lpVtbl; };
struct IDirectPlayLobby3  { void* lpVtbl; };
struct IDirectPlayLobbyA  { void* lpVtbl; };
struct IDirectPlayLobby2A { void* lpVtbl; };
//struct IDirectPlayLobby3A { void* lpVtbl; };

/* Pointer typedefs */
typedef IDirectPlayLobby*   LPDIRECTPLAYLOBBY;
typedef IDirectPlayLobby2*  LPDIRECTPLAYLOBBY2;
typedef IDirectPlayLobby3*  LPDIRECTPLAYLOBBY3;
typedef IDirectPlayLobbyA*  LPDIRECTPLAYLOBBYA;
typedef IDirectPlayLobby2A* LPDIRECTPLAYLOBBY2A;

/* Lobby connection info */
typedef struct _DPLCONNECTION {
    DWORD              dwSize;        /* Size of this structure */
    DWORD              dwFlags;       /* Flags */
    LPDPSESSIONDESC2   lpSessionDesc; /* Session description */
    LPDIRECTPLAY       lpDirectPlay;  /* DirectPlay interface */
    LPDIRECTPLAYLOBBY  lpLobby;       /* Lobby interface */
    LPDPNAME           lpPlayerName;
} DPLCONNECTION, *LPDPLCONNECTION;


/* ------------------------------------------------------------------
   Lobby-specific structures
   ------------------------------------------------------------------ */

/* Already defined earlier:
   - DPLCONNECTION
   - DPNAME
   - DPSESSIONDESC2
   Just ensure you have typedefs for pointers:
*/
typedef DPLCONNECTION*  LPDPLCONNECTION;
typedef DPNAME*         LPDPNAME;
typedef const DPNAME*   LPCDPNAME;
typedef DPSESSIONDESC2* LPDPSESSIONDESC2;
typedef const DPSESSIONDESC2* LPCDPSESSIONDESC2;

/* ------------------------------------------------------------------
   Minimal IDirectPlayLobby3A stub (ANSI interface)
   ------------------------------------------------------------------ */
struct IDirectPlayLobby3A {
    // IUnknown
    HRESULT QueryInterface(REFIID, void**) { return DPERR_UNSUPPORTED; }
    ULONG   AddRef() { return 1; }
    ULONG   Release() { return 1; }

    // Core lobby methods
    HRESULT Connect(DWORD /*dwFlags*/, LPDIRECTPLAY2* /*lplpDP*/, IUnknown* /*pUnk*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT CreateAddress(REFGUID /*guidSP*/, REFGUID /*guidDataType*/,
                          LPCVOID /*lpData*/, DWORD /*dwDataSize*/,
                          LPVOID /*lpAddress*/, LPDWORD /*lpdwAddressSize*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT EnumAddress(LPDPENUMADDRESSCALLBACK /*lpEnumAddressCallback*/,
                        LPCVOID /*lpAddress*/, DWORD /*dwAddressSize*/,
                        LPVOID /*lpContext*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT EnumAddressTypes(LPDPLENUMADDRESSTYPESCALLBACK /*lpEnumAddressTypesCallback*/,
                             REFGUID /*guidSP*/, LPVOID /*lpContext*/, DWORD /*dwFlags*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT EnumLocalApplications(LPDPLENUMLOCALAPPLICATIONSCALLBACK /*lpEnumLocalAppsCallback*/,
                                  LPVOID /*lpContext*/, DWORD /*dwFlags*/) {
        return DPERR_UNSUPPORTED;
    }

    // Connection settings
    HRESULT GetConnectionSettings(DWORD /*dwAppID*/, LPVOID /*lpData*/, LPDWORD /*lpdwSize*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT SetConnectionSettings(DWORD /*dwFlags*/, DWORD /*dwAppID*/, LPVOID /*lpConn*/) {
        return DPERR_UNSUPPORTED;
    }

    // Lobby messaging
    HRESULT ReceiveLobbyMessage(DWORD /*dwFlags*/, DWORD /*dwAppID*/,
                                LPDWORD /*lpdwMessageFlags*/, LPVOID /*lpData*/,
                                LPDWORD /*lpdwDataSize*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT SendLobbyMessage(DWORD /*dwFlags*/, DWORD /*dwAppID*/,
                             LPVOID /*lpData*/, DWORD /*dwDataSize*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT SetLobbyMessageEvent(DWORD /*dwAppID*/, HANDLE /*hEvent*/) {
        return DPERR_UNSUPPORTED;
    }

    // Application launching
    HRESULT RunApplication(DWORD /*dwFlags*/, LPDWORD /*lpdwAppID*/,
                           LPDPLCONNECTION /*lpConn*/, HANDLE /*hReceiveEvent*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT ReleaseApplication(DWORD /*dwAppID*/, DWORD /*dwFlags*/) {
        return DPERR_UNSUPPORTED;
    }

    // Extra Lobby3 methods
    HRESULT ConnectEx(DWORD /*dwFlags*/, REFIID /*riid*/, LPVOID* /*lplpDP*/, IUnknown* /*pUnk*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT RegisterApplication(DWORD /*dwFlags*/, LPVOID /*lpAppDesc*/, DWORD /*dwSize*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT UnregisterApplication(DWORD /*dwFlags*/, REFGUID /*guidApp*/) {
        return DPERR_UNSUPPORTED;
    }
    HRESULT WaitForConnectionSettings(DWORD /*dwFlags*/) {
        return DPERR_UNSUPPORTED;
    }
};

typedef IDirectPlayLobby3A* LPDIRECTPLAYLOBBY3A;

/* ------------------------------------------------------------------
   Common lobby function stubs
   ------------------------------------------------------------------ */

/* DirectPlayLobbyCreate (W/A) stubs */
static inline HRESULT DirectPlayLobbyCreateW(
    LPGUID lpGUID,
    LPDIRECTPLAYLOBBY* lplpDPLobby,
    IUnknown* pUnk,
    LPVOID lpData,
    DWORD dwFlags
) {
    (void)lpGUID; (void)lplpDPLobby; (void)pUnk; (void)lpData; (void)dwFlags;
    return DPERR_UNSUPPORTED;
}

static inline HRESULT DirectPlayLobbyCreateA(
    LPGUID lpGUID,
    LPDIRECTPLAYLOBBYA* lplpDPLobby,
    IUnknown* pUnk,
    LPVOID lpData,
    DWORD dwFlags
) {
    (void)lpGUID; (void)lplpDPLobby; (void)pUnk; (void)lpData; (void)dwFlags;
    return DPERR_UNSUPPORTED;
}

/* Real values from dplay.h (DirectX 6) */
static const GUID DPSPGUID_MODEM  = {0x36E95EE0,0x8577,0x11cf,{0x96,0xC,0x00,0x80,0xC7,0x53,0x4E,0x82}};
static const GUID DPSPGUID_TCPIP  = {0x36E95EE1,0x8577,0x11cf,{0x96,0xC,0x00,0x80,0xC7,0x53,0x4E,0x82}};
static const GUID DPSPGUID_SERIAL = {0x36E95EE2,0x8577,0x11cf,{0x96,0xC,0x00,0x80,0xC7,0x53,0x4E,0x82}};
static const GUID DPSPGUID_IPX    = {0x685BC400,0x9D2C,0x11cf,{0xA9,0xCD,0x00,0xAA,0x00,0x68,0x86,0xE3}};

/* DirectPlay EnumSessions flags */
#ifndef DPENUMSESSIONS_AVAILABLE
#define DPENUMSESSIONS_AVAILABLE  0x00000001
#endif

#ifndef DPENUMSESSIONS_ASYNC
#define DPENUMSESSIONS_ASYNC      0x00000002
#endif

/* ------------------------------------------------------------------
   DirectPlay session and open flags
   ------------------------------------------------------------------ */
#ifndef DPSESSION_DIRECTPLAYPROTOCOL
#define DPSESSION_DIRECTPLAYPROTOCOL   0x00000001
#endif

#ifndef DPSESSION_KEEPALIVE
#define DPSESSION_KEEPALIVE            0x00000008
#endif

#ifndef DPSESSION_OPTIMIZELATENCY
#define DPSESSION_OPTIMIZELATENCY      0x00000010
#endif

#ifndef DPOPEN_CREATE
#define DPOPEN_CREATE                  0x00000002
#endif


/* Calling convention */
#ifndef WINAPI
# if defined(__i386__) || defined(_M_IX86)
#  define WINAPI __attribute__((stdcall))
# else
#  define WINAPI
# endif
#endif

/* ------------------------------------------------------------------
   COM class IDs and interface IDs
   ------------------------------------------------------------------ */

/* CLSID for DirectPlay (used with CoCreateInstance) */
#ifndef CLSID_DirectPlay
static const GUID CLSID_DirectPlay =
{ 0xD1EB6D20, 0x8923, 0x11D0, {0x9D,0x97,0x00,0xA0,0xC9,0x0A,0x43,0xCB} };
#endif

/* IID for IDirectPlay4A (ANSI interface) */
#ifndef IID_IDirectPlay4A
static const GUID IID_IDirectPlay4A =
{ 0x9d460580, 0xa822, 0x11cf, {0x96,0x17,0x00,0x80,0xc7,0x53,0x4e,0x82} };
#endif

/* IID for IDirectPlay4 (Unicode interface) */
#ifndef IID_IDirectPlay4
static const GUID IID_IDirectPlay4 =
{ 0x9d460581, 0xa822, 0x11cf, {0x96,0x17,0x00,0x80,0xc7,0x53,0x4e,0x82} };
#endif

/* CoCreateInstance/CoInitialize/CoUninitialize are provided by
   WIN32_COMPAT.H (decl) + MFC_stub.cpp (impl returning failure) — the
   inlines that used to live here were removed to avoid redefinition. */

/* ------------------------------------------------------------------
   DirectPlay system message IDs
   ------------------------------------------------------------------ */
#define DPSYS_ADDGROUPTOGROUP     0x0009
#define DPSYS_ADDPLAYERTOGROUP    0x0003
#define DPSYS_CHAT                0x000A


/* ------------------------------------------------------------------
   Generic system message header
   ------------------------------------------------------------------ */
typedef struct _DPMSG_GENERIC {
    DWORD dwType;   /* One of the DPSYS_* constants */
    DWORD dwPlayerID;
} DPMSG_GENERIC, *LPDPMSG_GENERIC;

/* ------------------------------------------------------------------
   DPSYS_ADDGROUPTOGROUP message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_ADDGROUPTOGROUP {
    DWORD dwType;       /* DPSYS_ADDGROUPTOGROUP */
    DPID  idParentGroup;
    DPID  idGroup;
} DPMSG_ADDGROUPTOGROUP, *LPDPMSG_ADDGROUPTOGROUP;

/* ------------------------------------------------------------------
   DPSYS_ADDPLAYERTOGROUP message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_ADDPLAYERTOGROUP {
    DWORD dwType;       /* DPSYS_ADDPLAYERTOGROUP */
    DPID  dpIdGroup;
    DPID  dpIdPlayer;
} DPMSG_ADDPLAYERTOGROUP, *LPDPMSG_ADDPLAYERTOGROUP;

/* ------------------------------------------------------------------
   DPSYS_CHAT message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_CHAT {
    DWORD dwType;       /* DPSYS_CHAT */
    DPID  idFromPlayer;
    DPID  idToPlayer;
    DWORD dwFlags;
    LPVOID lpChat;      /* Pointer to chat text (ANSI/Unicode depending on build) */
} DPMSG_CHAT, *LPDPMSG_CHAT;

/* ------------------------------------------------------------------
   DirectPlay system message IDs
   ------------------------------------------------------------------ */
#define DPSYS_CREATEPLAYERORGROUP     0x0001
#define DPSYS_DESTROYPLAYERORGROUP    0x0002
#define DPSYS_ADDPLAYERTOGROUP        0x0003
#define DPSYS_DELETEPLAYERFROMGROUP   0x0004
#define DPSYS_ADDGROUPTOGROUP         0x0009
#define DPSYS_DELETEGROUPFROMGROUP    0x000B
#define DPSYS_CHAT                    0x000A

/* ------------------------------------------------------------------
   DPSYS_CREATEPLAYERORGROUP message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_CREATEPLAYERORGROUP {
    DWORD dwType;        /* DPSYS_CREATEPLAYERORGROUP */
    DWORD dwPlayerType;  /* DPPLAYERTYPE_PLAYER or DPPLAYERTYPE_GROUP */
    DPID  dpId;          /* ID of the new player or group */
    DPNAME dpnName;      /* Name of the player/group */
} DPMSG_CREATEPLAYERORGROUP, *LPDPMSG_CREATEPLAYERORGROUP;

/* ------------------------------------------------------------------
   DPSYS_DESTROYPLAYERORGROUP message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_DESTROYPLAYERORGROUP {
    DWORD dwType;        /* DPSYS_DESTROYPLAYERORGROUP */
    DWORD dwPlayerType;  /* DPPLAYERTYPE_PLAYER or DPPLAYERTYPE_GROUP */
    DPID  dpId;          /* ID of the player or group being destroyed */
} DPMSG_DESTROYPLAYERORGROUP, *LPDPMSG_DESTROYPLAYERORGROUP;

/* ------------------------------------------------------------------
   DPSYS_DELETEPLAYERFROMGROUP message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_DELETEPLAYERFROMGROUP {
    DWORD dwType;    /* DPSYS_DELETEPLAYERFROMGROUP */
    DPID  dpIdGroup;
    DPID  dpIdPlayer;
} DPMSG_DELETEPLAYERFROMGROUP, *LPDPMSG_DELETEPLAYERFROMGROUP;

/* ------------------------------------------------------------------
   DPSYS_DELETEGROUPFROMGROUP message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_DELETEGROUPFROMGROUP {
    DWORD dwType;    /* DPSYS_DELETEGROUPFROMGROUP */
    DPID  dpIdParentGroup;
    DPID  dpIdGroup;
} DPMSG_DELETEGROUPFROMGROUP, *LPDPMSG_DELETEGROUPFROMGROUP;

/* ------------------------------------------------------------------
   Player type constants
   ------------------------------------------------------------------ */
#define DPPLAYERTYPE_PLAYER  0x0001
#define DPPLAYERTYPE_GROUP   0x0002

/* ------------------------------------------------------------------
   Additional DirectPlay system message IDs
   ------------------------------------------------------------------ */
#define DPSYS_HOST                    0x0005
#define DPSYS_SECUREMESSAGE           0x0006
#define DPSYS_SESSIONLOST             0x0007
#define DPSYS_SETPLAYERORGROUPDATA    0x0008
#define DPSYS_SETPLAYERORGROUPNAME    0x000C
#define DPSYS_SETSESSIONDESC          0x000D

/* ------------------------------------------------------------------
   DPSYS_HOST message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_HOST {
    DWORD dwType;   /* DPSYS_HOST */
    DPID  dpIdNewHost;
} DPMSG_HOST, *LPDPMSG_HOST;

/* ------------------------------------------------------------------
   DPSYS_SECUREMESSAGE message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SECUREMESSAGE {
    DWORD dwType;   /* DPSYS_SECUREMESSAGE */
    DPID  dpIdFrom;
    DPID  dpIdTo;
    DWORD dwFlags;
    LPVOID lpData;
    DWORD dwDataSize;
} DPMSG_SECUREMESSAGE, *LPDPMSG_SECUREMESSAGE;

/* ------------------------------------------------------------------
   DPSYS_SESSIONLOST message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SESSIONLOST {
    DWORD dwType;   /* DPSYS_SESSIONLOST */
} DPMSG_SESSIONLOST, *LPDPMSG_SESSIONLOST;

/* ------------------------------------------------------------------
   DPSYS_SETPLAYERORGROUPDATA message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SETPLAYERORGROUPDATA {
    DWORD dwType;   /* DPSYS_SETPLAYERORGROUPDATA */
    DPID  dpId;
    DWORD dwPlayerType;  /* DPPLAYERTYPE_PLAYER or DPPLAYERTYPE_GROUP */
    LPVOID lpData;
    DWORD dwDataSize;
} DPMSG_SETPLAYERORGROUPDATA, *LPDPMSG_SETPLAYERORGROUPDATA;

/* ------------------------------------------------------------------
   DPSYS_SETPLAYERORGROUPNAME message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SETPLAYERORGROUPNAME {
    DWORD dwType;   /* DPSYS_SETPLAYERORGROUPNAME */
    DPID  dpId;
    DWORD dwPlayerType;
    DPNAME dpnName;
} DPMSG_SETPLAYERORGROUPNAME, *LPDPMSG_SETPLAYERORGROUPNAME;

/* ------------------------------------------------------------------
   DPSYS_SETSESSIONDESC message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SETSESSIONDESC {
    DWORD dwType;   /* DPSYS_SETSESSIONDESC */
    DPSESSIONDESC2 dpDesc;
} DPMSG_SETSESSIONDESC, *LPDPMSG_SETSESSIONDESC;


/* ------------------------------------------------------------------
   Additional DirectPlay system message IDs
   ------------------------------------------------------------------ */
#define DPSYS_STARTSESSION        0x000E
#define DPSYS_SENDCOMPLETE        0x000F
#define DPSYS_SETGROUPOWNER       0x0010

/* ------------------------------------------------------------------
   DPSYS_STARTSESSION message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_STARTSESSION {
    DWORD dwType;        /* DPSYS_STARTSESSION */
    GUID  guidInstance;  /* GUID of the session instance */
} DPMSG_STARTSESSION, *LPDPMSG_STARTSESSION;

/* ------------------------------------------------------------------
   DPSYS_SENDCOMPLETE message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SENDCOMPLETE {
    DWORD dwType;        /* DPSYS_SENDCOMPLETE */
    DWORD dwMsgID;       /* ID of the message that completed */
    HRESULT hr;          /* Result of the send (DP_OK or error) */
} DPMSG_SENDCOMPLETE, *LPDPMSG_SENDCOMPLETE;

/* ------------------------------------------------------------------
   DPSYS_SETGROUPOWNER message
   ------------------------------------------------------------------ */
typedef struct _DPMSG_SETGROUPOWNER {
    DWORD dwType;        /* DPSYS_SETGROUPOWNER */
    DPID  dpIdGroup;     /* Group whose owner changed */
    DPID  dpIdNewOwner;  /* New owner’s player ID */
} DPMSG_SETGROUPOWNER, *LPDPMSG_SETGROUPOWNER;

/* ------------------------------------------------------------------
   DirectPlay Lobby COM class and interface IDs
   ------------------------------------------------------------------ */

/* CLSID for DirectPlayLobby (used with CoCreateInstance) */
#ifndef CLSID_DirectPlayLobby
static const GUID CLSID_DirectPlayLobby =
{ 0x2fe8f810, 0xb2a5, 0x11d0, {0xa7,0x87,0x00,0x00,0xf8,0x03,0xab,0xfc} };
#endif

/* IID for IDirectPlayLobby3A (ANSI interface) */
#ifndef IID_IDirectPlayLobby3A
static const GUID IID_IDirectPlayLobby3A =
{ 0x2db72491, 0x652c, 0x11d1, {0xa7,0xa8,0x00,0x00,0xf8,0x03,0xab,0xfc} };
#endif

/* IID for IDirectPlayLobby3 (Unicode interface) */
#ifndef IID_IDirectPlayLobby3
static const GUID IID_IDirectPlayLobby3 =
{ 0x2db72490, 0x652c, 0x11d1, {0xa7,0xa8,0x00,0x00,0xf8,0x03,0xab,0xfc} };
#endif


/* ------------------------------------------------------------------
   DirectPlay Error Codes (HRESULT values)
   ------------------------------------------------------------------ */
#ifndef DPERR_ALREADYINITIALIZED
#define DPERR_ALREADYINITIALIZED   ((HRESULT)0x887700AAL)
#endif

#ifndef DPERR_ACCESSDENIED
#define DPERR_ACCESSDENIED         ((HRESULT)0x88770005L)
#endif

#ifndef DPERR_ACTIVEPLAYERS
#define DPERR_ACTIVEPLAYERS        ((HRESULT)0x8877001EL)
#endif

#ifndef DPERR_BUFFERTOOSMALL
#define DPERR_BUFFERTOOSMALL       ((HRESULT)0x88770014L)
#endif

#ifndef DPERR_CANTADDPLAYER
#define DPERR_CANTADDPLAYER        ((HRESULT)0x8877001FL)
#endif

#ifndef DPERR_CANTCREATEPLAYER
#define DPERR_CANTCREATEPLAYER     ((HRESULT)0x88770020L)
#endif

#ifndef DPERR_CANTCREATEPROCESS
#define DPERR_CANTCREATEPROCESS    ((HRESULT)0x88770021L)
#endif

#ifndef DPERR_CANTLOADCAPI
#define DPERR_CANTLOADCAPI         ((HRESULT)0x88770022L)
#endif

#ifndef DPERR_CANTLOADSECURITYPACKAGE
#define DPERR_CANTLOADSECURITYPACKAGE ((HRESULT)0x88770023L)
#endif

#ifndef DPERR_CANTLOADSSPI
#define DPERR_CANTLOADSSPI         ((HRESULT)0x88770024L)
#endif

#ifndef DPERR_CONNECTING
#define DPERR_CONNECTING           ((HRESULT)0x88770015L)
#endif

#ifndef DPERR_CONNECTIONLOST
#define DPERR_CONNECTIONLOST       ((HRESULT)0x88770016L)
#endif

#ifndef DPERR_INVALIDPASSWORD
#define DPERR_INVALIDPASSWORD      ((HRESULT)0x88770018L)
#endif

#ifndef DPERR_INVALIDPLAYER
#define DPERR_INVALIDPLAYER        ((HRESULT)0x88770019L)
#endif

#ifndef DPERR_INVALIDGROUP
#define DPERR_INVALIDGROUP         ((HRESULT)0x8877001AL)
#endif

#ifndef DPERR_INVALIDPARAMS
#define DPERR_INVALIDPARAMS        ((HRESULT)0x88770005L)
#endif

#ifndef DPERR_INVALIDOBJECT
#define DPERR_INVALIDOBJECT        ((HRESULT)0x8877001BL)
#endif

#ifndef DPERR_INVALIDFLAGS
#define DPERR_INVALIDFLAGS         ((HRESULT)0x8877001CL)
#endif

#ifndef DPERR_NOCONNECTION
#define DPERR_NOCONNECTION         ((HRESULT)0x8877001DL)
#endif

#ifndef DPERR_PENDING
#define DPERR_PENDING              ((HRESULT)0x88770083L)
#endif

#ifndef DPERR_SENDTOOBIG
#define DPERR_SENDTOOBIG           ((HRESULT)0x8877000DL)
#endif

#ifndef DPERR_NOMESSAGES
#define DPERR_NOMESSAGES           ((HRESULT)0x887700A2L)
#endif

/* ------------------------------------------------------------------
   DPLCONNECTION flags
   ------------------------------------------------------------------ */
#ifndef DPLCONNECTION_CREATESESSION
#define DPLCONNECTION_CREATESESSION   0x00000001
#endif

#ifndef DPLCONNECTION_JOINSESSION
#define DPLCONNECTION_JOINSESSION     0x00000002
#endif

#ifndef DPLCONNECTION_RETURNSTATUS
#define DPLCONNECTION_RETURNSTATUS    0x00000004
#endif

/* ------------------------------------------------------------------
   DPCONNECT flags
   ------------------------------------------------------------------ */
#ifndef DPCONNECT_RETURNSTATUS
#define DPCONNECT_RETURNSTATUS        0x00000008
#endif

#endif /* __DPLAY_H__ */
