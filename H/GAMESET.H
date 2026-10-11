//------------------------------------------------------------------------------
//Filename       gameset.h
//System         
//Author         Paul.   
//Date           Thu 25 Jun 1998
//Description    
//------------------------------------------------------------------------------
#ifndef	GAMESET_Included
#define	GAMESET_Included

#define	DEFAULT_GAMESET 0

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

struct GameSettings
{
 	bool 	m_bUse16;			//force the use of 16-bit textures
	bool 	m_bBypassMemCheck;	//ignore video RAM check
	bool 	m_bFullScreenMode;	//force 3D to full screen mode
	ULong 	m_dwWidth;			//width for full screen mode
	ULong 	m_dwHeight;			//height for full screen mode
	GameSettings()
	{
		//default values for command line switches

		m_bUse16=false;				// -USE16
		m_bBypassMemCheck=false;	// -NOMEM
	 	m_bFullScreenMode=false;		// -F640X480
		m_dwWidth=640;
		m_dwHeight=480;
	}
};

extern struct GameSettings gameSettings;

#pragma pack(pop)
#endif
