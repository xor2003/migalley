//------------------------------------------------------------------------------
//Filename       winsterr.h
//System         
//Author         Andrew McRae
//Date           Wed 2 Oct 1996
//Description    Error stubs for windows95 installer
//------------------------------------------------------------------------------
#ifndef	WINSTERR_Included
#define	WINSTERR_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_WINSTERR 0


class	 Error  
{	//can't allow this to be half commented out!

public:

		void Error::EmitSysErr(char *fmt, ...);

		Error& Error::ExitMode();
		Error& Error::Say(char *, ...);
		Error& Error::SayAndQuit(char *, ...);

};
extern	Error	_Error;


#pragma pack(pop)
#endif
