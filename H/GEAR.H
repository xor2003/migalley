//------------------------------------------------------------------------------
//Filename       gear.h
//System         
//Author         Andrew McRae
//Date           Thu 27 Feb 1997
//Description    
//------------------------------------------------------------------------------
#ifndef	GEAR_Included
#define	GEAR_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_GEAR 0

enum UC_STATUS
{
	OFFGROUND,
	TOUCHING,
	BOTTOMEDOUT,
	ALLBOTTOMEDOUT,
	CRASHED,
	UNDEFINED
};

enum GEAR_IDENT
{
	NOSE,
	TAIL,
	PORT,
	STBD
};

// Undercarraige
class Gear {

public:

	GEAR_IDENT Ident;

	PMODEL pModel;

	// suspension
	FP CStiffness;
	FP CDamping;

	// tyres & brakes
	FP RollRes;
	FP CFriction;
	FP CBrake;

	// in ac coords
	FCRD pos0;
	FCRD pos1;
	FP LegLength;
	FCRD UnitVec;
	FCRD LegVec;

	FCRD GroundForce;
	FCRD LegForce;
	FCRD TyreForce;

	FP LegPos;		//[0,1]
	FP OldLegPos;	//[0,1]
	FCRD WheelPos;

	FCRD IntPos;
	FP IntL;

	// Note:
	// Max(1) is fully pushed in.
	// Min(0) is fully extended.

	Bool Touched;

	void SetDimensions (FP, FP, FP, FP, FP, FP);
	void SetSuspension (FP, FP);
	void SetTyres (FP, FP, FP);

	Bool CalcIntL (UByte*);
	void CalcIntPos ();

	void CalcLegForce_Reaction(AirStrucPtr const ControlledAC, UByte* Texture);
	void CalcTyreSideForce (AirStrucPtr const ControlledAC, FP TotalReaction, FP forcex, UByte Texture);

//TempCode ARM 10Mar97 	void CalcGPos (FCRD&);
//TempCode ARM 10Mar97 	void CalcWorldGPos (FCRD&);
//TempCode ARM 10Mar97 	void CalcWPos (FCRD&);
//TempCode ARM 10Mar97 	void CalcWorldWPos (FCRD&);

	Gear (PMODEL, GEAR_IDENT);
	~Gear ();

	LINKEDLIST<Gear> List;

};
typedef class Gear  GEAR, *PGEAR, **PPGEAR;
//TempCode ARM 10Mar97 void CalcOriFromPoints (FORI& ori, FCRD& p1, FCRD& p2, FCRD& p3, Bool Tail);

#pragma pack(pop)
#endif

