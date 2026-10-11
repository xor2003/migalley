//------------------------------------------------------------------------------
//Filename       lbm.h
//System         flying Corps 
//Author         Dave Whiteside
//Date           Mon 19 Feb 1996
//
//------------------------------------------------------------------------------
#ifndef	LBM_Included
#define	LBM_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

const int DEFAULT_LBM 	= 8;
//#define	DEFAULT_LBM 8

typedef	UByte Masking;
#define	mskNone		0L
#define	mskHasMask	1L
#define	mskHasTransparentColour	2L
#define	mskLasso		3L

typedef	UByte Compression;
#define	cmpNone		0L
#define	cmpByteRun1	1L

typedef struct {
	UWord		width,height;					/* width and height */
	UWord		x,y;							/* position */
	UByte 		nPlanes;						/* No of bit planes */
	Masking 	masking;						/* contains mask flags */
	Compression	compression;					/* compression type */
	UByte		pad1;							/* fill byte */	
	UWord		transparentColour;				/* No of transparent colour */
	UByte		xAspect,yAspect;				/* aspects */
	UWord		pageWidth,pageHeight;			/* screen width and height */
}
LBMHeader;

typedef struct {
	unsigned char	red,green,blue;
}	RGBByte;

typedef struct {
	RGBByte	pal[255];
}	Palette;


class	LBM
{
	public:
		LBM();
		~LBM();
		LBM(MigWindow *screen);

	char*	GetBody(short PhysicalWidth,short PhysicalHeight,LogicalPtr logicalscreenptr);
	void	MakeHead(short PhysicalWidth,short PhysicalHeight);
	ULong	MakeBody(LogicalPtr logicalscreenptr,int bpsl);
	void	MakePall(char *);
	void	Save(UWord,ULong);

	private:
		LBMHeader	*hdr;
//		char		*body;
		UByte		*body;
		char		*pal;
};


#pragma pack(pop)
#endif
