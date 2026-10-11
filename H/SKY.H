//------------------------------------------------------------------------------
//Filename       sky.h
//System         
//Author         Andrew McRae
//Date           Mon 10 Mar 1997
//Description    
//------------------------------------------------------------------------------
#ifndef	SKY_Included
#define	SKY_Included

#define	DEFAULT_SKY 0

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)
#include "MODVEC.H"


class Gust
{
public:

	Bool ACTIVE;
	FCRD Vec;
	SWord Time;
	Gust ()
	{
		ACTIVE = BOOL_FALSE;
		Time = 0;
	}

};
typedef class Gust GUST, *PGUST;

//enum	{CLEAR, RAIN, SNOW}:

//#define DEFAULT_CONTRAIL_LEVEL FEET2CM(41000)	// put this in sky class
#define MAXIMUM_CONTRAIL_DURATION 90000
class AtmosphereData
{
public:
	FP	TempVar;			//K variation in temp last time
	FP	Temp0;				//K	temp at seal level
	FP	Press0;				//millibar
	FP  Density0;			//kg/m3 @ SL	//CSB 27/10/98
	FP	wind0;				//mph
	FP	dir0;				//direction in degrees
	FP	windalt;			//mph
	FP	diralt;				//direction in degrees
	FCRD Wind0;				// wind at zero height
	FCRD WindAlt;			// wind at altitude
	SLong WindAltHeight;		// height of above		//RJS 02Nov98
	SWord MinDuration;		// csecs
	SWord MaxDuration;		// csecs
	FP MinMagnitude;		// 1 = 1 m/s
	FP MaxMagnitude;		// 1 = 1 m/s
	UWord Frequency;		// 0 = always, 65535 = very infrequent
 	//contrails
	FP	ContrailBandCentre;
	FP	ContrailBandWidth;

	FP	Visibility;			// in cm
	FP	Conditions;			//CLEAR, RAIN, SNOW
	FP	CloudLayer;			// bottom layer in cm
	bool	MistInValley;

};
class	Curve;
typedef	 Curve CURVE, *PCURVE, **PPCURVE;

class Atmosphere:	public	AtmosphereData 
{
public:

	// *************************************

	// *************************************

	// conditions
//	FP Temp0;	// Temperature at sea level			   	(288)
//	FP Pres0;	// Pressure at sea level 				(101300)
	FP a;

	// wind
//	Bool WIND;
//	FCRD Wind;
	// new wind (variable with height)

	// gusts
//	Bool GUSTS;
//	GUST Gust;
	FP SeaLevelDensity;


//	Atmosphere ();
//	void Atmosphere::SetContrails (FP contraillevel);
	void SetMissionConditions ();
	void SetMissionTemp (SWord month);
	void SetMissionPress (SWord month);
	void SetMissionDensity();
	void SetMissionWind (SWord month);
	void SetMissionDawnDusk (SWord Latitude, ULong Date, SLong& Dawn, SLong& Dusk);

	Bool Ambient (FP, FP&, FP&, FP&);
	Bool GetWindDirVel (SLong, SWord&, SWord&);							//CSB 10/06/99	
	Bool GetWind (AirStrucPtr const ControlledAC,FCRD&, FP);		// wind and gusts
	Bool GetWind (SLong, SLong&, SLong&, SLong&); // wind only

//	void SetWind (FP, FP, FP);
	void SetWind (FCRD&, FCRD&, FP);
//	void SetWind (Bool state) { WIND = state; }

	void SetGusts (SWord, SWord, FP, FP, UWord);
//	void SetGusts (Bool state) { GUSTS = state; }

	Bool Contrails (AirStrucPtr, ULong&);

};
typedef class Atmosphere ATMOSPHERE, *PATMOSPHERE;


#pragma pack(pop)
#endif

