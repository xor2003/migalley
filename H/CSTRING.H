//------------------------------------------------------------------------------
//Filename       cstring.h
//System         
//Author         Robert Slater
//Date           Wed 27 Jan 1999
//Description    
//------------------------------------------------------------------------------
#ifndef	CSTRING_Included
#define	CSTRING_Included

// RERUN: natural-alignment self-guard - contains std/system types or
// port-side structs; never byte-pack regardless of ambient packing.
#pragma pack(push)
#pragma pack()

#if defined	(_MSC_VER)
#include "atlstr.h"
#else
#include "MigCstring.h"
#endif
#pragma pack(pop)
#endif

