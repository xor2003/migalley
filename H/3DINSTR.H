//------------------------------------------------------------------------------
//Filename       3dinstr.h
//System         
//Author         Paul.   
//Date           Wed 16 Aug 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	Dinstr_Included
#define	Dinstr_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

//Offset in animdata...
#define	ABITOFF		9											//RJS 21Apr99

enum	AnimSizes	{MINANIM=10,								//RJS 21Apr99
					AIRCRAFTANIM,
					WEAPANIM,	   
					BLOKESANIM,									//RJS 01Jun99
					POLYPITANIM,   
					SUNANIM,	   
					WINDSOCANIM,   
					MOVEGUNANIM,   
					BARAGEANIM,	   
					TANKANIM,	   
					TRUCKANIM,	   
					TRAINANIM,	   
					TROOPANIM,	   
					BRIDGEANIM,	   
					LAUNCHERANIM,  
					CLONEANIM,	   
					FLAREANIM,	   
					THUGANIM,	   
					MISSILEANIM,   
					DEBRISANIM,	   
					SMOKETRAILANIM,
					PARACHUTEANIM  
					};								

//������������������������������������������������������������������������������
//������������������������������������������������������������������������������

enum	LnchrType	{LT_BULLET = 0, LT_CANNON, LT_CANNON2,
					 LT_BOMB = 0x10, LT_BOMB500 = 0x10, LT_BOMB1000,LT_SMALLBOMB,LT_NAPALM,LT_BOMBB29,LT_BMB1B29,
					 LT_ROCKET = 0x20,LT_ROCKET4=0x20,LT_ROCKET6,LT_ROCKET8,
					 LT_WEAPON = 0x30,
					 LT_MOVEGUN = 0x70,
						LT_RPUFF =0x70,LT_RPUFFSMALL,LT_BPUFF,LT_REDSB12,LT_REDSB37,LT_BLUESB,
						LT_BARRAGE,LT_REDGUNBARRAGE,LT_BLUEGUNBARRAGE,
						LT_BTURRETTOP,LT_BTURRETBOT,
						LT_BTURRETTOPREAR,LT_BTURRETBOTREAR,
					 LT_SMKTRAILS = 0x80, LT_FUEL = 0x80, LT_VAPOUR, LT_CONTRAIL, LT_INTFUEL,
					 LT_DUST,LT_MUDDY,LT_JETDUST,LT_WINGVAP,LT_ENGINE, LT_FUNNEL,
					 LT_CONTACT = 0xd0, CT_WHLLEFT = 0xd0, CT_WHLRIGHT, CT_WHLFRONT,
					 CT_WHLBACK, CT_FRONT, CT_BACK, CT_LEFT, CT_RIGHT, CT_BELLY,
					 CT_EYE,CT_GRAVITY,CT_DUMMY,				//DAW 08Mar99
					 LT_MASK = 0xf0
					};											//RJS 01Oct98

enum	DialType	{DT_RPM = 0, DT_ALT = 1, DT_MPH = 2, DT_FUEL = 3};//RJS 16Sep96


//PD 17Apr96 - new a/c bits off code to cope with drawing dammaged
//				a/c sections

enum BitFlags {	BF_LBWing	=	0x00,
				BF_LTWing	=	0x02,
				BF_RBWing	=	0x04,
				BF_RTWing	=	0x06,
				BF_Tail		=	0x08,
				BF_Gear		=	0x0A,
				BF_Nose		=	0x0C,
				BF_TheRest	=	0x0E,							
				BF_Pilot	=	0x10,							
				BF_Engine	=	0x12,							
				BF_Fuel		=	0x14
			  };												//RJS 27Feb97

enum BitStates {BS_ALIVE=0,
				BS_DAMLV1=85,									//RJS 02Apr98
				BS_DAMLV2=170,									//RJS 02Apr98
				BS_DEAD = 255};

enum DamageTypes
{
	DMT_BULLET = 0,
	DMT_FIRE,
	DMT_SHOCKWAVE,												//RJS 06Dec98
	DMT_EXPLOSION												//RJS 06Dec98
};




#pragma pack(pop)
#endif
