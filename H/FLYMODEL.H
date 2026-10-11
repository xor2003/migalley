//------------------------------------------------------------------------------
//Filename       flymodel.h
//System         
//Author         R. Hyde 
//Date           Thu 26 Oct 1995
//Description    flymodel.cpp header file
//------------------------------------------------------------------------------
#ifndef	FLYMODEL_Included
#define	FLYMODEL_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

	#include	"PLANETYP.H"									//PD 01Nov95


 enum	{	MAXAILERON	=	32767,	//38,								//RDH 15Dec95
			AILERONFACTOR = 32767/38,
			MAXELEVATOR	=	32767,	//56,								//RDH 15Dec95
			ELEVATORFACTOR = 32767/56,
			RUDDERFACTOR = 32767/64,
			RUDDER_B000	=	-20480,
			MAXELEVATORMOD=	29000,
			MAXRUDDER	=	32767};	//64};							//RDH 19Dec95


#define	DEFAULT_FLYMODEL 0

typedef	void	*INSTABILITY;
//
// SupAirstruc replacements: Manual pilot replaces flight model bits
//
const int STALLTIME = 500;
class	ViewPoint;
class	ManualPilot
{
	ViewPoint*	View_Point;
	WorldStuff*	myworld;										//PD 25Jun96
public:
	enum	CONTROLMODE	{AUTO = 0,
						MANUAL,
						PILOTDEAD,
						TOGGLE,
						AUTOACCEL_WAYPT,
						AUTOACCEL_FWD,
						AUTOACCEL_MSG,
						AUTOACCEL_MISS,
						AUTOACCEL_HOME};
	CONTROLMODE	controlmode;		//auto/manual  			// piloted ac only (normal or acclerated)
	AirStruc*	ControlledAC2;
	ItemBasePtr	acceltochanged;
	UWord		pilotframetime;
	SLong		lift;											//RDH 30Mar96
	SWord		right_aoa,
				aoa_correction_right,
				aoa_correction_left,
				left_aoa;

private:
//temp vars														//RDH 20Dec95
	SWord		airdensity,
				cl_coeff,
				sos,
				sinoldacpitch,
				cosoldacpitch,
				sinoldachdg,
				cosoldachdg,
				sinslip,
				cosslip,
				totalslip,
				sinoldacroll,
				cosoldacroll;

	SLong		totaldrag,
				trim,											//RDH 15Nov96
				parasiticdrag,
				shockdrag,
				taildrag,
				rudderdrag,
				taillift,
				rudderlift,
				rightwinglift,
				leftwinglift,
				rightwingdrag,
				leftwingdrag,
				sideforce,
				dynamicpressure,
				groundlevel,
				vel_from_yaw,
				aoa_from_dihedral,
				vel_from_roll,
				hori_vel_left,
				hori_vel_right,
				vert_vel_left,
				vert_vel_right,
				aoa_from_aileron,
				vel_from_turn,
				aoa_from_turn,
				vel_from_pitch,
				aoa_from_pitch,
				mass,
				lastmsgtime,
				thrust;
	SWord		grndvel;
	SWord		flapsitu,flapcl,flapcmo;
	SWord	keyboardcontrol;									//RDH 29Apr96
	SLong		velocity_p2;
	UWord		cycletimeres;
//manual ac specific		   
	UWord		//sensitivity,										  //JIM 11/06/99
				cntstick_f,
				cntstick_b,
				cntstick_l,
				cntstick_r,
				cntrudder_l,
				cntrudder_r;
	SWord		device;	

UWordP		elevatordeflectiondata,
			machdata,
				airdensitydata,
				airdensityratio,
				shokdata,
				parasiticdata;

private:
	void	SubManualPilot(AirStrucPtr const ControlledAC);
//	void	ModifyControlInput(AirStrucPtr const ControlledAC);	//RDH 01Apr96
	void	FlightModelFixedTime(AirStrucPtr const ControlledAC2);
	void	MachDensity (AirStrucPtr const ControlledAC);
	void	WW2Thrust(AirStrucPtr const ControlledAC);
	void	CalcTailLift (AirStrucPtr const ControlledAC);
	void	CalcTailDrag (AirStrucPtr const ControlledAC);
	SLong	CalcTailLiftSub (AirStrucPtr const ControlledAC,SWord elevator,SWord aoa);
	void	CalcRudderLift (AirStrucPtr const ControlledAC);
	void	CalcRudderDrag (AirStrucPtr const ControlledAC);
	SLong	CalcLiftSub(AirStrucPtr const ControlledAC,SWord aoa,SLong vel);
	void	CalcSideForce (AirStrucPtr const ControlledAC);
	SWord	CalcK2 (AirStrucPtr const ControlledAC,SWord elevator);
	SWord	CalcK2Rudder (AirStrucPtr const ControlledAC);
	void	CalcDrag (AirStrucPtr const ControlledAC);
	void	CalcParasiteDrag (AirStrucPtr const ControlledAC);
	SLong	CalcDragSub(AirStrucPtr const ControlledAC,SWord aoa,SLong vel);
	void	CalcTrim (AirStrucPtr const ControlledAC);			//RDH 15Nov96
	void	CalcRotas (AirStrucPtr const ControlledAC);
	void	CalcDeltas (AirStrucPtr const ControlledAC);

	void 	TransformAngles(ANGLES, ANGLES, ANGLES, SLong&, SLong&, SLong&);//PD 01Jul96

	#ifndef	PUBLICVERSION										//PD 01Jul96
	void	CalcDeltasDebug ();									//PD 01Jul96
	#endif														//PD 01Jul96

	void	CalcVel (AirStrucPtr const ControlledAC);
	void	VcoAlfaBeta (AirStrucPtr const ControlledAC);
	void	CalcPosition (AirStrucPtr const ControlledAC);
	void	ChkLandedLocation (AirStrucPtr const ControlledAC);
	void	MoveToGrndPitchRoll(AirStrucPtr const ControlledAC,ANGLES grndroll,ANGLES grndpitch);//RJS 02Oct96
	void	CalcGrndInfo (AirStrucPtr const ControlledAC);		//RDH 05Jun96
	void	DataTrans (AirStrucPtr const ControlledAC);
//**	void	SetFlags ();
	void	DoTrim (AirStrucPtr const ControlledAC);			//RDH 15Nov96
	void	RotasFixedRate_x(AirStrucPtr const ControlledAC);	//RDH 19Mar96
	void	RotasFixedRate_y(AirStrucPtr const ControlledAC);	//RDH 09Apr96
	void	RotasFixedRate_z(AirStrucPtr const ControlledAC);	//RDH 09Apr96
	void	GetStickKeys (AirStrucPtr const ControlledAC);
	void	GetRPMABKeys(AirStrucPtr const ControlledAC);
	void	SensitivityMsg(AirStrucPtr const ControlledAC);		//RDH 03Dec96
	void	GetKeyCommon(AirStrucPtr const ControlledAC);
	void	SecondaryControls (AirStrucPtr const ControlledAC2);//ARM 12Aug97

	void	ApplyDeadZone(AirStrucPtr const ControlledAC, SWord SrcVal, int& DstVal, SWord DeadZone);
	void	ApplyDeadZone2(AirStrucPtr const ControlledAC, SWord SrcVal, int& DstVal, SWord DeadZone);
	void	ApplyDeadZone3(AirStrucPtr const ControlledAC, SWord SrcVal, int& DstVal, SWord DeadZone);
	//	void	FindMaxRPM (AirStrucPtr const ControlledAC);

//automove test




public:
	ManualPilot();
	void	CalcLift (AirStrucPtr const A);
	void	MainManualPilot();
	void	ACMManualPilot(AirStrucPtr const A);
	void	InitManualPilot (AirStrucPtr const A);				//RDH 05May96
	void	InitManualPilotSub (AirStrucPtr const A);				//RDH 01Nov96
//rdh	void	InitAircraft(AirStrucPtr const ControlledAC, Bool players);
	void InitFlightModelPos(AirStrucPtr const ControlledAC);
	void AutoToggleAircraft(AirStrucPtr const ControlledAC, CONTROLMODE mode);

	void InitSimpleAc(AirStrucPtr const ControlledAC);

	void	PrintStatistics (class MigWindow*);
	void	Associate(
			AirStruc*	manualas,				//item to move
			CONTROLMODE		inmanual=MANUAL);			//flag for man/auto
	void	SetWorldPtr(WorldStuff *worldptr)					//PD 25Jun96
			{myworld=worldptr;};								//PD 25Jun96
	void	AutoToggle(CONTROLMODE dead=TOGGLE);

	Bool	IsPiloted(ItemBasePtr i)	{return (Bool) (i==ControlledAC2);}//JIM 26Jul96
	Bool	DeathSequenceOverride(ItemPtr,int,bool commscalled=false);					//AMM 26Jan99

};

	extern	class ManualPilot Manual_Pilot;

#pragma pack(pop)
#endif

