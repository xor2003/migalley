//------------------------------------------------------------------------------
//Filename       modint.h
//System         
//Author         Andrew McRae
//Date           Wed 26 Feb 1997
//Description    Model Interface
//------------------------------------------------------------------------------
#ifndef	MODINT_Included
#define	MODINT_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_MODINT 0


enum MODELANIMTYPE
{
	MA_VOID,
	MA_CTRL,
	MA_SUSPENSION,
	MA_SURFACE,
	MA_GENERICFP,
	MA_AEROSURFACE
};

typedef class ModelAnim
{

public:

	MODELANIMTYPE Type;

	PCTRL pCtrl;
	PGEAR pGear;

	PMODEL pModel;

	ULong pOffset;	// offset from AirStrucPtr of destination
	SLong pSrcOffset;	// offset from AirStrucPtr of destination
	void* pAnim;	// destination
	void* pSrce;	// source

	SLong Min, Zero, Max;

	SLong Scale;
	SLong ScaleUp;

	// MA_SURFACE & MA_GENERICFP
	ModelAnim (AirStrucPtr const ControlledAC,MODELANIMTYPE, PMODEL, void*, void*, SLong minval=-32768,SLong maxval=32767,SLong scale=0,Bool AnimAC=BOOL_TRUE);//RJS 20Jul98

	// MA_SUSPENSION
	ModelAnim (AirStrucPtr const ControlledAC,MODELANIMTYPE, PGEAR, void*, ULong, ULong);

	~ModelAnim ();

	void ProcAnimCtrl ();
	void ProcAnimSuspension ();
	void ProcAnimSurface ();
	void ProcAnimGenericFP ();

	LINKEDLIST<ModelAnim> List;

} MODELANIM, *PMODELANIM;

#pragma pack(pop)
#endif
