/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

//------------------------------------------------------------------------------
//Filename       moveall.cpp
//System         
//Author         Jim Taylor
//Date           Thu 15 Feb 1996
//Description    The code which moves all the items in the world
//				 This module also manages the lists which move the objects
//------------------------------------------------------------------------------

#include "_MOVE.H" //RERUN

#include	"DOSDEFS.H"
	#define F_COMMON
	#define F_GRAFIX
	#define F_BATTLE
//RERUN #include	"files.g"
#include	"FILES.H"
#include	"TEXTREF.H"
#include	"WORLD.H"
#include	"WORLDINC.H"
#include	"FLYMODEL.H"
#include	"TRANSITE.H"
//RERUN #include	"Files.g"
#include	"PERSONS2.H"
#include	"AI.H"
#include	"RANGES.H"
#include	"MYMATH.H"
#include	"MILES.H"
#include	"WINMOVE.H"
#include	"REPLAY.H"
//RERUN #include	"files.g"
#include	"WORLDINC.H"
#include	"PLANETYP.H"
#include	"MYANGLES.H"
#include	"ENUMBITS.M"
#include	"MYTIME.H"
//MATHABLE	(ANGLES)
#include	"MYMATH.H"
#include	"VERTEX.H"
#include	"MATRIX.H"
#include	"TEXTREF.H"
#include	"VIEWSEL.H"
#include	"HARDPASM.H"
#include	"3DCOM.H"
#include	"LANDSCAP.H"
#include	"MISSMAN2.H"
#include	"SAVEGAME.H"
#include	"MILES.H"
#include	"WORLD.H"
#include	"TRANSITE.H"
#include	"COLLIDED.H"
#include	"SPEED.H"
#include	"RANGES.H"
#include	"MODVEC.H"
#include	"MODEL.H"
#include	"SHAPES.H"
#include	"MONOTXT.H"
#include "OVERLAY.H" //RERUN
//#define DEBUGMOVE

MobileItemPtr	mobileitem::ACList=NULL;
MobileItemPtr	mobileitem::MobileList=NULL;

//RERUN: splice an item out of the move lists before it is freed - a stale
//nextmobile left the freed slot to be reused (ShapeNumField) then walked.
void UnlinkFromMoveLists(MobileItemPtr it)
{
	for (MobileItemPtr* pp=&mobileitem::ACList;*pp;pp=&(*pp)->nextmobile)
		if (*pp==it)	{*pp=(*pp)->nextmobile;break;}
	for (MobileItemPtr* pp=&mobileitem::MobileList;*pp;pp=&(*pp)->nextmobile)
		if (*pp==it)	{*pp=(*pp)->nextmobile;break;}
	//RERUN: also clear transient back-links to the freed item - bullets outlive
	//their launcher and PersonalThreat dereferences Launcher unconditionally
	for (TransientItem* t=TransientItem::TransientList;t;t=t->nexttrans)
	{
		if (t->Launcher==(itemptr)it)	t->Launcher=NULL;
		if (t->Target==(itemptr)it)		t->Target=NULL;
	}
}
TransientItemPtr	TransientItem::TransientList=NULL;

//RERUN: splice a transient out of TransientList before it is freed
void TransientItem::UnlinkTransList(TransientItemPtr it)
{
	for (TransientItemPtr* pp=&TransientList;*pp;pp=&(*pp)->nexttrans)
		if (*pp==it)	{*pp=(*pp)->nexttrans;break;}
}
WorldStuff*		mobileitem::currworld=NULL;
UByte	mobileitem::timerseed=0;
int		mobileitem::timeofday=0;

//extern Replay	_Replay;
//extern DPlay _DPlay;

//------------------------------------------------------------------------------
//Procedure		MoveAll
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void mobileitem::MoveAll(WorldStuff& world)
{
//#pragma message("AC move code temporarily disabled...")
//#pragma message("....cos there was 0 frame rate!!!")

  		MoveList(ACList, world);
		MoveList(MobileList, world);	

}

//------------------------------------------------------------------------------
//Procedure		MoveAllTrans
//Author		Martin Alderton
//Date			Wed 20 Mar 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void TransientItem::MoveAllTrans(WorldStuff& world)
{
	MoveTransList(TransientList, world);
}

static UByte dontmove=0;
//------------------------------------------------------------------------------
//Procedure		MoveList
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void mobileitem::MoveList(MobileItemPtr entry,WorldStuff& world)
{
//	static x=0;
	currworld=&world;											//PD 12Jul96

#ifdef DEBUGMOVE
	FILE* fp;
	if (_Replay.Record)
	{
		fp=fopen("record.txt","at");
	}
	else if (_Replay.Playback)
	{
		fp=fopen("playback.txt","at");
	}
	else if (_DPlay.Implemented)
	{
		fp=fopen("commsac.txt","at");
	}
#endif

	while (entry)
	{
		MobileItemPtr	next=entry->nextmobile;
		UWord	oldsector=-1;										  //JIM 17/02/99
		if (entry->AutoMoveInfo.movecode!=AUTO_NOPPILOT)								  //JIM 17/02/99
			oldsector=	world.GetSector(*entry);					  //JIM 17/02/99
		else
			oldsector=-1;

		if (entry==Persons2::PlayerGhostAC)
		{
			INT3;
		}
		else if (entry==Persons2::PlayerSeenAC)
		{
// should go in here to keep ghostac upto date 

#ifdef DEBUGMOVE
			if (_Replay.Record || _Replay.Playback || _DPlay.Implemented)
			{
				UWord uid=entry->uniqueID.bitfieldUid.count;
				fprintf(fp,"%d ID=%x X=%d Y=%d Z=%d H=%d P=%d R=%d V=%d (ghost)\n",
					_Replay.replayframecount,
					uid,
					Persons2::PlayerGhostAC->World.X,
					Persons2::PlayerGhostAC->World.Y,
					Persons2::PlayerGhostAC->World.Z,
					Persons2::PlayerGhostAC->hdg.a,
					Persons2::PlayerGhostAC->pitch.a,
					Persons2::PlayerGhostAC->roll.a,
					Persons2::PlayerGhostAC->vel
					);
			}
#endif

			// want to update GoodAbsPos with predicted movement
			_DPlay.AddDeltas(Persons2::PlayerGhostAC,TRUE);
			_DPlay.PredictMove(Persons2::PlayerGhostAC);
			_DPlay.AddDeltas(Persons2::PlayerGhostAC,FALSE);
		}
		else
		{
#ifdef DEBUGMOVE
			if ((_Replay.Record || _Replay.Playback || _DPlay.Implemented) && entry->MigStatus.LaunchFlags.size==AIRSTRUCSIZE )
			{
				ULong m=entry->AutoMoveInfo.movecode;
				ULong man=((AirStrucPtr)entry)->ai.PilotSkills.manoeuvre;
				UWord uid=entry->uniqueID.bitfieldUid.count;
				UByte inform=((AirStrucPtr)entry)->information;

				fprintf(fp,"ID=%x X=%d Y=%d Z=%d H=%d P=%d R=%d V=%d inform=%d move=%d man=%d\n",
						uid,
						entry->World.X,
						entry->World.Y,
						entry->World.Z,
						entry->hdg.a,
						entry->pitch.a,
						entry->roll.a,
						entry->vel,
						inform,
						m,
						man
						);
			}

#endif
  			if (entry->uniqueID.bitfieldUid.commsmove)
 			{
				if (entry->shape!=EMPTY)
				{
// AC with empty shape are not in world, so dont move them

					if (!mobileitem::currworld->vp->Accel())
					{
// not in accel predict 

						_DPlay.PredictMove((AirStrucPtr)entry);
					}
					else
					{
// in accel need to move as no delta packets will arrive

						entry->AutoMoveInfo.movecode=AUTO_COMMSACCEL;
						MoveItem(entry,world);
					}

// update model pos.Y so that ac leaves behind contrails?

#if 0 // RERUN Removed. Unreliable and ac leaves contrails anyway.
					if (((AirStrucPtr)entry)->fly.pModel)
						((AirStrucPtr)entry)->fly.pModel->Pos.y=((AirStrucPtr)entry)->World.Y;
#endif //RERUN
				}
			}
			else
			{
				ANGLES currhdg=entry->hdg;						//AMM 31Jul98

				MoveItem(entry,world);

				if (entry->uniqueID.bitfieldUid.changed)					//AMM 31Jul98
				{												//AMM 31Jul98
					if (entry->MigStatus.LaunchFlags.size==AirStrucSize)		//AMM 31Jul98
					{											//AMM 31Jul98
						((AirStrucPtr)(entry))->fly.dhdg= (SWord)(entry->hdg - currhdg);			//AMM 31Jul98
					}											//AMM 31Jul98
				}												//AMM 31Jul98
			}
		}
#ifndef	NDEBUG
		if (!(entry->uniqueID.bitfieldUid.changed)						//AMM 31Jul98
			&& oldsector!=0xffff
			&& (oldsector!=world.GetSector(*entry)))				//AMM 31Jul98

		{
#ifdef __WATCOMC__
				INT3();
#endif
#ifdef __MSVC__
				INT3;
#else
			assert(FALSE);
#endif
		}
#endif
		if (entry->uniqueID.bitfieldUid.changed && oldsector!=0xFFFF)				  //JIM 17/02/99
		{
			UWord	newsector=	world.GetSector(*entry);
			if (newsector!=oldsector)
			{
				world.RemoveFromSector(entry,oldsector);
				world.AddToWorld(entry);
			}
			entry->uniqueID.bitfieldUid.changed = FALSE;						//RDH 26Jun96
		}
		else
			oldsector=oldsector;
		entry=next;												//RDH 23Feb96
	}
#ifdef DEBUGMOVE
	if (_Replay.Record || _Replay.Playback || _DPlay.Implemented)
	{
		fprintf(fp,"\n");
		fclose(fp);
	}
#endif
}


//------------------------------------------------------------------------------
//Procedure		MoveList
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#ifndef	NDEBUG
	  int	Count_SetFlightParams=0;
#endif
void mobileitem::MoveItem(MobileItemPtr entry,WorldStuff& world)
{
	switch (entry->AutoMoveInfo.movecode)
	{
	case AUTO_TRACKINGBOGIE:
		((AirStrucPtr) *entry)->TrackBogey();
		break;
	case AUTO_TRACKINGBANDIT:
		((AirStrucPtr) *entry)->TrackBandit();
		break;
	case AUTO_PRECOMBAT:
		((AirStrucPtr) *entry)->AutoPreCombat();
		break;
	case AUTO_FOLLOWWP:												  //RDH 13/06/99
	case AUTO_TELLLEADER:										//JIM 03Oct96
		((AirStrucPtr) *entry)->AutoFollowWp();
		break;
	case AUTO_FAC:
		((AirStrucPtr) *entry)->AutoFAC();
		break;
	case AUTO_CAS:
		((AirStrucPtr) *entry)->AutoCAS();
		break;
	case AUTO_CRASHROLL:										//RDH 12Sep96
		((AirStrucPtr) *entry)->AutoCrashRoll();						//RDH 12Sep96
		break;
	case AUTO_CRASHSKID:										//RDH 12Sep96
		((AirStrucPtr) *entry)->AutoCrashSkid();				//RDH 12Sep96
		break;
	case AUTO_TAKEOFF:
		((AirStrucPtr) *entry)->AutoTakeOff();
		break;
	case AUTO_COMBAT:
		((AirStrucPtr) *entry)->AirCombat();
		break;
	case AUTO_NOPPILOT:
		if (entry==Persons2::PlayerSeenAC)						//AMM 01Dec98
		{
			_DPlay.ResurrectMe(Persons2::PlayerSeenAC,false);
		}
		break;
	case AUTO_WAIT4TIME:
		break;
	case GROUND_TAXI:
		((FormationItemPtr) *entry)->AutoTaxi();
		break;
	case GROUND_TOGLOBREF:
		((FormationItemPtr) *entry)->WaitToTaxi();
		break;
//	case AUTO_TAXI:
//		((AirStrucPtr) *entry)->AutoTaxi();
//		break;
	case AUTO_SPIRAL2GROUND:
		((AirStrucPtr) *entry)->AutoSpiral2Ground(world);		//PD 01May96
		break;
	case AUTO_RESURRECT:
		((AirStrucPtr) *entry)->AutoResurrect((AirStrucPtr)entry);			//ARM 05Dec96
		break;
	case AUTO_DEATHSEQUENCE:
		((AirStrucPtr) *entry)->AutoDeathSequence();			//RDH 10Sep96
		break;
	case AUTO_LANDING:											//JIM 02Aug96
		((AirStrucPtr) *entry)->AutoLanding();				//JIM 02Aug96
		break;													//JIM 02Aug96
	case AUTO_STRAFFE:
		((AirStrucPtr) *entry)->AutoStraffe();				//JIM 02Aug96
		break;													//JIM 02Aug96
	case AUTO_BOMB:
		((AirStrucPtr) *entry)->AutoBomb();				//JIM 02Aug96
		break;													//JIM 02Aug96
	case AUTO_HITWATER:											//RJS 24Aug98
		((AirStrucPtr) *entry)->AutoCrashSkid();				//RJS 24Aug98
		break;													//RJS 24Aug98
	case AUTO_ACCELERATE:
		((AirStrucPtr) *entry)->AutoAccelerate();
		break;
	case AUTO_MISSIONDICETHROW:
		((AirStrucPtr) *entry)->MoveMissionDiceThrow();
		break;
	case AUTO_COMMSACCEL:
		((AirStrucPtr) *entry)->AutoCommsAccel();
		break;
	}
}

//------------------------------------------------------------------------------
//Procedure		MoveList
//LastModified:	PD 26Apr96
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
extern void	CheckE3();
void TransientItem::MoveTransList(TransientItemPtr entry,WorldStuff& world)
{
	currworld=&world;
	while (entry)
	{
		TransientItemPtr	next= entry->nexttrans;
		if (entry->AutoMoveInfo.movecode != MOBILE_NOPPILOT)						//RJS 09Dec98
		{
			UWord	oldsector=	world.GetSector(*entry);			//PD 25Jun96
	
			MoveTransItem(entry,world);
	
			if (oldsector!=world.GetSector(*entry))					//PD 25Jun96
			{
				world.RemoveFromSector((TransientItemPtr)entry,oldsector);
				world.AddToWorld((TransientItemPtr)entry);
			}
		}

		entry=next;
	}
}


//------------------------------------------------------------------------------
//Procedure		MoveList
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void TransientItem::MoveTransItem(TransientItemPtr entry,WorldStuff& world)//PD 26Apr96
{
#ifdef	DBGMEMTEST
#ifndef	NDEBUG
	TransientItem*	tmpentry = TransientList;
	while (tmpentry)
	{
		DbgMemTest2(tmpentry);
		tmpentry->Anim.MemTest();
		tmpentry = tmpentry->nexttrans;
	}
#endif
#endif
	switch (entry->AutoMoveInfo.movecode)// & AutoMoveCodeMask)					//RJS 24Nov97
	{
  	case	MOBILE_BURNIT:			Trans_Obj.MobileExplosion(entry,world);	break;//PD 26Apr96
 	case	MOBILE_CRATER:			Trans_Obj.MobileCrater(entry,world);	break;//PD 26Apr96
	case	MOBILE_BULLET:			Trans_Obj.MobileBullet(entry,world); 	break;//PD 26Apr96
	case    MOBILE_DEADACPART:		Trans_Obj.MobileDeadACPart(entry,world);break;//PD 26Apr96
	case 	MOBILE_SMOKETRAIL:		Trans_Obj.MobileSmokeTrail(entry,world);break;//PD 30Apr96
	case 	MOBILE_GANDF:			Trans_Obj.MobileGAndF(entry,world);		break;//PD 30Apr96
	case 	MOBILE_GROUNDEXP:		Trans_Obj.MobileGroundExp(entry,world);	break;//PD 01May96
	case 	MOBILE_DEBRIS:			Trans_Obj.MobileGroundDebris(entry,world);	break;//RJS 21Jun96
	case 	MOBILE_FIRE:			Trans_Obj.MobileFire(entry,world);	break;//RJS 01Jul96
	case 	MOBILE_TROOP:			Trans_Obj.MobileTroop(entry,world);	break;//RJS 01Jul96
	case 	MOBILE_PARACHUTE:		Trans_Obj.MobileParachute(entry,world);	break;//RJS 02Jul96
	case 	MOBILE_BARAGE:			Trans_Obj.MobileBarage(entry,world);	break;//RJS 02Jul96
	case 	MOBILE_DELEXP:			Trans_Obj.MobileDelayedExplosion(entry,world);	break;//RJS 02Jul96
	case 	MOBILE_FLAK:			Trans_Obj.MobileFlak(entry,world);	break;//RJS 02Jul96
	case 	MOBILE_RICOCHET:		Trans_Obj.MobileSparkTrail(entry,world);	break;//RJS 11Dec98
	case 	MOBILE_ATEAM:			Trans_Obj.MobileATeam(entry,world);	break;//RJS 10Jul96
	case 	MOBILE_BOMBDROP:		Trans_Obj.MobileBombDrop(entry,world);	break;//RJS 24Jul96
	case 	MOBILE_BIGEXP:			Trans_Obj.MobileBigExplosion(entry,world);	break;//RJS 12Aug96
	case 	MOBILE_MOVSMK:			Trans_Obj.MobileMovingSmoke(entry,world);	break;//RJS 02Sep96
	case 	MOBILE_STATIONARY:		Trans_Obj.MobileRicochetSmoke(entry,world);	break;//RJS 05Sep96
	case 	MOBILE_PUFFTRAVEL:		Trans_Obj.MobilePuffTravel(entry,world);	break;//RJS 13Oct96
	case 	MOBILE_BIRD:			Trans_Obj.MobileBird(entry,world);	break;//RJS 01Oct97
	case 	MOBILE_AMMODEATH:		Trans_Obj.MobileAmmoBoxes(entry,world);	break;//RJS 17Oct96
	case 	MOBILE_MUSHROOM:		Trans_Obj.MobileDummyBomb(entry,world);	break;//RJS 01Apr99
	case 	MOBILE_ROCKET:			Trans_Obj.MobileRocket(entry,world);	break;//RJS 04Jun97
	case 	MOBILE_CLONEGUN:		Trans_Obj.MobileCloneGun(entry,world);	break;//RJS 01Oct97
	case 	MOBILE_TIMEREXPLOSION:	Trans_Obj.MobileTimerExplosion(entry,world);	break;//RJS 22Jan98
	case 	MOBILE_NAPALM:			Trans_Obj.MobileNapalm(entry,world);	break;//RJS 22Jan98
	case 	MOBILE_DODAMAGE:		Trans_Obj.MobileDoDamage(entry,world);	break;//RJS 18Feb98
	case 	MOBILE_THUG:			Trans_Obj.MobileThuggery(entry,world);	break;//RJS 02Mar98
	case 	MOBILE_SMALLTHUG:		Trans_Obj.MobileSmallThug(entry,world);	break;//RJS 03Dec98
	case 	MOBILE_FUELDROP:		Trans_Obj.MobileFuelDrop(entry,world);	break;//RJS 28May98
	case 	MOBILE_DEADBRIDGE:		Trans_Obj.MobileDeadBridge(entry,world);	break;//RJS 30Oct98
	case 	MOBILE_COLLAPSE:		Trans_Obj.MobileCollapse(entry,world);	break;//RJS 18Nov98
	case 	MOBILE_CARPETBOMB:		Trans_Obj.MobileCarpetStrike(entry,world);	break;//RJS 31May99
	case 	MOBILE_SCALEFIST:		Trans_Obj.MobileScaleFist(entry,world);	break;//RJS 10Feb99
	case 	MOBILE_CANOPY:			Trans_Obj.MobileCanopy(entry,world);	break;//RJS 23Mar99
	case 	MOBILE_SINK:			Trans_Obj.MobileSink(entry,world);	break;//RJS 13May99
	case 	MOBILE_FLARE:			Trans_Obj.MobileFlare(entry,world);	break;//RJS 17May99

	}
#ifdef	DBGMEMTEST
#ifndef	NDEBUG
	tmpentry = TransientList;
	while (tmpentry)
	{
		DbgMemTest2(tmpentry);
		tmpentry->Anim.MemTest();
		tmpentry = tmpentry->nexttrans;
	}
#endif
#endif
}

//------------------------------------------------------------------------------
//Procedure		AddToList
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::AddToList()
{
	nextmobile=ACList;
	ACList=*this;
	Art_Int.Add(this);
}

void mobileitem::AddToList()
{
	nextmobile=MobileList;
	MobileList=*this;
}

void TransientItem::AddToList()
{
	nexttrans=TransientList;
	TransientList=this;

// transients have ID equal to frame when launched. When a collision occurs the player 
// who processes the collision (owner of transient) send a message indicating that all
// transients with that frame number launched by him should be removed from the world

	this->uniqueID.bitfieldUid.count=(UniqueID)_DPlay.FrameCount;

// if this was launched by a transient then it inherits its parents flags

	if (Launcher->MigStatus.LaunchFlags.size==TRANSIENTSIZE)
	{
		this->CollTestEnabled=((TransientItemPtr)Launcher)->CollTestEnabled;
		this->CollSuccessTrans=((TransientItemPtr)Launcher)->CollSuccessTrans;
	}
	else
	if ((Launcher==Persons2::PlayerSeenAC && !_Replay.Playback)
	|| (!_DPlay.Implemented && !_Replay.Record && !_Replay.Playback))
	{
// comms/replay off or player launched 
// if replay is turned on then these bullets will cause collisions to be recorded
// even though bullets are not seen on replay

		this->CollTestEnabled=TRUE;
		this->CollSuccessTrans=TRUE;
	}
	else if (!Launcher->uniqueID.bitfieldUid.commsmove && Launcher!=Persons2::PlayerSeenAC)
	{
// AI launched - want to test all AI bullets against all other AI planes and against player
// unless in playback, which is dealt with in PossTransmitColl

		this->CollTestEnabled=TRUE;

// if this hits player then collision will be sent if in record or comms. Other collisions do
// not need to be sent as they are AI against AI, or AI against comms player in which case
// that player will send his own collision info packets. 

// if this hits a player then check CommsOwner ID. If it is me then trasmit, otherwise ignore.

		this->CollSuccessTrans=FALSE;
	}
	else
	{
// comms received - dont test for anything just show them. All player collisions are dealt with
// via separate packet info

		this->CollTestEnabled=FALSE;
		this->CollSuccessTrans=FALSE;
	}
}

//------------------------------------------------------------------------------
//Procedure		ExecuteWayPointAction
//Author		R. Hyde 
//Date			Mon 19 Feb 1996
//
//Description	Now pseudo recursive. Compiler should optimise to itteration.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::ExecuteWayPointAction ()

{
	AutoMoveCodeTypeSelect	oldmove=AutoMoveInfo.movecode;
	WayPointPtr	oldwp=waypoint;

	//NULL WP MEANS START TO LAND...
	if (waypoint)
		DoWayPointAction(oldmove,oldwp,waypoint->action);
	else
		DoWayPointAction(oldmove,oldwp,wpforcelanding);
	if (MigStatus.LaunchFlags.size==AIRSTRUCSIZE)
	{
		AirStrucPtr a=*this;
		if (	a->ai.eventlog->stage==EventLog::TAKEOFF
			&&	waypoint && waypoint->target
			)
		{
			if	(	waypoint->action==wpbombno
				||	waypoint->action==wpengagetargetno
				||	waypoint->action==wpdivebombstraffeno
				||	waypoint->action==wpballoonbustno
				)
			{
				_Miles.SequenceAudible(FIL_MUSIC_OBJECTIVE_SIGHTED);
			}
		}
	}
}

void	mobileitem::DoWayPointAction (AutoMoveCodeTypeSelect	oldmove,WayPointPtr	oldwp,BFieldWaypointActions action)
{

	if (AutoMoveInfo.movecode==oldmove && waypoint==oldwp)
	{
		switch (action)
		{
		case	wpatakeoffno:
		case	wpacircleno:
		case	wptruckbridge:
		case	wptruckescort:
				NextWayPoint ();
		break;
//
//	PROPER WAYPOINT ACTIONS START HERE:
//	the ones above fall through to noaction
//
		case	wpacnoactionno:
				NextWayPoint ();
		break;

		case	wpdmbtrknoactno:
				NextWayPoint ();
				action=wpretrywp;
		break;

		case	wpballoonbustno:											//MS 18Jul96
		case	wpdivebombstraffeno:										//MS 18Jul96
			StartStraffe();
		break;

		case	wpbombno:													//MS 18Jul96
		case	wpengagetargetno:
		case	wpattackconvoyno:
			StartBombing();
		break;

		case	wpreccyno:										//JIM 17Dec96
 		case	wpartspotno:									//JIM 17Dec96
				KillItNow();
		break;

		case	wpteleportnext:
			TeleportWayPoint();
		break;

		case	wpstoploopchkno :							//JIM 30Jul96
			action=EndLoopWayPoint();								//JIM 30Jul96
		break;												//JIM 30Jul96

		case	wpbreakloop :							//JIM 30Jul96 //JIM 19/03/99
		case	wpstartloopno:										  //JIM 19/03/99
			action=MidLoopWayPoint();								//JIM 30Jul96
		break;												//JIM 30Jul96
		
		case	wpforcebreakloop:
			ForceBreakLoopWayPoint();
		break;

		case	wpwaituntildeadno :							//JIM 30Jul96
			WaitDeadWayPoint();								//JIM 30Jul96
		break;												//JIM 02Aug96
		case	wpalandingno:								//JIM 02Aug96
			LandingWayPoint();								//JIM 02Aug96
		break;												//JIM 02Aug96
		case	wpdmbtrkstopno:
		case	wpstopno:									//RDH 17Oct96
			velx=vely=velz=0;
			AutoMoveInfo.movecode=AUTO_NOPPILOT;
		break;
		case wpforcelanding:
		{
			AutoMoveInfo.movecode = AUTO_LANDING;
			if (Anim!=NULL)
				SHAPE.NavigationLightsActive(this, TRUE);
			AirStrucPtr AC = *this;
			AC->ai.PilotSkills.ManStep = 0;
			AC->SetManoeuvreTime(0);
			
			if(AC->formpos == 0)
			{
				AirStrucPtr ThePlayer = AC->PlayerInGroup();
				if((ThePlayer) && (AC != ThePlayer))
				{
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_STEER_RANGE, MSG_STATUSRESPONSE_DELAY_LP_RPT, VOICE_TOWER,
										AC->ai.homebase, AC));

			 		if(Save_Data.flightdifficulty [FD_WINDGUSTS])
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_SURFACEWIND_BEARINGNO_SPEED_GUSTS, MSG_STATUSRESPONSE_DELAY_LP_RPT,
										VOICE_TOWER, NULL, AC));
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_SURFACEWIND_BEARINGNO_SPEED, MSG_STATUSRESPONSE_DELAY_LP_RPT,
										VOICE_TOWER, NULL, AC));
				}
			}
			break;
		}

		case	wp_start_cas:
			NextWayPoint();
			if (	(AirStrucPtr(*this)->FormationInfo.duty&DUTYMASK)==DC_CAS 
				&& (AirStrucPtr(*this)->FormationInfo.duty&ACTIONCODE)!=DA_CAP
				)
				AirStrucPtr(*this)->SetToCAS(true);
		break;

		case	wp_threat_combfriendly:
		case	wp_threat_engfriendly:
		case	wp_threat_firing:
		case	wp_threat_weapon:
		case	wp_threat_danger:
		case	wp_threat_engage:
		case	wp_threat_any:
			assert (MigStatus.LaunchFlags.size==AIRSTRUCSIZE);
			AirStrucPtr(*this)->ai.threatlevel=tThreatLevel(action-wp_threat_combfriendly);
			NextWayPoint();
		break;


		case	wpretrywp:
		break;												//RDH 17Oct96
		}
	}

		if (MigStatus.LaunchFlags.size>=FormationSize)							//JIM 04Aug96
		{	//find a child and recurse down it					//JIM 04Aug96
			FormationItemPtr	f=*this;						//JIM 04Aug96
			if (f->follower)									//JIM 04Aug96
			f->follower											//JIM 04Aug96
				->DoWayPointAction(oldmove,oldwp,action);				//JIM 04Aug96
			elseif (MigStatus.LaunchFlags.size==AirStrucSize)					//JIM 04Aug96
				if (	f->leader								//JIM 04Aug96
					&& ((AirStrucPtr)*f->leader)->fly.nextflight)//JIM 04Aug96
					((AirStrucPtr)*f->leader)->fly.nextflight	//JIM 04Aug96
						->DoWayPointAction(oldmove,oldwp,action);		//JIM 04Aug96
				elseif (((AirStrucPtr)*this)->fly.nextflight)	//JIM 04Aug96
					((AirStrucPtr)*this)->fly.nextflight		//JIM 04Aug96
						->DoWayPointAction(oldmove,oldwp,action);		//JIM 04Aug96
		}														//JIM 04Aug96
		//else finished											//JIM 04Aug96

}


//------------------------------------------------------------------------------
//Procedure		CheckManualCourse
//Author		Jim Taylor
//Date			Sun 21 Jul 1996
//
//Description	
//				Basically does equivalents to auto AutoMoveInfo.movecode for player
//
//				Checks if manual is on course and has reached waypoints
//				Checks for all break off
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::CheckManualCourse()
{
	ANGLES	currintercept,originalintercept;
	SLong	range,dprange;

	COORDS3D* dp = NULL;
	if (Manual_Pilot.controlmode==Manual_Pilot.PILOTDEAD)
		return;
	if (AutoMoveInfo.movecode==AUTO_PRECOMBAT)
		if (ai.unfriendly==NULL)
			if (formpos)
			{
				InterceptandRange (dp=PositionWRTLeader ());
				range=Range;
				currintercept=originalintercept=HdgIntercept;
				dprange=0;
			}
			else
				AutoMoveInfo.movecode=AUTO_FOLLOWWP;
		else
		{

			dp=&ai.unfriendly->World;

			dprange=METRES500;
			InterceptandRange (dp);
			if (Range>dprange)
				InterceptandRange (dp=FindCombatPos());
			else
				currintercept=originalintercept=HdgIntercept;
			currintercept=originalintercept=HdgIntercept;
			range=Range;
		}
	elseif (AutoMoveInfo.movecode==AUTO_COMBAT)
	{
		if (formpos)
			if (fly.leadflight->AutoMoveInfo.movecode!=AUTO_COMBAT)
			{
				AutoMoveInfo.movecode=(AutoMoveCodeTypeSelect)fly.leadflight->AutoMoveInfo.movecode;
			}
			elseif (	fly.leadflight->ai.PilotSkills.manoeuvre==MANOEUVRE_TOPCOVER
					&&	ai.PilotSkills.manoeuvre!=MANOEUVRE_TOPCOVER
					)
			{
				ai.PilotSkills.manoeuvre=MANOEUVRE_TOPCOVER;
				if (	ai.unfriendly
					&&	ai.unfriendly->MigStatus.LaunchFlags.size==AIRSTRUCSIZE
					&&	((AirStrucPtr)ai.unfriendly)->ai.attacker==this
					)
					((AirStrucPtr)ai.unfriendly)->ai.attacker=NULL;
				ai.unfriendly=NULL;
			}

		if (ai.unfriendly)
		{
			InterceptandRange (dp=&ai.unfriendly->World);
			range=Range;
			currintercept=originalintercept=HdgIntercept;
			dprange=METRES100;
		}
		else
		{
			if (	//ai.PilotSkills.manoeuvre==MANOEUVRE_TOPCOVER &&			//JIM 03Dec96
				formpos==0)
				if (AllOnTopCover())
				{
					Art_Int.AllBreakOff(this);									//JIM 16Jul96
				}
				elseif (AllButLdrNotOnCombat ())
				{
					ai.PilotSkills.manoeuvre=MANOEUVRE_TOPCOVER;
				}

			return;
		}
	}
	elseif (AutoMoveInfo.movecode==AUTO_BOMB || AutoMoveInfo.movecode==AUTO_CAS)
	{
		if (!ai.unfriendly)
		{
			if (AutoMoveInfo.movecode==AUTO_BOMB)
				AutoMoveInfo.movecode=AUTO_FOLLOWWP;
		}
		else
		{
			if (!SelectedElement(ai.unfriendly,ai.PilotSkills.manoeuvre))
			{
				int newelt;
				ItemPtr trg=SelectTarget(ai.unfriendly,&newelt);
				if (!SelectedElement(trg,newelt))
					trg=NULL;
				OverLay.DecisionMessage(FACMsg,FAC_BombsAway,this,trg,this,FALSE);	//RJS 09Jun99
			}
		}
		OverLay.CancelAccel();
		range = 1;		//CSB 13/07/99	Force it to fail later test
		dprange = 0;	//CSB 13/07/99	
	}
	elseif (!FlyEscortTo())
	{	//followwp
		InterceptandRange (dp=FindDesPos ());
		dprange=desposrange+MILES01;	//make it easy for player
		if (PitchIntercept>ANGLES_85Deg)
			Range=0;
		if (waypoint && !formpos && fly.leadflight && fly.leadflight->waypoint==waypoint->next)
			dprange+=METRES10KM;			//escortee just gone through. See AutoFollowWP
		originalintercept=currintercept=HdgIntercept;
		range=Range;
		WayPointPtr	wp=fly.backupWP;
		if (wp==NULL)
			wp=waypoint;
		if (AutoMoveInfo.movecode==AUTO_TAXI && vel>5*60)					//RDH 11Dec96
			AutoMoveInfo.movecode=AUTO_FOLLOWWP;								//RDH 11Dec96

	}
	else if(!MigStatus.LaunchFlags.deadtime)	//CSB 09/06/99	
	{	//followwp wing
		InterceptandRange (dp=PositionWRTLeader ());
		range=Range;
		currintercept=originalintercept=HdgIntercept;
		dprange=0;
		if (range>METRES100)
		{
			FormationInfo.information=FALSE;
			AirStrucPtr leadac=Leader();
			if (!leadac)
				leadac=fly.leadflight;
			leadac->FormationInfo.slowdownleader=TRUE;
			if (fly.leadflight)
				fly.leadflight->FormationInfo.slowdownleader=TRUE;

			int r = (int)FSqrt(range/METRES500);				//CSB 07/06/99	
			if (ai.desiredrange)									  //JIM 18/05/99
				if (r>ai.desiredrange)								  //JIM 18/05/99
					if (r<8)
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CLOSER, (MsgType)(MSG_FORMATIONFLYING+MSG_REPEAT), leadac, NULL, this));		//CSB 26/05/99	
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_GETBACK, (MsgType)(MSG_FORMATIONFLYING+MSG_REPEAT), leadac, NULL, this));		//CSB 26/05/99	
			ai.desiredrange=r;										  //JIM 18/05/99

		}
		else
			FormationInfo.information=TRUE;

	}

	//now what?
	if (range<dprange)											//JIM 20Aug96
	{	//reached waypoint near enough							//JIM 20Aug96

		if (AutoMoveInfo.movecode<AUTO_PRECOMBAT)				//JIM 20Aug96
		{														//JIM 20Aug96
			//tests for special waypoints

			if (!waypoint || waypoint->MigStatus.LaunchFlags.deaded)			//AMM 30Nov98
			{
				ExecuteWayPointAction();
			}
			else
			{
				ExecuteWayPointAction();
			}
		}
		elseif (AutoMoveInfo.movecode==AUTO_PRECOMBAT)
		{	//switch to engage
			Art_Int.AllEngage(this);
		}
	}
	elseif (range>METRES500+dprange)
	{
		despos=*dp;
	}
	if (AutoMoveInfo.movecode != AUTO_COMBAT)
	{//don't want this message when in combat						  //RDH 30/04/99
		ItemBasePtr corner=Persons2::ConvertPtrUID(UID_StNamsanni);
		if (corner->World.X>World.X)
		{	//sinuiju corner
			if ((corner->World.X-World.X)>(corner->World.Z-World.Z))
			{
				AirStrucPtr buddy=FindBuddy();
				if (buddy)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CROSSRIVER, MSG_CROSSRIVER, buddy, NULL, this));
			}
		}
		else
		{	//north korea
			if (corner->World.Z<World.Z)
			{
				AirStrucPtr buddy=FindBuddy();
				if (buddy)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CROSSRIVER, MSG_CROSSRIVER, buddy, NULL, this));
			}
		}
	}
}
