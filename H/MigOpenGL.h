#ifndef MIGOPENGL_H
#define MIGOPENGL_H

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define GL3DTEXTUREFILTER D3DTEXTUREFILTER /* see d3dtypes.h for now */

#define GL3DTEXTUREBLEND D3DBLEND /* see d3dtypes.h for now */

struct MigGL_Direct3DDevice
{
    /*** IUnknown methods ***/
//   STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
//   STDMETHOD_(ULONG,AddRef)(THIS) PURE;
//   STDMETHOD_(ULONG,Release)(THIS) PURE;

    /*** IDirect3DDevice methods ***/
#if 0
    STDMETHOD(Initialize)(THIS_ LPDIRECT3D,LPGUID,LPD3DDEVICEDESC) PURE;
    STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC,LPD3DDEVICEDESC) PURE;
    STDMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE,LPDIRECT3DTEXTURE) PURE;
    STDMETHOD(CreateExecuteBuffer)(THIS_ LPD3DEXECUTEBUFFERDESC,LPDIRECT3DEXECUTEBUFFER*,IUnknown*) PURE;
    STDMETHOD(GetStats)(THIS_ LPD3DSTATS) PURE;
    STDMETHOD(Execute)(THIS_ LPDIRECT3DEXECUTEBUFFER,LPDIRECT3DVIEWPORT,DWORD) PURE;
    STDMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT) PURE;
    STDMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT) PURE;
    STDMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT,LPDIRECT3DVIEWPORT*,DWORD) PURE;
    STDMETHOD(Pick)(THIS_ LPDIRECT3DEXECUTEBUFFER,LPDIRECT3DVIEWPORT,DWORD,LPD3DRECT) PURE;
    STDMETHOD(GetPickRecords)(THIS_ LPDWORD,LPD3DPICKRECORD) PURE;
    STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK,LPVOID) PURE;
    STDMETHOD(CreateMatrix)(THIS_ LPD3DMATRIXHANDLE) PURE;
    STDMETHOD(SetMatrix)(THIS_ D3DMATRIXHANDLE,const LPD3DMATRIX) PURE;
    STDMETHOD(GetMatrix)(THIS_ D3DMATRIXHANDLE,LPD3DMATRIX) PURE;
    STDMETHOD(DeleteMatrix)(THIS_ D3DMATRIXHANDLE) PURE;
    STDMETHOD(BeginScene)(THIS) PURE;
    STDMETHOD(EndScene)(THIS) PURE;
    STDMETHOD(GetDirect3D)(THIS_ LPDIRECT3D*) PURE;
#else
	int dummyme; //RERUN
#endif
};

// extern struct R3DCOLOR; //RERUN

#include "WIN3D.H"

typedef struct _GL3DTLVERTEX
{
	union
	{
		float	sx;
		float	dvSX;
	};
	union
	{
		float	sy;
		float	dvSY;
	};
	union
	{
		float	sz;
		float	dvSZ;
	};
	union
	{
		float	rhw;
		float	dvRHW;
		UWord	clipFlags;	//new bit!
	};
	union
	{
		ULong	color;
		ULong	dcColor;
		R3DCOLOR	col;
	};
	union
	{
		ULong	specular;
		ULong	dcSpecular;
		R3DCOLOR	spec;
	};
	union
	{
		float	tu;
		float	dvTU;
		SWord	ix;
	};
	union
	{
		float	tv;
		float	dvTV;
		SWord	iy;
	};
} GL3DTLVERTEX, *PGL3DTLVERTEX;


#pragma pack(pop)
#endif //MIGOPENGL_H
