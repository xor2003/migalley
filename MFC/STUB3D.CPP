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

//
//WinMode:	stub3d.cpp
//
//TASK:		manages the 3d thread
//
//ROUTINES:	presently about 5 routines:
//
//	MakeInteractive(WinMode,Coords)
//	MakePassive(WinMode,Coords)
//	MakeResize(WinMode,Coords)
//	ToggleInteractive(bool)
//	TogglePaused(bool)
//	ToggleAccel(bool)
//	DeleteWorld()
//	
//	WinMode:	
//		FullScreen/Windowed/DualScreen 
//							0x0000,0x1000,0x2000
//		Accelerated			0x000=no,0x100=yes,
//							0x10n=specific card									
//							0x1m0=specific mode

#include "WIN32_COMPAT.H" //RERUN
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "RESOURCE.H" //RERUN
#define DIRECTINPUT_VERSION 0x0700 // RERUN
#include "dinput_stub.h"
#include "mssw_compat.h"
#include "DOSDEFS.H"

#include "STUB3D.H"
#include "KEYTEST.H"

#include	"HARDPASM.H"
#include	"WORLDINC.H"
#include	"AAA.H"
#include	"FLYMODEL.H"
#include	"MYTIME.H"
#include	"AI.H"
#include	"PERSONS2.H"
#include	"COLLIDED.H"
#include	"3DCODE.H"
#include	"WORLD.H"
#include	"SHAPES.H"
#include	"3DINSTR.H"
#include	"SAVEGAME.H"
#include	"WINMOVE.H"
#include	"MILES.H"
#undef LPDDCAPS
#include	"WIN3D.H"
#include	"POLYGON.H"
#include	"ANALOGUE.H"
#include	"TRANSITE.H"
#include	"VIEWSEL.H"
#include	"RCHATTER.H"										//RJS 11Jun98
#include	"REPLAY.H"
#include	"GLOBREFS.H"
#include	"REPLAY.H"
#include	"AGGRGTOR.H"
#include	"OVERLAY.H"
#include	"MESSENGN.H"										//RJS 27Jan99
#include	"3DCOM.H"											//RJS 08Apr99

#include "GAMESET.H" //RERUN
#include "DEBRIEF.H" //RERUN
#include "MISSMAN2.H" //RERUN

#include "GAMESET.H" //RERUN

#include "RTESTSH1.H" //RERUN

#include "MIGVIEW.H" //RERUN

#include "timeapi.h" //RERUN

//MAYBE, bool tempblockkeys; //RERUN

enum	{BUFFERED_KEYS=100};

static	volatile Bool	insidetimer = BOOL_FALSE;

extern Replay _Replay;
extern UByte videoBuffer;
extern bool mono3d;
LogStack	Log_Stack={{{0}}};
///////////////////////////////////////////////////////////////////////

//HANDLE	View3d::htable[NUM_EVENTS];
//LPDIRECTINPUT  Inst3d::Master_3d.g_lpDI=NULL;
//LPDIRECTINPUTDEVICE  Inst3d::Master_3d.g_lpDIDevice=NULL;
//HINSTANCE	View3d::hinst=NULL;
//HWND		View3d::hwind=NULL;
///View3d*		Inst3d::viewedwin=NULL;
//char		Inst3d::lastkeys[256]={0};
//Inst3d*		Inst3d::Master_3d.currinst=NULL;
KeyMap3d*	Inst3d::commonkeymaps=NULL;
extern	KeyMapping	*Debug3dMapTable;

Mast3d	Master_3d;
HANDLE			hTimerQueue;  //RERUN Added

static Uint32 SDLCALL Mast3d_TimerCallback(Uint32 interval, void *param) {
    Mast3d::StaticTimeProc(0, 0, (DWORD)(UINT_PTR)param, 0, 0);
    return interval;
}

///////////////////////////////////////////////////////////////////////
Mast3d::Mast3d()
{
#if 0 //RERUN try to avoid SDL code here. defined (__MSVC__)
	TIMECAPS tcaps;
	timeGetDevCaps ( &tcaps, sizeof (TIMECAPS));
	wTimerRes=std::max<int>(tcaps.wPeriodMin,1);
	timeBeginPeriod (wTimerRes);
#if USE_OLD_TIMER_CODE
	uTimerID = timeSetEvent (20, wTimerRes, Master_3d.StaticTimeProc, (int)&Master_3d, TIME_PERIODIC);
#else
	int periodicTime, dueTime;
	periodicTime = dueTime= 20;
	ULong myFlags = WT_EXECUTEDEFAULT;

	timerHandle = hTimerQueue = NULL;
	hTimerQueue = CreateTimerQueue();
	CreateTimerQueueTimer(&timerHandle, hTimerQueue, (WAITORTIMERCALLBACK)Master_3d.StaticTimeProc,
		                  this, periodicTime, dueTime, myFlags);
#endif
#else
	// Instead of timeGetDevCaps/timeBeginPeriod, just pick your interval
    int periodicTime = 20; // ms

    // Create repeating SDL timer
    SDL_TimerID timerID = SDL_AddTimer(periodicTime, Mast3d_TimerCallback, this);

    // Store timerID if you want to cancel it later with SDL_RemoveTimer
    this->timerHandle = reinterpret_cast<void*>(timerID);

    currinst = NULL;
    ticknum = 0;
#endif
	currinst=NULL;
	//int mv=0;
	ticknum=0;
}

void Mast3d::Init(HINSTANCE			Hinst,HWND				Hwind)
{
	winst=Hwind;
	hinst=Hinst;
	MainInit();

	if (FAILED(DirectInputCreate(hinst, DIRECTINPUT_VERSION, &g_lpDI, NULL)))
		_Error.EmitSysErr("Can't access DirectInput!");
			;;;;;//ERROR 
}

void	Mast3d::BigWin()
{
	_DirectDraw::lpDirectDD->SetSmackerMode(false,Master_3d.winst);
	AfxGetMainWnd()->ModifyStyle(WS_THICKFRAME,0); // Bye bye border!
	AfxGetMainWnd()->ShowWindow(SW_SHOWMAXIMIZED);
}
void	Mast3d::SmallWin()
{
	_DirectDraw::lpDirectDD->SetSmackerMode(true,Master_3d.winst);
	AfxGetMainWnd()->ModifyStyle(WS_THICKFRAME,0); // Bye bye border!
	AfxGetMainWnd()->ShowWindow(SW_SHOWMAXIMIZED);
}

void	Mast3d::MainInit(void)
{
	FILEMAN.InitFileSystem(); // moved to a place where it will happen sooner (Mainfrm.cpp) DBM

	Image_Map.InitImageMaps();									//DAW 27Oct98

	_DPlay.Implemented = BOOL_FALSE;

	_Miles.Init((ULong)winst);											//RJS 02Nov98
	MigDisplay* Whole_Screen;

	_DirectDraw::lpDirectDD=new DirectDD;
}
	
//////////////////////////////////////////////////////////////////////
Mast3d::~Mast3d()
{
	Inst3d*	ilist=currinst;
	while (ilist)
	{
		currinst=ilist->nextinst;
		delete ilist;
		ilist=currinst;
	}
#if USE_OLD_TIMER_CODE
	if (uTimerID)
		timeKillEvent(uTimerID);
#endif 
	uTimerID=0;
	if (wTimerRes)
		timeEndPeriod (wTimerRes);
	wTimerRes=0;
	if (Master_3d.g_lpDI)
	{
		Master_3d.g_lpDI->Release();
		Master_3d.g_lpDI=NULL;
	}
	delete _DirectDraw::lpDirectDD;
}

//MakeInteractive
void	Mast3d::Stop()
{
#if USE_OLD_TIMER_CODE
	if (uTimerID)
		timeKillEvent(uTimerID);
#endif
	uTimerID=0;
}

///////////////////////////////////////////////////////////////////////
Inst3d::Inst3d(bool flag)
{
	Log_Stack.Log(LogStack::MESSAGELOOP);
// set up semaphore and mutex objects

	ULong maxcount;

	if (_DPlay.Implemented)
		maxcount=1000;
	else
		maxcount=200;
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[2]='s';
	semaphore=CreateSemaphore(
		NULL,		// security attribs are default
		0,			// initial count
		maxcount,	// max count
		NULL		// name 
		);			

	if (!semaphore)
		_Error.EmitSysErr("Semaphore creation failed");
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[2]='m';
	mutex=CreateMutex(
		NULL,		// security attribs are default
		FALSE,		// owner?
		NULL		// name 
		);

	if (!mutex)
		_Error.EmitSysErr("Mutex creation failed");

#if 1 //RERUN lets hide the SDL code in the else block defined (__MSVC__) //RERUN
	movethread=AfxBeginThread(moveloop,this,THREAD_PRIORITY_ABOVE_NORMAL,50000,0); 
#else
	movethread = CreateThread(NULL, 50000, (LPTHREAD_START_ROUTINE)Inst3d::moveloop, this, 0, NULL);
	SetThreadPriority(movethread,THREAD_PRIORITY_ABOVE_NORMAL);
#endif
	mapview=flag;
	mono3d=false;
//DEADCODE JIM 29/03/99 	FILEMAN.flushcachedfiles();
 	paused=TRUE;
	accel=FALSE;
	interactive=FALSE;
	insidetimer=FALSE;									//RERUN: uninit
	nextinst=Master_3d.currinst;
	//RERUN: freeze every live move thread - all instances share the global
	//mobileitem lists. Mutex (not a flag) drains in-flight DoMoveCycle.
	for (Inst3d* t=nextinst; t; t=t->nextinst)
	{
		t->Paused(TRUE);
		WaitForSingleObject(t->mutex,INFINITE);
	}
	WaitForSingleObject(mutex,INFINITE);				//RERUN: BlockTick skips mapview, hold raw mutex
	Master_3d.currinst=this;	  //at this point the thread starts receiving timer messages.

	commonkeymaps=(KeyMap3d*) Key_Tests.reftable3d.flat;
	world=new WorldStuff;
	(&world)->SetViewpoint(NULL);
//DEADCODE DAW 17/02/00 	blocktick=blockticks=TRUE;	//no movecodes
	viewedwin=NULL;
	livelist=NULL;
	Manual_Pilot.ControlledAC2=NULL;
	GR_Quit3DNow=0;
	GR_OkToQuit3DNow=0;
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[2]='r';
	_Replay.DeleteReplayLog(); // want new log every time we enter non-playback 3D//AMM 07Oct98
	_Replay.SuperHeaderStored=false;
	_Miles.ResetSoundFlags();
	Three_Dee.InitialiseCache();
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[2]='l';
	Land_Scape.Init();
	Three_Dee.current_world=&world;
	//Load in the shape files requested by the battle field file loads
	SHAPESTUFF.LoadRequiredShapes();
	Three_Dee.livelist=livelist;
	_Collide.tdalt=BOOL_TRUE;
//DEADCODE DAW 28/03/99 	Timer_Code.accel=FALSE;
	Trans_Obj.View_Point=NULL;
	Key_Tests.Reg3dConv(FIL_3D_KEYBOARD_TABLE);
	timeofday=11*60*60*100;
	OverLay.SetToMapViewScreen();
	Dead_Stream.GetWorldDead(&world);				//RDH 02Oct96	  //JIM 03/06/99
	ReleaseMutex(mutex);							//RERUN: world ready, let moves run
	for (Inst3d* t=nextinst; t; t=t->nextinst)	//RERUN: release drained insts
		ReleaseMutex(t->mutex);

//do this as very last thing!!!
	Log_Stack.UnLog(LogStack::MESSAGELOOP);
}

Inst3d::Inst3d()
{
	Log_Stack.Log(LogStack::MESSAGELOOP);
// set up semaphore and mutex objects

	ULong maxcount;

	if (_DPlay.Implemented)
		maxcount=1000;
	else
		maxcount=200;

	semaphore=CreateSemaphore(
		NULL,		// security attribs are default
		0,			// initial count
		maxcount,	// max count
		NULL		// name 
		);			

	if (!semaphore)
		_Error.EmitSysErr("Semaphore creation failed");

	mutex=CreateMutex(
		NULL,		// security attribs are default
		FALSE,		// owner?
		NULL		// name 
		);

	if (!mutex)
		_Error.EmitSysErr("Mutex creation failed");

	int oldprec=GETPREC();
	SETPREC(3);

#if 1 //RERUN let's hide the SDL code in the else block defined (__MSVC__) //RERUN
	CWinThread* mainthread=AfxGetThread();
	mainthread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	movethread=AfxBeginThread(moveloop,this,THREAD_PRIORITY_ABOVE_NORMAL,50000,0); 
#else
	movethread = CreateThread(NULL, 50000, (LPTHREAD_START_ROUTINE)Inst3d::moveloop, this, 0, NULL);
	SetThreadPriority(movethread, THREAD_PRIORITY_ABOVE_NORMAL); // RERUN no effect in userspace linux.
#endif

	mapview=false;
	if (_Replay.Playback && !Save_Data.gamedifficulty[GD_GUNCAMERACOLOUR])
		mono3d=true;
	else
		mono3d=false;

 	paused=TRUE;
	accel=FALSE;
	interactive=FALSE;
	insidetimer=FALSE;									//RERUN: uninit before BlockTick spins on it
	nextinst=Master_3d.currinst;
	//RERUN: freeze every live move thread - all instances share the global
	//mobileitem lists this ctor is about to rebuild. Mutex (not a flag)
	//drains any in-flight DoMoveCycle before the lists are touched.
	for (Inst3d* t=nextinst; t; t=t->nextinst)
	{
		t->Paused(TRUE);
		WaitForSingleObject(t->mutex,INFINITE);
	}
	BlockTick(TRUE);
	Master_3d.currinst=this;

	commonkeymaps=(KeyMap3d*) Key_Tests.reftable3d.flat;

	world=new WorldStuff;
//DEADCODE DAW 17/02/00 	blocktick=blockticks=0;
	viewedwin=NULL;
	_Replay.inst=this;											//AMM 11May99
	livelist=NULL;
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[2]='B';
	Persons3	Persons_2(world,NULL,this);

	Manual_Pilot.ControlledAC2 = NULL;							//RDH 24Oct96
	GR_Quit3DNow=0;
	GR_OkToQuit3DNow=0;

	_Replay.DeleteFileAndGlobRefList(_Replay.bfgrlist);	//need to reset bfields if new mission

	if (!_Replay.Playback)
	{
		_Replay.badrecord=true;									//AMM 09Mar99
		_Replay.DeleteReplayLog(); // want new log every time we enter non-playback 3D//AMM 07Oct98
	}

	if (Save_Data.gamedifficulty[GD_GUNCAMERAATSTART] && !_DPlay.Implemented && !_Replay.Playback)			//AMM 05Jan99
	{
		_Replay.Record=BOOL_TRUE;
	}
	_Replay.SuperHeaderStored=false;

	Art_Int.CleanUp();			//A/C recognition list

	timeofday = 11*60*60*100;										//PD 19Jan99

	_Miles.ResetSoundFlags();									//DAW 01Sep98
	_Radio.SetUpRandomVoices();										//RJS 08Jun99
 	_Collide.tdalt = BOOL_FALSE;
	Three_Dee.InitialiseCache();

	Land_Scape.Init();
	Persons_2.LoadSetPiece(&world);//RDH 25Mar96
	Dead_Stream.GetWorldDead(&world);				//RDH 02Oct96

	Manual_Pilot.SetWorldPtr(&world);							//PD 25Jun96
	_Collide.SetWorld(&world);								//PD 22Mar96
	MobileItem::SetWorld(&world);
	Three_Dee.current_world=&world;
	BlockTick(FALSE);								//RERUN: world + lists ready, let moves run
	for (Inst3d* t=nextinst; t; t=t->nextinst)	//RERUN: release drained insts
		ReleaseMutex(t->mutex);
	Trans_Obj.CleanUpTrailList();								//JIM 26Mar99

	//Load in the shape files requested by the battle field file loads
	SHAPESTUFF.LoadRequiredShapes();
 	//Set up animation data...
	(*world).SetLaunchers();										//RJS 21Aug96

	livelist= new LiveList(&world);
	Three_Dee.livelist=livelist;

	Three_Dee.livelist->BackupSleepList();

	//Back-up miles flags first
	SWord	oldsfxvol = Save_Data.vol.sfx;							  //RJS 07/07/99
	SWord	oldsfxmusic = Save_Data.vol.music;						   //RJS 07/07/99
	SWord	oldsfxchat = Save_Data.vol.rchat;						   //RJS 07/07/99
																	  //RJS 07/07/99
	Save_Data.vol.sfx = 0;											   //RJS 07/07/99
	Save_Data.vol.music = 0;										  //RJS 07/07/99
	Save_Data.vol.rchat = 0;										  //RJS 07/07/99
	Log_Stack.DontCheck(LogStack::MOVECODE);
	for (int i=0;i<Art_Int.ACARRAYSIZE*4;i++)	//CSB 06/07/99	
	{	//SFX are disabled here, so it is safe to send SFX messages.
		Art_Int.VisibleCheck();
	}
	Log_Stack.DontCheck();
	//Restore miles flags...
	Save_Data.vol.sfx = oldsfxvol;									  //RJS 07/07/99
	Save_Data.vol.music = oldsfxmusic;								  //RJS 07/07/99
	Save_Data.vol.rchat = oldsfxchat;								  //RJS 07/07/99

	if (_Replay.Record)											//AMM 14Jun99
	{															//AMM 14Jun99
		_Replay.InitPosBuffer();								//AMM 14Jun99
		_Replay.StoreFinalRecordData();							//AMM 14Jun99
	}															//AMM 14Jun99
	else if (_Replay.Playback)									//AMM 14Jun99
	{															//AMM 14Jun99
		_Replay.LoadFinalPlaybackData();						//AMM 14Jun99
																//AMM 14Jun99
		if (!_Replay.Playback)									//AMM 14Jun99
			_Error.EmitSysErr("Error reading playback log");	//AMM 14Jun99
	}															//AMM 14Jun99
	else														//AMM 14Jun99
		_Replay.InitPosBuffer();								//AMM 14Jun99

	_Collide.tdalt = BOOL_TRUE;										//PD 30Sep96

	_MsgBuffer.SetPlayer(Persons2::PlayerGhostAC,Persons2::PlayerSeenAC);			//RJS 27Jan99

	Trans_Obj.View_Point=Manual_Pilot.ControlledAC2;

	SendInit2Packet ();											//ARM 15Aug96

	Key_Tests.Reg3dConv(FIL_3D_KEYBOARD_TABLE);

	if (_Replay.RestorePosition)
	{
		_Replay.RestorePlaybackPosition();
	}

	SETPREC(oldprec);

	Log_Stack.UnLog(LogStack::MESSAGELOOP);
}
///////////////////////////////////////////////////////////////////////
Inst3d::~Inst3d()
{
	Log_Stack.Log(LogStack::MESSAGELOOP);
	_Miles.KillAll();

	//FIRST, remove all viewports:
	Paused(TRUE);
	//RERUN CWinThread*	oldmovethread=movethread;
	movethread=0;
	{
		View3d* sptr=viewedwin;
		while (sptr)
		{
			sptr->SetEndDraw(View3d::D_CLOSE);
			sptr=sptr->nextview;
		}
		sptr=viewedwin;
		Interactive(NULL);
		viewedwin=NULL;
		while (sptr)
		{
			View3d* sp2=sptr;
			sptr=sptr->nextview;
			delete sp2;
		}
	}
	//Next, remove instance from list
	if (Master_3d.currinst)
	{
		Inst3d**	sptr=&Master_3d.currinst;
		while (*sptr!=this)
			sptr=&sptr[0]->nextinst;
		*sptr=nextinst;
	}

	//Next, start freeing resources:
	_DPlay.GameRunning=BOOL_FALSE;

	if (!mapview)
	{
	Persons3 	Persons_3(world,NULL,this);
	if (!_Replay.Playback)
	{
		Art_Int.FakeMissionSuccess();
		Dead_Stream.SetWorldDead(world);				//RDH 02Oct96
		Dead_Stream.ScoreSGKills();
#ifndef	MIG_DEMO_VER
		Persons_3.ProcessLogList();
#endif //	MIG_DEMO_VER
	}
	Persons_3.FreeRoutes();
	Persons_3.FreeEvents();
	}
	else 
		Persons4::ShutDownViewFromMap();

	delete	livelist;
	Three_Dee.livelist=NULL;
	SHAPESTUFF.UnLoadRequiredShapes();
	Three_Dee.Done3D();											//DAW 30Nov95
 	Image_Map.UnLoadImageMaps();								//RJS 14Jun99
	if (!mapview) (*world).ClearWorld();
	delete Persons2::PlayerGhostAC;
	Persons2::PlayerSeenAC=NULL;
	Persons2::PlayerGhostAC=NULL;

	_DPlay.DeleteClaimBuffer();									//AMM 12Mar99

	if (_Replay.Playback)
	{
// want to remove deadlist and restore from save game, otherwise it becomes garbage

		Dead_Stream.ClrWorldDead();								//AMM 29Mar99

		if (_Replay.MapPlayback)									  //AMM 25/06/99
			_Replay.RestoreSaveGame();
//DeadCode DAW 25Jun99 		_Replay.RestoreSaveGame();
		_Replay.ClosePlaybackLog();
		_Replay.RestorePrefs();
		RFullPanelDial::gamestate=(RFullPanelDial::GameStates)_Replay.PlaybackGameState;
	}
	else
	{
		_Replay.CloseRecordLog();
	}
	_Replay.inst=NULL;

	_Miles.ResetSoundFlags();									//DAW 01Sep98
	if (!mapview) Art_Int.CleanUp();
	delete world;
	MobileItem::SetWorld(NULL);

	OverLay.pCurScr=OverLay.pNewScr=NULL;

 	AfxGetMainWnd()->ModifyStyle(WS_THICKFRAME,0); // Bye bye border!
#if defined(NDEBUG)
	//AfxGetMainWnd()->SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOACTIVATE+SWP_NOMOVE+SWP_NOSIZE);
	AfxGetMainWnd()->SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOACTIVATE+SWP_NOMOVE+SWP_NOSIZE);
#endif
//	SystemParametersInfo(SPI_SETDRAGFULLWINDOWS,FALSE,NULL,NULL);
	AfxGetMainWnd()->ShowWindow(SW_SHOWMAXIMIZED);

// release semaphore and mutex handles
	if (semaphore==0 || mutex==0)
		_Error.EmitSysErr("Thread management resources null on release");
	CloseHandle(semaphore);
	CloseHandle(mutex);

	Log_Stack.UnLog(LogStack::MESSAGELOOP);
}
///////////////////////////////////////////////////////////////////////
bool	Inst3d::InThe3D()
{
	for (Inst3d*	t=Master_3d.currinst;t;t=t->nextinst)	
	{	 
		for (View3d*	v=t->viewedwin;v;v=v->nextview)
			if (v->drawing == View3d::D_YES)	// RERUN: Was v->drawing. Now check explicit D_YES to exclude D_CLOSE
				return TRUE;
	}
	return false;
}

//------------------------------------------------------------------------------
//Procedure		ReleaseDirectX
//Author		Paul.   
//Date			Mon 25 Jan 1999
//------------------------------------------------------------------------------
void Inst3d::ReleaseDirectX()
{
	//remove all viewports...
	for (Inst3d* t=Master_3d.currinst;t;t=t->nextinst)
	{
		t->Paused(TRUE);
		View3d *v=t->viewedwin;
		while (v)
		{
			v->WaitEndDraw(View3d::D_CLOSE);
			v->MakeResize(WinMode::NONE);
			v->CloseWindow(IDCANCEL);
			v=v->nextview;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		RestoreDirectX
//Author		Paul.   
//Date			Mon 25 Jan 1999
//------------------------------------------------------------------------------
void Inst3d::RestoreDirectX()
{
}

///////////////////////////////////////////////////////////////////////
View3d::View3d(Inst3d* ofinst,HWND newwin,CWnd* d):
	coords(0,0,0,0),E(NULL)

{
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[4]='V';
	Log_Stack.Log(LogStack::MESSAGELOOP);
//DeadCode RJS 18May98 	_Miles.Init((ULong)newwin);									//RJS 03Apr98
	mfcwin=d;
	window=newwin;
	inst=ofinst;
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[6]='i';
	if (ofinst->Interactive())
	{
		nextview=ofinst->Interactive()->nextview;
		ofinst->Interactive()->nextview=this;
	}
	else
	{
		nextview=ofinst->viewedwin;
		ofinst->viewedwin=this;
	}
 	lastrealframetime=5;			//should be in for photo rotation //DAW 19/6/99
	View_Point=NULL;
	Whole_Screen=NULL;
	mode=WinMode::NONE;
	drawing=D_NO;

	_Miles.SetDelayed(false);

#if 0 //RERUN test for SDL
	AfxBeginThread( drawloop, this, THREAD_PRIORITY_NORMAL, 0);
#endif

  	Image_Map.LoadImageMapPtrs();								//PD 29Nov95
	doneframe=TRUE;
	Log_Stack.UnLog(LogStack::MESSAGELOOP);
}
///////////////////////////////////////////////////////////////////////
View3d::~View3d()
{

	Three_Dee.ClearCurrentDisplay();
	Log_Stack.Log(LogStack::MESSAGELOOP);
	_Miles.DisableAll();

	Interactive(false);
	//free thread

	if (drawing!=D_NO)
	{
		WaitEndDraw(D_CLOSE);
	}
	if (mode!=WinMode::NONE)
	{	//screen is allocated - free resources
	}
	if (inst->viewedwin != NULL)
	{
		View3d**	sptr=&inst->viewedwin;
		while (*sptr!=this)
			sptr=&sptr[0]->nextview;
		*sptr=nextview;
	}

	MakeResize(WinMode::NONE);

	_Miles.InitUI((int)window);									//DAW 06Apr99
	Log_Stack.UnLog(LogStack::MESSAGELOOP);
}

void	View3d::CloseWindow(const ULong id)
{
	if (mfcwin)
		mfcwin->PostMessage(WM_COMMAND,id,0);
}
///////////////////////////////////////////////////////////////////////
int	View3d::MakeInteractive(WinMode::Mode	v,const CRect* pos,bool flag,bool flag2)
{
	Log_Stack.Log(LogStack::MESSAGELOOP);

	ASSERT(mode==WinMode::NONE);
	if (!MakePassive(v,pos,flag,flag2))
		return (0);
	Interactive(true);
	return 1;
}
///////////////////////////////////////////////////////////////////////
int	View3d::MakePassive(WinMode::Mode	v,const CRect* pos,bool flag,bool flag2)
{
	Log_Stack.Log(LogStack::MESSAGELOOP);

	ASSERT(mode==WinMode::NONE);
	Interactive(false);
	if (pos)
		coords=*pos;
	else
	{

	}

#ifdef _WIN32 //RERUN don't care about this
	HDC	dcTmp = GetDC(window);
	SetSystemPaletteUse(dcTmp,SYSPAL_NOSTATIC);		 //SYSPAL_ERROR
	ReleaseDC(window,dcTmp);
#endif

	//make sure loaderart is cached
	fileblockptr fbp=new fileblock(FIL_LOADERART);
	delete fbp;

	//handle resolution change
	delete _DirectDraw::lpDirectDD;
	_DirectDraw::lpDirectDD=new DirectDD;

	Whole_Screen=new MigDisplay(this);
	Whole_Screen->DD.hWnd = window;
	Whole_Screen->DD.hWndApp = Master_3d.winst;
	_Miles.SetDirectSoundWindow((ULong)Master_3d.winst);
	Whole_Screen->InitDirectDraw ();
	Whole_Screen->Init(v,coords.left,coords.top,coords.right,coords.bottom);
	View_Point=new ViewPoint(Whole_Screen,this);
	OverLay.SetScreen(Whole_Screen);
	OverLay.SetViewpoint(View_Point);
	OverLay.LoaderScreen(0);									//PD 02Mar99
	_Miles.Init3D();											//RJS 06Apr99
	_Miles.SetDirectSoundWindow((ULong)Master_3d.winst);
	OverLay.LoaderScreen(0+32);									//PD 02Mar99

	if (!flag)
	{
		_Miles.SetVolumes(BOOL_TRUE);								//RJS 13Apr99
		View_Point->SetToPiloted(Manual_Pilot.ControlledAC2);
		View_Point->SetReplayView(_Replay.Playback);//RJS 20Oct98
	}
	else 
		View_Point->SetToMapItem(NULL);

	OverLay.LoaderScreen(0+48);									//PD 02Mar99
	Trans_Obj.View_Point=View_Point;
	Trans_Obj.View_Object=View_Point;							//RJS 21Apr98
	Land_Scape.View_Point=View_Point;
	Land_Scape.currscreen=Whole_Screen;
	OverLay.LoaderScreen(0+64);									//PD 02Mar99
	Three_Dee.Init3D(Whole_Screen,View_Point);					//RJS 19Nov98

	if (flag2) 
		Land_Scape.RefreshLandscape();

	Three_Dee.ResetPalette();									//RJS 24Oct96
	doneframe=TRUE;		//forces frametime to zero
	Drawing(true);
	mode=v;

	if (_DPlay.Implemented)										//DAW 22Jun99
	{															//DAW 22Jun99
		_DPlay.csync=true;										//DAW 22Jun99
	}															//DAW 22Jun99
	WaitEndDraw(D_YES);

	WaitEndDraw(D_YES);

	if (_DPlay.Implemented)										//DAW 22Jun99
	{															//DAW 22Jun99
		_DPlay.csync=false;										//DAW 22Jun99
	}															//DAW 22Jun99
	_Replay.skipOK=true;
	return 1;
}
///////////////////////////////////////////////////////////////////////
int	View3d::MakeResize(WinMode::Mode	v,const CRect* pos)
{
	Log_Stack.Log(LogStack::MESSAGELOOP);
	if ((pos && *pos!=coords) || v!=mode)
	{
		//delete old surface...
		if (pos)
			coords=*pos;

		//some legality check before we start screwing things up 10 ways
		if (v!=mode)
		{	//delete windowing object
			v=WinMode::NONE;
			drawing=D_CLOSE;
			delete Whole_Screen;
			Whole_Screen=NULL;
			Trans_Obj.View_Point=Manual_Pilot.ControlledAC2;
			delete View_Point;
			View_Point=NULL;
#ifdef _WIN32 //RERUN don't care about this one.
			HDC	dcTmp = GetDC(window);
			SetSystemPaletteUse(dcTmp,SYSPAL_STATIC);
			ReleaseDC(window,dcTmp);
#endif
		}
		//make or convert to new diasplay 
		v=mode;
	}
	//else no change
	Log_Stack.UnLog(LogStack::MESSAGELOOP);

	return 1;
}
int	View3d::MakeResize(WinMode::Mode v)
{
	CRect* pos=NULL;
	return MakeResize(v, pos);
}
int	View3d::MakeResize ()
{
	return MakeResize(WinMode::FULL);
}
///////////////////////////////////////////////////////////////////////
View3d* Inst3d::Interactive(View3d* newwin)
{
	Log_Stack.Log(LogStack::MESSAGELOOP);
	View3d* rv=interactive?viewedwin:NULL;
	interactive=(newwin!=NULL);
	if (newwin)	
	{
		View3d** vw=&viewedwin;
		while (*vw!=newwin)
			vw=&vw[0]->nextview;
		*vw=newwin->nextview;				//unhooks newwin
		newwin->nextview=viewedwin;
		viewedwin=newwin;				//reinserts at front
	}
	if (newwin!=rv)
		if (newwin)
		{
			_Analogue.Initialise((int)Master_3d.winst,(int)Master_3d.hinst);
			if (Master_3d.g_lpDI)
			if (!Master_3d.g_lpDIDevice)
			{
				if (!FAILED(Master_3d.g_lpDI->CreateDevice(GUID_SysKeyboard, &Master_3d.g_lpDIDevice, NULL)))
				{
					if (!FAILED(Master_3d.g_lpDIDevice->SetDataFormat(&c_dfDIKeyboard)))
						if (!FAILED(Master_3d.g_lpDIDevice->SetCooperativeLevel(Master_3d.winst,DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
						{	//E_INVALIDARG	//DIERR_NOTINITIALIZED
							DIPROPDWORD	buffsize={{sizeof(DIPROPDWORD),sizeof(DIPROPHEADER),0,DIPH_DEVICE},BUFFERED_KEYS};
							DIDEVCAPS DIDevCaps={sizeof(DIDEVCAPS)};
							Master_3d.g_lpDIDevice->GetCapabilities(&DIDevCaps); 
							//DIDC_ATTACHED   DIERR_INVALIDPARAM 
							Master_3d.g_lpDIDevice->SetProperty(DIPROP_BUFFERSIZE,&buffsize.diph);
							if (!FAILED(Master_3d.g_lpDIDevice->SetEventNotification(Master_3d.htable[Master_3d.EVENT_KEYS])))
							{
								if (!FAILED(Master_3d.g_lpDIDevice->Acquire()))
								{
									// Interactive setup success
									return	rv;
								}
							}
						}
					Master_3d.g_lpDIDevice->Release();
					Master_3d.g_lpDIDevice=NULL;
				}
				// Interactive setup fail
				_Error.EmitSysErr("Couldn't access keyboard with directinput?");
			}
		}
		else
		{
			_Analogue.CloseDown();
			if (Master_3d.g_lpDIDevice)
			{
				Master_3d.g_lpDIDevice->Unacquire();
				Master_3d.g_lpDIDevice->SetEventNotification(NULL);
				Master_3d.g_lpDIDevice->Release();
				Master_3d.g_lpDIDevice=NULL;
			}
		}	 
	Log_Stack.UnLog(LogStack::MESSAGELOOP);
	return rv;
}
///////////////////////////////////////////////////////////////////////
bool Inst3d::Paused(bool newmode)
{
	bool rv=paused;
	paused=newmode;
	if (newmode!=rv)
		if (newmode)
		{
			_Miles.StopAll();				//RJS 07Jun99
			_Miles.SequenceMute(500);		//RJS 07Jun99
			_Miles.SetEngine();				//RJS 07Jun99
			accel=false;
		}
		else
		{
		}
	return rv;
}
///////////////////////////////////////////////////////////////////////
bool Inst3d::Accel(bool newmode)
{
	bool rv=accel;
	accel=newmode;
	if (newmode!=rv)
		if (newmode)
		{
			if (_Replay.Record)									//AMM 26May99
			{													//AMM 26May99
				if (Save_Data.gamedifficulty[GD_GUNCAMERAONTRIGGER]
				&& !Save_Data.gamedifficulty[GD_GUNCAMERAATSTART])
				{
					_Replay.guncameradelay=0;
//DeadCode AMM 08Jul99 					_Replay.clearreplaybuffers=true;
					_Replay.stopforaccel=false;
				}
				else
				{
					_Replay.stopforaccel=true;						//AMM 26May99
				}
				_Replay.StopRecord();							//AMM 26May99
			}													//AMM 26May99

			_Miles.StopAll();				//RJS 11May99
			_Miles.SequenceMute(500);		//RJS 11May99
			_Miles.SetEngine();									//RJS 01Jun99
			paused=false;
		}
		else
		{
			_Radio.ClearMessages(MSG_TAKEOFFROLLING);			//RJS 14Jun99
			_Radio.ClearMessages(PRI_LOW);						//RJS 03Jun99
			mobileitem::ResetACGears();							//RJS 01Jun99
//DeadCode AMM 10Jun99 			_DPlay.SendAccelDeselectedMessage();				//AMM 26May99

			if (_Replay.stopforaccel)							//AMM 26May99
			{													//AMM 26May99
				_Replay.stopforaccel=false;						//AMM 26May99
				_Replay.StartRecordFlag=BOOL_TRUE;					//AMM 26May99
				_Replay.DelayedGearUp=true;
			}													//AMM 26May99
		}

	Persons2::PlayerSeenAC->fly.pModel->Blood = 1;
	return rv;
}
///////////////////////////////////////////////////////////////////////
View3d* Inst3d::Interactive()
{
	return interactive?viewedwin:NULL;
}
///////////////////////////////////////////////////////////////////////
bool Inst3d::Paused()
{
	if (this==NULL)
		return	false;
	return paused;
}
///////////////////////////////////////////////////////////////////////
bool Inst3d::Accel()
{
	return accel;
}
///////////////////////////////////////////////////////////////////////
bool	View3d::Drawing(bool newval)
{
	bool rv=(drawing==D_YES);
	if (drawing<=D_YES)
	{
		drawing=newval?D_YES:D_NO;
	}
	return rv;
}
///////////////////////////////////////////////////////////////////////
void Inst3d::OnKeyInput()
{
    Log_Stack.Log(LogStack::MESSAGELOOP);

    std::deque<SdlKeyEvent> local_events;
    {
        std::lock_guard<std::mutex> lock(g_sdl_key_events_mutex);
        if (g_sdl_key_events.empty()) {
            Log_Stack.UnLog(LogStack::MESSAGELOOP);
            return;
        }
        local_events.swap(g_sdl_key_events);
    }

    for (const auto& evt : local_events) {
        if (evt.down) {
            OnKeyDown(evt.dik_code);
        } else {
            OnKeyUp(evt.dik_code);
        }
    }

    Log_Stack.UnLog(LogStack::MESSAGELOOP);
}
void	Inst3d::OnKeyDown(int keynum)
{
	if (!commonkeymaps)
	{
		std::cout << "Key input received before keymaps loaded?" << std::endl;
		commonkeymaps=(KeyMap3d*) Key_Tests.reftable3d.flat;
		return;
	}
	int	index=commonkeymaps->mappings[keynum][commonkeymaps->currshifts];
	if (index)
	{
		if (index<16)
		{	//it's a shift key!
			commonkeymaps->currshifts=index>>1;
		}
		else
		{
			if (!BITSET(commonkeymaps->bitflags,index+1))
				BITSET(commonkeymaps->bitflags,index);
		}
	}
}
void	Inst3d::OnKeyUp(int keynum)
{	//release	
	UWord*	indtable=commonkeymaps->mappings[keynum];
	for (int ent=8;ent;ent--)
	{
		int index=*indtable++;
		if (index)
		{
			if (index<16)
			{	//it's a shift key!
				if (commonkeymaps->currshifts==(index>>1))
					commonkeymaps->currshifts=0;
			}
			else
			{
				BITRESET(commonkeymaps->bitflags,index);
			}
		}
	}
}

UINT AFX_CDECL View3d::drawloop(LPVOID THISTHIS)
{	//can get hwnd etcetera no problem!
	Log_Stack.Log(LogStack::THREEDEE,40000);
	View3d* This=(View3d*) THISTHIS;
#if 0 //RERUN for SDL
	forever
#endif
	{
		if (This->drawing==D_YES)
		{
			//actual draw code here!
			if (This->mode!=WinMode::NONE)
			{
				MigWindow	window3d(*This->Whole_Screen,WINSH_MID,WINSH_MID);
 
				static UWord doit=0;							//AMM 07Jul99
				static UWord stage=0;							//AMM 07Jul99

				if (!_DPlay.Implemented || _DPlay.csync)		//DAW 22Jun99
					Three_Dee.render(&window3d,This->View_Point,This->inst->world);
				else if (doit&0x1000)							//AMM 08Jul99
				{												//AMM 07Jul99
// dont want resyncing message at all, but keep waiting message
					if (!_DPlay.resyncbar)						//AMM 28Sep99
						OverLay.CommsWaitingScreen(stage++);		//AMM 07Jul99
				}												//AMM 07Jul99
				doit++;											//AMM 07Jul99
				This->BlockTick(BOOL_TRUE);
				//Transients must always go last...
				if (!_DPlay.Implemented || _DPlay.csync)		//DAW 22Jun99
				{												//DAW 22Jun99
					SHAPE.GenerateProbeTrails();								//RJS 08Apr99
					SHAPE.KillVapourStreamDeadList();							//RJS 08Apr99
					Trans_Obj.RemoveDeadListFromWorld(This->inst->world);

					if (_Replay.Playback && _Replay.processsmokes)
						_Replay.UpdateSmokeInfo();
				}												//DAW 22Jun99
				else
				{
					if (_DPlay.Implemented && !_DPlay.csync)
					{
						if (_DPlay.CommsGameSync())
							This->Paused(FALSE);
					}
				}
				This->BlockTick(BOOL_FALSE);
// to stop screen strobe in comms sync phase!
				if (!_DPlay.Implemented || _DPlay.csync)		//AMM 09Jun99
					This->Whole_Screen->ScreenSwap();

				if (Key_Tests.KeyPress3d(SCREENSHOT)){
					if (!videoBuffer) This->Whole_Screen->ScreenShot();
					else
					{
						This->Whole_Screen->DoSaveVideoBuffer();
					}
				}
				Three_Dee.ClearCurrentScreen();
			}
		}
		if (This->E)
			This->E->SetEvent();
		if (This->drawing==D_CLOSE)
			return 0;
		This->DoneFrame();

//DEADCODE DAW 18/02/00 		UByteP(0xB0000)[162]='o';
		if ((Manual_Pilot.ControlledAC2==NULL || !Manual_Pilot.ControlledAC2->MigStatus.LaunchFlags.deadtime)
			&& !This->Accel() && OverLay.pCurScr==NULL && Key_Tests.KeyPress3d(KEY_CONFIGMENU))
		{
			if (!(_DPlay.Implemented || _Replay.Playback))		//AMM 14Apr99
			{													//AMM 14Apr99
// need to store state of record so that when we come back to 3d, I know
// where to start recording from again
				_Replay.prefscheck=true;

				This->inst->Paused(TRUE); //AMM 21/05/99
				This->MakeResize(WinMode::NONE);
				This->CloseWindow(IDCANCEL);
			}													//AMM 14Apr99
		}
		else if ((Manual_Pilot.ControlledAC2!=NULL && 
				((!_Replay.Playback && Key_Tests.KeyPress3d(EXITKEY)) || 
				_DPlay.SimulateExitKey)) || OverLay.quit3d)
		{
			OverLay.quit3d=0;
			This->inst->Paused(TRUE); //AMM 21/05/99
			_DPlay.ExitCommsAndReplay3D();						//AMM 11Dec98
			This->MakeResize(WinMode::NONE);
			This->CloseWindow(IDOK);
		}

	}
	Log_Stack.UnLog(LogStack::THREEDEE);
	return 0;
}

void	View3d::SetEndDraw(MigDrawState d)
{
	if (drawing==D_YES)
		if (!E)
			E=new CEvent();
	drawing=d;	
}

int	View3d::WaitEndDraw(MigDrawState d,int timeoutmilisec)
{
	if (drawing==D_YES)
		if (!E)
			E=new CEvent();
	drawing=d;	
	if (E)
		if (!E->Lock(timeoutmilisec))
			return false;
	CEvent* t=E;
	E=NULL;
	delete t;
	return true;
}

void	Mast3d::QuitGame(void )
{
}

//DEADCODE DAW 17/02/00 static int	howcamehere;
void CALLBACK Mast3d::StaticTimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
	ULong t=0;
	if (_DPlay.Implemented)
	{
		if (_DPlay.Host && _DPlay.GameRunning)
		{
			if (!_DPlay.ResyncPhase && !_DPlay.ServedGame)
			{
				_Agg.AggregatorGetPackets(BOOL_FALSE,BOOL_TRUE);
			}
		}
	}

		if (!_DPlay.Implemented || (_DPlay.Implemented && _DPlay.csync))//AMM 19Apr99
			((Mast3d*)dwUser)->TimeProc(uID,uMsg,dw1,dw2);

	Log_Stack.UnLog(LogStack::INTERRUPT);
	if (!Rtestsh1::tempblockkeys)
		Inst3d::OnKeyInput();

}

void Mast3d::TimeProc(UINT uID, UINT uMsg, DWORD dw1, DWORD dw2 )
{
	ticknum++;
	for (Inst3d* inst=currinst;inst;inst=inst->nextinst)
	{
		LONG prev;
		if (!inst->semaphore)	_Error.EmitSysErr("Semaphore null");

		ReleaseSemaphore(inst->semaphore,1,&prev);							//AMM 12Jan00

	}
}

UINT AFX_CDECL Inst3d::moveloop(LPVOID x)
{
	Inst3d* inst=(Inst3d*)x;

	forever
	{
		if (inst != NULL) // RERUN
		{
			if (!inst->semaphore)
				_Error.EmitSysErr("Semaphore null");
			WaitForSingleObject(inst->semaphore, INFINITE);			//AMM 13Jan00
			if (!inst->mutex)
				_Error.EmitSysErr("mutex null");
			WaitForSingleObject(inst->mutex, INFINITE);				//AMM 13Jan00

			if (inst->viewedwin)
			{
				inst->viewedwin->timeSinceLastMove = 0;
			}

			inst->insidetimer = TRUE;
			Log_Stack.Log(LogStack::MOVECODE, 40000);

			inst->DoMoveCycle();
			Log_Stack.UnLog(LogStack::MOVECODE);
			inst->insidetimer = FALSE;
			inst->framecount++;

			if (!inst->movethread)	//immediately after resume...
				return 0;

			if (!inst->mutex)	_Error.EmitSysErr("mutex null");
			ReleaseMutex(inst->mutex);								//AMM 13Jan00
		}
		else //RERUN
			return 0; //RERUN: error and break out
	}
}
 
#if defined (__MSVC__) //RERUN don't care about this one
WINBASEAPI BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
WINBASEAPI BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
#endif

void	Inst3d::DoMoveCycle()
{
	bool	timeout=false;
	OverLay.accelcountdown=1;
	for (View3d* view=viewedwin;view;view=view->nextview)
	{
		if (view->doneframe)
		{
			if (view->View_Point)
			{
				_Miles.ProcessSpot(view->View_Point);							//RJS 25May99
				_Radio.ProcessMessages(view->View_Point->FrameTime());			//RJS 25May99
			}
			if (!Paused())
				view->lastframetime=view->frametime;
			else
				view->lastframetime=0;
			view->lastrealframetime=view->frametime;
			view->frametime=0;
			view->realframetime=0;
			view->doneframe=FALSE;

			if (Accel())
				if (accelframes>0)
					if (view->lastrealframetime<MAX_ACCEL_FRAME)
					{
						accelframes++;
						if (	accelframes>10 
							&& (OverLay.keyFlags&KF_ACCELFRAMESMASK)!=KF_SLOWACCEL
							&& (OverLay.keyFlags&KF_ACCELFRAMESMASK)<KF_BESTACCEL)
						{
							accelframes=0;
							OverLay.keyFlags++;
						}
					}
					else
						accelframes=0;
				else
					if (view->lastrealframetime>=MAX_ACCEL_FRAME)
					{
						accelframes--;
						if (accelframes<-10 && (OverLay.keyFlags&KF_ACCELFRAMESMASK)>KF_SLOWACCEL+1)
						{
							accelframes=0;
							OverLay.keyFlags--;
						}
					}
					else
						accelframes=1;
		}
		
		UWord framemax=Accel()?MAX_ACCEL_FRAME:MAX_NORMAL_FRAME;


// for comms we want a higher number, because throwing away frames means that
// a machine will begin to lag behind other machines.

		if (_DPlay.Implemented)									//AMM 12Jan00
			framemax=500;										//AMM 12Jan00

		if (view->frametime<framemax)		//max buffer up of 2 seconds of action.
		{								//reduce this to 20 for profiling @ 5 fps
			view->frametime+=2;
			//Pause key && accel key must not be pressed on any 3D UI screens!
			if (OverLay.pCurScr!=NULL) 	//UNKLUDGE					  //JIM 08/06/99
			{
				Key_Tests.KeyPress3d(PAUSEKEY);
				Key_Tests.KeyPress3d(ACCELKEY);
			}
			else
			{
				if (!Accel())
				{
				if (!Paused())
				{
					if (Key_Tests.KeyPress3d(PAUSEKEY)
						&& !_DPlay.Implemented)							//AMM 23Nov98
					{
						Paused(TRUE);
						OverLay.TriggerMessage(COverlay::PAUSEMESS);
						if (_Replay.Playback)
						{
							_Replay.PlaybackPaused=BOOL_TRUE;
							OverLay.pCurScr=OverLay.pNewScr=&OverLay.replayScr;
						}
					}
				}
				else
				{
					_Analogue.PollPosition();
					if (Key_Tests.KeyPress3d(PAUSEKEY))
					{
						if (!_DPlay.Implemented)
						{
							Paused(FALSE);
							OverLay.TriggerMessage(COverlay::CLEARMESS);
							if (_Replay.Playback)
							{
								_Replay.PlaybackPaused=BOOL_FALSE;
								OverLay.pCurScr=OverLay.pNewScr=&OverLay.replayPlayingScr;
							}
						}
					}
				}
				}
				if (!Paused() && Key_Tests.KeyPress3d(ACCELKEY))
				{
					if (!_DPlay.Implemented)
					{
						if (!Accel())		
						{
							Accel(TRUE);
							OverLay.TriggerMessage(COverlay::ACCELMESS);
							Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_WAYPT);
							UByte okey=(OverLay.keyFlags&~KF_ACCELFRAMESMASK)+KF_SLOWACCEL;
							OverLay.keyFlags=okey;
						}	
						else
						{
							Accel(FALSE);
							OverLay.TriggerMessage(COverlay::CLEARMESS);
							Manual_Pilot.AutoToggle(ManualPilot::MANUAL);
							OverLay.keyFlags&=~KF_ACCELFRAMESMASK;
						}
					}
				}
			}
			if (Key_Tests.KeyPress3d(RECORDTOGGLE))				//AMM 16Apr99
			{													//AMM 16Apr99
				_Replay.ToggleRecord();							//AMM 16Apr99
			}													//AMM 16Apr99
			if (Key_Tests.KeyPress3d(RESETRECORD))				//AMM 16Apr99
			{													//AMM 16Apr99
				_Replay.ResetRecord();							//AMM 16Apr99
			}													//AMM 16Apr99
			if (Key_Tests.KeyPress3d(SUICIDE))					//AMM 16Apr99
			{													//AMM 16Apr99
				if (_DPlay.Implemented)							//AMM 16Apr99
					_DPlay.CommitSuicide();						//AMM 16Apr99
			}													//AMM 16Apr99

// pause/unpause now done in overlay for replay playback

			if (_Replay.Playback)
			{
				if (_Replay.PlaybackPaused)
				{
// stops animations from continuing when playback reaches end of file and pauses

					Paused(TRUE);
				}
			}

			UByte okey=OverLay.keyFlags;
			OverLay.keyFlags&=KF_ACCELFRAMESMASK;
			
			if (!_DPlay.Implemented)
			{
				if (okey&KF_PAUSEOFF)		Paused(FALSE);
				else if (okey&KF_PAUSEON)	Paused(TRUE);
			}

			if (okey&KF_ACCELOFF)
			{
				if (_DPlay.Implemented)
				{
					if (Accel())
						_DPlay.SendQuitAccelMessage();
				}
				else
				{
					Accel(FALSE);
				}
			}
			else if (okey&KF_ACCELON)	Accel(TRUE);

			if (Accel()) 
				OverLay.accelcountdown=(okey&KF_ACCELFRAMESMASK)<<2;	//Max value is 60 moves/timer call
		}
		else
		{
			timeout=true;
		}
	}
//DEADCODE DAW 18/02/00 	UByteP(0xB0000)[202]='t';
	if (!timeout)
	{
#ifndef NDEBUG
		static LARGE_INTEGER	pc,pc2;
		QueryPerformanceCounter(&pc);
		int cycles=OverLay.accelcountdown;
#endif
		while (OverLay.accelcountdown--)
		{
			MoveCycle(world);									//AMM 11May99
			if (OverLay.accelcountdown)
				if (	Key_Tests.KeyPress3d(RPM_00) 
					||	Key_Tests.KeyPress3d(RESETVIEW)
					||	Key_Tests.KeyPress3d(PADLOCKTOG)
					||	Key_Tests.KeyPress3d(SHOOT)
					||	Key_Tests.KeyPress3d(MENUSELECT)
					)
					OverLay.CancelAccel();
		}
	}
}


void	Inst3d::MoveCycle(WorldStuff* worldref)					//AMM 11May99
{
	ULong num=1;
	_Replay.replayskip=false;									//DAW 18Aug99
	_Miles.ProcessPercussion();									//DAW 18Aug99

	if (_Replay.Playback)										//AMM 08Feb99
	{
		if (_Replay.setpaused==1)								//AMM 22Apr99
		{														//AMM 14Apr99
			Paused(TRUE);										//AMM 14Apr99
			_Replay.setpaused=0;									//AMM 22Apr99
		}														//AMM 14Apr99
		else if (_Replay.setpaused==-1)							//AMM 22Apr99
		{														//AMM 22Apr99
			Paused(FALSE);										//AMM 22Apr99
			_Replay.setpaused=0;									//AMM 22Apr99
		}														//AMM 22Apr99

		switch (_Replay.ReplayFlag)									//AMM 14Jan99
		{
		case RF_NONE:
			break;

		case RF_SKIPFRAMES:
		case RF_BLOCKREWIND:
		case RF_SKIPTOPREVIOUSMARK:
		case RF_SKIPTONEXTMARK:
			_Replay.CalcCurrPos();									  //JIM 19/05/99
			if (_Replay.skipOK)
			{
				num=_Replay.framestoskip;//+1;
 				_Replay.ResetFlags();
				_Replay.replayskip=true;
			}
			break;

		case RF_BLOCKFORWARD:
 			_Replay.ResetFlags();
			break;

		case RF_FRAMEFORWARD:
			if (Paused())
				_Replay.FrameForward();
			else
				_Replay.ResetFlags();
			break;

		case RF_FRAMEREWIND:
			if (!Paused())
			{
				Paused(TRUE);
				_Replay.PlaybackPaused=BOOL_TRUE;
				OverLay.pCurScr=OverLay.pNewScr=&OverLay.replayScr;
			}
			num=_Replay.FrameRewind();
			_Replay.replayskip=true;

			break;
		}
	}

	if (!Paused()												//AMM 14Jan99
	|| _Replay.ReplayFlag==RF_FRAMEFORWARD						//AMM 14Jan99
	|| _Replay.ReplayFlag==RF_FRAMEREWIND						//AMM 14Jan99
	|| _Replay.replayskip)												//AMM 25Feb99
	{
		int oldprec=GETPREC();
		SETPREC(3);
		
		while (num--)											//AMM 06Oct98
		{
			MobileItem::WinMove(timeofday,worldref);

			if (_Replay.replayskip)
			{	
				if (viewedwin)
				{
					View3d* view=viewedwin;

					view->frametime=2;
					_Replay.UpdateSmokes();
				}
			}
		}

		if (_DPlay.Implemented)
		{
			if (_DPlay.ResyncPhase)
			{
				Paused(TRUE);
				_DPlay.csync=false;
			}
		}

#pragma message(__HERE__"GETPREC()!=3  !!!!")
		SETPREC(oldprec);

		if (_Replay.Playback)
			_Replay.CalcCurrPos();
	}

	if (_Replay.Playback)
	{
		if (_Replay.ReplayFlag==RF_FRAMEREWIND || _Replay.ReplayFlag==RF_FRAMEFORWARD)						//AMM 14Jan99
		{															//AMM 14Jan99
			_Replay.ResetFlags();										//AMM 18Feb99
		}															//AMM 14Jan99
	}
}

void	View3d::BlockTick(Bool setit)
{
	if (setit==BOOL_Align) 											  //DAW 10/02/00
	{	//Special flag to say that timer should be aligned
		while (	inst->framecount==waitforframecount)
		{
			Sleep(0);
		}
		if (Save_Data.detail_3d[DETAIL3D_FASTFRAME])
		{
			while (	inst->framecount==waitforframecount)
				Sleep(0);
			while (inst->framecount==waitforframecount+1)
			{
				Sleep(0);
			}
		}
		waitforframecount=inst->framecount;
	}
	inst->BlockTick(setit);
}

void	ViewPoint::BlockTick(Bool q)
{
	view3dwin->BlockTick(q);
}

void	Inst3d::BlockTick(Bool setit)
{
	static int nestingcheck=0;
	while (insidetimer == TRUE)
	{Sleep(0);}

	if (!mapview)
	{
		if (setit)
		{
			if (nestingcheck++)	
				{_Error.EmitSysErr("blocktick nesting error (setit == true)!");
				}
			if (!mutex)	
				{ _Error.EmitSysErr("blocktick mutex null (setit == true)!"); 
				}
			WaitForSingleObject(mutex,INFINITE);				//AMM 13Jan00
		}
		else
		{
			if (--nestingcheck)	
				{_Error.EmitSysErr("blocktick nesting error (setit == false)!");
				}
			if (!mutex)
				{_Error.EmitSysErr("blocktick mutex null (setit == false)!");
				}
			ReleaseMutex(mutex);
		}
	}
}

int	View3d::PeekFrameTime()
{
	if (doneframe)
		return 0;
	else
		return frametime;
}
int	View3d::LastFrameTime()
{
	return lastframetime;
}
int View3d::TimeSinceLastMove()
{
	return timeSinceLastMove;
}
int	MigDisplay::FrameTime()
{
	if (view3dwin)
		return	view3dwin->LastFrameTime();
	else
		return 0;
}

int	ViewPoint::FrameTime()
{
	return	view3dwin->LastFrameTime();
}

int ViewPoint::TimeSinceLastMove()
{
	return view3dwin->TimeSinceLastMove();
}

int	ViewPoint::RealFrameTime()
{
	return	view3dwin->RealFrameTime();
}

int	ViewPoint::TimeOfDay()
{
	if (!this)														  //AMM 25/06/99
		return 0;													  //AMM 25/06/99
	return	view3dwin->TimeOfDay();
}

void	View3d::DoneFrame()
{
	doneframe=TRUE;
}

void	AggTracer(char* str)
{
}

bool ViewPoint::Accel()
{
	if (!this) return false;
	if (!view3dwin) return false;
	return view3dwin->Accel();
}
int COverlay::GetTime()
{
	return 0;	//Master_3d.currinst->timeofday;
}
