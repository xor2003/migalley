/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

//------------------------------------------------------------------------------
//Filename       hardwin.cpp
//System         
//Author         Andrew McRae
//Date           Fri 7 Jun 1996
//Description    Direct Draw Stuff
//------------------------------------------------------------------------------
#include "_HARD.H"
#define WIN32_LEAN_AND_MEAN
#include	<stdio.h>
#include	<stdlib.h>
//SDL #include "DDRAW.H" // RERUN
#include	"DOSDEFS.H"
#include	"FILES.H"
#include "DISPLAY.H" // RERUN
#include	"MYERROR.H"
#include "MATHASM.H"
#include	"WIN3D.H"
#include "MATHASM.H"
#include "STUB3D.H" //RERUN

extern SWord	winmode_w, winmode_h; //RERUN

extern SWord TOPLINE_Y;
extern SWord TOPLINE_Y2;
extern SWord TOPLINE_Y3;
extern SWord TOPLINE_YY;

DirectDDP _DirectDraw::lpDirectDD; //here is the definition

//#define DDSCAPS_LOCALVIDMEM 0x10000000l
//RERUN #define DDSCAPS_LOCALVIDMEM 0x00000000l

#define		WINDOWED_PALETTE

#if 0 //SDL
inline void directdraw_ClearSurface(LPDIRECTDRAWSURFACE pDDS)
#else
inline void directdraw_ClearSurface(SDL_Surface* pDDS)
#endif
{
#if 0 //SDL
	LPDIRECTDRAWSURFACE lpDDS=pDDS;
	if (!lpDDS->IsLost())
		while (lpDDS!=NULL)
		{
			HRESULT hr;
			DDSURFACEDESC sd;
			DDSCAPS sc;
			sc.dwCaps=DDSCAPS_BACKBUFFER;
			memset(&sd,0,sizeof(sd));
			sd.dwSize=sizeof(sd);
			do{hr=lpDDS->Lock(NULL,&sd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);}
			while (hr!=DD_OK);
			ULong sw=sd.dwWidth>>1;

			if (sd.lPitch!=0){
				UByte* pDst=(UByte*)sd.lpSurface;
				for (SLong i=sd.dwHeight;i>0;i--){
					ULong* pDst2=(ULong*)pDst;
					for (SLong j=sw;j>0;j--) *pDst2++=ULong(0);
					pDst+=sd.lPitch;
				}
			}
			else{
				ULong* pDst=(ULong*)sd.lpSurface;
				for (SLong i=sw*sd.dwHeight;i>0;i--) *pDst++=ULong(0);
			}
			lpDDS->Unlock(sd.lpSurface);
			hr=lpDDS->GetAttachedSurface(&sc,&lpDDS);
		}
#else
	printf("SDL: %s Not yet implemented!\n", __FUNCTION__);
#endif //SDL
}

static LogicalPtr
bupLogicalScreenPtr=NULL;

extern UWord polyRedBits,polyRedShift,polyGreenBits,polyGreenShift,polyBlueBits,polyBlueShift;

//------------------------------------------------------------------------------
//Procedure		XX_SetGraphicsMode
//Author		Andrew McRae
//Date			Tue 25 Jun 1996
//
//Description	Setup Direct Draw Surfaces & Screen Mode
//
//Inputs		None
//
//Returns		None
//
//------------------------------------------------------------------------------
void	MigDisplay::XX_SetGraphicsMode()
{
	LogicalPtr	oldscreenaddr = NULL;
	int			oldbytesperscanline;
	int			oldbytesperpixel;
	int			oldscalex;
	int			oldscaley;

	if (bupLogicalScreenPtr!=NULL)
	{
		oldscreenaddr=logicalscreenptr;
		oldbytesperscanline=BytesPerScanLine;
		oldbytesperpixel=BytesPerPixel;
		oldscalex=virtualXscale;
		oldscaley=virtualYscale;

	}

	prevmodeAX = 0;

	VirtualMinX = 0;		//these should never be>word
	VirtualWidth  = FULLW;	//note that these virtual mins and sizes
	VirtualMinY = 0;		//must be proportional to 
	VirtualHeight = FULLH;	//physical mins and sizes so zero is in same place

	DD.pScreenB = (UByteP)NULL;

	ULong res;

	//DDCurrMode stays 0xffff when no enumerated mode matches the window
	//size (the SDL mode list needn't contain winmode_w/h); fall back to
	//the first mode rather than indexing DDModes[0xffff].
	if (DD.DDCurrMode>=DD.cModes)
		DD.DDCurrMode=0;
	res = SetDirectDrawMode (DD.DDModes[DD.DDCurrMode]);

	if (res != 0)
	{
		logicalscreenptr = (LogicalPtr)FALSE;
	}

	// Setup Vars
	//BytesPerPixel = DD.DDModes[DD.DDCurrMode].colourdepth/8; // RERUN: Same as SDL_PIXELBITCOUNT / 8
	BytesPerPixel = SDL_PIXELBITCOUNT / 8;

//RDH 16Oct97 - set screen RGB format information for non-8bpp modes
	if (BytesPerPixel!=1)
	{
		DWORD	bits = 0,shift = 0,mask = 0;

		for (shift = 0, mask = DD.lpddsd->ddpfPixelFormat.dwRBitMask; !(mask & 1); shift++, mask >>= 1);
		for (	bits=0; mask & 1; bits++, mask>>=1);
	
		polyRedBits = RedBits = bits;		polyRedShift = RedShift = shift;

		for (shift = 0, mask = DD.lpddsd->ddpfPixelFormat.dwGBitMask; !(mask & 1); shift++, mask >>= 1);
		for (	bits=0; mask & 1; bits++, mask>>=1);

		polyGreenBits = GreenBits = bits;	polyGreenShift = GreenShift = shift;

		for (shift = 0, mask = DD.lpddsd->ddpfPixelFormat.dwBBitMask; !(mask & 1); shift++, mask >>= 1);
		for (	bits=0; mask & 1; bits++, mask>>=1);

		polyBlueBits = BlueBits = bits;	polyBlueShift = BlueShift = shift;
	}
	ChangeBytesPerScanLine ();

	RECT	rect;
	//SDL_Rect rect;
	
	if (!_DirectDraw::lpDirectDD->isFullScreen())
	{
#if 1 //SDL
		::GetWindowRect(DD.hWnd,&rect);
#else
		rect.x = rect.y = 0;
		rect.w = winmode_w;
		rect.h = winmode_h;
#endif
	}
	else{
#if 1
		rect.top=0;rect.left=0;
		rect.bottom=DD.DDModes[DD.DDCurrMode].height;
		rect.right=DD.DDModes[DD.DDCurrMode].width;
#else
		rect.x=0;rect.y=0;
		rect.h=DD.DDModes[DD.DDCurrMode].height;
		rect.w=DD.DDModes[DD.DDCurrMode].width;
#endif
	}
#if 1
	int	window_width = rect.right - rect.left;
	int	window_height = rect.bottom - rect.top;
#else
	int	window_width = rect.w;
	int	window_height = rect.h;
#endif

	PhysicalMinX = 0;
	PhysicalWidth = window_width;		//Width of screen in pixels
	PhysicalMinY = 0;
	PhysicalHeight = window_height;		//Height of screen in pixels

	virtualXscale = (FULLW+window_width-1) / window_width;
	virtualYscale = (FULLH+window_height-1) / window_height;
	
	float scaleCorrection = ((4.0 / 3.0) * ((float)(winmode_h) / (float)(winmode_w)));
	virtualXscale /= scaleCorrection; //RERUN, x-scale correction for 16/9 instead of 4/3 monitors !

	// Memory for DirectDraw Palette Table
	if (DD.PaletteBuffer==NULL)
		DD.PaletteBuffer = new MYPALETTEENTRY [256];

	if (bupLogicalScreenPtr!=NULL && (int)oldscreenaddr!=-1)
	{
		MigWindow* window;

		window=(MigWindow*)windowlink;

		while (window)
		{
			if (window->logicalscreenptr!=oldscreenaddr)
			{
				int	oldx=window->logicalscreenptr-oldscreenaddr;
				int	oldy=oldx/oldbytesperscanline;
				oldx=oldx%oldbytesperscanline;
				oldx=oldx/oldbytesperpixel;

				oldx*=oldscalex;
				oldy*=oldscaley;
				oldx/=virtualXscale;
				oldy/=virtualYscale;

				oldx =	oldx*BytesPerPixel+
						oldy*BytesPerScanLine;

				window->logicalscreenptr=logicalscreenptr + oldx;
			}
			else
				window->logicalscreenptr=logicalscreenptr;

			window->NumberOfScreens=NumberOfScreens;
			window->virtualXscale*=virtualXscale;
			window->virtualXscale/=oldscalex;
			window->virtualYscale*=virtualYscale;
			window->virtualYscale/=oldscaley;

			window->BytesPerScanLine=BytesPerScanLine;
			window->BytesPerPixel   =BytesPerPixel;
			window->NumberOfScreens	=NumberOfScreens;
			window->PhysicalWidth=	window->VirtualWidth	/window->virtualXscale;
			window->PhysicalHeight=	window->VirtualHeight	/window->virtualYscale;
			window->PhysicalMinX=	window->VirtualMinX		/window->virtualXscale;
			window->PhysicalMinY=	window->VirtualMinY		/window->virtualYscale;
			window->PhysicalMaxX=window->PhysicalMinX+window->PhysicalWidth-1;
			window->PhysicalMaxY=window->PhysicalMinY+window->PhysicalHeight-1;
			window->VirtualWidth	=window->PhysicalWidth		*window->virtualXscale;
			window->VirtualHeight	=window->PhysicalHeight		*window->virtualYscale;
			window->VirtualMinX		=window->PhysicalMinX		*window->virtualXscale;
			window->VirtualMinY		=window->PhysicalMinY		*window->virtualYscale;


			window->RedBits			=RedBits;			//JIM 20Oct97
			window->GreenBits		=GreenBits;		//JIM 20Oct97
			window->BlueBits 		=BlueBits;		//JIM 20Oct97
			window->RedShift		=RedShift;		//JIM 20Oct97
			window->GreenShift		=GreenShift;		//JIM 20Oct97
			window->BlueShift 		=BlueShift;		//JIM 20Oct97

			window=(MigWindow*)window->windowlink;
		}

	}
}

//------------------------------------------------------------------------------
//Procedure		XX_ScreenFlip
//Author		Andrew McRae
//Date			Tue 25 Jun 1996
//
//Description	Flip Direct Draw Primary and Back surfaces
//
//Inputs		None
//
//Returns		None
//
//------------------------------------------------------------------------------
#include "XX_ScreenFlip_Vulkan.h"
void	MigDisplay::XX_ScreenFlip()
{
if (RFullPanelDial::in3d)
{
	_DirectDraw::lpDirectDD->XX_ScreenFlip_Vulkan(DD.lpDDSBack);
	return;
}
else
{
	RECT 	winrect; //RERUN
	SDL_Rect 	rect;
	DDBLTFX	ddbltfx;
//SDL	HRESULT ddrval;
	int ddrval = 0;

	if (DD.ModeX)	return;

	switch (NumberOfScreens)
	{
	case 0:
	break;
	case 1:
		::GetWindowRect(DD.hWnd,&winrect);
		rect.h = winrect.bottom - winrect.top;
		rect.w = winrect.right - winrect.left;
		rect.x = winrect.left;
		rect.y = winrect.top;

		ddbltfx.dwSize = sizeof (DDBLTFX);

#if 0 // SDL
		ddrval = DD.lpDDSPrimary->Blt(	&rect, //Destination rectangle
										DD.lpDDSBack, // Surface that is source of the blit
										NULL,	//&rect, // Source rectangle
										DDBLT_WAIT, 
										&ddbltfx); //Address of the DDBLTFX structure ???
#else
		ddrval = SDL_UpdateTexture(Master_3d.SdlMigTexture, &rect, DD.lpDDSBack->pixels, DD.lpDDSBack->pitch);
#endif
		if (ddrval != DD_OK)
		{			 //DDERR_SURFACELOST
			printf("SDL_UpdateTexture case 1 failed! error: %s\n", SDL_GetError());
		}

	break;

	case 2:
		if (DD.lpDDSPrimary)
#if 0 //SDL
			ddrval=DD.lpDDSPrimary->Flip ( NULL, DDFLIP_WAIT );
#else
		{ //flip in SDL2
			ddrval = SDL_UpdateTexture(Master_3d.SdlMigTexture, NULL, DD.lpDDSPrimary->pixels, DD.lpDDSPrimary->pitch);
			if (ddrval != DD_OK)
			{			 //DDERR_SURFACELOST
				printf("SDL_UpdateTexture case 2 failed! error: %s\n", SDL_GetError());
			}
		}
#endif
	break;

	case 3:
#if 0 //SDL
		if (DD.lpDDSPrimary)
		    DD.lpDDSPrimary->Flip ( DD.Screens[DD.DDSpareScreen],
									DDFLIP_WAIT );
		// rotate indexes
		if (++DD.DDOnScreen == 4) 		DD.DDOnScreen = 1;
		if (++DD.DDSpareScreen == 4) 	DD.DDSpareScreen = 1;
		if (++DD.DDLogicalScreen == 4) 	DD.DDLogicalScreen = 1;
#else
		printf("SDL_UpdateTexture case 3 not implemented! \n");
#endif
	break;

	}


	if (SDL_RenderClear(DD.lpDD2) != DD_OK)	{printf("Error: %s\n", SDL_GetError());}
	if (SDL_RenderCopy(DD.lpDD2, Master_3d.SdlMigTexture, NULL, NULL) != DD_OK)	{printf("Error: %s\n", SDL_GetError());}
	SDL_RenderPresent(DD.lpDD2);
}
}


//------------------------------------------------------------------------------
//Procedure		EnableGUI
//Author		Andrew McRae
//Date			Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::EnableGUI (Bool Ctrl)
{
	if (DD.ModeX)	return;

#if 0 //SDL
	HRESULT res;
	res = DD.lpDD2->FlipToGDISurface ();
#endif
}


//------------------------------------------------------------------------------
//Procedure		XX_PalChange
//Author		Andrew McRae
//Date			Tue 25 Jun 1996
//
//Description	Changes Primary surface Palette
//
//Inputs		None
//
//Returns		None
//
//------------------------------------------------------------------------------

void	MigDisplay::XX_PalChange()
{
	// Palette points to 768 table of colours
	if (Palette)
	{
		ULong n;

		LPPALETTEENTRY pDDPal;
		char * pMyPal;

		pDDPal = (LPPALETTEENTRY)DD.PaletteBuffer;
		pMyPal = (char *)Palette;

		for (int index = 0 /*index = 10*/; index < 256/*246*/; index++)
		{
			pDDPal[index].peFlags = PC_NOCOLLAPSE | PC_RESERVED;
			pDDPal[index].peRed = *pMyPal++;
			pDDPal[index].peGreen = *pMyPal++;
			pDDPal[index].peBlue = *pMyPal++;
		}

		if (DD.lpDirect3D!=NULL)	DD.lpDirect3D->SetPalette(&DD,(UByte*)Palette);
		else
		{
			UWord* plup = (UWord*)GetPaletteTable();
			UByte* palp = (UByte*)Palette;

			UWord redMask, greenMask, blueMask;
			SWord redShift1, greenShift1, blueShift1;
			SWord redShift2, greenShift2, blueShift2;

			GetColourInfo(redMask, redShift1, redShift2,
				greenMask, greenShift1, greenShift2,
				blueMask, blueShift1, blueShift2);

			for (int j = 0; j<MAX_PALS; j++)
				for (int i = 255; i >= 0; i--)
				{
					UWord red, green, blue;
					red = UWord(*palp++) & 0xFF;
					green = UWord(*palp++) & 0xFF;
					blue = UWord(*palp++) & 0xFF;
					red = ((red >> redShift2)&redMask);
					green = ((green >> greenShift2)&greenMask);
					blue = ((blue >> blueShift2)&blueMask);
					*plup++ = (red << redShift1) + (green << greenShift1) + (blue << blueShift1);
				}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		XX_CopyP2L
//Author		Andrew McRae
//Date			Tue 23 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	MigDisplay::XX_CopyP2L(LogicalPtr	T, int W,int H)
{
	if (DD.ModeX)	return FALSE;
#if 0 //SDL TODO
	RECT rect;
	DDBLTFX	ddbltfx;

	LogicalPtr	S;

	HRESULT ddrval;

	rect.top = (T - logicalscreenptr) / BytesPerScanLine;
	rect.left = ((T - logicalscreenptr) - (rect.top * BytesPerScanLine)) / BytesPerPixel;
	rect.right = rect.left + W;
	rect.bottom = rect.top + H;

	ddbltfx.dwSize = sizeof (DDBLTFX);

	if (NumberOfScreens == 2)
	{

		if (ScreenNumOnLogical)
			ddrval = DD.lpDDSPrimary->Blt(	&rect,
											DD.lpDDSBack,
											&rect,
											DDBLT_WAIT |
											DDBLT_KEYSRC,
											&ddbltfx);
		else
			ddrval = DD.lpDDSBack->Blt(	&rect,
										DD.lpDDSPrimary,
										&rect,
										DDBLT_WAIT |
										DDBLT_KEYSRC,
										&ddbltfx);

		if (ddrval == DD_OK)	return TRUE;

		// Direct Draw Blt Failed - So DIY

		if (ScreenNumOnLogical)
		{
			while (DD.lpDDSBack->Lock(	NULL,
										DD.lpddsd,
										DDLOCK_SURFACEMEMORYPTR |
										DDLOCK_WAIT,
										NULL) != DD_OK) {};

			S = (LogicalPtr)DD.lpddsd->lpSurface;

			DD.lpDDSBack->Unlock (DD.lpddsd->lpSurface );
		}	
		else
		{
			while (DD.lpDDSPrimary->Lock(	NULL,
											DD.lpddsd,
											DDLOCK_SURFACEMEMORYPTR |
											DDLOCK_WAIT,
											NULL) != DD_OK) {};

			S = (LogicalPtr)DD.lpddsd->lpSurface;

			DD.lpDDSPrimary->Unlock (DD.lpddsd->lpSurface );
		}

		S+=(T-logicalscreenptr);

		ULong	wd=1+(W+3)/4;
		ULong	bps=BytesPerScanLine;

		H++;
		while (--H!=0)
		{

			repmovsd(S,T,wd);
			S+=bps;
			T+=bps;
		}

	}
	else if (NumberOfScreens == 3)
	{

		if (ScreenNumOnLogical)
			ddrval = DD.lpDDSPrimary->Blt(	&rect,
											DD.lpDDSBack,
											&rect,
											DDBLT_WAIT |
											DDBLT_KEYSRC,
											&ddbltfx);
		else
			ddrval = DD.lpDDSBack2->Blt(	&rect,
											DD.lpDDSPrimary,
											&rect,
											DDBLT_WAIT |
											DDBLT_KEYSRC,
											&ddbltfx);

		if (ddrval == DD_OK)	return TRUE;

		// Direct Draw Blt Failed - So DIY

		if (ScreenNumOnLogical)
		{
			while (DD.lpDDSBack2->Lock(	NULL,
										DD.lpddsd,
										DDLOCK_SURFACEMEMORYPTR |
										DDLOCK_WAIT,
										NULL) != DD_OK) {};

			S = (LogicalPtr)DD.lpddsd->lpSurface;

			DD.lpDDSBack2->Unlock (DD.lpddsd->lpSurface );
		}	
		else
		{
			while (DD.lpDDSPrimary->Lock(	NULL,
											DD.lpddsd,
											DDLOCK_SURFACEMEMORYPTR |
											DDLOCK_WAIT,
											NULL) != DD_OK) {};

			S = (LogicalPtr)DD.lpddsd->lpSurface;

			DD.lpDDSPrimary->Unlock (DD.lpddsd->lpSurface );
		}

		S+=(T-logicalscreenptr);

		ULong	wd=1+(W+3)/4;
		ULong	bps=BytesPerScanLine;

		H++;
		while (--H!=0)
		{
			ULong*	S1=(ULong*) S;
			ULong*	T1=(ULong*) T;
		 	for (ULong W1=wd;--W1!=0;)
	 			*T1++=*S1++;
			S+=bps;
			T+=bps;
		}
	}
#else
	printf("SDL: %s, Not yet implemented!\n", __FUNCTION__);
#endif //SDL
	return(TRUE);
}

//------------------------------------------------------------------------------
//Procedure		XX_PlotPixelNoClip
//Author		Andrew McRae
//Date			Tue 23 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ULong	MigDisplay::XX_PlotPixelNoClip(UWord	x, UWord	y, ULong	thecol)
{
	if (DD.ModeX)	return FALSE;
#if 0 //SDL TODO
	ULong		old_col;
	LogicalPtr	screenaddr;
	SLong		yoffset, xoffset;

	switch (NumberOfScreens)
	{
	case 0:
		while (DD.lpDDSPrimary->Lock(	NULL,
										DD.lpddsd,
										DDLOCK_SURFACEMEMORYPTR |
										DDLOCK_WAIT,
										NULL) != DD_OK) {};

		screenaddr = (LogicalPtr)DD.lpddsd->lpSurface;

		yoffset = BytesPerScanLine * y;
		xoffset = BytesPerPixel * x;

		screenaddr += xoffset + yoffset;
	
		old_col = (ULong)	(*screenaddr);
		*screenaddr = thecol;

		DD.lpDDSPrimary->Unlock (DD.lpddsd->lpSurface );

		return(old_col);

	case 1:
		while (DD.lpDDSPrimary->Lock(	NULL,
										DD.lpddsd,
										DDLOCK_SURFACEMEMORYPTR |
										DDLOCK_WAIT,
										NULL) != DD_OK) {};

		screenaddr = (LogicalPtr)DD.lpddsd->lpSurface;

		yoffset = BytesPerScanLine * y;
		xoffset = BytesPerPixel * x;

		screenaddr += xoffset + yoffset;
	
		old_col = (ULong)	(*screenaddr);

		*screenaddr = thecol;

		DD.lpDDSPrimary->Unlock (DD.lpddsd->lpSurface );

		return(old_col);

	case 2:
	{
		while (DD.lpDDSPrimary->Lock(	NULL,
										DD.lpddsd,
										DDLOCK_SURFACEMEMORYPTR |
										DDLOCK_WAIT,
										NULL) != DD_OK) {};

		screenaddr = (LogicalPtr)DD.lpddsd->lpSurface;

		yoffset = BytesPerScanLine * y;
		xoffset = BytesPerPixel * x;

		screenaddr += xoffset + yoffset;
	
		if (BytesPerPixel==1)
		{
			old_col = (ULong)	(*screenaddr);
			*screenaddr = thecol;
		}
		else
		if (BytesPerPixel==2)
		{
			UWord	tempy=*(UWord*)screenaddr;
			old_col=(ULong)tempy>>8;

			tempy = (tempy&0x00FF)+(thecol<<8);

			*(UWord*)screenaddr = tempy;
		}
		else
		{
			ULong	tempy=*(ULong*)screenaddr;
			old_col=((ULong)tempy&0x00ff00)>>8;

			tempy = (tempy&0xffff00FF)+(thecol<<8);

			*(ULong*)screenaddr = tempy;
		}

		DD.lpDDSPrimary->Unlock (DD.lpddsd->lpSurface );

		return(old_col);
	}
	case 3:
		while (DD.Screens[DD.DDOnScreen]->Lock(	NULL,
												DD.lpddsd,
												DDLOCK_SURFACEMEMORYPTR |
												DDLOCK_WAIT,
												NULL) != DD_OK) {};

		screenaddr = (LogicalPtr)DD.lpddsd->lpSurface;

		yoffset = BytesPerScanLine * y;
		xoffset = BytesPerPixel * x;

		screenaddr += xoffset + yoffset;
	
		old_col = (ULong)	(*screenaddr);

		*screenaddr = thecol;

		DD.Screens[DD.DDOnScreen]->Unlock (DD.lpddsd->lpSurface );

		return(old_col);
	}
#else
	printf("SDL: %s, Not yet implemented!\n", __FUNCTION__);
#endif //SDL
	return 0;
}


extern void DDErrorCodes(const char*,int,HRESULT);

//-----------------------------------------------------------------------------
// Procedure    
// Author       Paul
// Date         20/03/98
//-----------------------------------------------------------------------------
ULong MigDisplay::SetDirectDrawMode (InitDisplayDef& Mode)
{
#if 1 //SDL
    HRESULT         ddrval;
    DDSCAPS         ddscaps;

	bool fullScreen = _DirectDraw::lpDirectDD->isFullScreen();
	int	curmode=DD.DDCurrMode;

	RECT 	winrect;
	SDL_Rect 	rect;
	if (!fullScreen)
	{

		::GetWindowRect(DD.hWnd,&winrect);

		rect.h = winrect.bottom - winrect.top;
		rect.w = winrect.right - winrect.left;
		rect.x = winrect.left;
		rect.y = winrect.top;
	}
	else
	{
#if 0
		rect.top=0;rect.left=0;
		rect.bottom=DD.DDModes[curmode].height;
		rect.right=DD.DDModes[curmode].width;
#else
		rect.x=0;rect.y=0;
		rect.h=DD.DDModes[curmode].height;
		rect.w=DD.DDModes[curmode].width;
#endif
	}
#if 0
	int	window_width=rect.right-rect.left;
	int window_height=rect.bottom-rect.top;
#else
	int	window_width=rect.w;
	int window_height=rect.h;
#endif

	// Select Single or Double Screens
	// If hardware is bank switched choose Single screen mode
#if 0
	if ((DD.lpDDDriverCaps->dwCaps & DDCAPS_BANKSWITCHED)||!fullScreen)	
#else
	if (!fullScreen)	
#endif
			NumberOfScreens=1;
	else	NumberOfScreens=2;

	if (fullScreen)
	{
#if 0
    	ddrval=DD.lpDD2->SetCooperativeLevel(DD.hWndApp,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
    	ddrval=DD.lpDD2->SetCooperativeLevel(DD.hWndApp,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
	    ddrval=DD.lpDD2->SetDisplayMode(DD.DDModes[curmode].width,DD.DDModes[curmode].height,
										DD.DDModes[curmode].colourdepth,0,0);
#else
		// Set fullscreen mode with given resolution
		auto* backend = backend_from_hwnd(DD.hWnd);
		SDL_SetWindowSize(backend->window, DD.DDModes[curmode].width, DD.DDModes[curmode].height);
		SDL_SetWindowFullscreen(backend->window, SDL_WINDOW_FULLSCREEN);

		// Optionally set display mode (refresh rate, format)
		SDL_DisplayMode mode;
		mode.w = DD.DDModes[curmode].width;
		mode.h = DD.DDModes[curmode].height;
		mode.format = SDL_PIXELFORMAT_RGB565;
		mode.refresh_rate = 0;                  // driver default
		mode.driverdata = nullptr;

		SDL_SetWindowDisplayMode(backend->window, &mode);
#endif
	}
	else{
		NumberOfScreens=1;
#if 0
    	ddrval=DD.lpDD2->SetCooperativeLevel(DD.hWndApp,DDSCL_NORMAL);
    	ddrval=DD.lpDD2->SetCooperativeLevel(DD.hWndApp,DDSCL_NORMAL);
#endif
	}

    // Set Video Mode
	DD.ModeX = FALSE;

	if (DD.ModeX)	NumberOfScreens = 1;

	DD.lpddsd->dwSize = sizeof(DDSURFACEDESC);

	int d3dcapsrequired = (DD.lpDirect3D==NULL) ? 0 : DDSCAPS_3DDEVICE;

	switch (NumberOfScreens)
	{
	case 0:
#if 0
	    // Create Primary Surface
		DD.lpddsd->dwFlags = DDSD_CAPS;

		DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|
									DDSCAPS_VIDEOMEMORY|
									DDSCAPS_LOCALVIDMEM|
									d3dcapsrequired;
	    ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSPrimary,NULL);

	    if (ddrval!=DD_OK)	return 3;
#endif
		break;
	case 1:
#if 0
	    // Create Primary Surface
	    DD.lpddsd->dwFlags = DDSD_CAPS;

	    DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|
									DDSCAPS_VIDEOMEMORY|
									DDSCAPS_LOCALVIDMEM;
	    ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSPrimary,NULL);

		DDErrorCodes(__FILE__,__LINE__,ddrval);

		if (d3dcapsrequired)
		{
			// Create 3d capeable backbuffer
	    	DD.lpddsd->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

			DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_3DDEVICE|
										DDSCAPS_VIDEOMEMORY|
										DDSCAPS_LOCALVIDMEM;
		}else
		{
			// Create off screen surface - Use system memory
	    	DD.lpddsd->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

			DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
										DDSCAPS_SYSTEMMEMORY;
		}
		DD.lpddsd->dwHeight =	window_height;	//DD.DDModes[DD.DDCurrMode].height;
		DD.lpddsd->dwWidth = 	window_width;	//DD.DDModes[DD.DDCurrMode].width;

	   	ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSBack,NULL);

		if (ddrval!=DD_OK)	return 4;
#else
		DD.lpDDSPrimary = SDL_CreateRGBSurface(0, (int)winmode_w, (int)winmode_h, SDL_PIXELBITCOUNT,
			0, 0, 0, 0);

		if (DD.lpDDSPrimary == NULL)
		{
			printf("Error creating primary surface: %s.\n", SDL_GetError());
		}
		DD.lpDDSBack = SDL_CreateRGBSurface(0, winmode_w, winmode_h, SDL_PIXELBITCOUNT,
			0, 0, 0, 0);

		if (DD.lpDDSBack == NULL)
		{
			printf("Error creating back surface: %s.\n", SDL_GetError());
		}
			
		DD.lpddsd->lpSurface = DD.lpDDSBack->pixels;
		DD.lpddsd->dwWidth = DD.lpDDSBack->w; // 800;
		DD.lpddsd->dwHeight = DD.lpDDSBack->h; // 600;
		DD.lpddsd->lPitch = DD.lpDDSBack->pitch; //  3200; //??
		DD.lpddsd->ddpfPixelFormat.dwRGBBitCount = SDL_PIXELBITCOUNT; //RERUN actually 32
		DD.lpddsd->ddpfPixelFormat.dwRBitMask = DD.lpDDSBack->format->Rmask;
		DD.lpddsd->ddpfPixelFormat.dwGBitMask = DD.lpDDSBack->format->Gmask;
		DD.lpddsd->ddpfPixelFormat.dwBBitMask = DD.lpDDSBack->format->Bmask;
#endif
		break;
	case 2:
#if 0
	    // Create Primary Surface
	    DD.lpddsd->dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		if (d3dcapsrequired)
		{
	    	DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|
										DDSCAPS_FLIP|
										DDSCAPS_COMPLEX|
										DDSCAPS_3DDEVICE| 
										DDSCAPS_VIDEOMEMORY|
										DDSCAPS_LOCALVIDMEM;
		}else
		{
	    	DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|
										DDSCAPS_FLIP|
										DDSCAPS_COMPLEX| 
										DDSCAPS_VIDEOMEMORY|
										DDSCAPS_LOCALVIDMEM;
		}

	    DD.lpddsd->dwBackBufferCount = 1;

    	ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSPrimary,NULL);

		if (ddrval!=DD_OK)	return 3;

		directdraw_ClearSurface(DD.lpDDSPrimary);

		// Attach a Back Surface
		if (d3dcapsrequired)
		{
	    	DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_BACKBUFFER|
							  			DDSCAPS_3DDEVICE| 
										DDSCAPS_VIDEOMEMORY|
										DDSCAPS_LOCALVIDMEM;

		}else
		{
	    	DD.lpddsd->ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
		}

	    ddrval =
		DD.lpDDSPrimary->GetAttachedSurface(&DD.lpddsd->ddsCaps,&DD.lpDDSBack);

    	if (ddrval!=DD_OK)	return 4;
#endif
		break;
	case 3:
#if 0
		// Create 3 surfaces
		// Cant use Primary with 2back buffers because we need ptrs to all 3 screens

	    // Create Primary Surface
	    DD.lpddsd->dwSize			= sizeof (DDSURFACEDESC);
	    DD.lpddsd->dwFlags 			= DDSD_CAPS;// | DDSD_BACKBUFFERCOUNT;
	    DD.lpddsd->ddsCaps.dwCaps 	= DDSCAPS_PRIMARYSURFACE;// | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_COMPLEX;
	    DD.lpddsd->dwBackBufferCount= 0;//2;

    	ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSPrimary,NULL);

		if (ddrval!=DD_OK)	return 3;

		directdraw_ClearSurface(DD.lpDDSPrimary);

	    DD.lpddsd->dwFlags 			= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		DD.lpddsd->ddsCaps.dwCaps 	= DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		DD.lpddsd->dwHeight 		= DD.DDModes[DD.DDCurrMode].height;
		DD.lpddsd->dwWidth 			= DD.DDModes[DD.DDCurrMode].width;

	    ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSBack,NULL);

		if (ddrval!=DD_OK)	return 4;

	    DD.lpddsd->dwFlags 			= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		DD.lpddsd->ddsCaps.dwCaps 	= DDSCAPS_OFFSCREENPLAIN;
		DD.lpddsd->dwHeight 		= DD.DDModes[DD.DDCurrMode].height;
		DD.lpddsd->dwWidth 			= DD.DDModes[DD.DDCurrMode].width;
	    
		ddrval =
		DD.lpDD2->CreateSurface(DD.lpddsd,&DD.lpDDSBack2,NULL);

		if (ddrval!=DD_OK)	return 5;

		DD.Screens[1] = DD.lpDDSPrimary;
		DD.Screens[2] = DD.lpDDSBack;
		DD.Screens[3] = DD.lpDDSBack2;

		DD.DDOnScreen 		= 1;
		DD.DDSpareScreen 	= 2;
		DD.DDLogicalScreen 	= 3;
#endif
		break;
	}

#if 0
	//do a lock/unlock to get the back buffer surface desc
	do
	{
		ddrval=
		DD.lpDDSBack->Lock(NULL,DD.lpddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	}
	while (ddrval!=DD_OK);
	ddrval=DD.lpDDSBack->Unlock(DD.lpddsd->lpSurface);
#endif

	//Only attach palette if doing SW 3D

	//create a new DD palette object here
	if (DD.PaletteBuffer == NULL)
		DD.PaletteBuffer = new MYPALETTEENTRY[256];

	memset(DD.PaletteBuffer, 0, 256 * sizeof(MYPALETTEENTRY));

	if (DD.lpDirect3D!=NULL)
		DD.lpDirect3D->SelectDriver(&DD);

#else
	NumberOfScreens = 1;
	int surfaceWidth = winmode_w;
	int surfaceHeight = winmode_h;
	DD.lpDDSPrimary = SDL_CreateRGBSurface(0, surfaceWidth, surfaceHeight, SDL_PIXELBITCOUNT,
		0, 0, 0, 0);

	if (DD.lpDDSPrimary == NULL)
	{
		printf("Error creating primary surface: %s.\n", SDL_GetError());
	}
	DD.lpDDSBack = SDL_CreateRGBSurface(0, surfaceWidth, surfaceHeight, SDL_PIXELBITCOUNT,
		0, 0, 0, 0);

	if (DD.lpDDSBack == NULL)
	{
		printf("Error creating back surface: %s.\n", SDL_GetError());
	}
		
	DD.lpddsd->lpSurface = DD.lpDDSBack->pixels;
	DD.lpddsd->dwWidth = DD.lpDDSBack->w; // 800;
	DD.lpddsd->dwHeight = DD.lpDDSBack->h; // 600;
	DD.lpddsd->lPitch = DD.lpDDSBack->pitch; //  3200; //??
	DD.lpddsd->ddpfPixelFormat.dwRGBBitCount = SDL_PIXELBITCOUNT; //RERUN actually 32
	DD.lpddsd->ddpfPixelFormat.dwRBitMask = DD.lpDDSBack->format->Rmask;
	DD.lpddsd->ddpfPixelFormat.dwGBitMask = DD.lpDDSBack->format->Gmask;
	DD.lpddsd->ddpfPixelFormat.dwBBitMask = DD.lpDDSBack->format->Bmask;

	//Only attach palette if doing SW 3D

	//create a new DD palette object here
	if (DD.PaletteBuffer == NULL)	DD.PaletteBuffer = new MYPALETTEENTRY[256];

	memset(DD.PaletteBuffer, 0, 256 * sizeof(MYPALETTEENTRY));

#endif //SDL
    return 0;
}

//------------------------------------------------------------------------------
//Procedure		KillSurfaces
//Author		Andrew McRae
//Date			Tue 26 Nov 1996
//
//Description	Release all opened surfaces
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::KillSurfaces ()
{
	HRESULT	dderrval;

	//clear the screens before deleting them to stop this frame of 3d being
	//shown on the next time into the 3d.

	if (DD.lpDirect3D!=NULL)									//PD 26Feb97
	{
		delete (DD.lpDirect3D);
		DD.lpDirect3D=NULL;
	}

	switch (NumberOfScreens)
	{
		case 1:
		RELEASE_SURFACE(DD.lpDDSPrimary,dderrval);
		RELEASE_SURFACE(DD.lpDDSBack,dderrval);
		RELEASE_SURFACE(DD.lpDDSBack2,dderrval);
		break;

		case 2:
		directdraw_ClearSurface(DD.lpDDSPrimary);
		RELEASE_SURFACE(DD.lpDDSPrimary,dderrval);
		DD.lpDDSBack=NULL;
		DD.lpDDSBack2=NULL;
		break;

		default:
		OutputDebugString("Divide by cucumber error\n");
		break;
	}

	//Discard palette object (if created)

	if (DD.lpDDPal != NULL)
	{
#if 0
		delete DD.lpDDPal;
		DD.lpDDPal = NULL;
#else
		SDL_FreePalette(DD.lpDDPal);
		DD.lpDDPal = nullptr;
#endif
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		DirectDraw_MemScreen
//Author		Andrew McRae
//Date			Tue 23 Jul 1996
//
//Description	Direct Draw version of MemScreen (as in hardinit)
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DirectDraw_MemScreen (ScreenPages dpg)
{
#if 0 //SDL
	switch (dpg)
	{
	case SCREEN_LOGICAL:
		while (DD.Screens[DD.DDLogicalScreen]->Lock(NULL,
					   								DD.lpddsd,
					   								00000001L,
					   								NULL) != DD_OK) {};

		logicalscreenptr = (LogicalPtr)DD.lpddsd->lpSurface;

		DD.Screens[DD.DDLogicalScreen]->Unlock (DD.lpddsd->lpSurface );
	break;
	case SCREEN_SPARE:
		while (DD.Screens[DD.DDSpareScreen]->Lock(	NULL,
					   								DD.lpddsd,
					   								00000001L,
					   								NULL) != DD_OK) {};

		logicalscreenptr = (LogicalPtr)DD.lpddsd->lpSurface;

		DD.Screens[DD.DDSpareScreen]->Unlock (DD.lpddsd->lpSurface );
	break;
	}
#else
	printf("SDL: %s Not yet implemented!\n", __FUNCTION__);
#endif
}

//------------------------------------------------------------------------------
//Procedure		XX_ResetGraphicsMode
//Author		Paul.   
//Date			Fri 28 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigDisplay::XX_ResetGraphicsMode()
{
	if (NumberOfScreens>1)
		logicalscreenptr=NULL;	//block deletion

	// do something here ... may be not

}

//------------------------------------------------------------------------------
//Procedure		ChangeBytesPerScanLine
//Author		Paul.   
//Date			Fri 28 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigDisplay::ChangeBytesPerScanLine ()
{
	BytesPerScanLine = DD.lpddsd->lPitch; // number of bytes in a line
}

//------------------------------------------------------------------------------
//Procedure		Body2Screen
//Author		Paul.   
//Date			Wed 25 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::Body2Screen(DoPointStruc& a)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->Body2Screen(a);}

//------------------------------------------------------------------------------
//Procedure		BackupLogicalPtr
//Author		Paul.   
//Date			Tue 21 Oct 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigDisplay::BackupLogicalPtr()
{
	bupLogicalScreenPtr=logicalscreenptr;	
}

//------------------------------------------------------------------------------
//Procedure		ResetLogicalPtr
//Author		Paul.   
//Date			Tue 21 Oct 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigDisplay::ResetLogicalPtr()
{
	WindowMoved (bupLogicalScreenPtr, logicalscreenptr);
}

//------------------------------------------------------------------------------
//Procedure		SmokedGlassBox
//Author		Paul    
//Date			Sun 7 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::SmokedGlassBox(SWord a,SWord b,SWord c,SWord d,ULong e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SmokedGlassBox(&DD,a,b,c,d,e);
}


//------------------------------------------------------------------------------
//Procedure		HWRegisterPal
//Author		Paul.   
//Date			Wed 21 Oct 1998
//------------------------------------------------------------------------------
void MigDisplay::HWRegisterPal(UByte* src,ULong length)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->RegisterPalette(&DD,src,length);
}

//------------------------------------------------------------------------------
//Procedure		ForceTQ
//Author		Paul.   
//Date			Thu 29 Oct 1998
//------------------------------------------------------------------------------
SWord MigDisplay::ForceTQ(SWord a)
{
 	if (DD.lpDirect3D!=NULL) return DD.lpDirect3D->ForceTQ(a);
	return a;
}

//------------------------------------------------------------------------------
//Procedure		FreeTexture
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void	MigDisplay::FreeTexture(const HTEXT& a)
{if (DD.lpDirect3D) DD.lpDirect3D->FreeTexture(a);}

//------------------------------------------------------------------------------
//Procedure		FreeTexture
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void	MigDisplay::FreeTextures()
{if (DD.lpDirect3D) DD.lpDirect3D->FreeTextures();}

//------------------------------------------------------------------------------
//Procedure		AllocateTexture
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
HTEXT	MigDisplay::AllocateTexture(UByte a)
{if (DD.lpDirect3D) return DD.lpDirect3D->AllocateTexture(a);return HTEXTNULL;}

//------------------------------------------------------------------------------
//Procedure		GetTextureScales
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::GetTextureScales(SLong& a,SLong& b,SLong& c,SLong& d,SLong& e,SLong& f)
{
	if (DD.lpDirect3D) 	DD.lpDirect3D->GetTextureScales(a,b,c,d,e,f);
	else				a=b=c=d=e=f=0;
}

//------------------------------------------------------------------------------
//Procedure		RenderTileToDDSurface
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::RenderTileToDDSurface(UByte* a,const HTEXT& b)
{if (DD.lpDirect3D) DD.lpDirect3D->RenderTileToDDSurface(&DD,a,b);}

void MigDisplay::GetSurfaceDimensions(SLong& a,SLong& b)
//RERUN {if (DD.lpDirectDD!=NULL) DD.lpDirectDD->GetSurfaceDimensions(&DD,a,b);}
{
	if (_DirectDraw::lpDirectDD != NULL) 
		_DirectDraw::lpDirectDD->GetSurfaceDimensions(&DD, a, b);
}

void MigDisplay::PutC(ImageMapDesc* a,DoPointStruc* b,bool fRefresh)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->PutC(&DD,a,b,fRefresh);}

void MigDisplay::PutA(ImageMapDesc* a,DoPointStruc* b)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->PutA(&DD,a,b);}

void MigDisplay::D2DLine(UWord a,UWord b,UWord c,UWord d)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->D2DLine(&DD,a,b,c,d);}

void MigDisplay::SetFontColour(ULong a)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SetFontColour(a);}

//------------------------------------------------------------------------------
//Procedure		PeripheralBlob
//Author		Paul.   
//Date			Mon 23 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::PeripheralBlob(Float* a,Float* b,Float* c,UByte d,SLong e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->PeripheralBlob(a,b,c,d,e);
}

//------------------------------------------------------------------------------
//Procedure		SetMipMap
//Author		Paul.   
//Date			Tue 24 Nov 1998
//------------------------------------------------------------------------------
bool MigDisplay::SetMipMap(bool a)
{
	if (DD.lpDirect3D!=NULL) return DD.lpDirect3D->SetMipMap(a);
	return a;
}

//------------------------------------------------------------------------------
//Procedure		SetSunPos
//Author		Paul.   
//Date			Fri 27 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::SetSunPos(DoPointStruc& a){if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SetSunPos(a);}

//------------------------------------------------------------------------------
//Procedure		SetEdgeTest
//Author		Paul.   
//Date			Thu 10 Dec 1998
//------------------------------------------------------------------------------
bool MigDisplay::SetEdgeTest(bool a)
{
	if (DD.lpDirect3D!=NULL)
		return DD.lpDirect3D->SetEdgeTest(a);

	return a;
}

//------------------------------------------------------------------------------
//Procedure		GammaCorr
//Author		Paul.   
//Date			Wed 23 Dec 1998
//------------------------------------------------------------------------------
void MigDisplay::GammaCorr(UByte& r,UByte& g,UByte& b)
{
	if (DD.lpDirect3D!=NULL)
		DD.lpDirect3D->GammaCorr(r,g,b);
}

void MigDisplay::CaptureScreen() {if (DD.lpDirect3D) DD.lpDirect3D->CaptureScreen(&DD);}
void MigDisplay::SaveVideoBuffer() {if (DD.lpDirect3D) DD.lpDirect3D->SaveVideoBuffer(&DD);}


//------------------------------------------------------------------------------
//Procedure		HardPoly
//Author		Paul.   
//Date			Fri 7 Feb 1997
//
//Description	
//
//Inputs		
//
//
//Returns	
//------------------------------------------------------------------------------
Bool	MigDisplay::HardPoly(LPPOLYTYPE lpPolyType)
{
	if (DD.lpDirect3D==NULL)	return (FALSE);

	switch (lpPolyType->ptType)
	{
		default:
		return (FALSE);

		case POLYTYPE::PT_PlainColour:
		case POLYTYPE::PT_MaskedImagem:
		case POLYTYPE::PT_PlainImagem:
		case POLYTYPE::PT_GColour:
		case POLYTYPE::PT_HighLand:
		case POLYTYPE::PT_FakeFog:
		DD.lpDirect3D->AddPoly(&DD,lpPolyType);
		break;
	}
	return (TRUE);
}

void MigDisplay::PushViewData() {if (DD.lpDirect3D) DD.lpDirect3D->PushViewData(&DD);}
void MigDisplay::PopViewData() {if (DD.lpDirect3D) DD.lpDirect3D->PopViewData(&DD);}
void MigDisplay::Set3DWin(SLong x,SLong y,SLong w,SLong h) {if (DD.lpDirect3D) DD.lpDirect3D->Set3DWin(&DD,x,y,w,h);}


bool MigDisplay::ScanForBottomPixelsOfText(SWord a,SWord b,SWord c)
{
 	if (DD.lpDirect3D)
		return DD.lpDirect3D->ScanForBottomPixelsOfText(&DD,a,b,c);
	return true;
}

void MigDisplay::PushPull(UByte a)
{
}

void	MigDisplay::MakePaletteFadeTable(UByte* a,UWord* b,UByte c,UByte d,UByte e)
{
	//if (DD.lpDirectDD)	DD.lpDirectDD->MakePaletteFadeTable(&DD,a,b,c,d,e);
	if (_DirectDraw::lpDirectDD)	
		_DirectDraw::lpDirectDD->MakePaletteFadeTable(&DD, a, b, c, d, e);
}

void MigDisplay::MakeColourRangeFadeTable(UWord* a,UByte b,UByte c,UByte d,UByte e,UByte f,UByte g)
{
	if (_DirectDraw::lpDirectDD) 
		_DirectDraw::lpDirectDD->MakeColourRangeFadeTable(&DD, a, b, c, d, e, f, g);
}

void MigDisplay::LockScr(ROWANSURFACEDESC* a)
{
	if (_DirectDraw::lpDirectDD) 
		_DirectDraw::lpDirectDD->LockScr(&DD, a);
}

void MigDisplay::UnlockScr(ROWANSURFACEDESC* a)
{
	if (_DirectDraw::lpDirectDD)
		_DirectDraw::lpDirectDD->UnlockScr(&DD, a);
}

void MigDisplay::GetColourInfo(UWord& redMask,SWord& redShift1,SWord& redShift2,
							UWord& greenMask,SWord& greenShift1,SWord& greenShift2,
							UWord& blueMask,SWord& blueShift1,SWord& blueShift2)
{
	if (_DirectDraw::lpDirectDD) 
		_DirectDraw::lpDirectDD->GetColourInfo(&DD,
										redMask,redShift1,redShift2,
										greenMask,greenShift1,greenShift2,
										blueMask,blueShift1,blueShift2);
}

//------------------------------------------------------------------------------
//Procedure		PutLine
//Author		Paul.   
//Date			Fri 26 Mar 1999
//------------------------------------------------------------------------------
void MigDisplay::PutLine(SWord a,SWord b,SWord c,SWord d,Colour e)
{
 	if (DD.lpDirect3D==NULL)
		return;
	DD.lpDirect3D->PutLine(&DD,a,b,c,d,e);
}

//------------------------------------------------------------------------------
//Procedure		FlushAll
//Author		Paul.   
//Date			Wed 28 Oct 1998
//------------------------------------------------------------------------------
void MigDisplay::FlushAll(Bool f)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->FlushAll(&DD,f);
}

//------------------------------------------------------------------------------
//Procedure		CalcItemFog
//Author		Paul.   
//Date			Wed 9 Sep 1998
//------------------------------------------------------------------------------
void MigDisplay::CalcItemFog(bool a,Float& b,Float& c,Float& d)
{
	if (DD.lpDirect3D!=NULL)
		DD.lpDirect3D->CalcItemFog(a,b,c,d);
}

bool  MigDisplay::SetTranspFade(bool a)
{
	if (DD.lpDirect3D!=NULL) 
	{
		return DD.lpDirect3D->SetTranspFade(a);
	}
	return a;
}


//------------------------------------------------------------------------------
//Procedure		NewTile
//Author		Paul.   
//Date			Mon 27 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::NewTile(SLong z,DoPointStruc* a,SLong b,UByte* c,SLong d)
{
	if (DD.lpDirect3D) DD.lpDirect3D->NewTile(&DD,z,a,b,c,d);
}
//------------------------------------------------------------------------------
//Procedure		CNewTile
//Author		Paul.   
//Date			Mon 27 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::CNewTile(SLong z,DoPointStruc* a,SLong b,UByte* c,SLong d)
{
 	if (DD.lpDirect3D) DD.lpDirect3D->CNewTile(&DD,z,a,b,c,d);
}

//------------------------------------------------------------------------------
//Procedure		TestAsyncBlt
//Author		Paul.   
//Date			Tue 21 Jul 1998
//
//------------------------------------------------------------------------------
bool	MigDisplay::TestAsyncBlt()
{
	if (DD.lpDirect3D) return DD.lpDirect3D->TestAsyncBlt();
	return false;
}

//------------------------------------------------------------------------------
//Procedure		BeginTextureScroll
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::BeginTextureScroll(const SWord a,void* b,void* c)
{
}

//------------------------------------------------------------------------------
//Procedure		WaitTextureScrollComplete
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::WaitTextureScrollComplete(const SWord a)
{
	if (DD.lpDirect3D) DD.lpDirect3D->WaitTextureScrollComplete(&DD,a);
}

//------------------------------------------------------------------------------
//Procedure		PartialTextureUpload
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::PartialTextureUpload(const SWord a,void* b,void* c)
{
 	if (DD.lpDirect3D) DD.lpDirect3D->PartialTextureUpload(&DD,a,b,c);
}

//------------------------------------------------------------------------------
//Procedure		ShowLoaderArt
//Author		Paul    
//Date			Sat 20 Feb 1999
//------------------------------------------------------------------------------
void MigDisplay::ShowLoaderArt(void* pArt)
{
	if (DD.lpDirect3D) DD.lpDirect3D->ShowLoaderArt(&DD,pArt);
}

//------------------------------------------------------------------------------
//Procedure		CreateLandTBuffers
//Author		Paul.   
//Date			Thu 16 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::CreateLandTBuffers()
{
	if (DD.lpDirect3D)	DD.lpDirect3D->CreateLandTBuffers(&DD);
}

void*	MigDisplay::GetLandBufferPtr(SWord a)
{
	if (!DD.lpDirect3D)
	return NULL;
	return DD.lpDirect3D->GetLandBufferPtr(&DD,a);
}
void*	MigDisplay::GetLandPalLookup()
{
	if (!DD.lpDirect3D)
	return NULL;

	return DD.lpDirect3D->GetLandPalLookup(&DD);
}
void	MigDisplay::SetLandPalLookup(UByte* a)
{
	if (!DD.lpDirect3D)
		return;

	DD.lpDirect3D->SetLandPalLookup(&DD,a);
}
SWord	MigDisplay::GetLandBpp()
{
	if (!DD.lpDirect3D)
		return 0;

	return DD.lpDirect3D->GetLandBpp(&DD);
}

//------------------------------------------------------------------------------
//Procedure		SetAspectRatio
//Author		Paul.   
//Date			Wed 1 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::SetAspectRatio(Float ar,Float fov)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SetAspectRatio(ar,fov);
}

//------------------------------------------------------------------------------
//Procedure		FlushLandDraw
//Author		Dallas  
//Date			Wed 1 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::FlushLandDraw()
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->FlushLandscapeBuffers(&DD);
}
//------------------------------------------------------------------------------
//Procedure		CLEB
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::CLEB()
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->CreateLandExecuteBuffer();
}
//------------------------------------------------------------------------------
//Procedure		AddTile
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::AddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->AddTileVertices(&DD,a,aa,b,c,d);
}
//------------------------------------------------------------------------------
//Procedure		AddTile
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::CAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->CAddTileVertices(&DD,a,aa,b,c,d);
}
//------------------------------------------------------------------------------
//Procedure		NearAddTile
//Author		Paul    
//Date			Sun 21 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::NearAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d,ULong e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->NearAddTileVertices(&DD,a,aa,b,c,d,e);
}
//------------------------------------------------------------------------------
//Procedure		CNearAddTile
//Author		Paul.   
//Date			Wed 24 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::CNearAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d,ULong e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->CNearAddTileVertices(&DD,a,aa,b,c,d,e);
}

//------------------------------------------------------------------------------
//Procedure		CNearAddTileX
//Author		Paul.   
//Date			Tue 10 Nov 1998
//------------------------------------------------------------------------------
void	MigDisplay::CNearAddTileX(ULong a,ULong b,DoPointStruc* c,ULong d,UByte*& e,const HTEXT& f,UWord flag)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->CNearAddTileVerticesX(&DD,a,b,c,d,e,f,flag);
}

//------------------------------------------------------------------------------
//Procedure		NearAddTileX
//Author		Paul.   
//Date			Tue 10 Nov 1998
//------------------------------------------------------------------------------
void	MigDisplay::NearAddTileX(ULong a,ULong b,DoPointStruc* c,ULong d,UByte*& e,const HTEXT& f,UWord flag)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->NearAddTileVerticesX(&DD,a,b,c,d,e,f,flag);
}

//------------------------------------------------------------------------------
//Procedure		NearAddDitherX
//Author		Paul.   
//Date			Tue 1 Dec 1998
//------------------------------------------------------------------------------
void	MigDisplay::NearAddDitherX(ULong a,ULong b,DoPointStruc* c,ULong d,UByte*& e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->NearAddTileDitherX(&DD,a,b,c,d,e);
}

//------------------------------------------------------------------------------
//Procedure		DoBeginFan
//Author		Paul.   
//Date			Tue 2 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::BeginFan(ULong hTexture)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->BeginFan(hTexture);}

//------------------------------------------------------------------------------
//Procedure		DoBeginFan
//Author		Paul.   
//Date			Tue 2 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::BeginFan(ImageMapDesc* pImage)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->BeginFan(&DD,pImage);}

//------------------------------------------------------------------------------
//Procedure		DoAddFanPoint
//Author		Paul.   
//Date			Tue 2 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::AddFanPoint(DoPointStruc* pdp,SLong vCnt,UByte*& pData)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->AddFanPoint(&DD,pdp,vCnt,pData);}

//------------------------------------------------------------------------------
//Procedure		UploadLandTexture
//Author		Paul.   
//Date			Mon 18 May 1998
//------------------------------------------------------------------------------
void MigDisplay::UploadLandTexture(ULong hTexture,ImageMapDesc* lpImage)
{
 	if (DD.lpDirect3D!=NULL)
		DD.lpDirect3D->UploadLandTexture(&DD,hTexture,lpImage);
}

//------------------------------------------------------------------------------
//Procedure		SetClipState
//Author		Paul.   
//Date			Wed 29 Apr 1998
//------------------------------------------------------------------------------
bool MigDisplay::SetClipState(bool newState)
{
	if (DD.lpDirect3D!=NULL) return DD.lpDirect3D->SetClipState(newState);
	return newState;
}

//------------------------------------------------------------------------------
//Procedure		ReleaseTextures
//Author		Paul.   
//Date			Tue 21 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::ReleaseTextures()
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->ReleaseTextures();}

//------------------------------------------------------------------------------
//Procedure		SetTextureQuality
//Author		Paul.   
//Date			Mon 20 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::SetTextureQuality(bool b)
{if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SetTextureQuality(&DD,b);}

//------------------------------------------------------------------------------
//Procedure		SetRangeFog
//Author		Paul.   
//Date			Wed 8 Apr 1998
//------------------------------------------------------------------------------
bool MigDisplay::SetRangeFog(bool b)
{
	if (DD.lpDirect3D!=NULL) return DD.lpDirect3D->SetRangeFog(b);
	return b;
}

//------------------------------------------------------------------------------
//Procedure		DoGetHorizonColour
//Author		Paul.   
//Date			Mon 6 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::GetHorizonColour(UByte& a,UByte& b,UByte& c)
{
	a=DD.hcRed;
	b=DD.hcGreen;
	c=DD.hcBlue;
}
//------------------------------------------------------------------------------
//Procedure		DoSetHorizonColour
//Author		Paul.   
//Date			Mon 6 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::SetHorizonColour(UByte a,UByte b,UByte c)
{
	DD.hcRed=a;
	DD.hcGreen=b;
	DD.hcBlue=c;
}
//------------------------------------------------------------------------------
//Procedure		DoSetShadeVals
//Author		Paul.   
//Date			Wed 1 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::SetShadeVals(SLong nr,SLong ng,SLong nb,SLong er,SLong eg,SLong eb)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SetShadeVals(nr,ng,nb,er,eg,eb);
}

//------------------------------------------------------------------------------
//Procedure		DoSetLShade
//Author		Paul.   
//Date			Wed 1 Apr 1998
//------------------------------------------------------------------------------
bool MigDisplay::SetLShade(bool b)
{
	if (DD.lpDirect3D!=NULL) return DD.lpDirect3D->SetLShade(b);
	return b;
}

//------------------------------------------------------------------------------
//Procedure		SetGlobalAlpha
//Author		Paul.   
//Date			Fri 27 Mar 1998
//------------------------------------------------------------------------------
UByte MigDisplay::SetGlobalAlpha(UByte a)
{
	if (DD.lpDirect3D!=NULL) return DD.lpDirect3D->SetGlobalAlpha(a);
	return a;
}

//------------------------------------------------------------------------------
//Procedure		ForceRender
//Author		Paul.   
//Date			Tue 24 Mar 1998
//------------------------------------------------------------------------------
void MigDisplay::ForceRender()
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->ForceRender(&DD);
}

//------------------------------------------------------------------------------
//Procedure		DomeHorizon
//Author		Paul.   
//Date			Wed 26 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DomeHorizon(void* lpData,void* lpPolys)
{
}

//------------------------------------------------------------------------------
//Procedure		SetFogState
//Author		Paul.   
//Date			Thu 9 Oct 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::SetFogState(Bool bFogState)
{
	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->SetFogState(bFogState);
}

//------------------------------------------------------------------------------
//Procedure		SetFadeValue
//Author		Paul.   
//Date			Sat 7 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::SetFadeValue(UByte newValue)
{
	if (DD.lpDirect3D==NULL)	return;

 	DD.lpDirect3D->SetFadeValue(newValue);
}

//------------------------------------------------------------------------------
//Procedure		EnableBackClip
//Author		Paul.   
//Date			Thu 5 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::EnableBackClip(Bool bFlag)
{
	if (DD.lpDirect3D==NULL)	return (bFlag);

	return (DD.lpDirect3D->EnableBackClip(bFlag));
}

//------------------------------------------------------------------------------
//Procedure		SetFade
//Author		Paul.   
//Date			Thu 5 Jun 1997
//
//Description	tCol = colour index into palette	
//				step = current stage of the fade 0(no fade)...31(full fade)
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::SetFade(Colour tCol,SLong step,TwoDeeEffect tdfx)
{
	if (DD.lpDirect3D==NULL)	return;
	
	DD.lpDirect3D->SetFade(tCol,step,tdfx);
}

//------------------------------------------------------------------------------
//Procedure		ModeSupported
//Author		Paul.   
//Date			Thu 29 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	MigDisplay::ModeSupported(ScreenRez* lpSRez)
{
	//RERUN return (DD.lpDirectDD->ModeSupported(lpSRez));
	return (_DirectDraw::lpDirectDD->ModeSupported(lpSRez));
}

//------------------------------------------------------------------------------
//Procedure		DriverIsCapable
//Author		Paul.   
//Date			Thu 29 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	MigDisplay::DriverIsCapable(SWord drvno)
{
	//RERUN return (DD.lpDirectDD->DriverIsCapable(drvno));
	return (_DirectDraw::lpDirectDD->DriverIsCapable(drvno));
}

//------------------------------------------------------------------------------
//Procedure		NameNumberedDriver
//Author		Paul.   
//Date			Fri 23 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UByte*	MigDisplay::NameNumberedDriver(SWord driverno)
{
	if (driverno==-1)	
		return (NULL);
	
	//RERUN return (DD.lpDirectDD->NameNumberedDriver(driverno));
	return (_DirectDraw::lpDirectDD->NameNumberedDriver(driverno));
}

//------------------------------------------------------------------------------
//Procedure		CountDrivers
//Author		Paul.   
//Date			Tue 27 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SWord MigDisplay::CountDrivers()
{
 	//RERUN return (DD.lpDirectDD->CountDrivers());
	return (_DirectDraw::lpDirectDD->CountDrivers());
}

//------------------------------------------------------------------------------
//Procedure		HardLine
//Author		Paul.   
//Date			Mon 28 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::HardLine(LPLINETYPE lpLineType)
{
	if (DD.lpDirect3D==NULL)	return (FALSE);

	DD.lpDirect3D->AddPlainLine(&DD,lpLineType);

	return (TRUE);
}

//------------------------------------------------------------------------------
//Procedure		HardPoint
//Author		Paul.   
//Date			Thu 8 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::HardPoint(LPPOINTTYPE lpPointType)
{
	if (DD.lpDirect3D==NULL)	return (FALSE);

	DD.lpDirect3D->AddPlainPoint(&DD,lpPointType);

	return (TRUE);
}

//------------------------------------------------------------------------------
//Procedure		SetViewParams
//Author		Paul.   
//Date			Fri 18 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::SetViewParams(ANGLES a,MigWindow* w,ViewPoint*)
{
	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->SetViewParams(a,w,&DD);
}


//------------------------------------------------------------------------------
//Procedure	GrafPrim::PlainRectangleFill
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Mon 3 Jul 1995
//Description	
//Externals
//------------------------------------------------------------------------------
Bool MigDisplay::HardPlainRectFill(MigWindow* win,SWord X,SWord Y,SWord W,SWord H)
{
	if (DD.lpDirect3D==NULL)	return (FALSE);

	LogicalPtr	scradr,
				scanlineaddr;

	int		i,j;

	ULong	tColour;

	//Clip the rectangle to the screen

	if((Y+H) <= win->PhysicalMinY)					return (TRUE);
	if(Y >= win->PhysicalMinY+win->PhysicalHeight)	return (TRUE);
	if((X+W) <= win->PhysicalMinX)					return (TRUE);
	if(X >= win->PhysicalMinX+win->PhysicalWidth)	return (TRUE);

	if(Y < win->PhysicalMinY)
	{
		H -=	(win->PhysicalMinY-Y);
		Y = 	win->PhysicalMinY;
	}

	if((Y+H) > (win->PhysicalMinY+win->PhysicalHeight))
	{
		H -=	((Y+H)-(win->PhysicalMinY+win->PhysicalHeight));
	}

	if(X < win->PhysicalMinX)
	{
		W -= (win->PhysicalMinX-X);
		X = 	win->PhysicalMinX;
	}

	if((X+W) > (win->PhysicalMinX+win->PhysicalWidth))
	{
		W -= ((X+W)-(win->PhysicalMinX+win->PhysicalWidth));
	}

	//Calculate the screen address of the top left corner of the rectangle

//DEAD 	LockBackScreen (FALSE);										//PD 11Jul97

	scradr = win->logicalscreenptr +
			 win->BytesPerScanLine * Y +
			 win->BytesPerPixel * X;

	ULong	red,green,blue;

	red = DD.PaletteBuffer[win->colour].peRed;
	green = DD.PaletteBuffer[win->colour].peGreen;
	blue = DD.PaletteBuffer[win->colour].peBlue;

	tColour = 	(red>>(8-RedBits))		<<RedShift;				//DAW 16Oct97
	tColour += 	(green>>(8-GreenBits))	<<GreenShift;			//DAW 16Oct97
	tColour += 	(blue>>(8-BlueBits))	<<BlueShift;			//DAW 16Oct97

	ULong	lcolour=((ULong)0x00010001)*tColour;

	for(i=H;i>0;i--)
	{
		scanlineaddr = scradr;
		j=W-3;
		if	((ULong )scanlineaddr &0x00000003)
		{
			*(UWord*) (scanlineaddr) = tColour;
			scanlineaddr+=2;
			j--;
			if	((ULong )scanlineaddr &0x00000003)
			{
				*(UWord*) (scanlineaddr) = tColour;
				scanlineaddr+=2;
				j--;
				if	((ULong )scanlineaddr &0x00000003)
				{
					*(UWord*) (scanlineaddr) = tColour;
					scanlineaddr+=2;
					j--;
		}	}	}
		scradr += BytesPerScanLine;
		while(j>0)
		{
			*(ULong *)scanlineaddr = lcolour;
			scanlineaddr+=4;
			j-=2;
		}
		for(j=j+3;j>0;scanlineaddr+=2,j--)
			*(UWord*) (scanlineaddr) = tColour;
	}

//DEAD 	UnLockBackScreen ();										//PD 11Jul97

	return(TRUE);
}


//------------------------------------------------------------------------------
//Procedure		SetZUpdate
//Author		Paul.   
//Date			Mon 7 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::SetZUpdate(Bool flag)
{
	if (DD.lpDirect3D==NULL)	return (flag);

	return (DD.lpDirect3D->SetZUpdate(&DD,flag));
}

//------------------------------------------------------------------------------
//Procedure		SetPlayerTextures
//Author		Paul.   
//Date			Thu 8 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::SetPlayerTextures(Bool flag)
{
	return (flag);
}


//------------------------------------------------------------------------------
//Procedure		Hardware3D
//Author		Paul.   
//Date			Tue 25 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::Hardware3D()
{
	if (DD.lpDirect3D==NULL)	return (FALSE);

	return (TRUE);
}

//------------------------------------------------------------------------------
//Procedure		SetMatrices
//Author		Paul.   
//Date			Mon 24 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::SetMatrices(SLong viewrange,ANGLES viewang)
{
	if (DD.lpDirect3D==NULL)	return (FALSE);

	DD.lpDirect3D->SetMatrices(&DD,viewrange,viewang);

	return (TRUE);
}

//------------------------------------------------------------------------------
//Procedure		SetFog
//Author		Paul.   
//Date			Mon 24 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::SetFog(UByte r,UByte g,UByte b)
{
	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->SetFog(r,g,b);
}

//------------------------------------------------------------------------------
//Procedure		SetFog
//Author		Paul.   
//Date			Fri 23 Oct 1998
//------------------------------------------------------------------------------
void	MigDisplay::SetFog(UByte r,UByte g,UByte b,SLong s,SLong e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->SetFog(r,g,b,s,e);
}

//------------------------------------------------------------------------------
//Procedure		GetFog
//Author		Paul.   
//Date			Fri 23 Oct 1998
//------------------------------------------------------------------------------
void	MigDisplay::GetFog(UByte& r,UByte& g,UByte& b,SLong& s,SLong& e)
{
	if (DD.lpDirect3D!=NULL) DD.lpDirect3D->GetFog(r,g,b,s,e);
}

//------------------------------------------------------------------------------
//Procedure		BeginScene
//Author		Paul.   
//Date			Mon 17 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::BeginScene()
{
	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->BeginScene(&DD);
}

//------------------------------------------------------------------------------
//Procedure		EndScene
//Author		Paul.   
//Date			Mon 17 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::EndScene()
{
	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->EndScene(&DD);
}
void MigDisplay::End3DScene()
{
	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->End3DScene(&DD);
}

//------------------------------------------------------------------------------
//Procedure		HCls
//Author		Paul.   
//Date			Mon 10 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::HCls(Bool bFlag)
{
 	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->ClearViewport(&DD,bFlag);
}


//------------------------------------------------------------------------------
//Procedure		TestHW3D
//Author		Paul.   
//Date			Tue 23 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::TestHW3D(MigWindow* w)
{
 	if (DD.lpDirect3D==NULL)	return;

	DD.lpDirect3D->TestHW3D(&DD,w);
}
