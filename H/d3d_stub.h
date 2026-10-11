#ifndef D3D_STUB_H
#define D3D_STUB_H

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// Basic Direct3D typedefs
typedef float D3DVALUE;
typedef uint32_t D3DCOLOR;
#define D3DDEB_BUFSIZE 4096

#define RGB_MAKE(r, g, b)       ((D3DCOLOR) (((r) << 16) | ((g) << 8) | (b)))

typedef struct _D3DRECT {
    union { LONG x1; LONG lX1; };
    union { LONG y1; LONG lY1; };
    union { LONG x2; LONG lX2; };
    union { LONG y2; LONG lY2; };
} D3DRECT, *LPD3DRECT;

typedef struct _D3DSTATUS {
    DWORD    dwFlags;   // status flags
    DWORD    dwStatus;  // result of clipping/intersection
    D3DRECT  drExtent;  // extent rectangle of the primitive
} D3DSTATUS, *LPD3DSTATUS;

typedef struct _D3DEXECUTEDATA {
  DWORD     dwSize;
  DWORD     dwVertexOffset;
  DWORD     dwVertexCount;
  DWORD     dwInstructionOffset;
  DWORD     dwInstructionLength;
  DWORD     dwHVertexOffset;
  D3DSTATUS dsStatus;
} D3DEXECUTEDATA, *LPD3DEXECUTEDATA;

// Simple vector and matrix stubs
typedef struct {
    D3DVALUE x, y, z;
} D3DVECTOR;

typedef struct {
    D3DVALUE _11, _12, _13, _14;
    D3DVALUE _21, _22, _23, _24;
    D3DVALUE _31, _32, _33, _34;
    D3DVALUE _41, _42, _43, _44;
} D3DMATRIX;

// Common constants
#define D3D_OK 0
#define D3DERR_GENERIC -1

#ifdef __cplusplus
}
#endif

// IDirect3DTexture
struct IDirect3DTexture {
    virtual ULONG   Release() = 0;
    virtual HRESULT Initialize(void* lpDevice, void* lpSurface) = 0;
    virtual HRESULT GetHandle(void* lpDevice, DWORD* lpHandle) = 0;
    virtual HRESULT PaletteChanged(DWORD dwStart, DWORD dwCount) = 0;
    virtual HRESULT Load(IDirect3DTexture* lpSrcTexture) = 0;
};
typedef IDirect3DTexture* LPDIRECT3DTEXTURE;

// Transformed & Lit vertex
typedef struct {
    D3DVALUE sx, sy, sz;   // screen coordinates
    D3DVALUE rhw;          // reciprocal homogeneous w
    D3DCOLOR color;        // diffuse color
    D3DCOLOR specular;     // specular color
    D3DVALUE tu, tv;       // texture coords
} D3DTLVERTEX;

// Triangle primitive (indices into vertex array)
typedef struct {
    DWORD v1, v2, v3;      // vertex indices
    DWORD wFlags;          // primitive flags
} D3DTRIANGLE;

// Line primitive
typedef struct {
    DWORD v1, v2;          // vertex indices
    DWORD wFlags;          // primitive flags
} D3DLINE;

// Point primitive
typedef struct {
    DWORD v;               // vertex index
    DWORD wFlags;          // primitive flags
} D3DPOINT;


// Minimal device description struct
typedef struct _D3DDEVICEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDevCaps;
    DWORD dwPrimitiveMiscCaps;
    DWORD dwRasterCaps;
    DWORD dwZCmpCaps;
    DWORD dwSrcBlendCaps;
    DWORD dwDestBlendCaps;
    DWORD dwAlphaCmpCaps;
    DWORD dwShadeCaps;
    DWORD dwTextureCaps;
    DWORD dwTextureFilterCaps;
    DWORD dwTextureBlendCaps;
    DWORD dwTextureAddressCaps;
    DWORD dwStippleWidth;
    DWORD dwStippleHeight;
    // … add more fields if your code references them
} D3DDEVICEDESC, *LPD3DDEVICEDESC;

#define RGBA_MAKE(r,g,b,a) \
    ((D3DCOLOR)((((uint32_t)(a) & 0xFF) << 24) | \
                (((uint32_t)(r) & 0xFF) << 16) | \
                (((uint32_t)(g) & 0xFF) <<  8) | \
                (((uint32_t)(b) & 0xFF))))
                
#define D3DENUMRET_OK     0
#define D3DENUMRET_CANCEL 1

typedef enum _D3DTEXTUREFILTER {
    D3DFILTER_NEAREST    = 0x00000001,
    D3DFILTER_LINEAR     = 0x00000002,
    D3DFILTER_MIPNEAREST = 0x00000003,
    D3DFILTER_MIPLINEAR  = 0x00000004,
    D3DFILTER_LINEARMIPNEAREST = 0x00000005,
    D3DFILTER_LINEARMIPLINEAR  = 0x00000006
} D3DTEXTUREFILTER;

typedef enum _D3DTEXTUREBLEND {
    D3DTBLEND_DECAL        = 1,
    D3DTBLEND_MODULATE     = 2,
    D3DTBLEND_DECALALPHA   = 3,
    D3DTBLEND_MODULATEALPHA= 4,
    D3DTBLEND_DECALMASK    = 5,
    D3DTBLEND_MODULATEMASK = 6,
    D3DTBLEND_COPY         = 7,
    D3DTBLEND_ADD          = 8
} D3DTEXTUREBLEND;

typedef struct _D3DCOLORVALUE {
    float r;   // red component
    float g;   // green component
    float b;   // blue component
    float a;   // alpha component
} D3DCOLORVALUE;

typedef struct _D3DMATERIAL {
    DWORD dwSize;
    D3DCOLORVALUE diffuse;
    D3DCOLORVALUE ambient;
    D3DCOLORVALUE specular;
    D3DCOLORVALUE emissive;
    float power;
    D3DTEXTUREHANDLE hTexture;
    DWORD dwRampSize;
} D3DMATERIAL;
typedef D3DMATERIAL* LPD3DMATERIAL;

typedef DWORD D3DMATERIALHANDLE;
typedef D3DMATERIALHANDLE* LPD3DMATERIALHANDLE;

struct IDirect3DDevice {
    void* lpVtbl; // placeholder for COM vtable
};
typedef IDirect3DDevice* LPDIRECT3DDEVICE;

typedef struct _D3DINSTRUCTION {
  BYTE bOpcode;
  BYTE bSize;
  WORD wCount;
} D3DINSTRUCTION, *LPD3DINSTRUCTION;

typedef struct _D3DEXECUTEBUFFERDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwCaps;
    DWORD dwBufferSize;
    LPVOID lpData;
} D3DEXECUTEBUFFERDESC;

struct IDirect3DLight {
    void* lpVtbl; // placeholder for COM vtable
};
typedef IDirect3DLight* LPDIRECT3DLIGHT;

struct IDirect3DExecuteBuffer {
    void* lpVtbl; // placeholder for COM vtable
};
typedef IDirect3DExecuteBuffer* LPDIRECT3DEXECUTEBUFFER;

typedef struct _D3DPROCESSVERTICES {
    DWORD dwFlags;
    WORD  wStart;
    WORD  wDest;
    DWORD dwCount;
    DWORD dwReserved;
} D3DPROCESSVERTICES, *LPD3DPROCESSVERTICES;


typedef struct _D3DVIEWPORT {
    DWORD     dwSize;     // size of this struct
    DWORD     dwX;        // viewport X origin
    DWORD     dwY;        // viewport Y origin
    DWORD     dwWidth;    // viewport width
    DWORD     dwHeight;   // viewport height
    D3DVALUE  dvMinZ;
    D3DVALUE  dvMaxZ;
    D3DVALUE  dvMinX;
    D3DVALUE  dvMinY;
    D3DVALUE  dvMaxX;
    D3DVALUE  dvMaxY;
    D3DVALUE  dvScaleX;   // X scale factor
    D3DVALUE  dvScaleY;   // Y scale factor
    D3DVALUE  dvScaleZ;   // Z scale factor
} D3DVIEWPORT;
typedef D3DVIEWPORT* LPD3DVIEWPORT;

typedef enum _D3DOPCODE {
  D3DOP_POINT           = 1,
  D3DOP_LINE            = 2,
  D3DOP_TRIANGLE        = 3,
  D3DOP_MATRIXLOAD      = 4,
  D3DOP_MATRIXMULTIPLY  = 5,
  D3DOP_STATETRANSFORM  = 6,
  D3DOP_STATELIGHT      = 7,
  D3DOP_STATERENDER     = 8,
  D3DOP_PROCESSVERTICES = 9,
  D3DOP_TEXTURELOAD     = 10,
  D3DOP_EXIT            = 11,
  D3DOP_BRANCHFORWARD   = 12,
  D3DOP_SPAN            = 13,
  D3DOP_SETSTATUS       = 14,

  D3DOP_FORCE_DWORD     = 0x7fffffff
} D3DOPCODE;

/* From wine */
typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTRANSFORMSTATE_WORLD         = 1,
    D3DTRANSFORMSTATE_VIEW          = 2,
    D3DTRANSFORMSTATE_PROJECTION    = 3,
    D3DTRANSFORMSTATE_WORLD1        = 4,
    D3DTRANSFORMSTATE_WORLD2        = 5,
    D3DTRANSFORMSTATE_WORLD3        = 6,
    D3DTRANSFORMSTATE_TEXTURE0      = 16,
    D3DTRANSFORMSTATE_TEXTURE1      = 17,
    D3DTRANSFORMSTATE_TEXTURE2      = 18,
    D3DTRANSFORMSTATE_TEXTURE3      = 19,
    D3DTRANSFORMSTATE_TEXTURE4      = 20,
    D3DTRANSFORMSTATE_TEXTURE5      = 21,
    D3DTRANSFORMSTATE_TEXTURE6      = 22,
    D3DTRANSFORMSTATE_TEXTURE7      = 23,
    D3DTRANSFORMSTATE_FORCE_DWORD   = 0x7fffffff
} D3DTRANSFORMSTATETYPE;

typedef enum {
  D3DLIGHTSTATE_MATERIAL      = 1,
  D3DLIGHTSTATE_AMBIENT       = 2,
  D3DLIGHTSTATE_COLORMODEL    = 3,
  D3DLIGHTSTATE_FOGMODE       = 4,
  D3DLIGHTSTATE_FOGSTART      = 5,
  D3DLIGHTSTATE_FOGEND        = 6,
  D3DLIGHTSTATE_FOGDENSITY    = 7,
  D3DLIGHTSTATE_COLORVERTEX   = 8,
  D3DLIGHTSTATE_FORCE_DWORD   = 0x7fffffff
} D3DLIGHTSTATETYPE;

typedef enum {
  D3DRENDERSTATE_TEXTUREHANDLE      = 1,
  D3DRENDERSTATE_ANTIALIAS          = 2,
  D3DRENDERSTATE_TEXTUREADDRESS     = 3,
  D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,
  D3DRENDERSTATE_WRAPU              = 5, /* <= d3d6 */
  D3DRENDERSTATE_WRAPV              = 6, /* <= d3d6 */
  D3DRENDERSTATE_ZENABLE            = 7,
  D3DRENDERSTATE_FILLMODE           = 8,
  D3DRENDERSTATE_SHADEMODE          = 9,
  D3DRENDERSTATE_LINEPATTERN        = 10,
  D3DRENDERSTATE_MONOENABLE         = 11, /* <= d3d6 */
  D3DRENDERSTATE_ROP2               = 12, /* <= d3d6 */
  D3DRENDERSTATE_PLANEMASK          = 13, /* <= d3d6 */
  D3DRENDERSTATE_ZWRITEENABLE       = 14,
  D3DRENDERSTATE_ALPHATESTENABLE    = 15,
  D3DRENDERSTATE_LASTPIXEL          = 16,
  D3DRENDERSTATE_TEXTUREMAG         = 17,
  D3DRENDERSTATE_TEXTUREMIN         = 18,
  D3DRENDERSTATE_SRCBLEND           = 19,
  D3DRENDERSTATE_DESTBLEND          = 20,
  D3DRENDERSTATE_TEXTUREMAPBLEND    = 21,
  D3DRENDERSTATE_CULLMODE           = 22,
  D3DRENDERSTATE_ZFUNC              = 23,
  D3DRENDERSTATE_ALPHAREF           = 24,
  D3DRENDERSTATE_ALPHAFUNC          = 25,
  D3DRENDERSTATE_DITHERENABLE       = 26,
  D3DRENDERSTATE_ALPHABLENDENABLE   = 27,
  D3DRENDERSTATE_FOGENABLE          = 28,
  D3DRENDERSTATE_SPECULARENABLE     = 29,
  D3DRENDERSTATE_ZVISIBLE           = 30,
  D3DRENDERSTATE_SUBPIXEL           = 31, /* <= d3d6 */
  D3DRENDERSTATE_SUBPIXELX          = 32, /* <= d3d6 */
  D3DRENDERSTATE_STIPPLEDALPHA      = 33,
  D3DRENDERSTATE_FOGCOLOR           = 34,
  D3DRENDERSTATE_FOGTABLEMODE       = 35,
  D3DRENDERSTATE_FOGTABLESTART      = 36,
  D3DRENDERSTATE_FOGTABLEEND        = 37,
  D3DRENDERSTATE_FOGTABLEDENSITY    = 38,
  D3DRENDERSTATE_FOGSTART           = 36,
  D3DRENDERSTATE_FOGEND             = 37,
  D3DRENDERSTATE_FOGDENSITY         = 38,
  D3DRENDERSTATE_STIPPLEENABLE      = 39, /* <= d3d6 */
  /* d3d5 */
  D3DRENDERSTATE_EDGEANTIALIAS      = 40,
  D3DRENDERSTATE_COLORKEYENABLE     = 41,
  D3DRENDERSTATE_BORDERCOLOR        = 43,
  D3DRENDERSTATE_TEXTUREADDRESSU    = 44,
  D3DRENDERSTATE_TEXTUREADDRESSV    = 45,
  D3DRENDERSTATE_MIPMAPLODBIAS      = 46, /* <= d3d6 */
  D3DRENDERSTATE_ZBIAS              = 47,
  D3DRENDERSTATE_RANGEFOGENABLE     = 48,
  D3DRENDERSTATE_ANISOTROPY         = 49, /* <= d3d6 */
  D3DRENDERSTATE_FLUSHBATCH         = 50, /* <= d3d6 */
  /* d3d6 */
  D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT = 51, /* <= d3d6 */

  D3DRENDERSTATE_STENCILENABLE      = 52,
  D3DRENDERSTATE_STENCILFAIL        = 53,
  D3DRENDERSTATE_STENCILZFAIL       = 54,
  D3DRENDERSTATE_STENCILPASS        = 55,
  D3DRENDERSTATE_STENCILFUNC        = 56,
  D3DRENDERSTATE_STENCILREF         = 57,
  D3DRENDERSTATE_STENCILMASK        = 58,
  D3DRENDERSTATE_STENCILWRITEMASK   = 59,
  D3DRENDERSTATE_TEXTUREFACTOR      = 60,

  D3DRENDERSTATE_STIPPLEPATTERN00   = 64,
  D3DRENDERSTATE_STIPPLEPATTERN01   = 65,
  D3DRENDERSTATE_STIPPLEPATTERN02   = 66,
  D3DRENDERSTATE_STIPPLEPATTERN03   = 67,
  D3DRENDERSTATE_STIPPLEPATTERN04   = 68,
  D3DRENDERSTATE_STIPPLEPATTERN05   = 69,
  D3DRENDERSTATE_STIPPLEPATTERN06   = 70,
  D3DRENDERSTATE_STIPPLEPATTERN07   = 71,
  D3DRENDERSTATE_STIPPLEPATTERN08   = 72,
  D3DRENDERSTATE_STIPPLEPATTERN09   = 73,
  D3DRENDERSTATE_STIPPLEPATTERN10   = 74,
  D3DRENDERSTATE_STIPPLEPATTERN11   = 75,
  D3DRENDERSTATE_STIPPLEPATTERN12   = 76,
  D3DRENDERSTATE_STIPPLEPATTERN13   = 77,
  D3DRENDERSTATE_STIPPLEPATTERN14   = 78,
  D3DRENDERSTATE_STIPPLEPATTERN15   = 79,
  D3DRENDERSTATE_STIPPLEPATTERN16   = 80,
  D3DRENDERSTATE_STIPPLEPATTERN17   = 81,
  D3DRENDERSTATE_STIPPLEPATTERN18   = 82,
  D3DRENDERSTATE_STIPPLEPATTERN19   = 83,
  D3DRENDERSTATE_STIPPLEPATTERN20   = 84,
  D3DRENDERSTATE_STIPPLEPATTERN21   = 85,
  D3DRENDERSTATE_STIPPLEPATTERN22   = 86,
  D3DRENDERSTATE_STIPPLEPATTERN23   = 87,
  D3DRENDERSTATE_STIPPLEPATTERN24   = 88,
  D3DRENDERSTATE_STIPPLEPATTERN25   = 89,
  D3DRENDERSTATE_STIPPLEPATTERN26   = 90,
  D3DRENDERSTATE_STIPPLEPATTERN27   = 91,
  D3DRENDERSTATE_STIPPLEPATTERN28   = 92,
  D3DRENDERSTATE_STIPPLEPATTERN29   = 93,
  D3DRENDERSTATE_STIPPLEPATTERN30   = 94,
  D3DRENDERSTATE_STIPPLEPATTERN31   = 95,

  D3DRENDERSTATE_WRAP0              = 128,
  D3DRENDERSTATE_WRAP1              = 129,
  D3DRENDERSTATE_WRAP2              = 130,
  D3DRENDERSTATE_WRAP3              = 131,
  D3DRENDERSTATE_WRAP4              = 132,
  D3DRENDERSTATE_WRAP5              = 133,
  D3DRENDERSTATE_WRAP6              = 134,
  D3DRENDERSTATE_WRAP7              = 135,
  /* d3d7 */
  D3DRENDERSTATE_CLIPPING            = 136,
  D3DRENDERSTATE_LIGHTING            = 137,
  D3DRENDERSTATE_EXTENTS             = 138,
  D3DRENDERSTATE_AMBIENT             = 139,
  D3DRENDERSTATE_FOGVERTEXMODE       = 140,
  D3DRENDERSTATE_COLORVERTEX         = 141,
  D3DRENDERSTATE_LOCALVIEWER         = 142,
  D3DRENDERSTATE_NORMALIZENORMALS    = 143,
  D3DRENDERSTATE_COLORKEYBLENDENABLE = 144,
  D3DRENDERSTATE_DIFFUSEMATERIALSOURCE    = 145,
  D3DRENDERSTATE_SPECULARMATERIALSOURCE   = 146,
  D3DRENDERSTATE_AMBIENTMATERIALSOURCE    = 147,
  D3DRENDERSTATE_EMISSIVEMATERIALSOURCE   = 148,
  D3DRENDERSTATE_VERTEXBLEND              = 151,
  D3DRENDERSTATE_CLIPPLANEENABLE          = 152,

  D3DRENDERSTATE_FORCE_DWORD        = 0x7fffffff

  /* FIXME: We have some retired values that are being reused for DirectX 7 */
} D3DRENDERSTATETYPE;

typedef struct _D3DSTATE {
    union {
        D3DTRANSFORMSTATETYPE dtstTransformStateType;
        D3DLIGHTSTATETYPE     dlstLightStateType;
        D3DRENDERSTATETYPE    drstRenderStateType;
    };
    union {
        DWORD    dwArg[1];
        D3DVALUE dvArg[1];
    };
} D3DSTATE, *LPD3DSTATE;

/////////////////

// Minimal COM-style interface stubs
typedef struct IDirect3DMaterialVtbl IDirect3DMaterialVtbl;
struct IDirect3DMaterial {
    virtual ULONG Release() = 0;
    virtual HRESULT Initialize(void* lpDirect3D) = 0;
    virtual HRESULT SetMaterial(void* lpMat) = 0;
    virtual HRESULT GetMaterial(void* lpMat) = 0;
    virtual HRESULT GetHandle(void* lpDirect3DDevice, DWORD* lpHandle) = 0;
};
typedef IDirect3DMaterial* LPDIRECT3DMATERIAL;


struct IDirect3DMaterialVtbl {
    ULONG (*Release)(IDirect3DMaterial* This);
    // add other methods as needed
};

// Clear flags
#define D3DCLEAR_TARGET   0x00000001
#define D3DCLEAR_ZBUFFER  0x00000002
#define D3DCLEAR_STENCIL  0x00000004

struct IDirect3DViewport {
    // IUnknown methods
    virtual ULONG   Release() = 0;

    // IDirect3DViewport methods
    virtual HRESULT Initialize(void* lpDirect3D) = 0;
    virtual HRESULT GetViewport(D3DVIEWPORT* lpData) = 0;
    virtual HRESULT SetViewport(D3DVIEWPORT* lpData) = 0;
    virtual HRESULT TransformVertices(DWORD dwVertexCount,
                                      void* lpData,
                                      DWORD dwFlags,
                                      void* lpStatus) = 0;
    virtual HRESULT LightElements(DWORD dwElementCount,
                                  void* lpData) = 0;
    virtual HRESULT SetBackground(D3DMATERIALHANDLE hMat) = 0;
    virtual HRESULT GetBackground(LPD3DMATERIAL lpMat,
                                  LPD3DMATERIALHANDLE lpHandle) = 0;
    virtual HRESULT Clear(DWORD dwCount,
                          D3DRECT* lpRects,
                          DWORD dwFlags) = 0;
};

typedef IDirect3DViewport* LPDIRECT3DVIEWPORT;

////////////

#pragma pack(pop)
#endif // D3D_STUB_H

