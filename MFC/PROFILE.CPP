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

// Profile.cpp : implementation file
//

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"
#include "RSTATIC.H" // RERUN for CRStatic
#include "RRADIO.H" // RERUN for CRRadio
#include "RCOMBO.H" // RERUN for CRCombo
#include "RCOMBOX.H" //RERUN
#include "RBUTTON.H" //RERUN for CRButton
#include "RTOOLBAR.H" //RERUN for CRToolBar
#include "REDIT.H" //RERUN for CREdit
#include "RLISTBOX.H" //RERUN for CRListBox
#include "MIGVIEW.H"
#include "BFNUMBER.H"
#include "PERSONS2.H"
#include "REDTBT.H" //RERUN for CREdtBt
#include "TASKTABS.H" //RERUN for CTaskTabs
#include "FLT_TASK.H" //RERUN for CFlt_Task
#include "WPBUT.H" //RERUN for WaypointButtons
#include "WPSHARE.H" //RERUN for CWPShare
#include "WPGROUND.H" //RERUN for CWPGround
#include "WPDETAIL.H" //RERUN for WPDetail
#include "ROUTE.H" //RERUN for CRoute
#include "WAVETABS.H" //RERUN for CWaveTabs
#include "WAVEINS.H" //RERUN for CWaveInsert
#include "TITLEBAR.H" //RERUN for TitleBar
#include "SAVEGAME.H"
#include "MAINTBAR.H" //RERUN see below
#include "MAINFRM.H" //RERUN combination with MAINTBAR for MainToolBar
#include "SAVEPROF.H" //RERUN for CSaveProf
#include "LOAD.H" //RERUN for CLoad

#include "PROFILE.H"


CWaveTabs	*CProfile::wavetabs=NULL;
CTaskTabs	*CProfile::tasktabs=NULL;
int			CProfile::wavepack=-1;
int			CProfile::wavegroup=-1;
/////////////////////////////////////////////////////////////////////////////
// CProfile dialog


CProfile::CProfile(bool enable, int package,CWnd* pParent /*=NULL*/)
	: RowanDialog(CProfile::IDD, pParent)
{
	SetProjSpecific(package);
	enableinput = enable;

	//{{AFX_DATA_INIT(CProfile)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProfile)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX,IDC_RLISTBOXCTRL1,m_IDC_RLISTBOXCTRL1);
	DDX_Control(pDX,IDC_INSERTWAVENEW,m_IDC_INSERTWAVENEW);
	DDX_Control(pDX,IDC_DELETEWAVENEW,m_IDC_DELETEWAVENEW);
	DDX_Control(pDX,IDC_TASK,m_IDC_TASK);
	DDX_Control(pDX,IDC_ROUTE,m_IDC_ROUTE);
	DDX_Control(pDX,IDC_SAVE,m_IDC_SAVE);
	DDX_Control(pDX,IDJ_TITLE,m_IDJ_TITLE);

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProfile, CDialog)
	//{{AFX_MSG_MAP(CProfile)
//	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfile message handlers

BEGIN_EVENTSINK_MAP(CProfile, CDialog)
    //{{AFX_EVENTSINK_MAP(CProfile)
	ON_EVENT(CProfile, IDC_PROFILE, 1 /* Clicked */, OnClickedProfile, VTS_NONE)
	ON_EVENT(CProfile, IDC_RLISTBOXCTRL1, 1 /* Select */, OnSelectRlistboxctrl1, VTS_I4 VTS_I4)
	ON_EVENT(CProfile, IDC_INSERTWAVE, 1 /* Clicked */, OnClickedInsertwave, VTS_NONE)
	ON_EVENT(CProfile, IDC_DELETEWAVE, 3 /* OK */, OnOKDeletewave, VTS_NONE)
	ON_EVENT(CProfile, IDC_DELETEWAVE, 1 /* Clicked */, OnClickedDeletewave, VTS_NONE)
	ON_EVENT(CProfile, IDC_DELETEWAVENEW, 1 /* Clicked */, OnClickedDeletewavenew, VTS_NONE)
	ON_EVENT(CProfile, IDC_INSERTWAVENEW, 1 /* Clicked */, OnClickedInsertwavenew, VTS_NONE)
	ON_EVENT(CProfile, IDC_OBJECTIVE, 1 /* Clicked */, OnClickedObjective, VTS_NONE)
	ON_EVENT(CProfile, IDC_CENTRE, 1 /* Clicked */, OnClickedCentre, VTS_NONE)
	ON_EVENT(CProfile, IDC_SAVE, 1 /* Clicked */, OnClickedSave, VTS_NONE)
	ON_EVENT(CProfile, IDC_TASK, 1 /* Clicked */, OnClickedTask, VTS_NONE)
	ON_EVENT(CProfile, IDC_ROUTE, 1 /* Clicked */, OnClickedRoute, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProfile::OnClickedProfile() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CProfile::OnInitDialog() 
{
	CDialog::OnInitDialog();
	currrow=1;
	currcol = 1;
   	RDialog* parent=(RDialog*)GetParent();

	CRButton* title;
	title = GETDLGITEM(IDJ_TITLE);
	title->SetString(GetTargName(Todays_Packages[packnum].packagetarget));

	CRListBox* rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);

	rlistbox->Clear();
	rlistbox->AddColumn(80);		//Wave
	rlistbox->AddColumn(45);		//Time
	rlistbox->AddColumn(80);		//Attack
	rlistbox->AddString(RESSTRING(WAVE),0);
	rlistbox->AddString(RESSTRING(TIMEOVERTARGET),1);
	rlistbox->AddString(RESSTRING(DUTY),2);
	rlistbox->AddColumn(80);		//AAA cover
	rlistbox->AddColumn(68);		//Air cover

	
	info_airgrp* a=*Persons2::ConvertPtrUID(Todays_Packages[packnum][0][0].uid);
	if (!(a->duty &DA_CAP)) //UI has simple duties
	{
		rlistbox->AddString(RESSTRING(GRNDESCORT),3);
		rlistbox->AddString(RESSTRING(AIRESCORT),4);
	}else
	{
		rlistbox->AddString("",3);
		rlistbox->AddString("",4);
	}

	ReDraw();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void	CProfile::ReDraw()
{
	maxrows=0;
	for (int row=0;row<Profile::MAX_WAVES;row++)
		if (Todays_Packages[packnum][row][0].uid)
			maxrows++;

	for (int row=0;row<maxrows;row++)
		if	(Todays_Packages[packnum][row][0].uid)
			FillWaveRow(row);

	CRListBox* rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);
	rlistbox->SetHilightRow(currrow);
	rlistbox->SetHilightCol(currcol);

	CRButton*	but = GETDLGITEM(IDC_INSERTWAVENEW);
	if  (		(enableinput)
			&&	(Todays_Packages.FindSquad(Todays_Packages[packnum].duty) >= 0)
		)
	{
		but->SetDisabled(false);
		but->SetForeColor(RGB(255,255,0));
	}
	else
	{
		but->SetDisabled(true);
		but->SetForeColor(RGB(80,80,80));
	}

	but = GETDLGITEM(IDC_DELETEWAVENEW);
	if  (		(enableinput)									//RDH 23Dec98
			&&	(currrow != 1)									//RDH 23Dec98
		)														//RDH 23Dec98
	{
		but->SetDisabled(false);
		but->SetForeColor(RGB(255,255,0));
	}else
	{
		but->SetDisabled(true);
		but->SetForeColor(RGB(80,80,80));
	}

	but = GETDLGITEM(IDC_ROUTE);
	if (enableinput)
	{
		but->SetDisabled(false);
		but->SetForeColor(RGB(255,255,0));
	}else
	{
		but->SetDisabled(true);
		but->SetForeColor(RGB(80,80,80));
	}
	but = GETDLGITEM(IDC_TASK);
	if (enableinput)
	{
		but->SetDisabled(false);
		but->SetForeColor(RGB(255,255,0));
	}else
	{
		but->SetDisabled(true);
		but->SetForeColor(RGB(80,80,80));
	}
}

void CProfile::MakeTaskTabs(int wave, int x, int y)
{
    wavegroup = wave;
    wavepack  = packnum;

    info_airgrp* a = *Persons2::ConvertPtrUID(
        Todays_Packages[packnum][wave][0].uid
    );

    int duty = IDS_L_DUTY_BOMB + a->duty.Evaluate() / ACTIONSUBCODEMAX;

    // --- Build dialog tree with no temporaries ---

    // Position
    CRect pos = Place(POSN_CENTRE, POSN_MIN, 0, 0);

    // Edges for the tasktabs DialBox
    Edges edgesTabs(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    // Create the tasktabs object
    tasktabs = new CTaskTabs;

    // First DialBox (tasktabs)
    DialBox boxTabs(
        FIL_NULL,          // IDB_BBC3 in original comment
        tasktabs,
        edgesTabs
    );

    // Edges for the HTabBox
    Edges edgesHTab(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    // IdList for the tabs
    IdList idlist(
        duty,
        IDS_GRNDESCORT,
        IDS_AIRESCORT
    );

    // Three DialBoxes for the tab pages
    DialBox boxStrike(
        FIL_MAP_STRIKE,
        new CFlt_Task(packnum, wave, 0)
    );

    DialBox boxAAA(
        FIL_MAP_AAA,
        new CFlt_Task(packnum, wave, 1)
    );

    DialBox boxMigCap(
        FIL_MAP_MIGCAP,
        new CFlt_Task(packnum, wave, 2)
    );

    // HTabBox containing the three task pages
    HTabBox htab(
        FIL_NULL,
        idlist,
        edgesHTab,
        boxStrike,
        boxAAA,
        boxMigCap
    );

    // DialList combining tasktabs + htab
    DialList list(
        boxTabs,
        htab
    );

    // Create the dialog
    RDialog* dlg = MakeTopDialog(pos, list);

    // Register as child 0
    LogChild(0, dlg);
}

void CProfile::MakeWaveTabs(int wave, int group, int x, int y)
{
    wavegroup = wave;
    wavepack  = packnum;

    info_airgrp* a = *Persons2::ConvertPtrUID(
        Todays_Packages[packnum][wave][0].uid
    );

    int duty = IDS_L_DUTY_BOMB + a->duty.Evaluate() / ACTIONSUBCODEMAX;

    // Position of the whole dialog
    CRect pos = Place(POSN_MAX, POSN_MAX);

    // --- Top-level wave tabs box ---

    Edges edgesWaveTabs(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    wavetabs = new CWaveTabs(packnum, wave);

    DialBox boxWaveTabs(
        FIL_NULL,      // /*IDB_BBC3*/
        wavetabs,
        edgesWaveTabs
    );

    // --- Common edges used in inner layouts ---

    Edges edgesFullInside(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    Edges edgesBottomStrip(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX,
        EDGE::ALIGN_MAX,
        EDGE::ALIGN_MAX
    );

    // --- Top HTab: MAIN ROUTE / TARGET AREA ---

    IdList idMainTabs(
        IDS_MAINROUTE,
        IDS_TARGETAREA1
    );

    Edges edgesMainTabs(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    // ----- MAIN ROUTE tab content -----

    // Route main
    Edges edgesRouteMain = edgesFullInside;

    DialBox boxRouteMain(
        FIL_MAP_ROUTE,
        new CRoute(wave, IDD_ROUTEMAIN),
        edgesRouteMain
    );

    // Main WP detail
    Edges edgesWPMainDetail = edgesFullInside;

    DialBox boxWPMainDetail(
        FIL_NULL,
        new WPDetail(packnum, wave, -1, UID_NULL, IDD_WPMAINDETAIL),
        edgesWPMainDetail
    );

    // Waypoint buttons (bottom strip)
    Edges edgesWPButtonsMain = edgesBottomStrip;

    DialBox boxWPButtonsMain(
        FIL_NULL,
        new WaypointButtons(),
        edgesWPButtonsMain
    );

    // WP share
    Edges edgesWPShare = edgesFullInside;

    DialBox boxWPShare(
        FIL_NULL,
        new CWPShare(packnum, wave, group),
        edgesWPShare
    );

    DialList listMainRoute(
        boxRouteMain,
        boxWPMainDetail,
        boxWPButtonsMain,
        boxWPShare
    );

    // --- Payload HTab (inside second page of main HTab) ---

    IdList idPayloadTabs(
        duty,
        IDS_GRNDESCORT,
        IDS_AIRESCORT
    );

    Edges edgesPayloadTabs(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    // Common route + detail + buttons edges for payload tabs
    Edges edgesRoutePayload = edgesBottomStrip;
    Edges edgesWPPayload    = edgesFullInside;
    Edges edgesButtonsPayload = edgesBottomStrip;

    // ----- Payload tab 0 (ground index 0) -----

    DialBox boxRoute0(
        FIL_NULL,
        new CRoute(wave),
        edgesRoutePayload
    );

    DialBox boxWPDetail0(
        FIL_NULL,
        new WPDetail(packnum, wave, -1, UID_NULL),
        edgesWPPayload
    );

    DialBox boxWPButtons0(
        FIL_NULL,
        new WaypointButtons(),
        edgesButtonsPayload
    );

    DialBox boxGround0(
        FIL_NULL,
        new CWPGround(packnum, wave, 0),
        edgesFullInside
    );

    DialList listPayload0(
        boxRoute0,
        boxWPDetail0,
        boxWPButtons0,
        boxGround0
    );

    // ----- Payload tab 1 (ground index 1) -----

    DialBox boxRoute1(
        FIL_NULL,
        new CRoute(wave),
        edgesRoutePayload
    );

    DialBox boxWPDetail1(
        FIL_NULL,
        new WPDetail(packnum, wave, -1, UID_NULL),
        edgesWPPayload
    );

    DialBox boxWPButtons1(
        FIL_NULL,
        new WaypointButtons(),
        edgesButtonsPayload
    );

    DialBox boxGround1(
        FIL_NULL,
        new CWPGround(packnum, wave, 1),
        edgesFullInside
    );

    DialList listPayload1(
        boxRoute1,
        boxWPDetail1,
        boxWPButtons1,
        boxGround1
    );

    // ----- Payload tab 2 (ground index 2) -----

    DialBox boxRoute2(
        FIL_NULL,
        new CRoute(wave),
        edgesRoutePayload
    );

    DialBox boxWPDetail2(
        FIL_NULL,
        new WPDetail(packnum, wave, -1, UID_NULL),
        edgesWPPayload
    );

    DialBox boxWPButtons2(
        FIL_NULL,
        new WaypointButtons(),
        edgesButtonsPayload
    );

    DialBox boxGround2(
        FIL_NULL,
        new CWPGround(packnum, wave, 2),
        edgesFullInside
    );

    DialList listPayload2(
        boxRoute2,
        boxWPDetail2,
        boxWPButtons2,
        boxGround2
    );

    // Payload HTab (three payload DialLists)
    HTabBox htabPayload(
        FIL_MAP_PAYLOAD,
        idPayloadTabs,
        edgesPayloadTabs,
        listPayload0,
        listPayload1,
        listPayload2
    );

    // --- Top HTab combining MAIN ROUTE and PAYLOAD HTab ---

    HTabBox htabMain(
        FIL_NULL,
        idMainTabs,
        edgesMainTabs,
        listMainRoute,
        htabPayload
    );

    // --- Root DialList: wave tabs + main HTab ---

    DialList rootList(
        boxWaveTabs,
        htabMain
    );

    RDialog* dlg = MakeTopDialog(pos, rootList);

    LogChild(1, dlg);
}


void	CProfile::ClearWaveRow(int wave)
{
	CRListBox* rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);
	rlistbox->ReplaceString("",wave+1,0);
	rlistbox->ReplaceString("",wave+1,1);
	rlistbox->ReplaceString("",wave+1,2);
	rlistbox->ReplaceString("",wave+1,3);
	rlistbox->ReplaceString("",wave+1,4);
}
void	CProfile::FillWaveRow(int wave)
{
	CRListBox* rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);
	rlistbox->SetNumberOfRows(maxrows+1);
	char buffer[10];
	int uid = (int)Todays_Packages[packnum][wave][0].uid;
	info_airgrp* a=*Persons2::ConvertPtrUID(Todays_Packages[packnum][wave][0].uid);
	sprintf(buffer,"%i.%s",wave+1,(const char*)RESLIST(DUTY_BOMB,a->duty.Evaluate()/ACTIONSUBCODEMAX));
	rlistbox->ReplaceString(buffer,wave+1,0);
	int t=Todays_Packages[packnum][wave].targettime/60;
	sprintf(buffer,"%02i:%02i",t/60,t%60);
	rlistbox->ReplaceString(buffer,wave+1,1);
	int ac = a->inform.Evaluate();
	if (ac)
	{
		sprintf(buffer,"%s (%i)",(const char*)RESLIST(SQ_BF_F86A,a->type.Evaluate()),a->inform.Evaluate()/4);
		rlistbox->ReplaceString(buffer,wave+1,2);
	}else
		rlistbox->ReplaceString("",wave+1,2);

	a=*Persons2::ConvertPtrUID(Todays_Packages[packnum][wave][1].uid);
	if (a)
	{
		sprintf(buffer,"%s (%i)",(const char*)RESLIST(SQ_BF_F86A,a->type.Evaluate()),a->inform.Evaluate()/4);
		rlistbox->ReplaceString(buffer,wave+1,3);
	}
	else
	{
		rlistbox->ReplaceString("",wave+1,3);
	}
	a=*Persons2::ConvertPtrUID(Todays_Packages[packnum][wave][2].uid);
	if (a)
	{
		sprintf(buffer,"%s (%i)",(const char*)RESLIST(SQ_BF_F86A,a->type.Evaluate()),a->inform.Evaluate()/4);
		rlistbox->ReplaceString(buffer,wave+1,4);
	}
	else
	{
		rlistbox->ReplaceString("",wave+1,4);
	}
}

void CProfile::OnSelectRlistboxctrl1(long row, long column) 
{
	if 	(enableinput)
	{
		if (column >= 2)
		{
			if (row && row<=Profile::MAX_PACKS)
				if	(Todays_Packages[packnum][row-1][0].uid)
				{
					SetTaskTabs(row-1,column-2);
					RDialog::m_pView->SetHiLightInfo(packnum, row-1, column-2, Todays_Packages.hiwp);
				}

		}else
		{
			if (row && row<=Profile::MAX_PACKS)
				if	(Todays_Packages[packnum][row-1][0].uid)
					if (column==0)
						SetWaveTabs(row-1,-1);
					else
						SetWaveTabs(row-1,-1);
		}
		currrow=row;
		currcol=column;
	}
	ReDraw();
}


void	CProfile::ChildDialClosed(int wave,RDialog* r,int code)
{	//User has closed the tabs sheet!
	if (wave==0)
		tasktabs=NULL;
	else if (wave == 1)
		wavetabs=NULL;
//DeadCode RDH 23Dec98 	FillWaveRow(wavegroup);
}
void	CProfile::RefreshData(int child)
{
//	FillWaveRow(wavegroup);
	if (child ==3)	 //
		SetWaveTabs(insertedwav,-1);

	ReDraw();
	if (child!=0 || (CFlt_Task::lastaction&CFlt_Task::REFRESHPARENT))
		RefreshParent();
	if (child==0 && LoggedChild(1) && wavetabs && (CFlt_Task::lastaction&CFlt_Task::REFRESHWPMASK))
		if ((CFlt_Task::lastaction&CFlt_Task::REFRESHWPMASK)==CFlt_Task::REFRESHWPMAIN)
			wavetabs->SelectTab(-1);
		else
				wavetabs->SelectTab((CFlt_Task::lastaction&CFlt_Task::REFRESHWPMASK)-CFlt_Task::REFRESHLOCALWP0);
}

void	CProfile::SetWaveTabs(int wave,int group)
{
	if (wave != wavegroup)
	{
		currrow = wave+1;
		if (	(group >2)	||	(group < 0)	)
			group = -1;

		currcol = group+2;
		ReDraw();

	}
	
	if  (			(wavetabs)
			&& 		(wave!=wavegroup || packnum!=wavepack)
		)
	{
		wavetabs->OnOK();
		wavetabs=NULL;
	}
	if  (			(tasktabs)
			&& 		(wave!=wavegroup || packnum!=wavepack)
		)
	{
		tasktabs->OnOK();
		tasktabs=NULL;
	}
	if (!wavetabs)
	{
		MakeWaveTabs(wave, group);
	}
	wavetabs->SelectTab(group);
}
void	CProfile::SetTaskTabs(int wave,int group)
{
	if  (			(wavetabs)
			&& 		(wave!=wavegroup || packnum!=wavepack)
		)
	{
		wavetabs->OnOK();
		wavetabs=NULL;
	}
	if  (			(tasktabs)
			&& 		(wave!=wavegroup || packnum!=wavepack)
		)
	{
		tasktabs->OnOK();
		tasktabs=NULL;
	}
	if (!tasktabs)
	{
		MakeTaskTabs(wave);
	}
	tasktabs->SelectTab(group);

}


void CProfile::EndDialog( int nResult )
{
//void CProfile::OnDestroy() 
//{
	CloseLoggedChildren();
	if (packnum==wavepack)
	{
//	if (wavetabs && packnum==wavepack)
//	{
//		wavetabs->OnOK();
		wavetabs=NULL;
//	}
//	if (tasktabs && packnum==wavepack)
//	{
//		tasktabs->OnOK();
		tasktabs=NULL;
	}
//	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	CDialog::EndDialog(nResult);
	
}

void CProfile::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}


void CProfile::OnClickedInsertwave() 
{
}

void CProfile::OnOKDeletewave() 
{
	// TODO: Add your control notification handler code here
	
}

void CProfile::OnClickedDeletewave() 
{
}

void CProfile::OnClickedDeletewavenew() 
{
	if (currrow)
	{
		int rv = RDialog::RMessageBox(IDS_DELETEWAVE, IDS_AREYOUSURE,0,IDS_L_YESNO_YES,IDS_CANCEL);
		if (rv == 1)
		{
			maxrows--;
			if (wavetabs)
				wavetabs->OnOK();
			if (tasktabs)
				tasktabs->OnOK();
			Todays_Packages[packnum].ClearWave(currrow-1);
			Todays_Packages[packnum].ShuntWave(currrow-1);
			if (currrow>=maxrows)
			{
				currrow=1;										//RDH 23Dec98
			}
		}
	}
	ReDraw();
}

void CProfile::OnClickedInsertwavenew() 
{
	int wav=0;
	while(wav<Profile::MAX_WAVES && Todays_Packages[packnum][wav].firstwp)
		wav++;
	if (wav==Todays_Packages[packnum][wav].firstwp)
		return;
	info_airgrp* a=*Persons2::ConvertPtrUID(Todays_Packages[packnum][0][0].uid);

	insertedwav = wav;
   	if (a->duty&DA_CAP)
	{
		maxrows++;
		currrow++;
		Todays_Packages[packnum].MakeWave(wav,a->duty);
		Todays_Packages[packnum][wav].targettime = Todays_Packages[packnum][0].targettime; 
		ReDraw();
		SetWaveTabs(wav,-1);

	}else
	{
		currrow++;

		typedef HideEdgeControl EDGE;

		if (LoggedChild(3))
			CloseLoggedChild(3);

		// Position
		CRect pos = Place(POSN_CALLER, POSN_CALLER, -3, -3);

		// Create the dialog content object
		CWaveInsert* insert = new CWaveInsert(
			packnum,
			wav,
			a->duty.Evaluate(),
			Todays_Packages[packnum][0].targettime
		);

		// Wrap it in a DialBox
		DialBox box(
			FIL_MAP_PROFILE,
			insert
		);

		// Create the dialog
		RDialog* dlg = RDialog::MakeTopDialog(pos, box);

		// Register as child 3
		LogChild(3, dlg);
	}
	
}

void CProfile::OnClickedObjective() 
{
	MainToolBar().OpenDossier(Todays_Packages[packnum].packagetarget);
//	CTargetDossier::MakeSheet(CRect(180,180,0,0),Todays_Packages[packnum].packagetarget);
}

void CProfile::OnClickedCentre() 
{
	m_pView->ZoomToUID(UniqueID(Todays_Packages[packnum].packagetarget),m_pView->m_zoom);

}

void CProfile::OnClickedSave()
{
    typedef HideEdgeControl EDGE;

    CString wildcard = "*.prf";

    CSaveProf::selectedfile =
        GetTargName(Todays_Packages[packnum].packagetarget) + ".prf";

    if (LoggedChild(2))
        CloseLoggedChild(2);

    // --- Build dialog tree safely ---

    // Position
    CRect pos = Place(POSN_CENTRE, POSN_CENTRE);

    // Edges for the first DialBox
    Edges edgesSave(
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MIN,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE,
        EDGE::ALIGN_MAX + EDGE::OCCLUDE_INSIDE
    );

    // First DialBox: CSaveProf
    DialBox boxSave(
        FIL_NULL,                 // /*IDB_BBC3*/
        new CSaveProf(packnum),
        edgesSave
    );

    // Second DialBox: CLoad
    DialBox boxLoad(
        FIL_MAP_PLAYER_LOG,       // /*IDB_BBC3*/
        new CLoad(
            IDD_SAVE,
            LSD_SAVE,
            FIL_USER_PROFILES_DIR,
            wildcard,
            CSaveProf::selectedfile
        )
    );

    // Combine into a DialList
    DialList list(
        boxSave,
        boxLoad
    );

    // Create the dialog
    RDialog* dlg = RDialog::MakeTopDialog(pos, list);

    // Register as child 2
    LogChild(2, dlg);
}


void CProfile::OnClickedTask() 
{
	// TODO: Add your control notification handler code here
	int col = currcol-2;
	if (col < 0)
		col = 0;
	SetTaskTabs(currrow-1, col);

}

void CProfile::OnClickedRoute() 
{
	SetWaveTabs(currrow-1,-1);
	
}
