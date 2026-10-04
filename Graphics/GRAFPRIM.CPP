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

#include "_GRAP.H" //RERUN

//------------------------------------------------------------------------------
//Filename       GrafPrim.cpp
//System         
//Author         Paul.   
//Date           Mon 3 Jul 1995
//Description    
//------------------------------------------------------------------------------
#include	"DISPLAY.H"
#include	"LIBDEFS.H"
#include    "MYMATH.H"
#include    "MATRIX.H"
#include	"MOUSE.H"
#include	"WORLDINC.H"
#include	"POLYGON.H"
#include	"FONTS.H"
#pragma warning (disable: 4701)
#ifdef	__MSVC__

	extern "C"	void XASM_SetColour(void);
	extern "C"	void XASM_SetPixelWidth(void);
	extern "C"	void XASM_GetPaletteTable(void);
	extern "C"	void XASM_SetTransparency(void);
	extern "C"	void XASM_PlainLine(void);
	extern "C"	void XASM_HoriLineAddr(void);
	extern "C"	void XASM_SmokedGlassLine(void);
	extern "C"	void XASM_GetTransparency(void);
	extern "C"	void XASM_SetLuminosity(void);
	extern "C"	void XASM_SetHeatHaze(void);
	extern "C"	void XASM_SelectPalette(void);
	extern "C"	void XASM_GetLandFadeTable(void);
	extern "C"	void XASM_GetHorizonFadeTable(void);
	extern "C"	void XASM_GetPaletteEntry(void);
	extern "C"	void XASM_SetPaletteEntry(void);

//-----------------------------------------------------------------------------
// Procedure    ASM_SetColour
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
inline void ASM_SetColour(Colour a,SLong b,SLong c,UByteP d)
{
	_asm
	{
		push	ecx
		mov		eax,a
		mov		edx,b
		mov		ebx,c
		mov		ecx,d
		call	XASM_SetColour
		pop		ecx
	}
}

#pragma warning (disable:4035)
inline void* ASM_GetPaletteTable()
{
	_asm {	call XASM_GetPaletteTable }
}

inline UWord ASM_GetPaletteEntry(UWord c)
{
	_asm{
		movzx eax,c;
		call XASM_GetPaletteEntry;
	}
}

inline void ASM_SetPaletteEntry(UWord c,UWord v)
{
	_asm{
		movzx	eax,c;
		mov		dx,v;
		call	XASM_SetPaletteEntry;
	}
}

inline void ASM_SelectPalette(SWord palNum)
{
	_asm {
		mov	ax,palNum
		call XASM_SelectPalette
	}
}
inline void* ASM_GetLandFadeTable()
{
	_asm{
		call XASM_GetLandFadeTable
	}
}
inline void* ASM_GetSkyFadeTable()
{
	_asm {
		call XASM_GetLandFadeTable;
		mov eax,edx
	}
}

inline void* ASM_GetHorizonFadeTable()
{
	_asm { call XASM_GetHorizonFadeTable }
}

//-----------------------------------------------------------------------------
// Procedure    ASM_SetPixelWidth
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
inline SWord	ASM_SetPixelWidth(SWord pw)
{
	_asm
	{
		mov		ax,pw
		call	XASM_SetPixelWidth
	}
}
#pragma warning (default:4035)

//-----------------------------------------------------------------------------
// Procedure    ASM_SetTransparency
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
inline	void ASM_SetTransparency(SWord t)
{
	_asm 
	{
		mov		ax,t
		call	XASM_SetTransparency
	}
}

//------------------------------------------------------------------------------
//Procedure		ASM_SetLuminosity
//Author		Robert Slater
//Date			Mon 19 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline	void ASM_SetLuminosity(SWord t)
{
	_asm 
	{
		push	eax
		mov		ax,t
		call	XASM_SetLuminosity
		pop		eax
	}
}

//------------------------------------------------------------------------------
//Procedure		ASM_SetHeatHaze
//Author		Robert Slater
//Date			Fri 30 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline	void ASM_SetHeatHaze(SWord t)
{
	_asm 
	{
		push	eax
		mov		ax,t
		call	XASM_SetHeatHaze
		pop		eax
	}
}

//------------------------------------------------------------------------------
//Procedure		ASM_GetTransparency
//Author		Robert Slater
//Date			Tue 13 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline	SWord ASM_GetTransparency(void)
{
	SWord	t;
	_asm 
	{
		push	eax
		call	XASM_GetTransparency
		mov		t,ax
		pop		eax
	}
	return t;
}


//-----------------------------------------------------------------------------
// Procedure    ASM_PlainLine
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
inline	void ASM_PlainLine(SLong* a,SLong* b,UByte* c,SLong d)
{
	_asm
	{
		push	ecx
		mov		eax,a
		mov		edx,b
		mov		ebx,c
		mov		ecx,d
		call	XASM_PlainLine
		pop		ecx
	}
}


//-----------------------------------------------------------------------------
// Procedure    ASM_HoriLineAddr
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
#pragma warning (disable:4035)
inline	HoriLineRtnPtr ASM_HoriLineAddr(SCANLINETYPE t)
{
	_asm
	{
		mov		eax,t
		call	XASM_HoriLineAddr
	}
}
#pragma warning (default:4035)

//-----------------------------------------------------------------------------
// Procedure    ASM_SmokedGlassLine
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
inline	void ASM_SmokedGlassLine(UByte * a, VERTEX& b, VERTEX& c)
{
	_asm
	{
		mov		eax,a
		mov		edx,b
		mov		ebx,c
		call	XASM_SmokedGlassLine
	}
}
#elif defined __GNUC__
// RERUN: GCC implementations go here
extern "C" {
	uint16_t PixelWidth = 4;
    SWord Trns3DEnabled = 0; // MASM: DW 0
    //RERUN: extent of the locked SDL surface; set in direct_draw::LockScr,
    //checked by the XASM_*Line* writers so a stray span can't write OOB
    uint8_t* xasm_surface_end = nullptr;
    uint8_t* xasm_surface_start = nullptr;

    // Global buffers (match MASM sizes and zero-init)
    uint8_t LandFadeData[4098 * 2] = {0};
    uint8_t SkyFadeData[4098 * 2]  = {0};
	// 32 bytes, zero-initialized
    uint8_t HorizonFadeData[16 * 2] = {0};
}

extern "C" {
	uint8_t Grey3DTable[256] = {0};
	int16_t  Grey3DEnabled   = 0;   // DW 0
	uint8_t  Lum3DEnabled    = 0;   // DB 0
	uint8_t  Haze3DEnabled   = 0;   // DB 0

	// Scratch temporaries (MASM “DD ?”); in C we’ll use locals,
	// but keep them here if other routines depend on shared state.
	int32_t IX_tmp     = 0;
	int32_t DIX_tmp    = 0;
	int32_t IY_tmp     = 0;
	int32_t DIY_tmp    = 0;
	int32_t INTENS_tmp = 0;
	int32_t DINTENS_tmp= 0;

	// JITTER DW 0,0
	uint16_t JITTER[2] = {0,0};

	// FUP pattern (exact bytes from MASM)
	uint8_t FUP[8] = {5,1,7,3,4,0,6,2};
	// The trailing “DW 9 dup (?)” is padding/alignment in MASM; ignore in C.

	// Local alias buffer (size 64, uninitialized); zero-init for safety.
	uint8_t LclAilBuf[64] = {0};
} // extern "C"

// TransparencyData is used by TF routine; declare if not already present.
// Size and contents should match original asset; placeholder zero-init here.
// TransparencyData table from MASM (0x79 bytes)
uint8_t TransparencyData[0x79] = {
    // First 8 entries: all zero
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,

    // Next 8 entries: single bits
    0b10000000, 0b01000000, 0b00100000, 0b00010000,
    0b00001000, 0b00000100, 0b00000010, 0b00000001,

    // Next 8 entries: alternating pairs
    0b10001000, 0b01000100, 0b00100010, 0b00010001,
    0b10001000, 0b01000100, 0b00100010, 0b00010001,

    // Next 8 entries: mixed diagonals
    0b10001001, 0b01010100, 0b00101010, 0b01010001,
    0b10101000, 0b01010100, 0b00101010, 0b00010101,

    // Next 8 entries: denser patterns
    0b10011001, 0b01010101, 0b01101010, 0b01010011,
    0b10101010, 0b01010101, 0b10101010, 0b01010101,

    // Next 8 entries: more complex diagonals
    0b11011001, 0b01011101, 0b01101110, 0b01011011,
    0b11101010, 0b01010111, 0b10111010, 0b01110101,

    // Next 8 entries: near‑full masks
    0b11011101, 0b11011101, 0b01101111, 0b01111011,
    0b11101110, 0b01110111, 0b10111011, 0b01111101,

    // Final 7 entries: full masks
    0b11011111, 0b11111101, 0b01111111, 0b11111011,
    0b11111110, 0b11110111, 0b10111111, 0b11111101
};



// 256 words for landscape palette
uint16_t landscape_palette[256] = {0};

// 256*8 words (2048 entries) for palette_buffer
uint16_t palette_buffer[256 * 8] = {0};

// 256 words for palette_table
uint16_t palette_table[256] = {0};


// Vertex is in 3DDEFS.H

#pragma pack(push, 1)
struct colourdata {
    uint32_t imageptr;     // +0
    uint32_t alphaptr;     // +4
    uint32_t imagexmask;   // +8
    uint32_t imageymask;   // +12
    uint32_t aliastblptr;  // +16
    uint32_t lumtblptr;    // +20
    uint8_t  col;          // +24
    uint8_t  imageyshift;  // +25
    uint8_t  pad2;         // +26
    uint8_t  pad3;         // +27
};
#pragma pack(pop)

// Global instance (MASM used: lea edi, [colour_data])
extern "C" {
	colourdata colour_data;
};

#if 0 // RERUN not needed
extern "C" {
	// Some forward declarators
    void XASM_PlainHoriLine1();
    void XASM_GouraudHoriLine1();
    void XASM_PlainHoriLine2();
    void XASM_ImageHoriLine1();
    void XASM_MImageHoriLine1();
    void XASM_SImageHoriLine1();
    void XASM_TFImageHoriline1();
    void XASM_AImageHoriline1();
    void XASM_CImageHoriLine1();

    void XASM_GouraudHoriLine2();
    void XASM_ImageHoriLine2();
    void XASM_MImageHoriLine2();
    void XASM_SImageHoriLine2();
    void XASM_TFImageHoriline2();
    void XASM_AImageHoriline2();
    void XASM_CImageHoriLine2();

    void XASM_GouraudHoriLine4();
    void XASM_ImageHoriLine4();
    void XASM_MImageHoriLine4();
    void XASM_SImageHoriLine4();
    void XASM_TFImageHoriline4();
    void XASM_AImageHoriline4();
    void XASM_CImageHoriLine4();

    void XASM_NullScanLine();

	// horiline_data1
void* horiline_data1[] = {
    (void*)&XASM_PlainHoriLine1,
    (void*)&XASM_GouraudHoriLine1,
    (void*)&XASM_PlainHoriLine1,
    (void*)&XASM_ImageHoriLine1,
    (void*)&XASM_MImageHoriLine1,
    (void*)&XASM_SImageHoriLine1,
    (void*)&XASM_ImageHoriLine1,
    (void*)&XASM_ImageHoriLine1,
    (void*)&XASM_ImageHoriLine1,
    (void*)&XASM_MImageHoriLine1,
    (void*)&XASM_TFImageHoriline1,
    (void*)&XASM_AImageHoriline1,
    (void*)&XASM_CImageHoriLine1
};

// horiline_data2
void* horiline_data2[] = {
    (void*)&XASM_PlainHoriLine1,   // “done”
    (void*)&XASM_GouraudHoriLine2,
    (void*)&XASM_PlainHoriLine2,
    (void*)&XASM_ImageHoriLine2,
    (void*)&XASM_MImageHoriLine2,
    (void*)&XASM_SImageHoriLine2,
    (void*)&XASM_ImageHoriLine2,
    (void*)&XASM_ImageHoriLine2,
    (void*)&XASM_ImageHoriLine2,
    (void*)&XASM_MImageHoriLine2,
    (void*)&XASM_TFImageHoriline2,
    (void*)&XASM_AImageHoriline2,
    (void*)&XASM_CImageHoriLine2
};

// horiline_data4
void* horiline_data4[] = {
    (void*)&XASM_PlainHoriLine1,   // “no point having x4”
    (void*)&XASM_GouraudHoriLine4,
    (void*)&XASM_PlainHoriLine2,   // “no point having x4”
    (void*)&XASM_ImageHoriLine4,
    (void*)&XASM_MImageHoriLine4,
    (void*)&XASM_SImageHoriLine4,
    (void*)&XASM_ImageHoriLine4,
    (void*)&XASM_ImageHoriLine4,
    (void*)&XASM_ImageHoriLine4,
    (void*)&XASM_MImageHoriLine4,
    (void*)&XASM_TFImageHoriline4,
    (void*)&XASM_AImageHoriline4,
    (void*)&XASM_CImageHoriLine4
};

// nullscan_data (single entry)
void* nullscan_data[] = {
    (void*)&XASM_NullScanLine
};

} // extern "C"
#endif // RERUN not needed

inline uint16_t ASM_SetPixelWidth(uint16_t pw) {
    uint16_t old = PixelWidth;
    PixelWidth = pw;
    return old;
}

// Function to return pointer to palette_buffer
inline void* ASM_GetPaletteTable() {
    return static_cast<void*>(palette_buffer);
}

inline void* ASM_GetLandFadeTable()
{
    return LandFadeData;
}

inline void* ASM_GetSkyFadeTable()
{
    return SkyFadeData;
}

inline void* ASM_GetHorizonFadeTable()
{
    return HorizonFadeData;
}

inline void ASM_SelectPalette(SWord palNum) {
    //RERUN: palNum<0 selects the landscape palette; MASM read the table
    //immediately preceding palette_buffer, which no longer maps here.
    const uint16_t* src = (palNum < 0)
        ? landscape_palette
        : &palette_buffer[palNum * 256];
    std::memcpy(palette_table, src, 512);
}

inline UWord ASM_GetPaletteEntry(UWord c) {
	return palette_table[c];
}

inline void ASM_SetPaletteEntry(UWord c,UWord v) {
	palette_table[c] = v;
}

inline void ASM_SetColour(Colour a, SLong b, SLong c, UByteP d)
{
    auto floor_log2 = [](uint32_t n) {
        int s = -1;
        do { n >>= 1; ++s; } while (n);
        return (s < 0 ? 0 : s);
    };

    uint8_t dl = floor_log2((uint32_t)b);
    uint8_t bl = floor_log2((uint32_t)c);

    if (a <= 0xFFu)
        colour_data.col = (uint8_t)a;
    else
        colour_data.alphaptr = a;

    colour_data.imagexmask  = ((1u << dl) - 1u) << 16;
    colour_data.imageyshift = (uint8_t)(16 - dl);
    colour_data.imageymask  = ((1u << bl) - 1u) << 16;
    colour_data.imageptr    = (uint32_t)(uintptr_t)d;
}

inline SWord ASM_GetTransparency()
{
    return Trns3DEnabled;
}

// Now the XASM replacement routines
extern "C" void XASM_PlainHoriLine1(Vertex* right, Vertex* left,
                                    uint16_t* screenBase)
{
    int xRight = right->sx >> 16;
    int xLeft  = left->sx >> 16;
    int width  = (xRight - xLeft) + 1;
    if (width <= 0) return;

    uint16_t* dst = screenBase + xLeft;
	uint16_t colour = palette_table[(unsigned)colour_data.col];
    std::fill_n(dst, width, colour);
}

extern "C" void XASM_GouraudHoriLine1(Vertex* right, Vertex* left,
                                      uint16_t* screenBase)
{
    // Compute integer screen X positions (>>16 for x1 resolution)
    int xRight = right->sx >> 16;
    int xLeft  = left->sx >> 16;
    int width  = (xRight - xLeft) + 1;
    if (width <= 0) return;

    // Starting screen address
    uint16_t* dst = screenBase + xLeft;

    // Intensity setup
    int INTENS    = left->intensity;
    int deltaI    = right->intensity - INTENS;
    int DINTENS   = (width != 0) ? (deltaI / width) : 0;

    // Seed jitter from LandFadeData[(left->sy & 0xFF)]
    const uint16_t* HorizonFade16 = reinterpret_cast<const uint16_t*>(HorizonFadeData);
    const uint16_t* LandFade16    = reinterpret_cast<const uint16_t*>(LandFadeData);
    uint16_t jitter = LandFade16[left->sy & 0xFFu];

    for (int cx = width; cx > 0; --cx, ++dst) {
        // Shade calculation
        int shade = (INTENS + jitter) >> 16;
        if (shade > 0xF) shade = 0xF;
        if (shade < 0)   shade = 0;

        // Lookup horizon fade color
        uint16_t color = HorizonFade16[shade];

        // Write pixel
        *dst = color;

        // Advance intensity
        INTENS += DINTENS;

        // Update jitter: add 0x9136, rotate right 3
        jitter = static_cast<uint16_t>(jitter + 0x9136u);
        jitter = static_cast<uint16_t>((jitter >> 3) | (jitter << (16 - 3)));
    }
}


extern "C" void XASM_ImageHoriLine1(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen x positions
    int rightX = (right->sx + 0x10000) >> 16;
    int leftX  = (left->sx) >> 16;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address
    uint16_t* dst = screenBase + leftX;

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;

    int dy = right->iy - IY;
    bool horizontal = false;
    if (dy == 0) horizontal = true;
    else if (dy >= (int)0xFFFEFF9A && dy <= (int)0x0010066) horizontal = true;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t yshift = colour_data.imageyshift;

    if (!horizontal) {
        int DIY = dy / width;

        // General loop
        for (int cx = width; cx > 0; --cx, ++dst) {
            uint32_t ebx = IX & xmask;
            IX += DIX;

            uint32_t eax = IY & ymask;
            eax >>= yshift;

            int texIndex = (ebx >> 16) + eax;
            uint8_t texel = image[texIndex];

            IY += DIY;

            // Palette lookup
            uint16_t color = palette_table[texel];
            *dst = color;
        }
    } else {
        // Horizontal case
        IY &= ymask;
        IY >>= yshift;
        image += IY;
        int step = DIX;

        for (int cx = width; cx > 0; --cx, ++dst) {
            uint32_t ebx = IX & xmask;
            IX += step;
            ebx >>= 16;

            uint8_t texel = image[ebx];
            uint16_t color = palette_table[texel];
            *dst = color;
        }
    }
}


// RERUN: e.g used for thread indicator and horizontal line of airplane
extern "C" void XASM_MImageHoriLine1(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen x positions
    int rightX = (right->sx + 0x10000) >> 16;
    int leftX  = (left->sx) >> 16;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address
    uint16_t* dst = screenBase + leftX;

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, ++dst) {
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        int texIndex = ebx + eax;
        uint8_t texel = image[texIndex];

        // Transparency check
        if (texel != 0xFE) {
            uint16_t color = palette_table[texel];
            *dst = color;
        }

        // Advance coords
        IX += DIX;
        IY += DIY;
    }
}


// RERUN: This part is involved in the landscape to sky foggy part
extern "C" void XASM_SImageHoriLine1(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Screen X (x1 resolution)
    int rightX = right->sx >> 16;
    int leftX  = left->sx  >> 16;
    int width  = (rightX - leftX) + 1;
    if (width <= 0) return;

    uint16_t* dst = screenBase + leftX;

    // Texture coordinates and per-pixel deltas (signed, not absolute)
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    // Intensity accumulator and delta (signed)
    int INTENS  = left->intensity;
    int DINTENS = (right->intensity - INTENS) / width;

    // Jitter seed from LandFadeData[(left->sy & 0xFF)]
    const uint16_t* LandFade16    = reinterpret_cast<const uint16_t*>(LandFadeData);
    const uint16_t* HorizonFade16 = reinterpret_cast<const uint16_t*>(HorizonFadeData); // not used here, but for reference
    uint16_t jitter = LandFade16[left->sy & 0xFFu];

    // Main loop
    for (int cx = width; cx > 0; --cx, ++dst) {
        // Masked texture coords -> texel index
        uint32_t ebx = static_cast<uint32_t>(IX) & xmask;
        uint32_t eax = static_cast<uint32_t>(IY) & ymask;
        ebx >>= 16;
        eax >>= yshift;

        uint8_t texel = image[ebx + eax];

        // Shade = (INTENS + jitter) >> 16, clamped to 0..15
        int shade = (INTENS + static_cast<int>(jitter)) >> 16;
        if (shade > 0xF) shade = 0xF;
        if (shade < 0)   shade = 0;

        // Fade index and color from LandFadeData (word table)
        uint32_t fadeIndex = (static_cast<uint32_t>(texel) << 4) | static_cast<uint32_t>(shade);
        uint16_t color = LandFade16[fadeIndex];

        *dst = color;

        // Advance accumulators
        IX     += DIX;
        IY     += DIY;
        INTENS += DINTENS;

        // Jitter update: add 0x9136, ror 3
        jitter = static_cast<uint16_t>(jitter + 0x9136u);
        jitter = static_cast<uint16_t>((jitter >> 3) | (jitter << 13));
    }
}

// RERUN: This seems to be involved with higher cloud part?
extern "C" void XASM_TFImageHoriline1(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    int rightX = right->sx >> 16;
    int leftX  = left->sx  >> 16;
    int width  = (rightX - leftX) + 1;
    if (width <= 0) return;

    uint16_t* dst = screenBase + leftX;

    // Transparency cycling seeds
    uint8_t TSHIFT = FUP[leftX & 7];
    uint8_t TOFF   = static_cast<uint8_t>(left->sy) & 0x07;

    // Intensity ramp (signed delta)
    int INTENS  = left->intensity;
    int DINTENS = (right->intensity - INTENS) / width;

    // Texture coordinates and signed deltas
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift= colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, ++dst) {
        // Masked coords
        uint32_t ebx = static_cast<uint32_t>(IX) & xmask;
        uint32_t eax = static_cast<uint32_t>(IY) & ymask;

        IX += DIX;
        IY += DIY;

        ebx >>= 16;
        eax >>= yshift;
        uint8_t texel = image[ebx + eax];

        // Intensity value
        int intensVal = (INTENS >> 14) & 0x78;
        if (intensVal > 0x38) intensVal = 0x38;
        INTENS += DINTENS;

        // Transparency cycling
        uint8_t t = static_cast<uint8_t>(TSHIFT + 1);
        if (t & 0x08) {
            TOFF   = static_cast<uint8_t>((TOFF + 1) & 0x07);
            TSHIFT = 0;
        } else {
            TSHIFT = t;
        }

        // Transparency test
        uint8_t mask = TransparencyData[intensVal];
        mask >>= TSHIFT;
        if (mask & 0x01) {
            uint16_t color = palette_table[texel];
            *dst = color;
        }
    }
}


// RERUN: Don't know this one yet
extern "C" void XASM_AImageHoriline1(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Integer screen X (x1 resolution)
    int rightX = (right->sx + 0x10000) >> 16;
    int leftX  = (left->sx) >> 16;
    int width  = rightX - leftX;
    if (width <= 0) return;

    uint16_t* dst = screenBase + leftX;

    // Transparency cycling
    uint8_t TSHIFT = FUP[leftX & 0x07];
    uint8_t TOFF   = static_cast<uint8_t>(left->sy) & 0x07;

    // Texture coordinates and signed deltas
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    const uint8_t* alpha = reinterpret_cast<const uint8_t*>(colour_data.alphaptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, ++dst) {
        // Masked coords -> texel index
        uint32_t xIndex = static_cast<uint32_t>(IX) & xmask;   xIndex >>= 16;
        uint32_t yIndex = static_cast<uint32_t>(IY) & ymask;   yIndex >>= yshift;
        int texIndex = static_cast<int>(xIndex + yIndex);

        // Advance coords
        IX += DIX;
        IY += DIY;

        // Alpha gating: treat alpha as byte per texel (safer than dword in the port)
        uint8_t alphaByte = alpha ? alpha[texIndex] : 0;
        int intensVal = (alphaByte >> 2) & 0x38;  // matches ASM bucketization

        // Cycle TSHIFT/TOFF
        uint8_t t = static_cast<uint8_t>(TSHIFT + 1);
        if (t & 0x08) {
            TOFF   = static_cast<uint8_t>((TOFF + 1) & 0x07);
            TSHIFT = 0;
        } else {
            TSHIFT = t;
        }

        // Transparency test
        uint8_t gate = TransparencyData[static_cast<uint32_t>(intensVal)];
        gate >>= TSHIFT;
        if (gate & 0x01) {
            uint8_t texel = image[texIndex];
            uint16_t color = palette_table[texel];
            *dst = color;
        }
        // else: transparent, skip write
    }
}


// RERUN: Clouds in the sky use this
extern "C" void XASM_CImageHoriLine1(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Integer screen X (x1 resolution)
    int rightX = right->sx >> 16;
    int leftX  = left->sx  >> 16;
    int width  = (rightX - leftX) + 1;
    if (width <= 0) return;

    uint16_t* dst = screenBase + leftX;

    // Texture coords and signed deltas
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    // Intensity accumulator and delta (signed)
    int INTENS  = left->intensity;
    int DINTENS = (right->intensity - INTENS) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    const uint16_t* SkyFade16 = reinterpret_cast<const uint16_t*>(SkyFadeData);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    // Jitter seed (ASM seeds from a fade table; for CImage1 use sky)
    uint16_t jitter = SkyFade16[left->sy & 0xFFu];

    for (int cx = width; cx > 0; --cx, ++dst) {
        // Masked coords -> texel index
        uint32_t xIndex = static_cast<uint32_t>(IX) & xmask;   xIndex >>= 16;
        uint32_t yIndex = static_cast<uint32_t>(IY) & ymask;   yIndex >>= yshift;

        uint8_t texel = image[xIndex + yIndex];

        // Shade = (INTENS + jitter) >> 16, clamp 0..15
        int shade = (INTENS + static_cast<int>(jitter)) >> 16;
        if (shade > 0xF) shade = 0xF;
        if (shade < 0)   shade = 0;

        // Sky fade lookup: (texel<<4)|shade
        uint16_t color = SkyFade16[(static_cast<uint32_t>(texel) << 4) | static_cast<uint32_t>(shade)];
        *dst = color;

        // Advance accumulators
        IX     += DIX;
        IY     += DIY;
        INTENS += DINTENS;

        // Jitter update: add 0x9136, ror 3
        jitter = static_cast<uint16_t>(jitter + 0x9136u);
        jitter = static_cast<uint16_t>((jitter >> 3) | (jitter << 13));
    }
}


extern "C" void XASM_PlainHoriLine2(Vertex* right, Vertex* left, uint16_t* screenBase) {
    printf("XASM_PlainHoriLine2 not yet implemented!\n");
}
extern "C" void XASM_GouraudHoriLine2(Vertex* right, Vertex* left, uint16_t* screenBase){
    printf("XASM_GouraudHoriLine2 not yet implemented!\n");
}
extern "C" void XASM_ImageHoriLine2(Vertex* right, Vertex* left, uint16_t* screenBase) {
    printf("XASM_ImageHoriLine2 not yet implemented!\n");
}
extern "C" void XASM_MImageHoriLine2(Vertex* right, Vertex* left, uint16_t* screenBase) {
    printf("XASM_MImageHoriLine2 not yet implemented!\n");
}
extern "C" void XASM_SImageHoriLine2(Vertex* right, Vertex* left, uint16_t* screenBase) {
    printf("XASM_SImageHoriLine2 not yet implemented!\n");
}
extern "C" void XASM_TFImageHoriline2(Vertex* right, Vertex* left, uint16_t* screenBase){
    printf("XASM_TFImageHoriline2 not yet implemented!\n");
}
extern "C" void XASM_AImageHoriline2(Vertex* right, Vertex* left, uint16_t* screenBase){
    printf("XASM_AImageHoriline2 not yet implemented!\n");
}
extern "C" void XASM_CImageHoriLine2(Vertex* right, Vertex* left, uint16_t* screenBase) {
    printf("XASM_CImageHoriLine2 not yet implemented!\n");
}

extern "C" void XASM_GouraudHoriLine4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen x positions with the ASM’s 18-bit shift
    // rightX = (right->sx + 0x40000) >> 18
	if (reinterpret_cast<uintptr_t>(right) < 0x1000) {return;}

    int32_t rightX = (right->sx + 0x40000) >> 18;
    // leftX = left->sx >> 18
    int32_t leftX  = (left->sx) >> 18;

    // Line width (inclusive): (rightX - leftX) + 1
    int32_t width = (rightX - leftX) + 1;
    if (width <= 0) return;

    // Starting byte address: screenBase + leftX * 8 bytes
    // The ASM multiplies leftX by 8 (add edi,edi x3) and uses 16-bit stores at offsets 0,2,4,6.
    uint8_t* dstBytes = reinterpret_cast<uint8_t*>(screenBase);
    dstBytes += static_cast<int64_t>(leftX) * 8;

    // Intensity ramp: DINTENS = (right->intensity - left->intensity) / width
    // (ASM sequence yields this signed division result)
    int32_t INTENS   = left->intensity;
    int32_t DINTENS  = (width != 0) ? ((right->intensity - left->intensity) / width) : 0;

    // Seed JITTER from LandFadeData[(left->sy & 0xFF)] as a 16-bit word
    const uint16_t* landFade16    = reinterpret_cast<const uint16_t*>(LandFadeData);
    const uint16_t* horizonFade16 = reinterpret_cast<const uint16_t*>(HorizonFadeData);
    uint32_t jitterSeedIndex      = static_cast<uint32_t>(left->sy) & 0xFFu;
    uint16_t jitter               = landFade16[jitterSeedIndex];
    JITTER[0]                     = jitter;

    // The ASM adjusts the pointer by -8 bytes before entering the loop
    dstBytes -= 8;

    // Main loop
    for (int32_t cx = width; cx > 0; --cx) {
        // Advance destination by 8 bytes at the top (add edi,8)
        dstBytes += 8;

        //RERUN: bound writes to the locked surface (original asm relied on
        //clipped verts; off-screen edges still overrun on the SDL heap buffer)
        if (xasm_surface_end && (dstBytes+8 > xasm_surface_end || dstBytes < xasm_surface_start))
            break;

        // Shade index from (INTENS + JITTER) >> 16, clamped to 0x0..0xF
        int32_t shade = INTENS + static_cast<int32_t>(jitter);
        shade >>= 16;
        if (shade > 0xF) shade = 0xF;
        if (shade < 0)   shade = 0;

        // Update jitter: add 0x9136, then rotate right by 3 bits
        jitter = static_cast<uint16_t>(jitter + 0x9136u);
        jitter = static_cast<uint16_t>((jitter >> 3) | (jitter << (16 - 3)));

        // Fetch horizon fade color (16-entry table, 16-bit words)
        uint16_t color = horizonFade16[static_cast<uint32_t>(shade)];

        // Write four 16-bit words at offsets 0,2,4,6
        uint16_t* dst16 = reinterpret_cast<uint16_t*>(dstBytes);
        dst16[0] = color;
        dst16[1] = color;
        dst16[2] = color;
        dst16[3] = color;

        // Advance intensity accumulator
        INTENS += DINTENS;

        // Store jitter back (MASM writes it each iteration)
        JITTER[0] = jitter;
    }
}


extern "C" void XASM_ImageHoriLine4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen X positions (>>18 for 4-pixel resolution)
    int rightX = (right->sx + 0x40000) >> 18; // add 0x40000 then >>18
    int leftX  = (left->sx) >> 18;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address (4 pixels per step)
    uint16_t* dst = screenBase + (leftX << 2);

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, dst += 4) {
        //RERUN: bound writes to the locked surface
        if (xasm_surface_end && ((uint8_t*)(dst+4) > xasm_surface_end || (uint8_t*)dst < xasm_surface_start))
            break;
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        // Texel fetch
        uint8_t texel = image[ebx + eax];

        // Palette lookup (word table, index*2)
        uint16_t color = palette_table[texel * 2];

        // Write 4 pixels
        dst[0] = color;
        dst[1] = color;
        dst[2] = color;
        dst[3] = color;

        // Advance texture coords
        IX += DIX;
        IY += DIY;
    }
}

extern "C" void XASM_MImageHoriLine4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen X positions (>>18 for 4-pixel resolution)
    int rightX = (right->sx + 0x40000) >> 18; // add 0x40000 then >>18
    int leftX  = (left->sx) >> 18;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address (4 pixels per step)
    uint16_t* dst = screenBase + (leftX << 2);

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, dst += 4) {
        //RERUN: bound writes to the locked surface
        if (xasm_surface_end && ((uint8_t*)(dst+4) > xasm_surface_end || (uint8_t*)dst < xasm_surface_start))
            break;
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        // Texel fetch
        uint8_t texel = image[ebx + eax];

        // Transparency check (0xFE = transparent)
        if (texel != 0xFE) {
            // Palette lookup (word table, index*2)
            uint16_t color = palette_table[texel * 2];

            // Write 4 pixels
            dst[0] = color;
            dst[1] = color;
            dst[2] = color;
            dst[3] = color;
        }
        // else: skip write (transparent)

        // Advance texture coords
        IX += DIX;
        IY += DIY;
    }
}

extern "C" void XASM_SImageHoriLine4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen X positions (>>18 for 4-pixel resolution)
    int rightX = (right->sx + 0x40000) >> 18; // add 0x40000 then >>18
    int leftX  = (left->sx) >> 18;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address (4 pixels per step)
    uint16_t* dst = screenBase + (leftX << 2);

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    // Intensity setup
    int INTENS    = left->intensity;
    int deltaI    = right->intensity - INTENS;
    int DINTENS   = (width != 0) ? (deltaI / width) : 0;

    // Seed jitter from LandFadeData[(left->sy & 0xFF)]
    const uint16_t* LandFade16 = reinterpret_cast<const uint16_t*>(LandFadeData);
    uint16_t seed = LandFade16[left->sy & 0xFFu];
    uint16_t jitter = seed;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, dst += 4) {
        //RERUN: bound writes to the locked surface
        if (xasm_surface_end && ((uint8_t*)(dst+4) > xasm_surface_end || (uint8_t*)dst < xasm_surface_start))
            break;
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        // Texel fetch
        uint8_t texel = image[ebx + eax];

        // Shade calculation
        int shade = (INTENS + jitter) >> 16;
        if (shade > 0xF) shade = 0xF;
        if (shade < 0)   shade = 0;

        // Fade index: (texel<<4)|shade
        uint32_t fadeIndex = ((uint32_t)texel << 4) | (uint32_t)shade;
        uint16_t color = LandFade16[fadeIndex];

        // Write 4 pixels
        dst[0] = color;
        dst[1] = color;
        dst[2] = color;
        dst[3] = color;

        // Advance coords
        IX += DIX;
        IY += DIY;

        // Advance intensity
        INTENS += DINTENS;

        // Update jitter: add 0x9136, rotate right 3
        jitter = static_cast<uint16_t>(jitter + 0x9136u);
        jitter = static_cast<uint16_t>((jitter >> 3) | (jitter << (16 - 3)));
    }
}

extern "C" void XASM_TFImageHoriline4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen X positions (>>18 for 4-pixel resolution)
    int rightX = (right->sx + 0x40000) >> 18; // add 0x40000 then >>18
    int leftX  = (left->sx) >> 18;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address (4 pixels per step)
    uint16_t* dst = screenBase + (leftX << 2);

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    // Intensity setup
    int INTENS    = left->intensity;
    int deltaI    = right->intensity - INTENS;
    if (deltaI < 0) deltaI = -deltaI;
    int DINTENS   = (width != 0) ? (deltaI / width) : 0;

    // Transparency cycling
    uint8_t TSHIFT = FUP[leftX & 7];
    uint8_t TOFF   = static_cast<uint8_t>(left->sy) & 0x07;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, dst += 4) {
        //RERUN: bound writes to the locked surface
        if (xasm_surface_end && ((uint8_t*)(dst+4) > xasm_surface_end || (uint8_t*)dst < xasm_surface_start))
            break;
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        // Texel fetch
        uint8_t texel = image[ebx + eax];

        // Intensity value
        int intensVal = (INTENS >> 14) & 0x78;
        if (intensVal > 0x38) intensVal = 0x38;

        // Advance intensity
        INTENS += DINTENS;

        // Cycle TSHIFT/TOFF
        uint8_t t = static_cast<uint8_t>(TSHIFT + 1);
        if (t & 0x08) {
            TOFF   = static_cast<uint8_t>((TOFF + 1) & 0x07);
            TSHIFT = 0;
        } else {
            TSHIFT = t;
        }

        // Transparency test
        uint8_t mask = TransparencyData[intensVal];
        mask >>= TSHIFT;

        if ((mask & 0x01) != 0) {
            // Palette lookup (word table, index*2)
            uint16_t color = palette_table[texel * 2];

            // Write 4 pixels
            dst[0] = color;
            dst[1] = color;
            dst[2] = color;
            dst[3] = color;
        }
        // else: skip write (transparent)

        // Advance coords
        IX += DIX;
        IY += DIY;
    }
}

extern "C" void XASM_AImageHoriline4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen X positions (>>18 for 4-pixel resolution)
    int rightX = (right->sx + 0x40000) >> 18; // add 0x40000 then >>18
    int leftX  = (left->sx) >> 18;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address (4 pixels per step)
    uint16_t* dst = screenBase + (leftX << 2);

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    // Transparency cycling
    uint8_t TSHIFT = FUP[leftX & 7];
    uint8_t TOFF   = static_cast<uint8_t>(left->sy) & 0x07;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    const uint32_t* alpha = reinterpret_cast<const uint32_t*>(colour_data.alphaptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, dst += 4) {
        //RERUN: bound writes to the locked surface
        if (xasm_surface_end && ((uint8_t*)(dst+4) > xasm_surface_end || (uint8_t*)dst < xasm_surface_start))
            break;
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        int texIndex = ebx + eax;

        // Texel fetch
        uint8_t texel = image[texIndex];

        // Alpha fetch (32-bit word at alphaptr+texIndex)
        uint32_t alphaWord = alpha[texIndex];
        int intensVal = (alphaWord >> 2) & 0x38; // shr by 2, mask 0x38
        if (intensVal > 0x38) intensVal = 0x38;

        // Cycle TSHIFT/TOFF
        uint8_t t = static_cast<uint8_t>(TSHIFT + 1);
        if (t & 0x08) {
            TOFF   = static_cast<uint8_t>((TOFF + 1) & 0x07);
            TSHIFT = 0;
        } else {
            TSHIFT = t;
        }

        // Transparency test
        uint8_t mask = TransparencyData[intensVal];
        mask >>= TSHIFT;

        if ((mask & 0x01) != 0) {
            // Palette lookup (word table, index*2)
            uint16_t color = palette_table[texel * 2];

            // Write 4 pixels
            dst[0] = color;
            dst[1] = color;
            dst[2] = color;
            dst[3] = color;
        }
        // else: skip write (transparent)

        // Advance coords
        IX += DIX;
        IY += DIY;
    }
}

extern "C" void XASM_CImageHoriLine4(Vertex* right, Vertex* left, uint16_t* screenBase)
{
    // Compute integer screen X positions (>>18 for 4-pixel resolution)
    int rightX = (right->sx + 0x40000) >> 18; // add 0x40000 then >>18
    int leftX  = (left->sx) >> 18;
    int width  = rightX - leftX;
    if (width <= 0) return;

    // Starting screen address (4 pixels per step)
    uint16_t* dst = screenBase + (leftX << 2);

    // Texture coordinates
    int IX  = left->ix;
    int IY  = left->iy;
    int DIX = (right->ix - IX) / width;
    int DIY = (right->iy - IY) / width;

    const uint8_t* image = reinterpret_cast<const uint8_t*>(colour_data.imageptr);
    const uint8_t* alias = reinterpret_cast<const uint8_t*>(colour_data.aliastblptr);
    uint32_t xmask = colour_data.imagexmask;
    uint32_t ymask = colour_data.imageymask;
    uint8_t  yshift = colour_data.imageyshift;

    for (int cx = width; cx > 0; --cx, dst += 4) {
        //RERUN: bound writes to the locked surface
        if (xasm_surface_end && ((uint8_t*)(dst+4) > xasm_surface_end || (uint8_t*)dst < xasm_surface_start))
            break;
        // Masked texture coords
        uint32_t ebx = IX & xmask;
        uint32_t eax = IY & ymask;
        ebx >>= 16;
        eax >>= yshift;

        int texIndex = ebx + eax;

        // Texel fetch
        uint8_t texel = image[texIndex];

        // Alias remap
        texel = alias[texel];

        // Palette lookup (word table, index*2)
        uint16_t color = palette_table[texel * 2];

        // Write 4 pixels
        dst[0] = color;
        dst[1] = color;
        dst[2] = color;
        dst[3] = color;

        // Advance coords
        IX += DIX;
        IY += DIY;
    }
}


extern "C" void XASM_NullScanLine(Vertex*, Vertex*, uint16_t*) {
    printf("XASM_NullScanLine not yet implemented!\n");
}
///////// TODO

// RERUN, now in 3DDEFS.H using HoriLineFunc = void(*)(Vertex* right, Vertex* left, uint16_t* screenBase);

// 1-byte-per-pixel table
extern "C" {
	HoriLineFunc horiline_data1[] = {
		XASM_PlainHoriLine1,
		XASM_GouraudHoriLine1,
		XASM_PlainHoriLine1,
		XASM_ImageHoriLine1,
		XASM_MImageHoriLine1,
		XASM_SImageHoriLine1,
		XASM_ImageHoriLine1,
		XASM_ImageHoriLine1,
		XASM_ImageHoriLine1,
		XASM_MImageHoriLine1,
		XASM_TFImageHoriline1,
		XASM_AImageHoriline1,
		XASM_CImageHoriLine1
	};
}; // extern C

// 2-byte-per-pixel table
extern "C" {
	HoriLineFunc horiline_data2[] = {
		XASM_PlainHoriLine2,
		XASM_GouraudHoriLine2,
		XASM_PlainHoriLine2,
		XASM_ImageHoriLine2,
		XASM_MImageHoriLine2,
		XASM_SImageHoriLine2,
		XASM_ImageHoriLine2,
		XASM_ImageHoriLine2,
		XASM_ImageHoriLine2,
		XASM_MImageHoriLine2,
		XASM_TFImageHoriline2,
		XASM_AImageHoriline2,
		XASM_CImageHoriLine2
	};
}; // extern C

// 4-byte-per-pixel table
extern "C" { 
	HoriLineFunc horiline_data4[] = {
		XASM_PlainHoriLine1, /* No point in x4 */
		XASM_GouraudHoriLine4,
		XASM_PlainHoriLine2, /* No point in x4 */
		XASM_ImageHoriLine4,
		XASM_MImageHoriLine4,
		XASM_SImageHoriLine4,
		XASM_ImageHoriLine4,
		XASM_ImageHoriLine4,
		XASM_ImageHoriLine4,
		XASM_MImageHoriLine4,
		XASM_TFImageHoriline4,
		XASM_AImageHoriline4,
		XASM_CImageHoriLine4
	};
};

// MASM-compatible export name
extern "C" {
	HoriLineFunc ASM_HoriLineAddr(unsigned int lineType) asm("_XASM_HoriLineAddr");
};

extern "C" {
	HoriLineFunc ASM_HoriLineAddr(unsigned int lineType)
	{
		HoriLineFunc* table;

		if (PixelWidth == 1)
			table = horiline_data1;
		else if (PixelWidth == 2)
			table = horiline_data2;
		else
			table = horiline_data4;

		return table[lineType];
	}
};
#endif //RERUN end GCC PORT of GRAFPASM.ASM

//------------------------------------------------------------------------------
//Procedure	GrafPrim::ClearScreen
//------------------------------------------------------------------------------
//Author		JIM
//Date			Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
void Graphic::ClearScreen()
{
	short	wx1,wy1;
	short	w,h;

	wx1 = 	PhysicalMinX;
	w = 	PhysicalWidth;
	wy1 = 	PhysicalMinY;
	h = 	PhysicalHeight;

	PlainRectangleFill(wx1,wy1,w,h);			//PD 29Nov95

}

//------------------------------------------------------------------------------
//Procedure		SetPixelWidth
//Author		Paul.   
//Date			Wed 3 Jan 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetPixelWidth(SWord &pixwid)
{
	pixwid = ASM_SetPixelWidth(pixwid);
}

void* Graphic::GetPaletteTable()
{
	void* pal=ASM_GetPaletteTable();
	return pal;
}

void* Graphic::GetLandFadeTable()
{
	void* ptable=ASM_GetLandFadeTable();
	return ptable;
}

void* Graphic::GetSkyFadeTable()
{
	void *ptable=ASM_GetSkyFadeTable();
	return ptable;
}

void* Graphic::GetHorizonFadeTable()
{
	void* ptable=ASM_GetHorizonFadeTable();
	return ptable;
}

void Graphic::SelectPalette(SWord palNum)
{
	ASM_SelectPalette(palNum);
}

UWord Graphic::GetPaletteEntry(UWord c)
{
	return ASM_GetPaletteEntry(c);
}

void Graphic::SetPaletteEntry(UWord c,UWord v)
{
	ASM_SetPaletteEntry(c,v);
}

//------------------------------------------------------------------------------
//Procedure		SetTransparency
//Author		Paul.   
//Date			Tue 2 Apr 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetTransparency(SWord flag)
{
}

//------------------------------------------------------------------------------
//Procedure		SetLuminosity
//Author		Robert Slater
//Date			Mon 19 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetLuminosity(SWord flag)							//RJS 19Jan98
{																//RJS 19Jan98
}																//RJS 19Jan98

//------------------------------------------------------------------------------
//Procedure		SetHeatHaze
//Author		Robert Slater
//Date			Fri 30 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetHeatHaze(SWord flag)
{
}

//------------------------------------------------------------------------------
//Procedure		SetColour
//Author		Paul.   
//Date			Wed 29 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetColour(Colour Col)
{
	colour = Col;

	//Identify which colour range we are in and set 'Range' to be 
	//the full range

	//The first sixteen colours are fixed single colours
	//Followed by one 16 colour range
	//Then one 32 colour range
	//Then ten 16 colour ranges
	//Then the explosion range
	//Then two masking colours
	//Then a single off-white colour

	if(Col<16 || Col>=FONTMASK)
	{
		range = 1;
	}
	else
	{
		range	= SPREAD00;
	}

	ASM_SetColour(colour,4,4,imageptr);					//RJS 09Feb98
}

//------------------------------------------------------------------------------
//Procedure	SetColour
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 14 Aug 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Graphic::SetColour(Colour Col,ColourRange CRng)
{
	if(CRng==-1)
	{
		//The first sixteen colours are fixed single colours
		//Followed by one 16 colour range
		//Then one 32 colour range
		//Then ten 16 colour ranges
		//Then the explosion range
		//Then two masking colours
		//Then a single off-white colour
	
		if(Col<16 || Col>=FONTMASK)
		{
			range = 1;
		}
		else
		{
			range = SPREAD00 - (Col&(SPREAD00-1));
		}
	}
	else
	{
		range 	= CRng;
	}
	colour 	= Col;

	ASM_SetColour(colour,4,4,imageptr);					//RJS 09Feb98
}

//------------------------------------------------------------------------------
//Procedure		SetGlassRange
//Author		Paul.   
//Date			Mon 1 Apr 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetGlassRange(Colour clr)
{
	glasscolour = clr;
}

//------------------------------------------------------------------------------
//Procedure		SetColour
//Author		Paul.   
//Date			Mon 30 Oct 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetColour(ImageMapDescPtr image)
{
	SWord	image_width,										//PD 05Dec95
			image_height;										//PD 05Dec95

	image_width = image->w-1;									//RJS 06Feb98
	image_height= image->h-1;									//RJS Fe

	image_x_mask	= ((SLong )image_width)<<16;				//PD 05Dec95
	image_y_mask	= ((SLong )image_height)<<16;				//PD 05Dec95

	imageptr = image->body;										//RJS 06Feb98

	Colour c=(Colour)(ULong)image->alpha;

	ASM_SetColour(c,image->w,image->h,imageptr);	//PD 21Dec95
}

void Graphic::SetColour(ROWANSURFACEDESC* rsd)
{
	SWord image_width,image_height;
	image_width=rsd->dwWidth-1;
	image_height=rsd->dwHeight-1;
	image_x_mask=SLong(image_width)<<16;
	image_y_mask=SLong(image_height)<<16;
	imageptr=(UByte*)rsd->lpSurface;
	ASM_SetColour(colour,image_width+1,image_height+1,imageptr);
}


//------------------------------------------------------------------------------
//Procedure		CirclePoints
//Author		Martin Alderton
//Date			Thu 21 Sep 1995
//
//Description	Dead Code.
//				
//Returns		
//
//Inputs			
//
//------------------------------------------------------------------------------
void Graphic::CirclePoints(SWord x, SWord y, SWord radius)
{	
}


//------------------------------------------------------------------------------
//Procedure	grafprim::PlainCircle
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	Martin 29 Nov 1995.
//
//Description Circle drawing routine using integer maths.	
//			  rewritten but untested. Problems with circles look here first.
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Graphic::PlainCircle(SWord radius, SWord x, SWord y)
{
	short x1,y1,x2,y2,x3,y3,x4,y4,d;
	SWord dist,sin_ang,cos_ang;

	Math_Lib.sin_cos(ANGLES_45Deg,sin_ang,cos_ang);

	dist = radius*cos_ang;

	x1=x3=x;
	y1=y+radius;
	y2=y4=y;
	x2=x+radius;
	y3=y-radius;
	x4=x-radius;
	d=1-radius;
	PlotPixel(x1,y1);
	PlotPixel(x2,y2);
	PlotPixel(x3,y3);
	PlotPixel(x4,y4);

	while (y1>y)
	{
		if (y1>=(y+dist))
		{
			if (d<0)
			{
				d+=x*2+3;
				x1++;
				y2--;
				x3--;
				y4++;
			}
			else
			{
				d+=(x-y)*2+5;
				x1++;
				y2--;
				x3--;
				y4++;
				y1--;
				x2--;
				y3++;
				x4++;
			}
		}
		else
		{
			if (d<0)
			{
				d+=x*2+3;
				y1--;
				x2--;
				y3++;
				x4++;
			}
			else
			{
				d+=(x-y)*2+5;
				y1--;
				x2--;
				y3++;
				x4++;
				x1++;
				y2--;
				x3--;
				y4++;
			}

		}
		PlotPixel(x1,y1);
		PlotPixel(x2,y2);
		PlotPixel(x3,y3);
		PlotPixel(x4,y4);
	}

}

//------------------------------------------------------------------------------
//Procedure	GrafPrim::PlainLine
//------------------------------------------------------------------------------
//Author	Paul & Martin   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Graphic::PlainLine(SWord x1,SWord y1,SWord x2,SWord y2)
{
	SWord		transp = ASM_GetTransparency();

	{
		ROWANSURFACEDESC rsd;
		Master()->DoLockScr(&rsd);

		UWord*		scradr;

		SLong 		distx,	disty,
					x,		y,
					const1,	const2,
					p;

		UByte		linecol = colour;


		distx = x2 - x1;

		disty = y2 - y1;

		if (distx < 0)	distx = -distx;

		if (disty < 0)	disty = -disty;

		//Vertical case

		if (distx == 0)
		{
			y = (y2>y1)?y1:y2;

			x = x1;

			scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

			*scradr = linecol;	// PlotPixel(x,y);

			for (;disty--;scradr=(UWord*)(ULong(scradr)+rsd.lPitch),y++)
				*scradr = linecol;	//	PlotPixel(x,y);

			Master()->DoUnlockScr(&rsd);
			return;
		}

		//Horizontal case

		if (disty==0)
		{
			x = (x2>x1)?x1:x2;

			y = y1;

			scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

			*scradr = linecol;	// PlotPixel(x,y);

			for (;distx--;scradr++,x++)	*scradr = linecol;	// PlotPixel(x,y);

			Master()->DoUnlockScr(&rsd);
			return;
		}

		if (distx>disty)
		{
			const1 = disty + disty;

			const2 = const1 - distx - distx;

			p = const1 - distx;

			if (x1>x2)
			{
				x = x2; y = y2;
			}
			else
			{
				x = x1; y = y1;
			}

			scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

			*scradr = linecol;	// PlotPixel(x,y);

			if ((x==x1 && y2>y1) || (x==x2 && y1>y2))
			{
				for (;distx--;)
				{
					x++;

					scradr++;

					if (p<0)
						p += const1;
					else
					{
						y++;

						scradr=(UWord*)(ULong(scradr)+rsd.lPitch);

						p += const2;
					}

					*scradr = linecol;	// PlotPixel(x,y);
				}
				Master()->DoUnlockScr(&rsd);
				return;
			}
			else
			{
				for (;distx--;)
				{
					x++;

					scradr++;

					if (p<0)
						p += const1;
					else
					{
						y--;

						scradr=(UWord*)(ULong(scradr)-rsd.lPitch);

						p += const2;
					}

					*scradr = linecol;	// PlotPixel(x,y);

				}
				Master()->DoUnlockScr(&rsd);
				return;
			}
		}

		if (disty>distx)
		{
			const1 = distx + distx;

			const2 = const1 - disty - disty;

			p = const1 - disty;

			if (y1>y2)
	 		{
	 			x = x2; y = y2;
	 		}
			else
	 		{
	 			x = x1; y = y1;
	 		}

			scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

			*scradr = linecol;	// PlotPixel(x,y);
		
			if ((y==y1 && x2>x1) || (y==y2 && x1>x2))
 			{
 				for (;disty--;)
 				{
 					y++;

					scradr=(UWord*)(ULong(scradr)+rsd.lPitch);

	 				if (p<0)
	 					p += const1;
	 				else
	 				{
	 					x++;

						scradr++;

	 					p += const2;
	 				}

	 				*scradr = linecol;	// PlotPixel(x,y);
	 			}
				Master()->DoUnlockScr(&rsd);
				return;
	 		}
			else
	 		{
	 			for (;disty--;)
	 			{
	 				y++;

					scradr=(UWord*)(ULong(scradr)+rsd.lPitch);

	 				if (p<0)
	 					p += const1;
	 				else
	 				{
	 					x--;

						scradr--;

	 					p += const2;
	 				}

	 				*scradr = linecol;	// PlotPixel(x,y);
	 			}
				Master()->DoUnlockScr(&rsd);
				return;
	 		}
		}

		//If we get here distx == disty

		if (x1>x2)
		{
			x = x2; y = y2;
		}
		else
		{
	 		x = x1; y = y1;
		}

		scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

		*scradr = linecol;	// PlotPixel(x,y);

		if ((x==x1 && y2>y1) || (x==x2 && y1>y2))
		{
			for (;distx--;)
			{
				x++; y++;

				scradr=(UWord*)(ULong(scradr)+rsd.lPitch+2);

				*scradr = linecol;	// PlotPixel(x,y);
			}
		}
		else 
		{
			for (;distx--;)
			{
				x++; y--;

				scradr=(UWord*)(ULong(scradr)-rsd.lPitch+2);

				*scradr = linecol;	// PlotPixel(x,y);
			}
		}
		Master()->DoUnlockScr(&rsd);
	}
}

//------------------------------------------------------------------------------
//Procedure		PlainLine16
//Author		Paul.   
//Date			Wed 28 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
extern UWord polyRedBits,polyRedShift,polyGreenBits,polyGreenShift,polyBlueBits,polyBlueShift;

void Graphic::PlainLine16(SWord x1,SWord y1,SWord x2,SWord y2)
{
	UWord*	scradr;
	ROWANSURFACEDESC rsd;
	Master()->DoLockScr(&rsd);

	SLong 		distx,	disty,
				x,		y,
				const1,	const2,
				p;

	UWord		red,green,blue;

	typedef	struct rgb { UByte r,g,b; }	*rgbptr;

	rgbptr	lpPal = (rgbptr)Palette;

	red = lpPal[colour].r; green = lpPal[colour].g; blue = lpPal[colour].b;

	UWord
	linecol =	(red>>(8-polyRedBits))		<< polyRedShift;			//RDH 16Oct97
	linecol += 	(green>>(8-polyGreenBits))	<< polyGreenShift;			//RDH 16Oct97
	linecol += 	(blue>>(8-polyBlueBits))	<< polyBlueShift;			//RDH 16Oct97

	distx = x2 - x1;

	disty = y2 - y1;

	if (distx < 0)	distx = -distx;

	if (disty < 0)	disty = -disty;

	//Vertical case

	if (distx == 0)
	{
		y = (y2>y1)?y1:y2;

		x = x1;

		scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

		*(UWord*)scradr = linecol;					// PlotPixel(x,y);

		for (;disty--;scradr=(UWord*)(ULong(scradr)+rsd.lPitch),y++)
			*(UWord*)scradr = linecol;				//	PlotPixel(x,y);

		Master()->DoUnlockScr(&rsd);
		return;
	}

	//Horizontal case

	if (disty==0)
	{
		x = (x2>x1)?x1:x2;

		y = y1;

		scradr=(UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch) + (x<<1));

		*(UWord*)scradr = linecol;					// PlotPixel(x,y);

		for (;distx--;scradr++,x++)
			*(UWord*)scradr = linecol;				// PlotPixel(x,y);

		Master()->DoUnlockScr(&rsd);
		return;
	}

	if (distx>disty)
	{
		const1 = disty + disty;

		const2 = const1 - distx - distx;

		p = const1 - distx;

		if (x1>x2)
		{
			x = x2; y = y2;
		}
		else
		{
			x = x1; y = y1;
		}

		scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

		*(UWord*)scradr = linecol;					// PlotPixel(x,y);

		if ((x==x1 && y2>y1) || (x==x2 && y1>y2))
		{
			for (;distx--;)
			{
				x++;

				scradr++;

				if (p<0)
					p += const1;
				else
				{
					y++;

					scradr=(UWord*)(ULong(scradr)+rsd.lPitch);

					p += const2;
				}

				*(UWord*)scradr = linecol;			// PlotPixel(x,y);
			}
			Master()->DoUnlockScr(&rsd);
			return;
		}
		else
		{
			for (;distx--;)
			{
				x++;

				scradr++;

				if (p<0)
					p += const1;
				else
				{
					y--;

					scradr=(UWord*)(ULong(scradr)-rsd.lPitch);

					p += const2;
				}

				*(UWord*)scradr = linecol;				// PlotPixel(x,y);

			}
			Master()->DoUnlockScr(&rsd);
			return;
		}
	}

	if (disty>distx)
	{
		const1 = distx + distx;

		const2 = const1 - disty - disty;

		p = const1 - disty;

		if (y1>y2)
	 	{
	 		x = x2; y = y2;
	 	}
		else
	 	{
	 		x = x1; y = y1;
	 	}

		scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

		*(UWord*)scradr = linecol;			// PlotPixel(x,y);
		
		if ((y==y1 && x2>x1) || (y==y2 && x1>x2))
 		{
 			for (;disty--;)
 			{
 				y++;

				scradr=(UWord*)(ULong(scradr)+rsd.lPitch);

	 			if (p<0)
	 				p += const1;
	 			else
	 			{
	 				x++;

					scradr++;

	 				p += const2;
	 			}

	 			*(UWord*)scradr = linecol;			// PlotPixel(x,y);
	 		}
			Master()->DoUnlockScr(&rsd);
			return;
	 	}
		else
	 	{
	 		for (;disty--;)
	 		{
	 			y++;

				scradr=(UWord*)(ULong(scradr)+rsd.lPitch);

	 			if (p<0)
	 				p += const1;
	 			else
	 			{
	 				x--;

					scradr--;

	 				p += const2;
	 			}

	 			*(UWord*)scradr = linecol;				// PlotPixel(x,y);
	 		}
			Master()->DoUnlockScr(&rsd);
			return;
	 	}
	}

	//If we get here distx == disty

	if (x1>x2)
	{
		x = x2; y = y2;
	}
	else
	{
	 	x = x1; y = y1;
	}

	scradr = (UWord*)(ULong(rsd.lpSurface)+(y*rsd.lPitch)+(x<<1));

	*(UWord*)scradr = linecol;							// PlotPixel(x,y);

	if ((x==x1 && y2>y1) || (x==x2 && y1>y2))
	{
		for (;distx--;)
		{
			x++; y++;

			scradr=(UWord*)(ULong(scradr)+rsd.lPitch+2);

			*(UWord*)scradr = linecol;					// PlotPixel(x,y);
		}
	}
	else 
	{
		for (;distx--;)
		{
			x++; y--;

			scradr=(UWord*)(ULong(scradr)-rsd.lPitch+2);

			*(UWord*)scradr = linecol;					// PlotPixel(x,y);
		}
	}
	Master()->DoUnlockScr(&rsd);
}

//------------------------------------------------------------------------------
//Procedure		PlainLineTransparent
//Author		Robert Slater
//Date			Tue 13 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::PlainLineTransparent(SWord x1,SWord y1,SWord x2,SWord y2, SWord depth)
{	
}
//------------------------------------------------------------------------------
//Procedure		CompOutCode
//Author		Martin Alderton
//Date			Wed 20 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
OutCode Graphic::CompOutCode(SWord x,SWord y)
{
	short XMin,XMax,YMin,YMax;
	OutCode code;
	code.top=0,code.bottom=0,code.right=0,code.left=0,code.all=0;
	XMin = PhysicalMinX;
	XMax = PhysicalMaxX;	// + PhysicalWidth;					//RDH 06Nov96
	YMin = PhysicalMinY;
	YMax = PhysicalMaxY;	// + PhysicalHeight;				//RDH 06Nov96

	if (y>YMax)
		{
			code.top=8;
			code.all +=code.top;
		}
	else if (y<YMin)
		{
			code.bottom=4;
			code.all +=code.bottom;
		}
	if (x>XMax)
		{
			code.right=2;
			code.all +=code.right;
		}
	else if (x<XMin)
		{
			code.left=1;
			code.all +=code.left;
		}
	return code;
}
	


//------------------------------------------------------------------------------
//Procedure		ClipLine
//Author		Martin Alderton
//Date			Wed 20 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::ClipLine(SWord x1, SWord y1, SWord x2, SWord y2)
{
	SWord done,accept,x,y,XMin,XMax,YMin,YMax;
	outcode outcode1, outcode2, outcodeOut,ClipCode;


	XMin = PhysicalMinX;
	XMax = PhysicalMaxX; //+ PhysicalWidth;						//RDH 06Nov96
	YMin = PhysicalMinY;
	YMax = PhysicalMaxY; //+ PhysicalHeight;					//RDH 06Nov96
	
	done=0;
	accept=0;
	outcode1=CompOutCode(x1,y1);
	outcode2=CompOutCode(x2,y2);
	do
		{
			if (outcode1.all==0 && outcode2.all==0)
				{
					done=1;
					accept=1;
				}
			else if ((outcode1.all & outcode2.all) !=0)
				{
					done =1;
				}
			else
				{
					if(outcode1.all != 0)
						ClipCode=outcodeOut=outcode1;
					else
						ClipCode=outcodeOut=outcode2;
					if (outcodeOut.top)
						{
							x=x1+(((x2-x1)*(YMax-y1))/(y2-y1));
							y=YMax;
						}
					else if(outcodeOut.bottom)
						{
							x=x1+(((x2-x1)*(YMin-y1))/(y2-y1));
							y=YMin;
						}
					else if(outcodeOut.right)
						{
							y=y1+(((y2-y1)*(XMax-x1))/(x2-x1));
							x=XMax;
						}
					else if(outcodeOut.left)
						{
							y=y1+(((y2-y1)*(XMin-x1))/(x2-x1));
							x=XMin;
						}
					if (outcodeOut.all==outcode1.all)
						{
							x1=x;
							y1=y;
							outcode1=CompOutCode(x1,y1);
						}
					else
						{
							x2=x;
							y2=y;
							outcode2=CompOutCode(x2,y2);
						}
				}
		}
	while (done !=1);

	if (accept==1)
		{
		if (BytesPerPixel==1)		PlainLine(x1,y1,x2,y2);
		else if (BytesPerPixel==2)	PlainLine16(x1,y1,x2,y2);
		}
}

//------------------------------------------------------------------------------
//Procedure		CompOutCode
//Author		Martin Alderton
//Date			Wed 20 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
OutCode Graphic::Comp3DOutCode(SLong x,SLong y)
{
	SLong XMin,XMax,YMin,YMax;
	OutCode code;
	code.top=0,code.bottom=0,code.right=0,code.left=0,code.all=0;
	XMin = 0;
	XMax = PhysicalWidth;
	YMin = 0;
	YMax = PhysicalHeight;

	if (y>YMax)
		{
			code.top=8;
			code.all +=code.top;
		}
	else if (y<YMin)
		{
			code.bottom=4;
			code.all +=code.bottom;
		}
	if (x>XMax)
		{
			code.right=2;
			code.all +=code.right;
		}
	else if (x<XMin)
		{
			code.left=1;
			code.all +=code.left;
		}
	return code;
}

//------------------------------------------------------------------------------
//Procedure		ClipLine
//Author		Martin Alderton
//Date			Wed 20 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
enum	OutCodes
{
	oc_NULL		= 0x0000,
	oc_Top		= 0x0001,
	oc_Bottom	= 0x0002,
	oc_Left		= 0x0004,
	oc_Right	= 0x0008
};

OutCodes operator|=(OutCodes oc,OutCodes oc2) {return (OutCodes)((int)oc|(int)oc2);}

void Graphic::ClipLine3D(SLong x1, SLong y1, SLong x2, SLong y2,SWord Xscale,SWord Yscale)
{

#define COMPOCODE(x,y,oc)\
{\
	if ((x)<PhysicalMinX)		oc=oc_Left;\
	else if ((x)>PhysicalMaxX)	oc=oc_Right;\
	else						oc=oc_NULL;\
	\
	if ((y)<PhysicalMinY)		oc|=oc_Top;\
	else if ((y)>PhysicalMaxY)	oc|=oc_Bottom;\
}

#define COMPFCODE(x,y,oc)\
{\
	if ((x)<XMin)		oc=oc_Left;\
	else if ((x)>XMax)	oc=oc_Right;\
	else						oc=oc_NULL;\
	\
	if ((y)<YMin)		oc|=oc_Top;\
	else if ((y)>YMax)	oc|=oc_Bottom;\
}

#define	CLIPPER(inval,outval,clipval,otherin,otherout)\
{\
	Float	frac;\
	frac = ((clipval)-(inval))/((outval)-(inval));\
	(outval)=(clipval);\
	(otherout)=(otherin) + frac * ((otherout)-(otherin));\
}

	OutCodes	oc1,oc2;

	COMPOCODE(x1,y1,oc1);

	COMPOCODE(x2,y2,oc2);

	//Test for trivial accept/reject

	if ((oc1 & oc2)!=oc_NULL)	return;		//REJECT!

	if ((oc1 | oc2)==oc_NULL)
	{
	 	//ACCEPT!

		if (BytesPerPixel==1)
			PlainLine((SWord)x1,(SWord)y1,(SWord)x2,(SWord)y2);
		else if (BytesPerPixel==2)
			PlainLine16((SWord)x1,(SWord)y1,(SWord)x2,(SWord)y2);
	}
	else
	{
		//CLIP!!
	
		Float	XMin,	XMax,	YMin,	YMax;

		Float	fx1,	fy1,	fx2,	fy2;

		XMin = PhysicalMinX;	XMax = PhysicalMaxX;
		YMin = PhysicalMinY;	YMax = PhysicalMaxY;

		fx1 = x1;	fy1 = y1;
		fx2 = x2;	fy2 = y2;

		//Clip to top

		if ((oc1 & oc_Top)!=oc_NULL)
		{
			CLIPPER(fy2,fy1,YMin,fx2,fx1);

			COMPFCODE(fx1,fy1,oc1);
		}
		else if ((oc2 & oc_Top)!=oc_NULL)
		{
			CLIPPER(fy1,fy2,YMin,fx1,fx2);

			COMPFCODE(fx2,fy2,oc2);
		}

		if ((oc1 & oc2)!=oc_NULL)	return;		//REJECT!

		//Clip to bottom

		if ((oc1 & oc_Bottom)!=oc_NULL)
		{
			CLIPPER(fy2,fy1,YMax,fx2,fx1);

			COMPFCODE(fx1,fy1,oc1);
		}
		else if ((oc2 & oc_Bottom)!=oc_NULL)
		{
			CLIPPER(fy1,fy2,YMax,fx1,fx2);

			COMPFCODE(fx2,fy2,oc2);
		}

		if ((oc1 & oc2)!=oc_NULL)	return;		//REJECT!

		//Clip to left

		if ((oc1 & oc_Left)!=oc_NULL)
		{
			CLIPPER(fx2,fx1,XMin,fy2,fy1);

			COMPFCODE(fx1,fy1,oc1);
		}
		else if ((oc2 & oc_Left)!=oc_NULL)
		{
			CLIPPER(fx1,fx2,XMin,fy1,fy2);

			COMPFCODE(fx2,fy2,oc2);
		}

		if ((oc1 & oc2)!=oc_NULL)	return;		//REJECT!

		//Clip to right

		if ((oc1 & oc_Right)!=oc_NULL)
		{
			CLIPPER(fx2,fx1,XMax,fy2,fy1);

			COMPFCODE(fx1,fy1,oc1);
		}
		else if ((oc2 & oc_Right)!=oc_NULL)
		{
			CLIPPER(fx1,fx2,XMax,fy1,fy2);

			COMPFCODE(fx2,fy2,oc2);
		}

		if ((oc1 & oc2)!=oc_NULL)	return;		//REJECT!

		x1 = fx1; y1 = fy1;
		x2 = fx2; y2 = fy2;

		if (BytesPerPixel==1)
			PlainLine((SWord)x1,(SWord)y1,(SWord)x2,(SWord)y2);
		else if (BytesPerPixel==2)
			PlainLine16((SWord)x1,(SWord)y1,(SWord)x2,(SWord)y2);
	}
}

//------------------------------------------------------------------------------
//Procedure		ClipLine3D
//Author		Robert Slater
//Date			Tue 25 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::ClipLine3D(Float X1, Float Y1, Float X2, Float Y2,
							SWord	Xscale, SWord	Yscale)
{
	SLong	x1,y1,x2,y2;
	SLong x,y,XMin,XMax,YMin,YMax;
	outcode outcode1, outcode2, outcodeOut,ClipCode;
	Bool  done,accept;

	x1 = X1 / (Float)Xscale; //RERUN, reenabled the division with Xscale
	x2 = X2 / (Float)Xscale; //RERUN, reenabled the division with Xscale
	y1 = Y1 / (Float)Yscale; //RERUN, reenabled the division with Yscale
	y2 = Y2 / (Float)Yscale; //RERUN, reenabled the division with Yscale

	XMin = 0;	//PhysicalMinX;
	XMax = PhysicalWidth;						//RDH 06Nov96
	YMin = 0;	//PhysicalMinY;
	YMax = PhysicalHeight;					//RDH 06Nov96
	
	done=FALSE;
	accept=FALSE;
	outcode1=Comp3DOutCode(x1,y1);
	outcode2=Comp3DOutCode(x2,y2);
	do
	{
		if (outcode1.all==0 && outcode2.all==0)
		{
			done=TRUE;
			accept=TRUE;
		}
		else if ((outcode1.all & outcode2.all) !=0)
			{
				done =TRUE;
			}
		else
			{
				if(outcode1.all != 0)
					ClipCode=outcodeOut=outcode1;
				else
					ClipCode=outcodeOut=outcode2;

				if (outcodeOut.top)
				{
					x=x1+(((x2-x1)*(YMax-y1))/(y2-y1));
					y=YMax;
				}
				else if(outcodeOut.bottom)
					{
						x=x1+(((x2-x1)*(YMin-y1))/(y2-y1));
						y=YMin;
					}
				else if(outcodeOut.right)
					{
						y = y1 + (((y2 - y1)*(XMax - x1)) / (x2 - x1));
						x=XMax;
					}
				else if(outcodeOut.left)
					{
						y=y1+(((y2-y1)*(XMin-x1))/(x2-x1));
						x=XMin;
					}
				if (outcodeOut.all==outcode1.all)
				{
					x1=x;
					y1=y;
					outcode1=CompOutCode(x1,y1);
				}
				else
				{
					x2=x;
					y2=y;
					outcode2=CompOutCode(x2,y2);
				}
			}
	}
	while (!done);

	if (accept)
	{
 		PlainLine16((SWord)x1,(SWord)y1,(SWord)x2,(SWord)y2);
	}
}


//------------------------------------------------------------------------------
//Procedure	GrafPrim::RegisterPalette
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
void Graphic::RegisterPalette(UByte *src, SLong length)
{
	int		i;
	UByte	*dest;

	if(Palette == NULL)
		Palette = new PAL;

	dest = (UByte *)Palette;

	while(--length!=0)
	{
		if (*dest++ != *src++)
		{
			*(dest-1) = *(src-1);
			while (--length!=0)
				*dest++ = *src++;
			UseNewPal = TRUE;
			return;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ReinitDisplay
//Author		Paul.   
//Date			Thu 3 Jul 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::ReinitDisplay()
{
	UseNewPal=TRUE;
}


//------------------------------------------------------------------------------
//Procedure	GrafPrim::PlainCircleFill
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Graphic::PlainCircleFill()
{

}

//------------------------------------------------------------------------------
//Procedure	GrafPrim::PlainRectangleFill
//LastModified:	PD 20Dec95
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Mon 3 Jul 1995
//Modified		Put Jims optimised version back in and made alterations to
//				fit in with his other graphics library edits. 
//				New version is nothing like the old one!!!!!!!
//
//Description	
//
//Inputs	
//
//Returns	clip result
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UWord Graphic::PlainRectangleFill(SWord X,SWord Y,SWord W,SWord H)
{	
	LogicalPtr	scradr,
				scanlineaddr;

	int		i,j;

	UWord	clipvar = 0;

	//Clip the rectangle to the screen

	if((Y+H) <= PhysicalMinY)
		clipvar |= 0x0003;

	if(Y >= PhysicalMinY+PhysicalHeight)
		clipvar |= 0x0003;
	
	if((X+W) <= PhysicalMinX)
		clipvar |= 0x000C;

	if(X >= PhysicalMinX+PhysicalWidth)
		clipvar |= 0x000C;

	if(clipvar)
		return(clipvar);

	if(Y < PhysicalMinY)
	{
		H -=	(PhysicalMinY-Y);
		Y = 	PhysicalMinY;
		clipvar |= 0x0001;
	}

	if((Y+H) > (PhysicalMinY+PhysicalHeight))
	{
		H -=	((Y+H)-(PhysicalMinY+PhysicalHeight));
		clipvar |= 0x0002;
	}

	if(X < PhysicalMinX)
	{
		W -= (PhysicalMinX-X);
		X = 	PhysicalMinX;
		clipvar |= 0x0004;
	}

	if((X+W) > (PhysicalMinX+PhysicalWidth))
	{
		W -= ((X+W)-(PhysicalMinX+PhysicalWidth));
		clipvar |= 0x0008;
	}

	//Calculate the screen address of the top left corner of the rectangle

	scradr = logicalscreenptr +
			 BytesPerScanLine * Y +
			 BytesPerPixel * X;

	ULong	lcolour=((ULong)0x01010101)* ((UByte) colour);

//DEAD 	Master()->LockBackScreen (TRUE,LOWLOCK);			//AMM 02May97

	for(i=H;i>0;i--)
	{
		scanlineaddr = scradr;
		j=W-3;
		if	((ULong )scanlineaddr &0x00000003)
		{
			*(char*) (scanlineaddr++) = colour;
			j--;
			if	((ULong )scanlineaddr &0x00000003)
			{
				*(char*) (scanlineaddr++) = colour;
				j--;
				if	((ULong )scanlineaddr &0x00000003)
				{
					*(char*) (scanlineaddr++) = colour;
					j--;
		}	}	}
		scradr += BytesPerScanLine;
		while(j>0)
		{
			*(ULong *)scanlineaddr = lcolour;
			scanlineaddr+=4;
			j-=4;
		}
		for(j=j+3;j>0;j--)
			*(char*) (scanlineaddr++) = colour;
	}

//DEAD 	Master()->UnLockBackScreen (LOWLOCK);			//AMM 02May97

	return(clipvar);
}

//------------------------------------------------------------------------------
//Procedure	GrafPrim::GlassRectangleFill
//LastModified:	PD 02Apr96
//------------------------------------------------------------------------------
//Author		Paul.   
//Date			Tue 2 April 1996
//
//Description	
//
//Inputs	
//
//Returns	clip result
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UWord Graphic::GlassRectangleFill(SWord X,SWord Y,SWord W,SWord H)
{
	LogicalPtr	scradr,
				scanlineaddr;

	int		i,j;

	UWord	clipvar = 0;

	//Clip the rectangle to the screen

	if((Y+H) <= PhysicalMinY)
		clipvar |= 0x0003;

	if(Y >= PhysicalMinY+PhysicalHeight)
		clipvar |= 0x0003;
	
	if((X+W) <= PhysicalMinX)
		clipvar |= 0x000C;

	if(X >= PhysicalMinX+PhysicalWidth)
		clipvar |= 0x000C;

	if(clipvar)
		return(clipvar);

	if(Y < PhysicalMinY)
	{
		H -=	(PhysicalMinY-Y);
		Y = 	PhysicalMinY;
		clipvar |= 0x0001;
	}

	if((Y+H) > (PhysicalMinY+PhysicalHeight))
	{
		H -=	((Y+H)-(PhysicalMinY+PhysicalHeight));
		clipvar |= 0x0002;
	}

	if(X < PhysicalMinX)
	{
		W -= (PhysicalMinX-X);
		X = 	PhysicalMinX;
		clipvar |= 0x0004;
	}

	if((X+W) > (PhysicalMinX+PhysicalWidth))
	{
		W -= ((X+W)-(PhysicalMinX+PhysicalWidth));
		clipvar |= 0x0008;
	}	

	//Calculate the screen address of the top left corner of the rectangle

	scradr = logicalscreenptr +
			 BytesPerScanLine * Y +
			 BytesPerPixel * X;

	UByte	rngmin,rngmax,val;

//DEAD 	Master()->LockBackScreen (TRUE,LOWLOCK);			//AMM 02May97

	rngmin = glasscolour;
	rngmax = glasscolour + range;

	for(i=H;i>0;i--)
	{
		scanlineaddr = scradr;

		for (j=W;j>0;j--)
		{
			val = *(UByte *)scanlineaddr;

			if (val>= rngmin && val<rngmax)						//RDH 14Aug96
			{
				val = val - glasscolour + colour;
				*(UByte *) (scanlineaddr++) = val;
			}
			else
				scanlineaddr++;
		}
		scradr += BytesPerScanLine;
	}

//DEAD 	Master()->UnLockBackScreen (LOWLOCK);			//AMM 02May97

	return(clipvar);
}

//------------------------------------------------------------------------------
//Procedure		AilRectangleFill
//Author		Paul.   
//Date			Fri 21 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UWord Graphic::AilRectangleFill(SWord X,SWord Y,SWord W,SWord H)
{
	LogicalPtr	scradr,
				scanlineaddr;

	int		i,j;

	UWord	clipvar = 0;

	//Clip the rectangle to the screen

	if((Y+H) <= PhysicalMinY)
		clipvar |= 0x0003;

	if(Y >= PhysicalMinY+PhysicalHeight)
		clipvar |= 0x0003;
	
	if((X+W) <= PhysicalMinX)
		clipvar |= 0x000C;

	if(X >= PhysicalMinX+PhysicalWidth)
		clipvar |= 0x000C;

	if(clipvar)
		return(clipvar);

	if(Y < PhysicalMinY)
	{
		H -=	(PhysicalMinY-Y);
		Y = 	PhysicalMinY;
		clipvar |= 0x0001;
	}

	if((Y+H) > (PhysicalMinY+PhysicalHeight))
	{
		H -=	((Y+H)-(PhysicalMinY+PhysicalHeight));
		clipvar |= 0x0002;
	}

	if(X < PhysicalMinX)
	{
		W -= (PhysicalMinX-X);
		X = 	PhysicalMinX;
		clipvar |= 0x0004;
	}

	if((X+W) > (PhysicalMinX+PhysicalWidth))
	{
		W -= ((X+W)-(PhysicalMinX+PhysicalWidth));
		clipvar |= 0x0008;
	}	

	//Calculate the screen address of the top left corner of the rectangle

	scradr = logicalscreenptr +
			 BytesPerScanLine * Y;

	VERTEX	lv,rv;

//DEAD 	Master()->LockBackScreen (TRUE,LOWLOCK);			//AMM 02May97

	lv.sx.i = X; rv.sx.i = X + W -1;

	for(i=H;i>0;i--)
	{
// Bug found in this function - 11 Jul 96
//		ASM_SmokedGlassLine((UByte *)scradr, lv, rv);
		scradr += BytesPerScanLine;
	}

//DEAD 	Master()->UnLockBackScreen (LOWLOCK);					//AMM 02May97

	return(clipvar);
}

//------------------------------------------------------------------------------
//Procedure	GrafPrim::PlainPolygonFill
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Graphic::PlainPolygonFill()
{

}

//------------------------------------------------------------------------------
//Procedure	Graphic::TranslateColour
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ULong Graphic::TranslateColour(Colour C)
{
	return(C);
}

//------------------------------------------------------------------------------
//Procedure	Graphic::PlotPixel
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 5 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Colour Graphic::PlotPixel(SWord X, SWord Y,Colour colour)
{
	SWord		MinX,MaxX,MinY,MaxY;
	Colour		ret_val;
	LogicalPtr	screenaddr;
	SLong		yoffset,xoffset;

	MinX = PhysicalMinX;
	MaxX = MinX + PhysicalWidth;//-1							//JIM 09Apr96

	MinY = PhysicalMinY;
	MaxY = MinY + PhysicalHeight;//-1							//JIM 09Apr96

	if(X>=MinX && X<MaxX && Y>=MinY && Y<MaxY)				//PD 28Jul97
	{
		yoffset = BytesPerScanLine * Y;
		xoffset = BytesPerPixel * X;
		screenaddr = logicalscreenptr + xoffset + yoffset;

		if (BytesPerPixel<2)
		{
			ret_val = (Colour)(*screenaddr);
			*screenaddr = colour;
		}
		else
		if (BytesPerPixel==2)
		{
			UWord t=*(UWord*)screenaddr;
			ret_val=(Colour)((t&0xff00)>>8);
			t=(t&0xffff00ff)+(colour<<8);
			*(UWord*)screenaddr=t;
		}
		else
		{
			ULong t=*(ULong*)screenaddr;
			ret_val=(Colour)((t&0xff00)>>8);
			t=(t&0xffff00ff)+(colour<<8);
			*(ULong*)screenaddr=t;
		}
	}

	return(ret_val);
}

//------------------------------------------------------------------------------
//Procedure	Graphic::PlotPixel
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 5 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Colour Graphic::PlotPixel(SWord X, SWord Y)
{
	SWord		MinX,MaxX,MinY,MaxY;
	Colour		ret_val;
	LogicalPtr	screenaddr;
	SLong		yoffset,xoffset;

	MinX = PhysicalMinX;
	MaxX = MinX + PhysicalWidth;//-1;							//PD 28Jul97

	MinY = PhysicalMinY;
	MaxY = MinY + PhysicalHeight;//-1;							//PD 28Jul97

	if(X>=MinX && X<MaxX && Y>=MinY && Y<MaxY)					//PD 07Nov95
	{
//DEAD 		Master()->LockBackScreen (TRUE,LOWLOCK);			//AMM 02May97

		yoffset = BytesPerScanLine * Y;
		xoffset = BytesPerPixel * X;
		screenaddr = logicalscreenptr + xoffset + yoffset;
		if (BytesPerPixel<2)
		{
			ret_val = (Colour)(*screenaddr);
			*screenaddr = colour;
		}
		else
		if (BytesPerPixel==2)
		{
			UWord t=*(UWord*)screenaddr;
			ret_val=(Colour)((t&0xff00)>>8);
			t=(t&0xffff00ff)+(colour<<8);
			*(UWord*)screenaddr=t;
		}
		else
		{
			ULong t=*(ULong*)screenaddr;
			ret_val=(Colour)((t&0xff00)>>8);
			t=(t&0xffff00ff)+(colour<<8);
			*(ULong*)screenaddr=t;
		}

//DEAD 		Master()->UnLockBackScreen (LOWLOCK);				//AMM 02May97
	}

	return(ret_val);
}

//------------------------------------------------------------------------------
//Procedure	Graphic::PlotPixelNoClip
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 12 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void	Graphic::PlotPixelNoClip(short X, short Y, short colour)
{
	long	scradr;

	scradr = ((long )BytesPerScanLine * Y) + ((long )BytesPerPixel * X) + ((long )logicalscreenptr);
	if (BytesPerPixel<2)
		*(char *)scradr = colour;
	else
	if (BytesPerPixel==2)
	{
		UWord t= *(UWord *)scradr;
		t=(t&0x00ff)+(colour<<8);
		*(UWord *)scradr=t;
	}
	else
	{
		ULong t= *(ULong *)scradr;
		t=(t&0x00ff00ff)+(colour<<8);
		*(ULong *)scradr=t;
	}
}

//------------------------------------------------------------------------------
//Procedure	Graphic::PlotPixelNoClip
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 12 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void	Graphic::PlotPixelNoClip(short X, short Y)
{
	long	scradr;

	scradr = ((long )BytesPerScanLine * Y) + ((long )BytesPerPixel * X) + ((long )logicalscreenptr);
	if (BytesPerPixel<2)
		*(char *)scradr = colour;
	else
	if (BytesPerPixel==2)
	{
#ifdef __WATCOMC__
		INT3();
#endif
#ifdef __MSVC__
		INT3;
#else
		assert(false);
#endif
		UWord t= *(UWord *)scradr;
		t=(t&0x00ff)+(colour<<8);
		*(UWord *)scradr=t;
	}
	else
	{
		ULong t= *(ULong *)scradr;
		t=(t&0x00ff00ff)+(colour<<8);
		*(ULong *)scradr=t;
	}
}


//------------------------------------------------------------------------------
//Procedure	GrafPrim.PrintMonoBMP
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Tue 8 Aug 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
short Graphic::PrintMonoBMP(CharTableEntryPtr)
{
	return(0);
}

//------------------------------------------------------------------------------
//Procedure	GrafPrim.PrintMonoRUN
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Tue 8 Aug 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
short Graphic::PrintMonoRUN(CharTableEntryPtr)
{
	return(0);
}

//------------------------------------------------------------------------------
//Procedure		GetScanLineAddr
//Author		Paul.   
//Date			Wed 3 Jan 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
HoriLineRtnPtr Graphic::GetScanLineAddr(SCANLINETYPE type)
{
	return(ASM_HoriLineAddr(type));
}

//------------------------------------------------------------------------------
//Procedure		SetGrey3DType
//Author		Paul.   
//Date			Sun 7 Jan 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetGrey3DType(GREY3DTYPE &grey3dtype)
{
//	grey3dtype=ASM_SetGrey3D(grey3dtype);
}

//------------------------------------------------------------------------------
//Procedure		SetFadeData
//Author		Paul.   
//Date			Mon 24 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetFadeData(SLong ftdt)
{
//	ASM_SetFadeData(ftdt);
}

//------------------------------------------------------------------------------
//Procedure		SetFaded3DType
//Author		Paul.   
//Date			Fri 21 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::SetFaded3DType(GREY3DTYPE &grey3dtype)
{
}


//------------------------------------------------------------------------------
//Procedure		MakeIntensityData
//Author		Paul.   
//Date			Wed 8 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::MakeIntensityData()
{
	int	i,j;

	SLong	range,base,delta;

	for(i=0;i<16;i++)
	{
		range = (16-i)<<8;

		base = 0;

		delta = range/32;

		for(j=0;j<32;j++)
		{
			intensityindex[i][j]=base>>8;
			base += delta;
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		CompZOutCode
//Author		Martin Alderton
//Date			Tue 21 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ZOutCode Graphic::CompZOutCode(SLong z,SLong ZMin)
{
	ZOutCode code;
	code.behind=code.all=0;										//RJS 13Jun97
																//RJS 13Jun97
	if (z<ZMin)													//RJS 13Jun97
	{															//RJS 13Jun97
		code.behind=1;											//RJS 13Jun97
		code.all +=code.behind;									//RJS 13Jun97
	}															//RJS 13Jun97
	return code;
}

//------------------------------------------------------------------------------
//Procedure		zclipline
//Author		Martin Alderton
//Date			Mon 20 Nov 1995
//
//Description	takes in two dopointstrucs works out where (if anywhere) they
//				cross the zclipping planes and adjusts the bodyx and bodyy
//				coordinates of each point as required. Hey Presto a line
//				clipped in the z plane ready to go to ClipLine function
//
//Inputs		2* dopointstruc
//
//Returns		2 * adjusted DoPointStruc	
//
//------------------------------------------------------------------------------
void Graphic::zclipline(DoPointStruc &start_vertex,DoPointStruc &end_vertex)
{
	Bool	accept, done;
 	Float	z,x,y,x1,x0,y0,y1,z0,z1;
 	SLong	ZMin;												//RJS 25Feb97
 	ZOutCode outcode1, outcode2, outcodeOut;
 
 	x0 = start_vertex.bodyx.f;									//RJS 25Feb97
 	x1 = end_vertex.bodyx.f;									//RJS 25Feb97
 	y0 = start_vertex.bodyy.f;									//RJS 25Feb97
 	y1 = end_vertex.bodyy.f;									//RJS 25Feb97
 	z0 = start_vertex.bodyz.f;									//RJS 25Feb97
 	z1 = end_vertex.bodyz.f;									//RJS 25Feb97
 
   	ZMin = 100;													//RJS 25Jun97
 		 
 	done=FALSE;													//RJS 25Feb97
 	accept=FALSE;												//RJS 25Feb97
 	outcode1=CompZOutCode(z0,ZMin);
 	outcode2=CompZOutCode(z1,ZMin);
 	do
 	{
 		if (!outcode1.all && !outcode2.all)
 		{
 			done=TRUE;									//RJS 25Feb97
 			accept=TRUE;								//RJS 25Feb97
 		}
 		else if ((outcode1.all & outcode2.all) !=0)
 			{
 				done =TRUE;									//RJS 25Feb97
 			}
 		else
 		{
 			if(outcode1.all)									//RJS 13Jun97
 			{
 				outcodeOut=outcode1;
 				z=z0;
 				x=x0;
 				y=y0;
 			}
 			else
 			{
 				outcodeOut=outcode2;
 				z=z1;
 				x=x1;
 				y=y1;
 			}
 			if (outcodeOut.behind)
 			{
 				x= x0 + (((ZMin-z0)*(x1-x0))/(z1-z0));
 				y= y0 + (((ZMin-z0)*(y1-y0))/(z1-z0));
 				z=ZMin;
 			}
 			if (outcodeOut.all==outcode1.all)
 			{
 				z0=z;
 				x0=x;
 				y0=y;
 				outcode1=CompZOutCode(z,ZMin);
 			}
 			else
 			{
 				z1=z;
 				x1=x;
 				y1=y;
 				outcode2=CompZOutCode(z,ZMin);
 			}
 		}
 	}
 	while (!done);												//RJS 25Feb97
 
 	if (accept)													//RJS 25Feb97
 	{
 		start_vertex.bodyx.f = x0;
 		end_vertex.bodyx.f = x1;
 		start_vertex.bodyy.f = y0;
 		end_vertex.bodyy.f = y1;
 		start_vertex.bodyz.f = z0;
 		end_vertex.bodyz.f = z1;
 	}
}

//------------------------------------------------------------------------------
//Procedure		FillPalFxTable
//Author		Paul.   
//Date			Fri 5 Jan 1996
//
//Description	Uses the current palette to set up the colour tables
//				used for palette indirection in the 'grey' 3D horizontal
//				line draw routines.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::FillPalFxTable()
{
}

//------------------------------------------------------------------------------
//Procedure		TransparentBlob
//Author		Paul.   
//Date			Mon 28 Jul 1997
//
//Description	
//
//Inputs		clipInfo =  /|\
//						   |_|_| .... 0
//							___
//						   | | | .... 1
//						    \|/
//						   
//						   |-\  
//						   |--	 .... 2
//						   |-/
//
//						    /-| 
//							--|	 .... 3
//							\-|
//
//Returns	
//
//------------------------------------------------------------------------------

#define CIRCLE_POINTS()\
		FuzzyLine2(CentXLocal,CentYLocal,var_ax,-var_si,radius,intens);\
		FuzzyLine2(CentXLocal,CentYLocal,var_ax,var_si,radius,intens);\
		FuzzyLine2(CentXLocal,CentYLocal,var_si,-var_ax,radius,intens);\
		FuzzyLine2(CentXLocal,CentYLocal,var_si,var_ax,radius,intens);

void Graphic::TransparentBlob(	SWord clipInfo,
								SWord intens,
								SWord radius,
								SWord x,
								SWord y)
{
	SWord	var_ax,var_si,var_dx,var_bp;

	SWord	CentXLocal,CentYLocal;

	if (intens > 255)
		intens = 255;

	CentXLocal = x;
	CentYLocal = y;

	var_si = 0;
	var_ax = var_dx = radius;

	var_dx = 3;
	var_dx -= var_ax<<1;

loopC:

	CIRCLE_POINTS();

	if (var_dx>=0)	goto Over;

	var_dx += 6;

	var_bp = var_si<<2;

	var_dx += var_bp;

	var_si++;

	if (var_si<var_ax)	goto loopC;

	if (var_si!=var_ax)	goto Out;
	
	CIRCLE_POINTS();

	goto Out;

Over:
	var_bp = var_si;

	var_bp -= var_ax;

	var_bp >>= 2;

	var_dx += var_bp;

	var_dx += 10;

	var_ax--;

	var_si++;

	if (var_si<var_ax) goto loopC;

	if (var_si!=var_ax)	goto Out;

	CIRCLE_POINTS();
Out:

	return;
}

#undef	CIRCLE_POINTS					//PD 13Nov97

//------------------------------------------------------------------------------
//Procedure		FuzzyLine
//Author		Paul.   
//Date			Mon 28 Jul 1997
//
//Description	Draws a horizontal line from (cx-dx,cy+dy) to
//				(cx+dx,cy+dy) transparent at endpoints but
//				of opacity cInt in the center
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::FuzzyLine(SWord cx,SWord cy,SWord dx,SWord dy,SWord rad,SWord cInt)
{
	SWord		MinX,MaxX,MinY,MaxY;

	MinY = PhysicalMinY;
	MaxY = MinY + PhysicalHeight;

	// Y-clipping

	if (cy+dy<MinY || cy+dy>=MaxY || dx==0)	return;

	MinX = PhysicalMinX;
	MaxX = MinX + PhysicalWidth;

	LogicalPtr	scradr;

	SLong		xoffset,yoffset;

	//use dx,dy & cInt to generate an intensity
	//value at either end of the line and at it's
	//center point.

	dx = (dx<0)?-dx:dx;

	//y first

	SWord	abs_dy = dy<0?-dy:dy;

	abs_dy <<= 8;

	abs_dy /= rad;

	abs_dy = 256 - abs_dy;

	//abs_dy == 256 when dy == +/- radius
	//abs_dy == 0   when dy == 0

	SWord	dInt,rInt,lInt;

	dInt = abs_dy/dx;

	rInt = 0;

	yoffset = BytesPerScanLine * (cy+dy);
	xoffset = BytesPerPixel * (cx-dx+1);

	scradr = logicalscreenptr + xoffset + yoffset;
	
	int	tmpx = cx-dx+1;

	int	cnt;

	UWord	*wscradr;

	switch (BytesPerPixel)
	{
		case 1:

		lInt = rInt;

		for (cnt=1;cnt<=dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				scradr++;
			else
				*scradr++ = GREY00 + (lInt>>4);

			rInt+=dInt;

			lInt = rInt;

			if (lInt>0xFF)	lInt=0xFF;
		}

		for (cnt=0;cnt<dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				scradr++;
			else
				*scradr++ = GREY00 + (lInt>>4);

			rInt-=dInt;

			lInt = rInt;

			if (lInt<0)	lInt=0;
		}
		break;

		case 2:

		rInt = 0xFF - rInt;

		dInt =- dInt;

		wscradr = (UWord*)scradr;

		lInt = rInt;

		for (cnt=1;cnt<=dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				wscradr++;
			else
			{
				UWord	srccol,newcol;

				ULong	tmp;

				srccol = *wscradr;

				tmp = srccol & 0x001F; tmp *= lInt; tmp >>= 8;

				newcol = (UWord)tmp;

				tmp = srccol & 0x07E0; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0x07E0);

				tmp = srccol & 0xF800; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0xF800);

				*wscradr++ = newcol;
			}

			rInt+=dInt;

			lInt = rInt;

			if (lInt<0)	lInt=0;
		}

		for (cnt=0;cnt<dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				wscradr++;
			else
			{
				UWord	srccol,newcol;
				
				ULong	tmp;

				srccol = *wscradr;

				tmp = srccol & 0x001F; tmp *= lInt; tmp >>= 8;

				newcol = (UWord)tmp;

				tmp = srccol & 0x07E0; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0x07E0);

				tmp = srccol & 0xF800; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0xF800);

				*wscradr++ = newcol;
			}

			rInt-=dInt;

			lInt = rInt;

			if (lInt>0xFF)	lInt=0xFF;
		}
		break;

		default:
		break;
	}

}

//------------------------------------------------------------------------------
//Procedure		FuzzyLine2
//Author		Robert Slater
//Date			Thu 6 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Graphic::FuzzyLine2(SWord cx,SWord cy,SWord dx,SWord dy,SWord rad,SWord cInt)
{
	SWord		MinX,MaxX,MinY,MaxY;

	MinY = PhysicalMinY;
	MaxY = MinY + PhysicalHeight;

	// Y-clipping

	if (cy+dy<MinY || cy+dy>=MaxY || dx==0)	return;

	MinX = PhysicalMinX;
	MaxX = MinX + PhysicalWidth;

	LogicalPtr	scradr;

	SLong		xoffset,yoffset;

	//use dx,dy & cInt to generate an intensity
	//value at either end of the line and at it's
	//center point.

	dx = (dx<0)?-dx:dx;

	//y first

	SWord	abs_dy = dy<0?-dy:dy;

	abs_dy <<= 8;

	abs_dy /= rad;

	abs_dy = 256 - abs_dy;

	//abs_dy == 256 when dy == +/- radius
	//abs_dy == 0   when dy == 0

	SWord	dInt,rInt,lInt;
	SWord	BaseCol = 16;
	SWord	ColRange = cInt - BaseCol;

	dInt = abs_dy/dx;

	rInt = BaseCol;

	yoffset = BytesPerScanLine * (cy+dy);
	xoffset = BytesPerPixel * (cx-dx+1);

	scradr = logicalscreenptr + xoffset + yoffset;
	
	int	tmpx = cx-dx+1;

	int	cnt;

	UWord	*wscradr;

	switch (BytesPerPixel)
	{
		case 1:

		lInt = rInt;

		for (cnt=1;cnt<=dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				scradr++;
			else
				*scradr++ = lInt;

 			rInt = ((rInt*dx)+ColRange)/dx;

			lInt = rInt;

			if (lInt>cInt)	lInt=cInt;
		}

		for (cnt=0;cnt<dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				scradr++;
			else
				*scradr++ = lInt;

 			rInt = ((rInt*dx)-ColRange)/dx;

			lInt = rInt;

			if (lInt<BaseCol)	lInt=BaseCol;
		}
		break;

		case 2:

		rInt = 0xFF - rInt;

		dInt =- dInt;

		wscradr = (UWord*)scradr;

		lInt = rInt;

		for (cnt=1;cnt<=dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				wscradr++;
			else
			{
				UWord	srccol,newcol;

				ULong	tmp;

				srccol = *wscradr;

				tmp = srccol & 0x001F; tmp *= lInt; tmp >>= 8;

				newcol = (UWord)tmp;

				tmp = srccol & 0x07E0; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0x07E0);

				tmp = srccol & 0xF800; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0xF800);

				*wscradr++ = newcol;
			}

			rInt+=dInt;

			lInt = rInt;

			if (lInt<0)	lInt=0;
		}

		for (cnt=0;cnt<dx;cnt++,tmpx++)
		{
			if (tmpx<MinX || tmpx>=MaxX)
				wscradr++;
			else
			{
				UWord	srccol,newcol;
				
				ULong	tmp;

				srccol = *wscradr;

				tmp = srccol & 0x001F; tmp *= lInt; tmp >>= 8;

				newcol = (UWord)tmp;

				tmp = srccol & 0x07E0; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0x07E0);

				tmp = srccol & 0xF800; tmp *= lInt; tmp >>= 8;

				newcol += (UWord)(tmp & 0xF800);

				*wscradr++ = newcol;
			}

			rInt-=dInt;

			lInt = rInt;

			if (lInt>0xFF)	lInt=0xFF;
		}
		break;

		default:
		break;
	}

}
