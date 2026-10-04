//------------------------------------------------------------------------------
//Filename       worldinc.h
//System         
//Author         Paul.   
//Date           Thu 19 Oct 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	WORLDINC_Included
#define	WORLDINC_Included
#include	"MOVEMENT.H"										//RDH 19Feb96
#include	"MYANGLES.H"
#include	"SHAPENUM.G"										//JIM 31Oct95
#include	"UNIQUEID.H"										//JIM 31Oct95
#include	"FLYINIT.H"
#include	"BITCOUNT.H"
#include	"MODVEC.H"
#include	"ANIMPTR.H"
#define	DEFAULT_WORLDINC 0

typedef class Model* PMODEL;									//ARM 07Mar97
typedef class AcmModel* PACMMODEL;								//ARM 14May97
typedef class _instruments *PINSTRUMENTS;						//ARM 27Aug97

#ifndef	NDEBUG
void	DbgMemTest2(void*);
#endif

class	MinAnimData;
 
typedef	char	(MinAnimData::*Animfieldref);
typedef	char	(MinAnimData::*AFR);

//FileItem - part of worldload - discarded
//

//Protected animdata stuff....
class	GrpMinAnimData;
class	SunAnimData;
class	ExplodeAnimData;
class	FireAnimData;
class	BalloonAnimData;
class	BarageAnimData;
class	TroopAnimData;
class	TrainAnimData;
class	BlokesAnimData;					
class	WeapAnimData;
class	MoveGunAnimData;
class	TwoGunAnimData;
class	AircraftAnimData;
class	PolyPitAnimData;
class	LauncherAnimData;
class	WindsocAnimData;
class	CloneAnimData;
class	FlareAnimData;
class	ParachuteAnimData;
class	ThugAnimData;
class	MissileAnimData;
class	DebrisAnimData;
class	SmokeTrailAnimData;
struct	Coords3DList;

typedef	struct WayPoint	*WayPointPtr;
typedef	FormationTypeIndex	FORMATIONDESC;
typedef	void	*msgptr;
struct	WEAPCLASS
{
	UWord	group,entry;
};
typedef	void	*CALLPTR;
typedef	void	*LIGHTS;
typedef	void	*animrtnref;
typedef	void	*shpintrpptr;
typedef	void	*weapptr;

struct	PlaneType;
typedef	PlaneType	*ClassPtr;

//Blues are aggressive friendly forces (do the killing)
//Reds are aggressive unfriendly forces (get killed)
//green are non-aggressive neutral forces (bugger 'em)
//amber are aggressive neutral forces (bugger me)

#define	TDXBASE	0
#define	TDZBASE	0

#define MAXNUMBEROFTRANSIENTS 64								//MGA 27Feb96

struct UNIQUE_ID
{
    BEGIN_BITFIELD_STRUCT(bitfieldUid, UWord)          // ← safe member name
        FIRST_BITFIELD(UniqueID, count,     13)   // type is still UniqueID
        BITFIELD(      Bool,     changed,   14,14)
        LAST_BITFIELD( Bool,     commsmove, 15,15)
    END_BITFIELD_STRUCT(bitfieldUid, UWord)            // creates member "bitfieldUid"
};

enum ITEM_SIZE {
	ItemBaseSize,				//this layout is intended
	WayPointSize,				//to show the hierachy of
	ItemSize,					//ItemBase derived types.					
	HdgSize,
	HPSize,
	RotatedSize,
	MovingSize,
	MobileSize,
	TransientSize,
	VIEWPOINTSIZE2,
	FormationSize,
	ShipSize,
	AirStrucSize,

	ITEMBASESIZE=0,
	WAYPOINTSIZE,				//this layout is intended
	ITEMSIZE,					//to show the hierachy of
	HDGSIZE,					//ItemBase derived types.
	HPSIZE,
	ROTATEDSIZE,
	MOVINGSIZE,
	MOBILESIZE,
	TRANSIENTSIZE,
	VIEWPOINTSIZE,
	FORMATIONSIZE,
	SHIPSIZE,
	AIRSTRUCSIZE,

	ITEM_SIZE_ALIGN=255
};

struct	ITEM_STATUS
{
	BEGIN_BITFIELD_STRUCT(LaunchFlags, UWord)       // UWord = unsigned 16-bit
    FIRST_BITFIELD (Bool,   BlueLaunch,    0)          // bit 0
    	BITFIELD       (Bool,   ManLaunch,     1, 1)       // bit 1
    	BITFIELD       (Bool,   Drawn,         2, 2)       // bit 2
    	BITFIELD       (ITEM_SIZE, size,       3, 7)       // bits 3–7  (5 bits)
    	BITFIELD       (UByte,  deadtime,      8,12)       // bits 8–12 (5 bits)
    	BITFIELD       (UByte,  deadscale,    13,14)       // bits 13–14 (2 bits)
    LAST_BITFIELD  (Bool,   deaded,       15,15)       // bit 15
	END_BITFIELD_STRUCT(LaunchFlags, UWord)

//	Bool		BlueLaunch:	1,
//				ManLaunch:	1,
//				Drawn:		1;
//	ITEM_SIZE	size:		5;
//	UByte		deadtime:	5,	//max=32
//				deadscale:	2;	//�hours/2hours/days/weeks

	ITEM_STATUS() noexcept
	{
		LaunchFlags.value = 0;
	}
	ITEM_STATUS(Bool bl,Bool ml=FALSE,Bool d=FALSE,ITEM_SIZE is=ITEMBASESIZE,UByte dt=0,UByte ds=0) noexcept
	{
		LaunchFlags.BlueLaunch=bl;
		LaunchFlags.ManLaunch=ml;
		LaunchFlags.Drawn=d;
		LaunchFlags.size=is;
		LaunchFlags.deadtime=dt;
		LaunchFlags.deadscale=ds;
	}
};

inline ITEM_STATUS SetStatus(UByte dt,UByte ds) {ITEM_STATUS rv(TRUE,TRUE,TRUE,ITEMBASESIZE,dt,ds);return rv;};

			
struct	EventLog;
typedef	EventLog	*EventLogPtr;

typedef Bool	TellLeader;										//DAW 30Jun96
enum	NextLeadAct		{NL_OnCourse,NL_CircleFlights,NL_CircleSingles,NL_Flights,NL_DoubleFlighs,NL_PrevRipple,NL_PrevDouble,NL_align=255};
enum	NextFollAct		{NF_PersonalThreat,NF_CircleSingle,NF_Single,NF_Double,NF_ReDouble,NF_align=255};
class	ItemBase		;
class	item			;
class	hdgitem			;
class	hpitem			;
class	rotitem			;
class	MovingItem		;
class	mobileitem		;
class	TransientItem	;
class	formationitem	;
class	shipitem		;
class	AirStruc		;
struct	info_itemS;
struct	info_base;
struct	info_waypoint;
struct	info_grndgrp;
struct	info_airgrp;

typedef ItemBase		*ItemBasePtr;							//JIM 14Feb96
typedef item			Item,*ItemPtr,*itemptr;
typedef hdgitem			*HdgItemPtr;
typedef hpitem			*HpItemPtr;
typedef rotitem			*RotItemPtr;
typedef MovingItem		*MovingItemPtr;
typedef mobileitem		MobileItem,*MobileItemPtr;
typedef TransientItem	*TransientItemPtr;
typedef formationitem	FormationItem,*FormationItemPtr;
typedef shipitem		*ShipItemPtr;
typedef	AirStruc		*AirStrucPtr;

//RERUN: splice an item out of ACList/MobileList before it is freed - stale
//nextmobile links let a freed slot be reused then walked by the move code
extern	void	UnlinkFromMoveLists(MobileItemPtr it);

class ItemBase//:public	TextBlockKnown
{protected:
	ItemBase()	
	{
		MigStatus.LaunchFlags.size=ItemBaseSize;
		(UWord&)uniqueID=(UWord&)MigStatus=0;
	}//JIM 12Sep96
public:
	~ItemBase();
	static	ULong	desposrange;
	static	COORDS3D despos;										//RDH 27Feb96

	ITEM_STATUS	MigStatus;		//o02s02//access with myitem->MigStatus.LaunchFlags.size
	void	operator	delete(void*);
	UNIQUE_ID	uniqueID;	//o00s02//access with myitem->uniqueID.count
	COORDS3D	World;		//o04s0C//access with myitem->World.X
// Fix against "condition is always true"
#ifdef __WATCOMC__
#pragma warning	690	6
#endif
#ifdef	worldincassert
	#undef	worldincassert
#endif

#ifdef	assert
#ifndef	NDEBUG
#ifdef __MSVC__
	#define	worldincassert
#endif
#endif
#endif

 	operator	ItemBase* ()
		{return(ItemBase*) (this);}

#ifndef	worldincassert
 	operator	item* ()	  	{
	 			return(item*) (this);	}
 	operator	WayPoint* ()  	{
	 			return(WayPoint*) (this);	}
	 
 	operator	hdgitem* ()  	{
	 			return(hdgitem*) (this);	}
	 
 	operator	hpitem* ()  	{
	 			return(hpitem*) (this);	}
	 
 	operator	rotitem* ()  	{
	 			return(rotitem*) (this);	}
	 
 	operator	mobileitem* ()  	{
	 			return(mobileitem*) (this);	}
	 
 	operator	MovingItem* ()  	{
	 			return(MovingItem*) (this);	}
	 
 	operator	TransientItem* ()  	{
	 			return(TransientItem*) (this);	}
	 
 	operator	FormationItemPtr ()  	{
	 			return(FormationItemPtr) (this);	}
	 
 	operator	ShipItemPtr ()  	{
	 			return(ShipItemPtr) (this);	}
	 
 	operator	AirStrucPtr ()  	{
	 			return(AirStrucPtr) (this);	}
 
	operator    info_itemS*	()		{
				return	(info_itemS*)(this);	}

	operator    info_base*	()		{
				return	(info_base*)(this);	}

	operator    info_waypoint*	()		{
				return	(info_waypoint*)(this);	}

	operator    info_grndgrp*	()		{
				return	(info_grndgrp*)(this);	}

	operator    info_airgrp*	()		{
				return	(info_airgrp*)(this);	}
#else
 	operator	item* ()	  	{
			 if (this)
	 		 assert (MigStatus.LaunchFlags.size>=ItemSize);
	 			return(item*) (this);	}
 	operator	WayPoint* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size==WayPointSize);
	 			return(WayPoint*) (this);	}
	 
 	operator	hdgitem* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=HdgSize);
	 			return(hdgitem*) (this);	}
	 
 	operator	hpitem* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=HPSize);
	 			return(hpitem*) (this);	}
	 
 	operator	rotitem* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=RotatedSize);
	 			return(rotitem*) (this);	}
	 
 	operator	MovingItem* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=MovingSize);
	 			return(MovingItem*) (this);	}
	 
 	operator	mobileitem* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=MobileSize);
	 			return(mobileitem*) (this);	}
	 
 	operator	TransientItem* ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=TransientSize);
	 			return(TransientItem*) (this);	}
	 
 	operator	FormationItemPtr ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=FormationSize);
	 			return(FormationItemPtr) (this);	}
	 
 	operator	ShipItemPtr ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=ShipSize);
	 			return(ShipItemPtr) (this);	}
	 
 	operator	AirStrucPtr ()  	{
			 if (this)
	 		 assert ( MigStatus.LaunchFlags.size>=AirStrucSize);
	 			return(AirStrucPtr) (this);	}

	operator    info_itemS*	()		{
				return	(info_itemS*)(this);	}

	operator    info_base*	()		{
				return	(info_base*)(this);	}

	operator    info_waypoint*	()		{
				return	(info_waypoint*)(this);	}

	operator    info_grndgrp*	()		{
				return	(info_grndgrp*)(this);	}

	operator    info_airgrp*	()		{
				return	(info_airgrp*)(this);	}
#endif
#ifdef	worldincassert
	#undef	worldincassert
#endif
};

inline	COORDS3D&	operator += (COORDS3D& trg,COORDS3D& src)	{trg.X+=src.X;trg.Y+=src.Y;trg.Z+=src.Z;return(trg);}
inline	COORDS3D&	operator -= (COORDS3D& trg,COORDS3D& src)	{trg.X-=src.X;trg.Y-=src.Y;trg.Z-=src.Z;return(trg);}
inline	COORDS3D&	operator += (COORDS3D& trg,ItemBase* src)	{trg.X+=src->World.X;trg.Y+=src->World.Y;trg.Z+=src->World.Z;return(trg);}
inline	COORDS3D&	operator -= (COORDS3D& trg,ItemBase* src)	{trg.X-=src->World.X;trg.Y-=src->World.Y;trg.Z-=src->World.Z;return(trg);}


//Nasty cludge number 67: This field contains -1 for map items...
struct WPOverlayItem:public ItemBase			//o10s04
{	WayPointPtr	next;	WPOverlayItem()	{next=NULL;}	};
struct ItemOverlayWP:public ItemBase
{	ItemPtr	Next;		ItemOverlayWP()	{Next=NULL;}	};
struct InfoItemFlag:public ItemBase
{	int		Minus1;};
typedef	InfoItemFlag*	InfoItemFlagPtr;

//
//Waypoint structure											//RDH 28Nov95
//Like an item, only different!
//
enum	WPNames
{
	WPNAME_None,
	WPNAME_Rendevous,
	WPNAME_Ingress,
	WPNAME_Target,
	WPNAME_Regroup,
	WPNAME_Egress,
	WPNAME_Disperse,
	WPNAME_Landing
};

struct	WayPoint:public	WPOverlayItem	//o14 //o30	//uniqueid and position	
{
	friend	COORDS3D* FindDesPos(WayPointPtr);
	void	operator delete(void* obj)	{UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN unlink before free

	WayPointPtr		prev;							//o14s04
	SLong			range;							//o18s04
	itemptr			target;							//o1Cs04
	SLong			eta;							//o20s04
	SWord			vel;							//o24s02
	ONLYFIELD(UWord,BFieldWaypointActions,action);	//o26s02
	ONLYFIELD(UByte,WPNames,wpname);				//o28s01//WPNAME_None means no name
	UByte			wpnum;							//o29s01//0 means no number
	//During init I use 1 upper bit of skip UID as forward reference flag.
	ONLYFIELD(UWord,UniqueID,skipunder);			//o2As02
	ONLYFIELD(UWord,UniqueID,skipover);				//o2Cs02
	SWord			DUMMYSPARE;						//o2Es02
	WayPoint*	NextWP(UniqueID uniqueID);

	WayPoint()	{MigStatus.LaunchFlags.size=WayPointSize;target=NULL;next=prev=NULL;
					skipunder=UID_Null;skipover=IllegalSepID;}
	COORDS3D*	FindDesPos();
	WayPointPtr		FindWP(WPNames name);										  //RDH 08/03/99

#ifdef TRACKBADASSIGNMENTS
	operator=(WayPoint&);
#endif

};
COORDS3D* FindDesPos(WayPointPtr w);

//
//	Basic item in world must have shape and world pos + unique ID
//  Do we need briefID?
//
//
class	item:public	ItemOverlayWP				//o14	//o1C
{
public:
	animptr		Anim;						//o14s04//This is the new shape element linked data area//PD 26Mar96
	ONLYFIELD(UWord,ShapeNum,shape);		//o18s02
	ONLYFIELD(UWord,UniqueID,SGT);			//o1As02

static	ANGLES	PitchIntercept;									//RDH 01Dec95
static	ANGLES	HdgIntercept;									//RDH 01Dec95
static	SLong	Range;											//RDH 01Dec95
	void	firstshape(UWord ,animptr );
	void	nextshape(UWord ,animptr );
	void	firstweap(UWord ,animptr );
	void	nextweap(UWord	,animptr );
	int		Distance3D(COORDS3D*);
static int	Distance3D(COORDS3D*, COORDS3D*);	//CSB 26/04/99	
	FP		Distance3DSquared(COORDS3D*);
	void InterceptandRange (COORDS3D*);
static	void InterceptandRange (COORDS3D* subject, COORDS3D* target);
	void InterceptandRange (ItemBasePtr i) {InterceptandRange(&i->World);}//JIM 31Jul96
	UByte&	AnimByte(Animfieldref);

	item()	{MigStatus.LaunchFlags.size=ItemSize;Next=NULL;SGT=UID_NULL;}		//RJS 21Apr99
	int	FACReportDamage(bool	waskill);
	static	void	FACReportGround();
	void	ScoreSimpleItemKilled(bool itsdead,AirStrucPtr killer);
#ifdef TRACKBADASSIGNMENTS
	operator=(Item&);
#endif
};

inline	ItemBase::~ItemBase()									//RJS 21Apr99
{	if ((int)MigStatus.LaunchFlags.size>=(int)ItemSize) 						//RJS 21Apr99
	{															//RJS 21Apr99
		((ItemPtr) this)->Anim.Delete();						//RJS 21Apr99
	}															//RJS 21Apr99
}																//RJS 21Apr99

//
//	Subdividing rotated item to allow simpler objects
//
//
class	hdgitem:public	item			//o1Cs02
{
public:
	ANGLES		hdg;
	hdgitem()	{MigStatus.LaunchFlags.size=HdgSize;}
	void	operator delete(void* obj)	{UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN unlink before free
};

//
//
//
//
class	hpitem:public	hdgitem			//o1Es02
{
public:
	ANGLES		pitch;
	hpitem()	{MigStatus.LaunchFlags.size=HPSize;}
	void	operator delete(void* obj) {UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN unlink before free
};

//
//
//
//
class	rotitem:public	hpitem			//o20s02
{
public:
	ANGLES		roll;
	rotitem()	{MigStatus.LaunchFlags.size=RotatedSize;}
	void	operator delete(void* obj) {UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN unlink before free
};

//
// Mobile item subdivided into vel movement and controled movement
//
class	MovingItem:public	rotitem		//o22
{
public:

void CalcNewPos(SLong deltax,SLong deltay,SLong deltaz);
    SWord		velx,					//o22s02						//RDH 13Nov95
				vely,					//o24s02						//RDH 30Oct95
				velz,					//o26s02						//RDH 30Oct95
				velhori,				//o28s02						//RDH 30Oct95
				vel;					//o2As02//units mm/cs - max 3276m/s=mach 10		//RDH 30Oct95
	//VelX,VelY is not equivalent to VelHori,Hdg when sliding.
	//no longer want deltas inside moving item - calc. as local longs
	//they are useful for collision, of course
	//Delta was Vel * cs /10 -> cm. Max cs=33
	//Vel*4 is a good overestimator throwout before recalc.

	MovingItem()	{MigStatus.LaunchFlags.size=MovingSize;}
};
//
// MobileItem allows a AutoMoveInfo.movecode and a waypoint
//
//
class WorldStuff;
class	shape;
typedef shape Shape;
class	mobileitem:public	MovingItem	//o2C
{
	friend class WorldStuff; // RERUN
	friend class item;
	friend class ManualPilot;									//AMM 01Jun98
	friend class Replay;										//AMM 01Jun98
	friend class DPlay;											//AMM 01Jun98
	friend class Persons2;	friend class Persons3;
	friend class TransObj;										//PD 01May96
	friend	void SendInit2Packet ();							//ARM 17Sep96
	friend Shape;											//RJS 05Aug96
	friend class State3dOnGoing;											//RJS 05Aug96
	friend class ArtInt;											//RJS 05Aug96
	friend class TrgListProc;											//RJS 05Aug96
	friend class BoxCol;											//RJS 05Aug96

private:
	static	void MoveList(MobileItemPtr entry,WorldStuff& world);
	static	MobileItemPtr	MobileList;
	friend	void UnlinkFromMoveLists(MobileItemPtr it);				//RERUN
protected:
	static	void MoveItem(MobileItemPtr entry,WorldStuff&);				//PD 02May96
	static	MobileItemPtr	ACList;
public:
	static  WorldStuff*		currworld;								//DAW 02Apr96
	static	UByte	timerseed;
	static int	timeofday;
	static	void SetWorld(WorldStuff* w)	{currworld=w;}					//JIM 25Jul96
	static	WorldStuff& GetWorld() {return *currworld;}				//ARM 25Jul96
	void	operator delete(void* obj)	{UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN
	static	void MoveAll(WorldStuff& world);

	MobileItemPtr			nextmobile;			//o2Cs04
	WayPointPtr	waypoint;						//o30s04

	BEGIN_BITFIELD_STRUCT(AutoMoveInfo, UByte)          // 8-bit storage
    FIRST_BITFIELD(AutoMoveCodeTypeSelect, movecode,    5)   // bits 0–5  (6 bits wide)
    LAST_BITFIELD (Nationality,            nationality,6,7) // bits 6–7  (2 bits wide)
	END_BITFIELD_STRUCT(AutoMoveInfo, UByte)

	mobileitem()	{MigStatus.LaunchFlags.size=MobileSize;waypoint=NULL;}
	void AddToList();

	void	ExecuteWayPointAction ();								//RDH 19Feb96
	void	DoWayPointAction (AutoMoveCodeTypeSelect	oldmove,WayPointPtr	oldwp,BFieldWaypointActions wpa);

	void	NextWayPoint ();										//RDH 19Feb96
	void	TeleportWayPoint ();										//RDH 19Feb96
	BFieldWaypointActions EndLoopWayPoint ();										//RDH 19Feb96
	BFieldWaypointActions MidLoopWayPoint ();										//RDH 19Feb96
	bool	EscorteePastWP(WayPointPtr);
	void	ForceBreakLoopWayPoint();		

	void	WaitDeadWayPoint ();										//RDH 19Feb96
	void	StartBombing ();										//RDH 19Feb96
	void	StartStraffe ();										//RDH 19Feb96
	void	LandingWayPoint ();
	void	KillItNow ();
	COORDS3D*	FindDesPos();
	void	CalcXYZVel ();
	void	NewPosition ();
	static	void	WinMove (int timeofday,WorldStuff* worldref);							//ARM 16Sep96
	static	void DosMove (int timeofday,WorldStuff* worldref);										//ARM 09Oct96
	WorldStuff* GetCurrWorld () { return currworld; }				//ARM 06Sep96

	static	void	ResetACGears();									//RJS 01Jun99
};

//
//transient item has targets and timers
//
class	TransientItem:public	mobileitem		//o35	//o50
{
public:
	static int transcount;
	void	operator delete(void* obj)	{UnlinkTransList((TransientItemPtr)obj);UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN
	static	void UnlinkTransList(TransientItemPtr it);					//RERUN
	TransientItem()
	{
		MigStatus.LaunchFlags.size=TransientSize;														//RJS 15Jun99
		transcount++;
		isDeleted=0;
		isArmed=0;
	}																					//RJS 15Jun99
	void AddToList();
	static	TransientItemPtr	TransientList;						//MGA 13Mar96
	static	void MoveTransList(TransientItemPtr entry, WorldStuff& world);//DAW 12Mar96
	static	void MoveTransItem(TransientItemPtr entry,WorldStuff&);		//PD 26Apr96
	static	void MoveAllTrans(WorldStuff& world);						//MGA 20Mar96

	UByte	CollTestEnabled		:1,				//o35s01				//RJS 21Jul98
			CollSuccessTrans	:1,								//RJS 21Jul98
			isOwned				:1,								//RJS 29Mar99
			isDeleted			:1,								//DAW 19May99
			isArmed				:1;								//DAW 19May99

	UWord	DrawOffset;							//o36s02				//RJS 27Nov98
	itemptr	Launcher;							//o38s04				//MGA 22Feb96
	itemptr Target;								//o3Cs04				//MGA 22Feb96
	SLong	LaunchTime;							//o40s04

	TransientItemPtr	nexttrans;				//o44s04			//MGA 28Feb96
	TransientItemPtr	nexttogo;				//o48s04

	UWord	TransRandom;						//o4Cs02				//PD 29Apr96
	UWord	TmpLaunchTime;							//o4Es02	//DAW 19May99
	ULong	TimeOfLaunch;										//DAW 19May99
};												//o50


//
// FormationItem allows mobiles to link together
//
//
inline DutyType	BasicDuty(DutyType duty)
{
	if ((int)duty&(int)ACTIONSUBCODE)
		return DutyType((int)duty&(int)DUTYMASK);
	else
		return duty;
}

struct ContourList;
class formationitem:public mobileitem		//o35	//o4A
{
public:
	ONLYFIELD(UByte,FormationIndex,formpos);	//o35s01//alignment optimisation			//JIM 20Aug96

	BEGIN_BITFIELD_STRUCT(FormationInfo, UWord)     // 16-bit storage
    FIRST_BITFIELD(FORMATIONDESC, formation,      5)   // bits 0–5  (6 bits wide)
    	BITFIELD(     Bool,           slowdownleader,6,6) // bit 6
    	BITFIELD(     Bool,           information,   7,7) // bit 7
    	BITFIELD(     DutyType,       duty,          8,13)// bits 8–13 (6 bits wide)
    	BITFIELD(     Bool,           advantage,    14,14)// bit 14
    LAST_BITFIELD(Bool,           disadvantage, 15,15)// bit 15
	END_BITFIELD_STRUCT(FormationInfo, UWord)

	FormationItemPtr
				leader,							//o38s04
				follower;						//o3Cs04

FormationItemPtr&	Leader()	{return (leader);}
FormationItemPtr&	Follower()	{return (follower);}
	ClassPtr	classtype;					   //o40s04

	ContourList*	contourlist;				//o44s04
	
	UWord		manoeuvretime;					//o48s02//max 655sec=11min								//JIM 20/01/99

	DutyType	BasicDuty()
	{
		if ((int)FormationInfo.duty&(int)ACTIONSUBCODE)
			return DutyType((int)FormationInfo.duty&(int)DUTYMASK);
		else
			return DutyType(FormationInfo.duty); //RERUN
	}

	formationitem()
	{	MigStatus.LaunchFlags.size=FormationSize;
		leader=NULL;follower=NULL;contourlist=NULL;
	}

	void	operator delete(void* obj)	{UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN
	SWord TimeLeft();
	void SetManoeuvreTime (UWord time=6000);
	void	AutoTaxi ();
	void	WaitToTaxi();
	SLong CalcReqVel ();
	Bool CalcVel ();
	Bool CalcVelSub (SLong reqvel);			//RDH 28Feb96

	Bool CalcHdgRollFlat ();
	Bool AutoCalcPitch ();
	void MoveRollToZero(SWord	reqdeltahdg);	
	void GetFollower_xyz (FormationTypeIndex formationtype, FormationItemPtr leader = NULL);			//RDH 27Feb96	//CSB 25/05/99	
	void BreakForm();
	void RemoveDeadMember();
#ifdef	FILE_Included
	bool	PlayerSequenceAudible(FileNum f); //msgai
#endif

	ItemPtr				SelectTarget(ItemPtr currtarget,int* selectedelt=NULL,int counter=0);
	int					SelectElement(ItemPtr currtarget,int counter=0);
	FormationItemPtr	SelectConvoyMember(FormationItemPtr curr,int count);
	bool				EnoughDamage3D(ItemPtr currtarget);
	bool				SelectedElement(ItemPtr currtarget,int counter=0,Coords3D* coords=&despos); //alive flag

	Bool CheckForWeapon(UByte ltype);							//CSB 26/03/99	
};

class	shipitem:public	formationitem
{
public:
	shipitem()	{MigStatus.LaunchFlags.size=ShipSize;}
};

struct	ai_info						//contains floats	//o??s60
{
	itemptr		unfriendly,											//o00s04//save
				homebase;											//o04s04//unchanged
	AirStrucPtr	Unfriendly()	{return *unfriendly;}
	AirStrucPtr	attacker;											//o08s04//save

																//o0Cs04
	BEGIN_BITFIELD_STRUCT(PilotSkills, ULong)          // 32-bit storage
    FIRST_BITFIELD(SkillType,      combatskill,  3)   // bits 0–3   (4 bits)
    	BITFIELD(      SkillType,      flyingskill,  4, 7) // bits 4–7   (4 bits)
    	BITFIELD(      UByte,          ManStep,      8,11) // bits 8–11  (4 bits)
    	BITFIELD(      CharacterType,  character,   12,13) // bits 12–13 (2 bits)
    	BITFIELD(      MANOEUVRE,      manoeuvre,   14,20) // bits 14–20 (7 bits)
    	BITFIELD(      Bool,           radiosilent, 21,21) // bit 21
    	BITFIELD(      Bool,           simpleacm,   22,22) // bit 22
    	BITFIELD(      Bool,           flatturn,    23,23) // bit 23
    LAST_BITFIELD( int,            pilotnum,    24,31) // bits 24–31 (8 bits)
	END_BITFIELD_STRUCT(PilotSkills, ULong)

	UByte	morale;												//o10s01	//save
	SByte				moraleinc;								//o11s01	//save

	ONLYFIELD(UByte,tThreatLevel,			threatlevel);		//o12s01						//unchanged
	ONLYFIELD(UByte,tAggressionLevel,		aggressionlevel);	//o13s01						//unchanged
	ONLYFIELD(UByte,tFirstEncounterTactics,	firsttactic);		//o14s01					//unchanged
	ONLYFIELD(UByte,AcTactics,		elementtactics);			//o15s01						//unchanged	
	ONLYFIELD(UByte,AcTactics,		flighttactics);				//o16s01					//unchanged
	ONLYFIELD(UByte,AcTactics,		squadrontactics);			//o17s01					//unchanged	

	SLong		lastdelta;										//o18s04	//save, //delta from ideal shooting position

	ANGLES		desiredroll,									//o1Cs02	//save
				desiredpitch,									//o1Es02	//save

 				desiredaoa,										//o20s02	//save
				oldpitchI,										//o22s02	//save
				oldoldpitchI,									//o24s02	//save
				oldoldoldpitchI,								//o26s02	//save
				oldhdgI,										//o28s02	//save
				oldoldhdgI,										//o2As02	//save
				oldoldoldhdgI,									//o2Cs02	//save
				desiredhdg;										//o3Es02	//save

	Float		desiredturn,									//o40s08	//save
				desiredpitchrate;								//o48s08	//save
	ULong		desiredrange,									//o50s04	//save
				desiredalt;										//o54s04	//save
//there is also a deltapitchsum in acm. The acm one controls the 
//elevator integral control. This one is at a higher level
	SLong		deltapitchsum;									//o58s04	//save

	EventLogPtr	eventlog;										//o5Cs04	//ask jim

	 							//Use fixed speed looping to process times.
	ai_info()	{unfriendly=homebase=NULL;attacker=NULL;eventlog=NULL;}//JIM 08Aug96
	enum	{PROPER_PILOT_MAX=200};
	int	squadnum()	{
		if (PilotSkills.pilotnum<(int)PROPER_PILOT_MAX) 
			return (PilotSkills.pilotnum*11)>>8; 
		else 
			return PilotSkills.pilotnum-(int)PROPER_PILOT_MAX;
	}
};

struct	weap_info					   //o??s20
{
	int		reloadmass;			//o00s04//0.01 g includes all weapons and weight of fuel tank but not weight of fuel
	int		reloaddrag;			//o04s04//N
	int		mass;				//o08s04//0.01 g includes all weapons and weight of fuel tank but not weight of fuel
	int		drag;				//o0Cs04//N
	int		int_fuel;			//o10s04//0.01 g
	int		ext_fuel;			//o14s04//0.01 g
	SWord	int_fuel_leakage;	//o18s02//g/s
	SWord	ext_fuel_leakage;	//o1As02//g/s
	SByte	int_launcher;		//o1Cs01									//RJS 25Mar99
	SByte	ext_launcher;		//o1Ds01									//RJS 25Mar99
	UWord	DUMMYWASTAGE;		//o1Es02
};
struct	weap_ctl			//o??s6C
{
	weap_info	right;			//o00s20
	weap_info	centre;			//o20s20
	weap_info	left;			//o40s20
	
	SWord		ShootDelay;		//o60s02									//save
	BEGIN_BITFIELD_STRUCT(AircraftStatus, UWord)          // 16-bit storage
    FIRST_BITFIELD(Bool, UseIntFuel,         0)      // bit 0
    	BITFIELD(      Bool, FuelDumped,         1, 1)   // bit 1
    	BITFIELD(      Bool, StoresDumped,       2, 2)   // bit 2
    	BITFIELD(      Bool, Ejected,            3, 3)   // bit 3
    	BITFIELD(      Bool, reportedbingo,      4, 4)   // bit 4
    	BITFIELD(      Bool, reportedbingohurting,5, 5)  // bit 5
    // bits 6–10 are intentionally left unused (spare)
    LAST_BITFIELD( int,  Weapons,           11,15)   // bits 11–15 → 5 bits wide
	END_BITFIELD_STRUCT(AircraftStatus, UWord)

	UByte	weapontype;						//o64s01		//LT_ byte
	UByte	shpweapindex;					//o65s01		//Index is basically aircraft type...
	SWord		weapforce;					//o66s02					//RJS 16Sep98
	TransientItem*	currentbullet;			//o68s04					//RJS 29Mar99
};

struct	flight_ctl						//o??s40
{
										//save //RDH 03/03/99
	AirStrucPtr	nextflight,				//o00s04							//save
				leadflight;				//o04s04							//save
	AirStrucPtr	leadelt();
	SWord		rpm,					//o08s02							//save
				thrustpercent,			//o0As02							//save
				afterburner;			//o0Cs02							//not used
	ANGLES		cpitch;					//o0Es02							//calc
	WayPointPtr	backupWP;				//o10s04							//save
	SLong		//cposy,
				gforce;					//o14s04							//save
	SLong		groundlevel;			//o18s04							//calc _Collide.LowestSafeAlt(this,					//JIM 29Oct96
																	//			pitch,				//JIM 29Oct96
																	//		roll);	
	ONLYFIELD(UByte,FormationIndex,originalformpos);	//o1Cs01

	BEGIN_BITFIELD_STRUCT(ComeFromInfo, UByte)              // 8-bit storage
    FIRST_BITFIELD(UByte, comefrommap, 6)               // bits 0–6  → 7 bits wide
    LAST_BITFIELD (Bool,  inRadarSight, 7, 7)          // bit 7     → 1 bit
	END_BITFIELD_STRUCT(ComeFromInfo, UByte)

	SByte		redeffect;								//o1Es01	//ask Paul
	UByte		callname;								//o1Fs01		//ask Jim
	UByte		callnum;								//o20s01		//sak Jim
	UByte		AeroDeviceCnt;							//o21s01		//RJS 18Sep98

	SWord		i_a_s,									//o22s02		//calc from SetInstruments
				vel_cms;								//o24s02		//calc from setFlightParams, questio need
																
	SWord		aileron,								//o26s02		//save
				elevator,								//o28s02		//save
				rudder;									//o2As02		//save

	PMODEL		pModel;									//02Cs04		//model.h	
	PACMMODEL	pAcmModel;								//030s04		//acmmodel.h
	PINSTRUMENTS	pInst;								//034s04		//ask Robert

	Float		dhdg;									//038s08		//save

	flight_ctl()	{backupWP=NULL;nextflight=leadflight=NULL;}

};

struct RequiredBankDataElement									//RDH 18Dec95
	{															//RDH 18Dec95
		ANGLESInst hdg; ANGLESInst roll;								//RDH 18Dec95
	};															//RDH 18Dec95

// Don't want to include "model.h"								//ARM 01May97
Bool NewModel (AirStrucPtr const);								//ARM 01May97
void DeleteModel (AirStrucPtr const);							//ARM 01May97


//variables commented to indicate action required for replay, rdh
//				save			need to savethe variable in the replay file
//				set by ***		use the function *** to set the variable
//				unchanged		the variable, once set is never changed
//				zero			the variable should be zeroed on replaying 


class AirStruc : public shipitem		//o4A
{							//0					1							2							3
public:
// used in coop multiplayer games to decide which player transmits an AI ACs collisions
// with players
	UNIQUE_ID	lasthitter;					//o4As02					//save

public:		
	ai_info		ai;							//o4Cs60
	weap_ctl	weap;						//oACs6C	//ask Rob
	flight_ctl	fly;						//o118s40

	ONLYFIELD(UWord,UniqueID,CommsOwner);	//o158s02

	static	bool		playermoving;
	static	AirStrucPtr	playernotmovingheldac;
	static	bool		PlayerStartedMoving();

	static UWord airdensityratiosquareroot[];

	static UWord vel_to_aoa[];

	//constructors
	AirStruc()													//ARM 01May97
	{															//ARM 01May97
		MigStatus.LaunchFlags.size = AirStrucSize;								//ARM 01May97
		NewModel (this);										//ARM 01May97
	}															//ARM 01May97
	~AirStruc()													//ARM 01May97
	{															//ARM 01May97
		DeleteModel (this);										//ARM 01May97
	}															//ARM 01May97
#ifdef TRACKBADASSIGNMENTS
	operator=(AirStruc&);
#endif

	void AddToList();
	void	operator delete(void* obj)	{UnlinkFromMoveLists((MobileItemPtr)obj);::operator delete(obj);}	//RERUN

//movecodes
	void	AutoCommsAccel();
	void	MoveMissionDiceThrow();
	void AutoTaxi();											//RDH 28Feb96
	void AutoFollowWp ();
	void AutoCrashSkid();
	void AutoAccelerate();
	void AutoCrashRoll();
	void AutoFollowWpWing (AirStrucPtr MyLeader = NULL);		//RDH 27Feb96	//CSB 20/04/99	
	void	AutoTakeOff ();										//RDH 17Jun96
	void
 	AutoLanding ();
	void	HeldACCheck();

	void	AutoTakeOffWing ();									//RDH 17Jun96
	void	TrackBogey();
	void	TrackBandit();

	void	FirstTimeInCombatMsg();						  //RDH 04/03/99
	bool	OutNumbered(AirStruc* trg);								  //RDH 04/03/99

	void	InformOfDanger(AirStruc* caller, AirStruc* callee);													  //RDH 04/03/99
	void AutoPreCombat ();
	void AirCombat();
	void FrightenBogie();						//rdh 17/12/98
	void SelectNextTwoSeaterManoeuvre ();				//RDH 19Nov96
	void TwoSeaterDefence();										//RDH 19Nov96
	void SimpleACM();	
	void ManualACM();
	void BreakCallandReaction(AirStrucPtr buddy,AirStrucPtr currac,AirStrucPtr unf); //RDH 13/06/99
	void BreakCallReaction(AirStrucPtr currac,AirStrucPtr unf);		  //RDH 27/06/99


	void SetDesiredVel();			//rdh9/12/98
	void SelectNextEngageManoeuvre();									//RDH 12Mar96
	void	SetACMType();											  //RDH 26/03/99
	void	ChooseRadioMessage();							  //RDH 04/03/99
	void SelectNextEvadeManoeuvre();									//RDH 12Mar96
	void AutoSpiral2Ground(WorldStuff&);							
	void AutoDeathSequence();									//RDH 10Sep96
	void AutoResurrect(AirStrucPtr);							//ARM 05Dec96
	void AutoToGrndPitchRoll(ANGLES grndroll,ANGLES grndpitch);	//RJS 03Oct96
	void AutoStraffe();											//MS 18Oct96
	void	CheckBombingStatus();
	void	AutoBomb();											//MS 18Oct96
	void	AutoFAC ();
	int		GetFACStage();

	void	AutoCAS();
	AirStrucPtr	FindMyFAC();
	void	SetToCAS(bool firsttime=true);

	void AutoBombHigh();										//CSB 25/03/99	
	void AutoBombLow();											//CSB 25/03/99	
	void AutoBombDive();										//CSB 25/03/99	
	void AutoBombFollow();										//CSB 11/05/99	
	void AutoOrbit(COORDS3D centre, int radius);				//CSB 31/03/99	

	Bool CheckForAnyWeapon();									//CSB 26/03/99	
	Bool ChooseAWeapon();										//CSB 02/04/99	
	Bool CarryingNapalm();										//CSB 29/04/99	
	Bool SelectBombTarget(Bool NewTarg);						//CSB 03/04/99	
	Bool AnyoneInPhaseN(char PhaseNum);							//CSB 03/04/99
	Bool IsThisBombLeader();									//CSB 23/04/99	
	bool IsLocalLeader();									//CSB 11/05/99	
	UWord CountGroup();											//CSB 23/04/99	
	COORDS3D* RotateToHdg(COORDS3D GlobDelta, COORDS3D& LocalDelta, ANGLES Heading);	//CSB 23/04/99	
	UniqueID FindLeadUID();	//CSB 07/06/99
	void WindTrackCorrection(SWord, SWord, SWord, SWord&, SWord&, SWord&);	//CSB 10/06/99	
	void WindBulletDrift    (SWord, SWord, SWord, SWord&, SWord&, SWord&);	//CSB 10/06/99	
	void CalcBulletVel(SWord MuzVel, SWord &Hdg, SWord &Pitch, SWord &BullVel);	//CSB 11/06/99	

	ItemPtr	CheckFACTarget(ItemBasePtr targ);
	void	LogAutoCASKill();

	void SendFACMessage(PhaseFAC phase);
	Bool	HasSmoked();										//RJS 12Mar99	
	AirStrucPtr	FlyEscortTo();	//returns null if independent
	AirStrucPtr	FindLeadCandidate();	//CSB 20/04/99	
	void	ChainBingoMessage(bool atbingohurting);

//secondary stuff
public:
	AirStrucPtr&	Leader()	{return *(AirStrucPtr*)(&leader);}
	AirStrucPtr&	Follower()	{return *(AirStrucPtr*)(&follower);}
	UByte	CombatFactor();
	UByte	FlyingFactor();

	void BreakForm();

	Bool	IsUsingPilotedAcLog();
	COORDS3D* FindDesPos ();
	COORDS3D* FindCombatPos ();
	COORDS3D* PositionWRTLeader (AirStrucPtr lf = NULL);										//RDH 27Feb96	//CSB 25/05/99	
	COORDS3D* PositionTakeOffWRTLeader ();						//RDH 17Jun96
	void SetManualFlightParams ();								//RDH 22Apr96

	void LeaderBreakOff ();									//RDH 24Sep96

	SWord RequiredDRoll (SWord);
	SWord RequiredDRollSub (SWord);								//RDH 11Oct96
	void RemoveFuelLeakage(weap_info weapinfo, SWord Xpos, UWord NFrames);

	void FuelLeft (FP FuelRate);								//RDH 03Jun96	//CSB 15/09/98

	Bool CalcVel ();											//RDH 22Nov96
	Bool AutoCalcPitch ();
	void NewPosition ();
	void NewDeadPosition ();									//RJS 23Feb98
	void CalcNewPosWithWind(SLong deltax,SLong deltay,SLong deltaz,SLong groundlevel);
	void	CalcXYZVel ();
	void	CheckManualCourse();
	Bool	FuelShort(int excesstime,int* avail=NULL,int* safety=NULL );
	Bool	AircraftDamaged();

	static	void PrelimActivePilotChk ();					//RDH 11Dec96
	static	SkillType IncSkill (AirStrucPtr,SWord aircraft_no);			//RDH 04Dec96
	static	void PlayerDecision ();						//RDH 09Sep96
	static	void ModifySquadronMorale(MoraleType);		//RDH 09Sep96
	static	void ModifyPilotMorale(SWord, MoraleType);				//RDH 09Sep96
	static	void ModifyPlayerMerit(DeltaMeritType);				//RDH 09Sep96
	ANGLES BestClimbAngle();
	Bool	IsHealthyMovecode(ULong&);//RJS 28Aug98

	SLong  CalcBestClimbSpeed();	//CSB 28/09/98
	SWord  CalcBestClimbPitch();	//CSB 28/09/98
	SLong  CalcMaxVelSL ();			//CSB 28/09/98
	SLong  CalcMaxVelDive ();		//CSB 28/09/98
	SWord  CalcAccel(SWord);	//CSB 02/02/99
	void   AddAccel2Vel(SWord Accel);	//CSB 12/02/99
	Bool CalcVelAlt ();				//CSB 29/09/98
	Bool CalcVelAlt (SLong	reqvel);								  //JIM 01/02/99
	Bool CalcHdgRollLimited ();		//CSB 29/09/98
	Bool CalcHdgRollLimited (SWord reqdeltahdg);		//CSB 29/09/98
	void	SetWeaponForce(SLong mass=0,SLong velocity=0,SLong delay=1);//RJS 16Sep98
	void SetFlightParams (bool dotimestuff=true);									//AMM 24Sep98
	void	SetMaccaVels();										//AMM 20Oct98

	Coords3D*	GetTakeOffPos(UniqueID takeoffID,Coords3D& postofill=despos);
	Bool	AmISmoking();										//RJS 18Feb99

	AirStrucPtr	GroupLeader()
	{	AirStrucPtr	rv=this;
		if (leader)			rv=Leader();
		if (rv->formpos)	rv=rv->fly.leadflight;
		return	rv;
	}
	Bool	TargetOnSubjectTail(ANGLES);
	Bool	SubjectOnTargetTail(ANGLES);
	Bool	TargetFacingSubject(ANGLES);
	Bool	SubjectFacingTarget(ANGLES);
	void	SetFollowersTactic(int);	//CSB 05/07/99	

private:
	void CopyLeader (AirStrucPtr lf);											//RDH 27Feb96
	Bool CopyLeaderSub (AirStrucPtr lf);											//RDH 27Feb96
	void GetFlightLeader_xyz (SquadronTypeIndex 	formationtype);//RDH 17Jun96
	SWord ReachedDesPos ();
	void SetRpm ();
	Bool RollLevel (int delta);
	Bool CalcVelWing (AirStrucPtr lf, SLong Dist);										//JIM 29Oct96
	Bool CalcHdgRoll ();									//RDH 28Feb96
	Bool CalcHdgRoll (SWord delta);									//RDH 28Feb96
	SLong CalcHdg ();											//RDH 11Oct96
	void WindCorrection();
	Bool SideSlip (AirStrucPtr);											//RDH 28Feb96
	Bool SideSlide(AirStrucPtr);											//CSB 20/04/99	
	SWord FindRequiredRoll (SWord, RequiredBankDataElement*);
	void	SetControlSurfaces (); //RDH 25Mar96
	//ACM member functions
	void	MoveToHdg (ANGLES desiredhdg, ANGLES desiredpitch);		  //RDH 21/02/99
 
	void	MoveToHdgOnThisRoll (ANGLES,ANGLES);
	void	MoveToAoA (ANGLES);
	void	MoveToRudder (SWord);
	void	MoveToSlip (ANGLES);
	void	MoveToPitch (ANGLES,SWord);
	void	MoveToAcPitch (ANGLES);
	SWord	MoveToDesiredPitch (ANGLES,ANGLES);
	SWord	DesiredPitchChk (ANGLES,ANGLES);
	SWord	DesiredAcPitchChk (ANGLES,ANGLES);					//RDH 01Nov96
	Bool	MoveToDesiredHdg (ANGLES desiredhdg,ANGLES desiredpitch,ANGLES target);

	Bool	ChkAtDesiredHdg (ANGLES,ANGLES);						//RDH 01Apr96
	Bool	ChkAtDesiredAcHdg (ANGLES,ANGLES);					//RDH 01Nov96
	Bool	ChkAtDesiredRoll (ANGLES desiredroll,ANGLES target);

	SWord	MoveToDesiredHdgOnThisRoll (ANGLES,ANGLES,ANGLES);
	void	ClimbAtBestSpeed ();
	void	MoveToRoll (SWord,SWord);
	void	MoveToThrust (SWord);
	void	SimpleMoveToThrust (SWord);							//RDH 11Oct96
	void	SimpleCalcVel ();
	SWord 	ManoeuvreTimeLeft ();
	bool	WeldedWingSixOK();
	bool	BreakCallNecessary(AirStrucPtr currac,AirStrucPtr unf, int range);

 	void	AutoShoot (Bool manual);							//RDH 22Oct96
	void	FireABullet(SWord delaybetweenbursts,Bool isArmed=TRUE); //RJS 27May99
	void	FireARocket(SWord delaybetweenbursts,Bool isArmed=TRUE); //RJS 27May99
	void	SplitS ();
	void	ZoomandDrop ();
	void	StandOnTail ( );
	void	GainHeight ( );
	void	ShootToFrighten ( );
	void	DiveAndZoom ( );
	void	FlatTurn ();										//RDH 28Mar96
	void 	GainSpeed();										//RDH 27Apr98
	void 	LookAround ();										//RDH 19Aug96
	void	WeldedWingMan();
	void	BailOut();
	void 	TopCover ();										//RDH 19Aug96
	void	LineAbreast();
	void	Scatter();
	void	Pincer();
	void	MultiWave();

	void	LeadPursuit();
	void	LagPursuit();
	void	Circumvent();
	void	RoundAbout();
	void	SplitManoeuvre();
	void	ResetUnfriendly();
	void	HeadOn ( );
	void	LineAstern();
	void ShootTooEarly();

	void	LagRoll();
	void	ExtensionManoeuvre();
	void	DivingSpin();
	void	ReverseTurn();
	void	SelfAsBait();
	void	Jink();
	void	BreakTurn();
	void	LazyTurn();
	void	Break180();
	void	BreakLow();
	void	BreakHigh();
	void	Break90();
	void	HiGBarrelRoll();
	void	PanicTurn();
	void	UnBalancedFlight();
	void	LowAlt();
	void	SnapShot();
	void	StayOnTail();
	Bool	TurnToHdgAndPitch();
	SWord	RollforTurn(SWord reqdeltapitch,SWord deltahdg, Float& desiredturn,COORDS3D	target);
	void	CalcDesiredPitchRate(SWord	reqdeltapitch, SWord	reqdeltapitch_sign, UWord	reqdeltapitch_pos);
	FP		CoarseRollAdjust(FP rroll, FP fpitch);
	FP		FineRollAdjust(SWord reqdeltapitch,SWord deltahdg,FP rroll, FP fpitch, FP fhdg);
	Bool	UnfriendlyOnSameRoll();

	void	TurningFight();
	void	CalcLead(AirStrucPtr  subject, AirStrucPtr target, SLong& velx,SLong& vely,SLong& velz, Bool cutcorner);

	void	TurnFightSub(ANGLES	HdgIntercept, Bool sitbehindtarget, ANGLES	deltapitch,COORDS3D	target);
	void	TurnFightPhase1(ANGLES	HdgIntercept, Bool sitbehindtarget, ANGLES	deltapitch,COORDS3D	target);

	void	SlowDownCheck(UWord reqdeltahdg_pos);
	void	SlowDown(FP velc);
	void	SpeedUp(FP velc);

	SWord	ThrustfromFF();

	void	TurnWithPitchChange(ANGLES	HdgIntercept, Bool sitbehindtarget,COORDS3D	target);

	void	HighYoYo();
	void	Zoom();
	void	LowYoYo ( );
	void	TurnFightTargetNotOnNose(SWord	reqdeltahdg, SWord	reqdeltapitch, SWord	reqdeltapitch_sign,UWord	reqdeltapitch_pos, ANGLES	deltapitch);
	void	RollAndPull(SWord	reqdeltahdg, SWord dpitch);

	void	InterceptHigh();
	void	ClimbAtSustainedTurnSpeed();

	Bool	BanditOnColdSide();
	Bool	BanditHasLead();

	Bool 	AllOnTopCover ();										//RDH 19Aug96
	Bool	AllButLdrNotOnCombat ();								//RDH 11Nov96
	void	BarrelRollAttack ( );
	void	Scissors ( );
	void	MildScissors ( );
	void	StraightandLevel ( );
	void	SpinOut ( );
	void	DiveforHome ( );
	void	ClimbforHome ( );
	void	StraightDive ( );
	void	GoHome ( );
	void	MakeForFriendly ( );
	void	MoveAway ( );
	void	SpinRecovery ( );								//RDH 21Aug96
	void	LowGYoYo ( );
	void	RollAway ( );
	void	HeadOnOffset ( );
	void	Immelmann ( );
	void	StayWithPrey ( );
	void	LeadUnFriendly ();

	Float	CalcSimpleAccel ();
	UWord	CalcMaxPitchRate();	//CSB 01/06/99	

	void	SimpleLowAlt();											  //RDH 25/05/99

	void	SimpleTurningFight();
	void	SimpleTurnWithPitchChange(ANGLES	HdgIntercept, Bool sitbehindtarget,COORDS3D	target);
	void	SimpleTurnFightSub(ANGLES	HdgIntercept, Bool sitbehindtarget, ANGLES	deltapitch,COORDS3D	target);
	void	SimpleTurnFightTargetNotOnNose(SWord	reqdeltahdg, SWord	reqdeltapitch, SWord	reqdeltapitch_sign, UWord	reqdeltapitch_pos, ANGLES	deltapitch);

	void	SimpleWeldedWingMan();
	void	SimpleBailOut();
	void	SimpleLineAbreast();
	void	SimpleScatter();
	void	SimplePincer();
	void	SimpleMultiWave();
	void	SimpleDiveAndZoom ();
	void	SimpleLeadPursuit();
	void	SimpleLagPursuit();
	void	SimpleCircumvent();
	void	SimpleRoundAbout();
	void	SimpleSplitManoeuvre();
	void	SimpleHeadOn ( );
	void	SimpleLineAstern();
	void SimpleShootTooEarly();
	void	SimpleLagRoll();
	void	SimpleExtensionManoeuvre();
	void	SimpleDivingSpin();
	void	SimpleReverseTurn();
	void	SimpleSelfAsBait();
	void	SimpleJink();
	void	SimpleBreakTurn();
	void	SimpleLazyTurn();
	void	SimpleBreak180();
	void	SimpleBreakLow();
	void	SimpleBreakHigh();
	void	SimpleBreak90();
	void	SimpleHiGBarrelRoll();
	void	SimplePanicTurn();
	void	SimpleUnBalancedFlight();
	void	SimpleTrackingShot();
	void	SimpleSnapShot();
	void	SimpleStayOnTail();
	Bool	SimpleTurnToHdgAndPitch();
	void	SimpleHotSideLagPursuitFight();
	Bool	SimpleSandwichManoeuvres();
	void	SimpleSustainedTurn();
	void	SimpleHighYoYo();
	void	SimpleZoom();
	void	SimpleLowYoYo ( );
	void	SimpleInterceptHigh();
	void	SimpleClimbAtSustainedTurnSpeed();
	void	SimpleMakeForFriendly();
	void	SimpleImmelmann();
	void	SimpleGainSpeed();									//RDH 27Apr98

	void	SimpleGatherSpeed ();
	void	SimpleLeadUnFriendly ();
	void	SimpleSplitS ();
	void	SimpleZoomandDrop ();
	void	SimpleGainHeight ( );
	void 	SimpleTopCover ();
	void 	SimpleLookAround ();
	void 	SimpleBarrelRollAttack ( );
	void 	SimpleScissors ( );
	void 	SimpleMildScissors ( );
	void 	SimpleStraightandLevel ( );
	void 	SimpleAcmSideSlip ( );
	void 	SimpleSpinRecovery ();
	void 	SimpleSpinOut ( );
	void 	SimpleDiveforHome ( );
	void 	SimpleClimbforHome ( );
	void 	SimpleStraightDive ( );
	void 	SimpleStandOnTail ( );
	void 	SimpleShootToFrighten ( );
	void 	SimpleGoHome ( );
	void 	SimpleMoveAway ( );
	void 	SimpleAccidentalSpinOut ( );

	void 	SimpleLowGYoYo ( );
	void 	SimpleVerticalLoop ( );
	void	SimpleImmelmannTurn ( );
	void 	SimpleHeadOnOffset ( );
	void 	SimpleStayWithPrey ( );
	void 	SimpleTightTurn (SLong recovery_alt);
	void 	SimpleTightTurnSub (SLong recovery_alt,COORDS3D	target);
 	void	SimpleTightTurnSubSub (SWord	deltapitch);									//RDH 13Oct96
 	void	SimpleTightClimbingTurn (ANGLES);
	Bool	SimpleMoveToDesiredHdg (ANGLES,ANGLES);
	void	SimpleMoveToHdg (ANGLES);
	void	SimpleMoveToRoll (SWord);
	SWord	SimpleMoveToDesiredPitch (ANGLES,ANGLES);

	Bool	TimeToChangeToEvade ();
	Bool	WeHaveNumbersAdvantage ();
	Bool	DisengageAppropriate ();
	Bool	NoseToTail (ANGLES angle);
	void	SelectNextDisengageManoeuvre ();
	void	PanicManoeuvres();
	void	BanditHighManoeuvre();
	void	BanditLowManoeuvre();
	void	LowAltManoeuvre();
	Bool	TooLow();

	void	MoraleMod();

	Bool	LooseDeuceSituation();
	void	TakeLooseDeuceOption();
	Bool	TimeToChangeToDisengage ();
	SLong SubjectClosure();
	Bool SubjectClosureGreaterThan(SLong dvel);
	Bool SubjectLeadingTarget();
	Bool TargetClosureGreaterThan(SLong dvel);
	Bool ClosureLessThan(SLong dvel);
	Bool M_LagRoll();
	Bool M_FlightSeparation();
	bool AboveGround(SLong height);
	void ManoeuvreBasedOnClosure();
	Bool M_BarrelRoll();
	Bool M_ZoomAndDrop();
	Bool M_SpinOut();
	Bool M_SelfAsBait();
	Bool TargetHasCorrectLead(ANGLES lead, ANGLES maxlead);
	ANGLES ActualLead(AirStrucPtr  subjt, AirStrucPtr trgt);
	MANOEUVRE ManoeuvreBasedOnSkill(SkillType skill, MANOEUVRE goodman, MANOEUVRE badman);
	Bool SubjectHasEnergyAdvantage();
	Bool M_GunDefence(ANGLES lead);
	Bool M_UnSighted();
	Bool M_DiveForHome();
	Bool M_SplitS();
	Bool M_Scissors(ANGLES lead);
	Bool M_ShootTooEarly();
	void DefenceManoeuvre();
	Bool M_ShootToFrighten();
	SLong DeltaAltitude();
	void AggressivePassManoeuvre();
	void DefensivePassManoeuvre();
	void SelectNoseToNoseManoeuvre();
	void SelectNoseToTailManoeuvre();
	void SelectNoseToBeamManoeuvre();
	void SelectBeamToNoseManoeuvre();
	void SelectBeamToTailManoeuvre();
	void SelectBeamToBeamManoeuvre();
	void SelectTailToTailManoeuvre();
	void SelectTailToBeamManoeuvre();
	void SelectTailToNoseManoeuvre();
	SWord CornerSpeed();
	SWord SustainedTurnSpeed();
	Float	Model_InstTurnRate(AirStrucPtr subject);

public:
	bool		AcIsFlightLeader()	{return	((int)position()==0);}
	bool		AcIsGroupLeader()	{return	((int)formpos==0);}
	AirStrucPtr		AcIsPlayer();
	AirStrucPtr		InPlayersElement();
 	AirStrucPtr		FindBuddy();
 	AirStrucPtr		PlayerInGroup();
 	AirStrucPtr		PlayerInFlight();
 	AirStrucPtr		FindAirEscortLeader();
 	AirStrucPtr		AcCloseAttackingOneOfGroup(bool& victimfound);	  //RDH 27/06/99
 	AirStrucPtr		RemoveElement();								  //RDH 03/03/99
	AirStruc*		FindGroupLeaderInFormation();
	AirStruc*		FindGroupLeaderClose();						  //RDH 23/06/99
	AirStruc*		FindGroupLeader();								  //RDH 03/03/99
	AirStrucPtr		FindABuddyWithPlayerGivenPriority();			  //RDH 04/03/99
	AirStrucPtr		FindABuddyNotPlayer();							  //RDH 20/05/99

	AirStrucPtr		FindBandit(int& count);									  //RDH 05/03/99
	AirStrucPtr		FindAnyBandit(SLong& range, int& count);			  //RDH 08/03/99
	AirStrucPtr		FindAcInGroup();						  //RDH 05/03/99
	bool			InSameWave(AirStruc* trg);	  //RDH 04/05/99

	AirStrucPtr		FindBanditOnTail();
	void			SetGroupWP(WayPoint*	newwp);					  //RDH 09/03/99
	AirStruc*	AcShouldGoHome();									  //RDH 09/03/99
	void	AiLeaderBingoInstructions();
	void	SayEndPatrolFuel();
	void	PublicFireABullet(SWord delaybetweenbursts,Bool isArmed=TRUE); //RJS 27May99
	int	position() {return (formpos&InWingMAX);}
	void	MissionDiceThrow(WayPointPtr targetwp=NULL);
};

//
// Shapes: Now 2 parts to each shape... the shape data and the anim data
// There is only 1 instance of the shape data which all items share.
// There is 1 copy of the anim data for each item.
// This anim data may include weaponry records.
//

enum	ScaleType {SHP_1CM=0,SHP_4CM,SHP_16CM,SHP_GRP};			//PD 06Mar96

#endif
