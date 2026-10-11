// d3dx.h - Linux stub for D3DX (DX7 helper library)
// Non-functional placeholder for compilation only.

#pragma once

// RERUN: natural-alignment self-guard - contains std/system types or
// port-side structs; never byte-pack regardless of ambient packing.
#pragma pack(push)
#pragma pack()
#include "WIN32_COMPAT.H"
#include "d3d_stub.h"


static inline HRESULT D3DXMatrixIdentity(D3DMATRIX* out)
{
    if (!out) return D3DERR_GENERIC;
    out->_11 = 1.0f; out->_12 = 0.0f; out->_13 = 0.0f; out->_14 = 0.0f;
    out->_21 = 0.0f; out->_22 = 1.0f; out->_23 = 0.0f; out->_24 = 0.0f;
    out->_31 = 0.0f; out->_32 = 0.0f; out->_33 = 1.0f; out->_34 = 0.0f;
    out->_41 = 0.0f; out->_42 = 0.0f; out->_43 = 0.0f; out->_44 = 1.0f;
    return D3D_OK;
}


#pragma pack(pop)