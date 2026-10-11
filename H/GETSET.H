//------------------------------------------------------------------------------
//Filename       getset.h
//System         
//Author         Paul.   
//Date           Mon 24 Jul 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	getset_Included
#define	getset_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#include	"DISPLAY.H"

class	GetSet:public InitDisplayDef	//defines resolution request.
{
	public:
		GetSet(void );

		void ReadArgs(int argc, char *argv[]);
		void ReadArgsInstall(int argc, char *argv[]);

	private:

		void EchoParameters();

	protected:

};


#pragma pack(pop)
#endif
