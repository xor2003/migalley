#ifndef	AAA_Included
#define	AAA_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

// RERUN moved start
#include "FILES.H" // RERUN added
enum	AaaGunTypes
{
	AAA_HITSTATICS,
	AAA_ANTIAIRCRAFT,
	AAA_ANTITANK,
	AAA_BARAGE,
	AAA_BARAGEGUN
};

struct	AaaParams
{
	AaaGunTypes				guntype;
	Nationality				nat;
	ANGLESInst					minhdg, hdgrange,
		minpitch, pitchrange;
	ANGLESInst					turnpersec;							//JIM 05Dec96
	SLong					minalt, altrange, flatrange;
	SWord					reloadtime;
	UWord					muzzelvel;
	SLong					lifetime;
	ShapeNum				shapenum;
	AutoMoveCodeTypeSelect	movecode;
	FileNum					sfx;								//RJS 22Nov96
	Bool					doinaccel;
};


#define	SPECIALLIFE	0x70000000

extern AaaParams	aaaguntypelist[];
// RERUN moved end

struct	AaaMainList
{
	enum	{SUBLISTSIZE=16};
	struct	AaaSubList
	{
		ItemPtr	aaaitem[SUBLISTSIZE];
		AaaSubList*	next;
		ItemPtr&	operator[](int i) {return (aaaitem[i]);}

	CON	AaaSubList()	{next=this;for (int i=0;i<SUBLISTSIZE;i++)	aaaitem[i]=NULL;}
	};

	ULong	aaaind;
	AaaSubList	*aaalist;

	CON		AaaMainList()	{aaaind=0;aaalist=new AaaSubList;}
	DES		~AaaMainList();	

	void	operator +=(ItemPtr i);
	void	operator -=(ItemPtr i);

};

struct	EventRec;

struct	SleepList:AaaMainList
{
	AaaMainList& Live_List;
	CON	SleepList(AaaMainList& Live):	Live_List(Live) {blockradio=NULL;}
	void	Event();
	void	AddItemEvents(EventRec* evlist);
	AaaSubList* blockradio;
};

struct	LiveList:AaaMainList
{
  	SleepList	sleeplist;
  	SleepList	sleeplistbackup;
	CON		LiveList(WorldStuff* world);
	void	Event();
	void	operator +=(ItemPtr i)	{sleeplist+=i;}	  
	void	BackupSleepList();
	int		ArchiveLiveList(UWord* archive,int archivesize,int skipsize=0);
	void	RestoreSleepList();
	void	ApplyLiveList(UWord* archive,int archivesize);

};	
#pragma pack(pop)
#endif
