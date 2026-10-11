//------------------------------------------------------------------------------
//Filename       maclib.h
//System         
//Author         Paul.   
//Date           Thu 6 Jul 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	maclib_Included
#define	maclib_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	abs(n)	((n)<0 ? 0-(n) : (n))
#define sgn(n)	((n) ? (n)/abs((n)) : 0)


#pragma pack(pop)
#endif
