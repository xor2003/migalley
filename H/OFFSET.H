//------------------------------------------------------------------------------
//Filename       offset.h
//System         
//Author         James Woodworth
//Date           Mon 21 Dec 1998
//Description    
//------------------------------------------------------------------------------
#ifndef	OFFSET_Included
#define	OFFSET_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_OFFSET 0


enum tOffset{ 
		OFFSET_CRECT = 17,
		OFFSET_BMAPXEDIT = 14,
		OFFSET_BMAPYEDIT = 12,
		OFFSET_BMAPXCOMBO = 14,
		OFFSET_BMAPYCOMBO = 12,
		OFFSET_BMAPXCIRCCOMBO = 22,
		OFFSET_BMAPYCIRCCOMBO = 13,
		};




#pragma pack(pop)
#endif
