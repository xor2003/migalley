//------------------------------------------------------------------------------
//Filename       intconv.h
//System         
//Author         Martin Alderton
//Date           Wed 24 Jan 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	INTCONV_Included
#define	INTCONV_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_INTCONV 0

//TempCode MGA 26Jan96 #include "textref.h"

struct GetRetVal
{
  	ULong			value;
  	Bool			furtherindirection;
  	CharOutputForm	outputform;
};

	GetRetVal getoneval(TextRef* &);
	GetRetVal getval(TextRef);
	GetRetVal getval(GetRetVal&, TextRef);
	
	GetRetVal getval(TextConst);
	GetRetVal getval(TextAddr);
	GetRetVal getval(GetRetVal&, TextOffset);
	GetRetVal getval(TextAddr, TextOffset);
	TextBuild Conv(GetRetVal);
	TextBlockPtr loadintblock(TextRef );
	char *	  TextintDeref(TextBlockPtr, TextRef);

#pragma pack(pop)
#endif
