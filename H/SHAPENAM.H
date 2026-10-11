//------------------------------------------------------------------------------
//Filename       shapenam.h
//System         
//Author         Robert Slater
//Date           Fri 20 Nov 1998
//Description    
//------------------------------------------------------------------------------
#ifndef	SHAPENAM_Included
#define	SHAPENAM_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_SHAPENAM 0

#include	"SHAPENUM.G"

#ifndef	NDEBUG

#include "WIN32_COMPAT.H" // RERUN
//#define AFX_CDECL __cdecl
//typedef char CHAR;
//#define CONST               const
//typedef CONST CHAR *LPCSTR, *PCSTR;
//typedef LPCSTR LPCTSTR;
//void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);

#endif


const	ShapeNum	MiniExplosionShape	=	SMOKE;				
const	ShapeNum	BigExplosionShape	=	EXPLO;				
const	ShapeNum	GunfireShape		=	BULLET;				
const	ShapeNum	SmokinShape			=	SMOKE;				
const	ShapeNum	BigSmokeShape		=	BSMKE;				
const	ShapeNum	InvisibleLauncher	=	EMPTY;				
const	ShapeNum	GroundExplosionShape=	EXPLO;				
const	ShapeNum	CraterShape			=	CRATER;				
const	ShapeNum	SmallFireShape		=	SMLFIR;				
const	ShapeNum	LargeFireShape		=	FIRE;				
const	ShapeNum	FlakShape			=	SMOKE;				
const	ShapeNum	SmallCraterShape	=	HOLE;				
const	ShapeNum	HugeExplosionShape	=	BEXP;				
const	ShapeNum	RicochetSmokeShape	=	RSMOKE;				
const	ShapeNum	ShockwaveShape		=	EMPTY2;				
const	ShapeNum	BoxShape			=	AMMOC;				
const	ShapeNum	BirdShape			=	EMPTY;				
const	ShapeNum	ChickenShape		=	EMPTY;				
const	ShapeNum	DustShape			=	SDUST;				
const	ShapeNum	GermFlakShape		=	EMPTY;				
const	ShapeNum	BritFlakShape		=	EMPTY;				
const	ShapeNum	TrenchShapeBrit		=	EMPTY;				
const	ShapeNum	TroopLauncherShape	=	EMPTY;				
const	ShapeNum	FuelBarrelShape		=	EMPTY;				
const	ShapeNum	TinyFireShape		=	EMPTY;				
const	ShapeNum	TrenchShapeGerm		=	EMPTY;				
const	ShapeNum	VapourShape			=	VAPSHP;				
const	ShapeNum	VapourStreamShape	=	HOLE;				
const	ShapeNum	CloneShape			=	GCLONE;				
const	ShapeNum	MigPuffShape		=	DUMMY2;				
const	ShapeNum	NapalmShape			=	DUMY10;

const	ShapeNum	BlokeFlyingShape	=	BLOKFY;				
const	ShapeNum	BlokeRunShape		=	BLOKRN;				
const	ShapeNum	BlokeFireShape		=	BLOKFR;				

const	ShapeNum	DebrisDirtShape		=	DEB4;				
const	ShapeNum	DebrisWoodShape		=	DEB1;				
const	ShapeNum	DebrisBrickShape	=	DEB3;				
const	ShapeNum	DebrisMechShape		=	DEB5;				
const	ShapeNum	DebrisStartShape	=	DEB1;				
const	ShapeNum	DebrisEndShape		=	DEB7;				

const	ShapeNum	ExplosionShockShape =	CTWR2;				
const	ShapeNum	NapalmThugShape 	=	CONSHP;				
const	ShapeNum	NapalmFingerShape 	=	ROKSHP;				
const	ShapeNum	ExplosionThugShape 	=	DUMMY3;				

const	ShapeNum	FireballShape 		=	DUMMY9;
const	ShapeNum	SmallFireballShape 	=	HOLE;

const	ShapeNum	SmallScatFireShape	=	CLSFIR;
const	ShapeNum	LargeScatFireShape	=	CLLFIR;

const	ShapeNum	SparkShape	=	TRACER;

const	ShapeNum	SmallFlashShape		=	WRECKT;
#pragma pack(pop)
#endif
