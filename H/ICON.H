//------------------------------------------------------------------------------
//Filename       icon.h
//System         
//Author         R. Hyde 
//Date           Wed 7 Aug 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	ICON_Included
#define	ICON_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_ICON 0
enum	Icons	{	NOMOUSE = 0, MOUSE = 1,

			POINT_UP = 3,POINT_UPRIGHT = 4,POINT_RIGHT = 5,POINT_DOWNRIGHT = 6,
			POINT_DOWN = 7,
			POINT_DOWNLEFT = 8,POINT_LEFT = 9,POINT_UPLEFT = 10, WP_CENTRE = 23,

			TRIANGLELEFT_LO = 11,TRIANGLELEFT_HI = 12,
			TRIANGLERIGHT_LO = 13,TRIANGLERIGHT_HI = 14,



			MAP_TARGET = 24,
			MAP_MAPSIZE = 26,
			MAP_TYPE = 28,
			MAP_RANGES = 30,
			MAP_FACTORY = 32,
			MAP_TUNNEL = 34,
			MAP_AIRFIELD = 36,
			MAP_MYARD = 38,
			MAP_FACTORY2 = 40,
			MAP_DAM = 42,
			MAP_DEPRESS = 1,
			MOUSE_FILE = 50,
			MOUSE_MAP = 51,
			MOUSE_TELEPHONE = 52,
			MOUSE_RADIO = 53,
			MOUSE_TELEPRINT = 54,
			MOUSE_EXIT = 55,
			MOUSE_ACELIST = 56,
			MOUSE_FILM = 57,
			MOUSE_AIRCRAFT = 58,
			MOUSE_HILIGHT = 59,


//the following are no longer available							//RDH 28Feb97
			TICKICONLO = 57,
			TICKICONHI = 58, CROSSICONLO = 59, CROSSICONHI = 60,
			SFXSLIDERLO = 61, SFXSLIDERHI = 62,
			INSTSLIDERLO = 63, INSTSLIDERHI = 64,				//DAW 14Aug96
			MAPRETURNTOJUNCTIONLO = 78,MAPWPLO = 79,
			G_AIRCRAFT = 63,B_AIRCRAFT = 64,
			MAPCONTROLICONOFFSET = 5,
			VERTICALSLIDERPUCKLO = 15,VERTICALSLIDERUPLO = 17,VERTICALSLIDERDOWNLO = 19,
			VERTICALSLIDERPUCKHI = 16,VERTICALSLIDERUPHI = 18,VERTICALSLIDERDOWNHI = 20,
			H2HVERTICALSLIDERPUCKLO = 75, H2HVERTICALSLIDERUPLO = 76, TOGGLEITEMICONSLO = 77,
			H2HVERTICALSLIDERPUCKHI = 80, H2HVERTICALSLIDERUPHI = 81, ITEMTOGGLEICONSHI = 82,
			MAPRETURNTOJUNCTIONHI = 83,MAPWPHI = 84,
			};
#pragma pack(pop)
#endif
