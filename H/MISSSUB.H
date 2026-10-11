//------------------------------------------------------------------------------
//Filename       misssub.h
//System         
//Author         Jim Taylor
//Date           Thu 22 Aug 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	MISSSUB_Included
#define	MISSSUB_Included

#define	DEFAULT_MISSSUB 0

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#include "FILES.H" // RERUN
#include "WORLDINC.H" // RERUN
#include "BITFIELD.H" // RERUN
#include "FLYINIT.H" // RERUN
#include "MYANGLES.H" //RERUN
#include	"TEXT.H"
#include	"RANGES.H"

enum	{	
			MAXTNKWPINROUTE = 2
		};

enum					{LOWCLOUD = 0,CLOUDAT10000FT,BADWEATHER,BADWEATHER10000FT, MAXWEATHERNUM = 4};

enum	SpecialEventBit {SpecialEventMIN = 0,HOMEBASEDESTROYED = 0,
						PLAYERKILLED, RESERVEEXHAUSTED,
//above this you are dead
						PLAYERWOUNDED,RESERVES_2LEFT,RESERVES_6LEFT,
						ACEKILLED,NEARTARGET,
						FATALITYINSQUAD,WOUNDEDINSQUAD,
						MORALEEXCELLENT,MORALEGOOD,MORALEPOOR,MORALEROCKBOTTOM,
						BESTDAY,
						LIEUTENANTPROMOTION,CAPTAINPROMOTION,

//Spring Offensive training
						CRASHLAND,BENTGEAR,GOODLANDING,
						QUITTOOSOON,QUITTOOSOONSO_1,

						CAMPAIGNLOSTTAEJONOVERRUN,
						CAMPAIGNWONENTEREDPYONGYANG,
						CAMPAIGNLOST,
//awarded medals
						AWARDED_MEDALOFHONOUR,
//landing
						PLAYERLANDEDINCOUNTRY,PLAYERCAPTURED,	//RDH 29Nov96
//target hit
						ATTACKEDAIRFRIENDLY,KILLEDAIRFRIENDLY,KILLEDGROUNDFRIENDLY,
						ENEMYDESTROYEDTARGET,FRIENDLYDESTROYEDTARGET,
					  SpecialEventMAX};
//			In 3d Temp_Pilots morale

MAKEFIELD (SpecialEventBit,SpecialEventMIN,SpecialEventMAX);


enum	NewsEvent {NewsEventMIN = 0, NEWSWOUNEDRECOVERED, NEWSACEIN,
									 NEWSACEOUT,NEWSMISSION,NEWSMERITPROMOTION,
									 NEWSDEADMANPROMOTION,NEWSTHEPROMOTION,
									NEWSWOUNDEDMANPROMOTION,	//DAW 23Aug96
									//Spring Offensive: go operational
									 NEWSOPERATIONALGOOD,NEWSOPERATIONALBAD,
//last mission
						NEWSLASTMISSIONOFCAMPAIGN,
//updates
						NEWSCRCUPDATE,	
						NEWS_HAT_25_GOOD,
						NEWS_HAT_25_BAD,
						NEWS_HAT_50_GOOD,
						NEWS_HAT_50_BAD,
						NEWS_HAT_75_GOOD,
						NEWS_HAT_75_BAD,
						NEWS_SPRING_25_GOOD,
						NEWS_SPRING_25_BAD,
						NEWS_SPRING_50_GOOD,
						NEWS_SPRING_50_BAD,
						NEWS_SPRING_75_GOOD,
						NEWS_SPRING_75_BAD,



									//Spring Offensive: lastday
									NEWSSTRONGPOSITION,NEWSWEAKPOSITION,NEWSAVERAGEPOSITION,
									NEWSBESTDAY,				//DAW 23Aug96
										NewsEventMAX};


MAKEFIELD (NewsEvent,NewsEventMIN,NewsEventMAX);

enum	pilotstatus	{NOTFLYING = 0,FLYING, WOUNDED, MIA, IS_DEAD = MIA, KIA};

enum 	RankType {SECLIEUTENANT= 0,SECLIEUTENANT_EXPERT,LIEUTENANT,CAPTAIN,MAJOR, RANK_INC = 1};
enum					{MAXCONVOYS = 12,MAXTANKCONVOYS = 8,MAXTRUCKCONVOYS = 6,MAXROUTES = 4,MAXGUNS = 16};

enum	HatMissionFamily {HP_VOLUNTEER = 0,
			HP_NOCHOICE = 0, HP_BALLOON,HP_PATROL,HP_ESCORT,HP_ROVING};

enum	SquadPanelSelection {PERSONNEL = 0, CONFIGURE, PREFERENCES, PAINTSHOP,SAVEGAME};
enum	ConfigureSelection	{SAVECONFIGURE,RETURNFROMCONFIGURE,CANCELCONFIGURE,GOTOJOYSTICKCONFIG,
								CANCELSCRNCONFIG,ACCEPTSCREENCONFIG,TRYSELECTEDSCREENCONFIG

								};
enum	CtrlVar	{
								GOTOACCEL,GOTOMAP
				};
enum	FlightInfoSelection { FLYNOW= 0, JUSTRETURN};

enum	{
//		MAXCIRCUSMISSIONS 		= 21,
//		MAXHATMISSIONS 			= 21,
//		MAXSPRINGMISSIONS 		= 15,							//RDH 23Nov96
//		MAXTANKMISSIONS 		= 16,

//		CIRCUSPLAYERSLOT 		= 0,
//		HATPLAYERSLOT 			= 11,

//		CIRCUSTARGETKILL 		= 52,
		RADIOMESSAGELEN			= 128,
		PASSWORDNAMELEN			= 11,
		SESSIONNAMELEN			= 128,
		PLAYERNAMELEN			= 21,
		PILOTS_PER_SQUADRON 	= 24,
		PILOTABLE_SQUADRONS		=	5,
		PILOTS_PER_CAMPAIGN 	= (int)PILOTS_PER_SQUADRON* (int)PILOTABLE_SQUADRONS,
		PILOTACTIVE 			= 0,
		PILOTDEAD 				= -1,
		PILOTS_IN_RESERVE 		= 80,
		ACESLIST 				= 0,
		CASUALTYLIST 			= 20,		//(int)ACESLIST + (int)PILOTS_IN_RESERVE,
		FLIGHTSPERSQUADRON	 	= 4,
		FLYINGFLIGHTSPERSQUADRON= 32,//(int)FLIGHTSPERSQUADRON - 1,//Jim 30 jun 98

		WRAPMIGSQUADRONS		=4,
		MAXMIGSQUADRONS			=12,		
//		MAXIMAGEMAPSPERAIRCRAFT = 8,
		};

struct	MissionType;
typedef	MissionType* DayMission [3];
typedef	MissionType* SingleMission;

typedef	MissionType* MissionFamily [6];
typedef MissionFamily MissionArea [5];

extern	SingleMission Spring51_Missions [];
extern	SingleMission PeaceTalk_Missions [];
extern	SingleMission Intro_Missions [];
extern	SingleMission Scramble_Missions [];						//RDH 02Aug96
extern	SingleMission H2H_Missions [];						//RDH 02Aug96
extern 	MissionArea Hat_Missions [2];

struct FormationPos
{
	SLong	range;
	ANGLESInst	bearing;
	SLong	delta_alt;
	SWord	alt_icon;
};

	enum					{MAXFLIGHT_FORMS_NUM = 8};
	enum					{MAXAC_FORMATIONS_NUM = 16,MAXAC_EXTENDEDFORMATIONS_NUM=11};
struct FormationType
{
	int		numentries;
//	TextRef			name;
	FormationPos	wingpos[MAXAC_FORMATIONS_NUM];

};



	enum					{MAXSQUAD_FORMS_NUM = 5};			//RDH 18Aug96
struct SquadronType
{
	int numentries;
//	TextRef			name;
	FormationPos	wingpos [FLYINGFLIGHTSPERSQUADRON];

};

typedef FormationType* SingleFormation;
extern SingleFormation Aircraft_Formations [];

typedef SquadronType* SquadFormation;
extern SquadFormation Squadron_Formations [];

#define MAXPLAYERS 8

struct 	PilotName	{ UWord nameref;char initial1,initial2; };

struct PilotHist
{
	typedef PilotName Name;
	Name	name;
	SWord	kills;
	pilotstatus	status;
	RankType	rank;
	SLong		lastdate;
};

//structure redefined 11/8/98 rdh
enum	HideSkills
{
	HSflyingskill=0,
	HScombatskill=8,
	HSflyingpotential=16,
	HScombatpotential=24,
	HStoskill=4,
	HSmask=255,
	HSskillmask=15,
	HSallskills=0x01010101
};
struct PilotType
{
	typedef PilotName Name;
	Name		name;
	HideSkills	hideskills;
#ifndef	__BCPLUSPLUS__
	int	BigFlyingSkill()		{return (UByte)((hideskills>>HSflyingskill)&HSmask);}
	int	BigCombatSkill()		{return (UByte)((hideskills>>HScombatskill)&HSmask);}
	int	BigFlyingPotential()	{return (UByte)((hideskills>>HSflyingpotential)&HSmask);}
	int	BigCombatPotential()	{return (UByte)((hideskills>>HScombatpotential)&HSmask);}
	SkillType	FlyingSkill()		{return (SkillType)(UByte)((hideskills>>(HSflyingskill+HStoskill))&HSskillmask);}
	SkillType	CombatSkill()		{return (SkillType)(UByte)((hideskills>>(HScombatskill+HStoskill))&HSskillmask);}
	void	SetFlySkill(SkillType s)	{hideskills=HideSkills((hideskills&~(HSmask<<HSflyingskill))+(s<<(HSflyingskill+HStoskill)));}//DAW 26May99
	void	SetCombSkill(SkillType s)	{hideskills=HideSkills((hideskills&~(HSmask<<HScombatskill))+(s<<(HScombatskill+HStoskill)));}//DAW 26May99
	void	SetBigFlySkill(int s)	{hideskills=HideSkills((hideskills&~(HSmask<<HSflyingskill))+(s<<HSflyingskill));}//DAW 26May99
	void	SetBigCombSkill(int s)	{hideskills=HideSkills((hideskills&~(HSmask<<HScombatskill))+(s<<HScombatskill));}//DAW 26May99
#endif
	pilotstatus	status;
	MoraleType	morale;
	RankType	rank;
	SWord		missionscompleted;
	NextFollAct	nextfollact;
	SWord	dayssincerest; //actually periods.
	SWord	kills;
	SWord	airkillslastmsn;
	SWord	daystorecover;
	SWord	aircraftnum;
	UByte	decal;
	UByte	grndhitslastmiss;
};



 	enum		{MAX_PACKAGE_NUM = 5,MAX_COMPONENT_NUM = 4};

	enum	FamilyType{PFT_NULL = 4,
					PFT_HEAVYSTRIKE = 0,
					PFT_STRIKE = 1,		
					PFT_PATROL = 2,		
					PFT_CAS = 3,
					PFT_ALIGN = 0xffff
					};
inline	SWord&	operator *(FamilyType& f){return(SWord&) f;}
					

	enum	Type{
			HS_BASE = 0,	
			HS_ALPHADIRECT = HS_BASE,	
			HS_ALPHAINDIRECT,	
			HS_BETADIRECT,		
			HS_BETAINDIRECT,	
			HS_UNESCORTED,		
			HS_UNESCORTEDNIGHT,
			S_BASE,	
			S_BOMB = S_BASE,	
			S_ROCKET,	
			S_CRACKBURN,
			S_NAPALM,	
			S_STRAFF,
			PTRL_BASE,		
			PTRL_BARCAP = PTRL_BASE,		
			PTRL_TACTICALINTERDICT,
			PTRL_AGGCAP,
			CAS_BASE,	
			CAS_BOMB = CAS_BASE,	
			CAS_ROCKET,	
			CAS_CRACKBURN,
			CAS_NAPALM,	
			CAS_STRAFF,
			T_NULL,
			T_ALIGN = 0xffff
				};

inline	SWord&	operator *(Type& f){return(SWord&) f;}


enum	Stores	{
				GUNSONLY = 0,	B29_STD = 0,
				LR_BOMB = 1,	B29_TARZON = 1,
				LR_HDBOMB,
				LR_ROCKET,
				LR_NAPALM,
				CAS_NB,
				CAS_RB,
				CAS_NR,
				STORES_ALIGN = 0xffff
			};

inline	SWord&	operator *(Stores& f){return(SWord&) f;}

enum	Squadron{
		F86Squad = 0,
		F84Squad,
		F80Squad,
		F51Squad,
		B29Squad,




		};

inline	SWord&	operator *(Squadron& f){return(SWord&) f;}


//MARK, this is the list of events in the order of use			//MS 04Mar99

enum	RedEvents
{
	MIGEV_SANCPAT,
	MIGEV_SAFEPAT,
	MIGEV_INTERSOUTH1,
	MIGEV_STRAIGHTRUN,	//straight run down with no diversions - easy to stop
	MIGEV_INTERCEPT1,	
	MIGEV_BOMBERS1,
//-------------------------------------------------------------------
	MIGEV_INTERSOUTH2,
	MIGEV_INTERCEPT2,
	MIGEV_WAITWEAK,
	MIGEV_CUTWEAK,	
//-------------------------------------------------------------------
	MIGEV_CONTRAILFOCUS,
	MIGEV_BOMBERS2,
//-------------------------------------------------------------------
	MIGEV_INTERSOUTH3,
	MIGEV_INTERCEPT3,
//-------------------------------------------------------------------
	MIGEV_THREELEVELS,
	MIGEV_BOMBERS3,
//-------------------------------------------------------------------
	MIGEV_MOVINGROUND,
	MIGEV_STATICROUND,
//-------------------------------------------------------------------
	MIGEV_ENDRUNDECOY,
	MIGEV_LOWSWEEP,
//-------------------------------------------------------------------
	MIGEV_UPPERCUT,
	MIGEV_INTERCEPT4,
//-------------------------------------------------------------------
	MIGEV_STAIRCASE,
	MIGEV_SUPERIOR1,
//-------------------------------------------------------------------
	MIGEV_SLEDGEHAMMER,
	MIGEV_SUPERIOR2,
//-------------------------------------------------------------------
	MIGEV_WESTTRAIN,
	MIGEV_BIGTRAIN1,
//-------------------------------------------------------------------
	MIGEV_BIGTRAIN2,
	MIGEV_BIGTRAIN3,
	MIGEV_BIGTRAIN4,
	MIGEV_BIGTRAIN5,
	MIGEV_BIGTRAIN6,
	MIGEV_EASTCOAST,
	MIGEV_CENTTRAIN,
	MIGEV_WPOORTRAIN,
	MIGEV_CAREFUL,
	MIGEV_CONFRONT,		//migs attack sabres to clear them
	MIGEV_CAREFUL2,
	MIGEV_MAX
};

struct	MigEvent
{
	FileNum					eventbf,alteventbf;			//2 versions: 50-50 chance
	tAggressionLevel		minlevel;
	int						takeofftime;
	SWord					minformsize,maxformsize;	//both 0 = no limit
	enum					{	TARGET=1,		//goes for a target
								DIRECT=2,		//goes direct - no decoy element
								BARCAP=4,		//goes for barcaps
								LOCAL=8,		//stays in Yalu region
								STARTSOUTH=16,	//starts south of MA
								BIGTRAIN=32,	//Is a bigtrain. If selected first then all missions that day will be that train.
								DECOY=64,		//Has a decoy aspect
								TRAIN_TARGET=33,//Its a big train with a target!
								SOUTH_TARGET=17,//starts south with a target
								DIRECT_TARGET=3,//direct run to a target
								LOCAL_TARGET=9,	//local with a target
								DECOY_TARGET=65,//local with a target
								DECLOC_TARGET=73,
							};	//this enum could be more compact...
	int						eventdoes;		
	tFirstEncounterTactics	firsttactics;
	tThreatLevel			mainthreat, secthreat;
};


struct	SupplyNode;
struct	SupplyRoute;
struct	SupplyLine;

enum	{MAXMISSIONS = 20, MAXDIRECTIVES = 8,  MAXACPERSQUAD = 16, MAX_DIRGROUPS = 5,
				WAVE2_FLAK=3, WAVE2_ESCORT=4};
struct	DirGroup
{
	char		squadron;
	char		numflights;
};

struct	Mission
{
	int			score;
	FileNum		profile;
	char		note;
	UniqueID	target;
	UniqueID	suppliee;
	DirGroup	group[MAX_DIRGROUPS];		//MAX_GROUPS];
	bool		escort;
	bool		missionset;
	SupplyNode*	node;
};

struct	SG_Directive
{
	char		type;
	char		strikeac;
	char		fighterac;
};
struct	Directive:SG_Directive
{
	int			targets;
	int			missionsset;
	Mission 	missions[MAXMISSIONS];
};
struct	DirControlData
{
	bool		autogen;
	bool		autodisplay;
	bool		B29avail;
	bool		alphastrikesallowed;
	char		dayssinceB29;

};
struct	SG_DirControl:DirControlData
{
	SG_Directive	directives[MAXDIRECTIVES];
	void	ResetForNewCampaign();
};
struct	DirControl:DirControlData
{
	Directive	directives[MAXDIRECTIVES];

	CON	DirControl();
		void	AllocateAc();

   enum	{ALPHASTRIKE, MEDIUMBOMBERSTRIKE, FBSTRIKE, UNDECIDEDSTRIKE};

UniqueID targetsalloc[10];		//Profile::MAX_PACKS];
int TotalStrikeAc;
int TotalFighterAC; 

int TotalStrikeAcInUse;
int TotalFighterACInUse; 
int	MissionsAlreadyDefined;
int	TargetsAlreadyDefined;
enum {FBTYPES = 3, ACTYPES = 7};
enum	{ACT_NOTSET = 0,ACT_PROP, ACT_JET};
char	acresource[ACTYPES];
char	acalreadyinuse[ACTYPES];

int firsttype;
DirGroup	tmpgroup[MAX_DIRGROUPS];				//MAX_GROUPS];

int maxscore[7];			//number of directives - resting

char	jetfirst[SQ_B_NONFLYABLE]; 
char	propfirst[SQ_B_NONFLYABLE];


	void	FindScoresAndProfile(int pack, UniqueID uid, SupplyNode* node, int& score, int& maxscore, FileNum& profile, int& type);
	FileNum	FindProfile();
	void	AllocateAcToOneTarget(int pack, UniqueID uid, bool countall);
	void	MakeMiniCampaignMission();								  //RDH 21/03/99
	bool	ModifyRedoIfNecessary(int currpack);			  //RDH 21/06/99

	void AddMission(int d,int j,SupplyNode*  currnode);
	void AddAfMission(int d, int j, int af, int score);
	void ListAirfields(int d);

	void ListSupplyNodes(int d);
	void AddChokeMission(int d, int j, SupplyRoute*  currroute);
	void ListChokes(int d);
	void ListRoutes(int d, SWord type);
	void AddRouteMission(int d, int j, SupplyRoute*  currroute, SWord type);
	void ListFrontLines(int d);

	void	ProcessFrontLine(int d, SupplyLine* sl);
	void	AddCASMission(int d, int j, int score, SupplyLine* sl);


	void	FindTargets(int directive);
	void	AllocateStrike(int d, int i, char& note, int profile);
	void	AllocateFighters(int fightersrequired, int d, int i);

	bool	AllocateTargets(int d, char striketype, bool escortreq);

	void	AircraftAlreadyUsed(int excludedpack, bool countall);
	void	InitAc();

	int 	MedBomberStrikeRequired(int d, int i, UniqueID target);
	bool 	AllocMedBomberStrike(int strike,  int type);
	int 	FBStrikeRequired(int d, int i, UniqueID target);
	int 	EscortRequired(SupplyNode*  node);
	bool	AllocAc(int grptype,int ac, int type, int d);
	void	AllocateAlphaStrike(int d, int i);
	int		StrikeTypeRequired(int d, int i);
	bool	CloseToChina(int node_num, SupplyNode*  node);
	int 	FlakRequired(UniqueID target, SupplyNode*  node);
	void	UpdatePackageAircraft(int packnum, int d, int i);
	void	SetPackagesFromDirectives();
	int		CategoryAvailable(int d, int squad);

	void AutoGenerate();
	int		CalcAcUsedofType(int squad);
	bool	TargetAlreadyAlloc(UniqueID target);
	void	SaveDirectiveChanges();
	void	LoadDirectiveChanges();
	void	InitDirectivesStruct();
	void	ModifyAcInGrp(int packnum, int wavenum, int groupnum, int squad, int numflights);
	int		MakePackage(int d, int i);

};

#pragma pack(pop)
#endif
