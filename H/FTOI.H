//------------------------------------------------------------------------------
// Float -> int fixed-point vertex conversion shared by the polygon paths.
// Extracted verbatim from POLYGON.CPP so the unit tests can link the exact
// code the renderer runs (the function is inline -> no exported symbol).
//------------------------------------------------------------------------------
#ifndef FTOI_H
#define FTOI_H

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

inline void FtoITexture(VERTEX_PTR* ppv)
{
	while (*ppv)
	{
		(*ppv)->ix.i=SLong((*ppv)->ix.f);
		(*ppv)->iy.i=SLong((*ppv)->iy.f);
		ppv++;
	}
}

#pragma pack(pop)
#endif
