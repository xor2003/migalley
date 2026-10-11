//------------------------------------------------------------------------------
//Filename       ctrl.h
//System         
//Author         Andrew McRae
//Date           Thu 24 Apr 1997
//Description    
//------------------------------------------------------------------------------

#ifndef	CTRL_Included
#define	CTRL_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)


// control types
enum CTRL_TYPE
{
	CT_INACTIVE,
	CT_ROTATION,
	CT_AERODYNAMIC
};

class Ctrl
{
public:

	CTRL_TYPE Type;

	PMODEL pModel;

	// All controls rotate about X axis
	// only one CT_ROTATION per surface

//	SWord* pInput;		// ptr to actual input value
	int* pInput;		// ptr to actual input value
	SWord OldInput;	// previous input value

	FP Value;	// actual output

	FP ControlScale1;	// scale and sign for the control
	FP ControlOffset1;

	FP ControlScale2;	// scale and sign for the control
	FP ControlOffset2;

	// 1 = min
	// 0 = zero
	// 2 = max

	FP In1,In0,In2;
	FP Out1,Out0,Out2;

	PCURVE pDeltaClCurve;
	PCURVE pDeltaCdCurve;
	PCURVE pDeltaCmCurve;

//	PMODELANIM pAnim;

	PSURFACE pBaseSurface;

	Ctrl (PSURFACE, CTRL_TYPE, SWord*, SWord, SWord, SWord, FP, FP, FP);
	Ctrl (PSURFACE, CTRL_TYPE, int*, SWord, SWord, SWord, FP, FP, FP);
	~Ctrl ();

	LINKEDLIST<Ctrl> List;

};
typedef class Ctrl CTRL, *PCTRL;

class NewCtrl
{
public:

	LINKEDLIST<NewCtrl> List;

	PSURFACE pSurface;
	PMAINPLANE pMainPlane;

//	SWord* pInput;
	int* pInput;
	FP Scale_Pos;
	FP Scale_Neg;
	UWord Chord;

	FP factor;

//	NewCtrl (PSURFACE psurf, SWord* input, FP scale);
//	NewCtrl (PMAINPLANE pmain, SWord* input, FP scale);

	NewCtrl (PSURFACE psurf, int* input, UWord chord, FP scale_pos, FP scale_neg);
	NewCtrl (PMAINPLANE pmain, int* input, UWord chord, FP scale_pos, FP scale_neg);


	~NewCtrl ();

	void Process (FP&, FP&, Bool);

};
typedef class NewCtrl NEWCTRL, *PNEWCTRL;

#pragma pack(pop)
#endif
