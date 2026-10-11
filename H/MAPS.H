//------------------------------------------------------------------------------
//Filename       maps.h
//System         
//Author         R. Hyde 
//Date           Tue 3 Sep 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	MAPS_Included
#define	MAPS_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_MAPS 0

enum	{	
					MAPICONSIZE = PIXEL640 * 20,
				TOWNMAPICONSIZE = PIXEL640 * 12,
				CONTROLICONSIZE = PIXEL640 * 20,
				WPICONW = PIXEL640 * 10,
//provided by macca												//RDH 20Nov96
//				SOMMEMAPORIGINX = 179569, 						//RDH 20Nov96
  //				SOMMEMAPORIGINY = 238551, 						//RDH 20Nov96
//				SOMMEMAPHEIGHT = 40280, 
  //				SOMMEMAPWIDTH = 52440, 
//provided by macca												//RDH 20Nov96
//				VERDUNMAPORIGINX = 367002, 						//RDH 20Nov96
  //				VERDUNMAPORIGINY = 103547, 						//RDH 20Nov96
				KOREAMAPORIGINX=0,
				KOREAMAPORIGINY=0,
//				TANKMAPORIGINX = 243466, 
  //				TANKMAPORIGINY = 278400, 
//				TANKMAPORIGINBTMY = 269445, 
  //				TANKMAPHEIGHT = 19200, 
	//			TANKMAPWIDTH = 25600, 
//				VERDUNMAPHEIGHT = 51160, 
  //				VERDUNMAPWIDTH = 53720, 
				WPICONH = PIXEL640 * 14,			//CONVERTTOSCRN = 2,
//				CONVERTTOSCRN_DIV = 11884,
				MAXWPINROUTE = 10, MAXTOWNS = 130,MAXI_INT_ITEMS = 20,
				WPDELTAX = PIXEL640 * 10,
				WPDELTAY = PIXEL640 * 7
			};

enum	{ XLEFT = 1, XRIGHT = 2, YTOP = 4, YBOTTOM = 8, NOTAVAILABLE = 16,NOTVISIBLE = 32};

enum	{ NUMBEROFPAPERMAPS = 10};

enum	{	NODISPLAY = 0,REDAIRFIELDBAND,REDMILITARYBAND,
					REDPORTBAND,REDCCCBAND,REDGICBAND,
					REDSAMBAND,BLUESAMBAND,

					BLUEAIRFIELDBAND,BLUEPORTBAND,BLUECCCBAND,BLUEGICBAND,

			MAXDISPLAY = BLUEGICBAND};

#pragma pack(pop)
#endif
