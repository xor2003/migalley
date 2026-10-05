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

// MIG.cpp : Defines the class behaviors for the application.
//

#include "WIN32_COMPAT.H" //RERUN
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"

#include "UNIQUEID.H"
#include "RBUTTON.H" //RERUN for CRButton
#include "RTOOLBAR.H" //RERUN for CRToolBar
#include "TITLEBAR.H" //RERUN for TitleBar
#include "SAVEGAME.H" // RERUN for MAPFILTERSMAX
#include "RMDLDLG.H" // RERUN Added
#include "HINTBOX.H" // RERUN Added
#include "LISTBX.H" //RERUN Added
#include "MAINTBAR.H" //RERUN Added for include

#include "MAINFRM.H"
#include "MIGDOC.H"
#include "MIGVIEW.H"
#include "RDEMPTYD.H"
#include	"STUB3D.H"
//#include "Targets.h"
#include "WAVETABS.H"
//#include "Wave.h"
//#include "Chain.h"
//#include "Filters.h"
//#include "Title.h"
#include "GAMESET.H"
#include "SMACK.H"
#include "LISTBX.H"
#include "MONOTXT.H"
#include "RTESTSH1.H"

#include "WINMOVE.H" //RERUN for DPlay
extern DPlay _DPlay; //RERUN Added

/* #ifdef _DEBUG
#define new DEBUG_NEW
#ifndef	THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif */

//extern inline	int	LockExchange(int* loc,int newval=0);
//inline	int	LockExchange(int* loc,int newval=0)
//{
//	int rv;
//	_asm	{	mov		eax,newval
//				mov		ebx,loc
//				xchg	ds:[ebx],eax
//				mov		rv,eax
//			}
//	return	rv;
//}
/////////////////////////////////////////////////////////////////////////////
// CMIGApp

BEGIN_MESSAGE_MAP(CMIGApp, CWinApp)
	//{{AFX_MSG_MAP(CMIGApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMIGApp construction

CMIGApp::CMIGApp()
{
   resourceInst=NULL;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

GameSettings gameSettings;

class RCommandLineInfo:public CCommandLineInfo
{
public:
	GameSettings m_gameSettings;
	RCommandLineInfo() {};
	~RCommandLineInfo() {}
	void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );
};

extern	int introsmk;
//------------------------------------------------------------------------------
//Procedure		ParseParam
//Author		Paul.   
//Date			Thu 25 Jun 1998
//------------------------------------------------------------------------------
void RCommandLineInfo::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
	if (bFlag)
	{
		if (strcasecmp(lpszParam, "USE16") == 0)
		{
			//force use of 16 bit textures
			m_gameSettings.m_bUse16=true;
		}
		else if (strcasecmp(lpszParam,"NOMEM")==0)
		{
			//ignore video RAM test results
			m_gameSettings.m_bBypassMemCheck=true;
		}
		else if (strcasecmp(lpszParam,"NOINTRO")==0)
		{
			//ignore video RAM test results
			introsmk=0;
		}
		else if (strcasecmp(lpszParam,"WINDOWED")==0)
			m_gameSettings.m_bFullScreenMode=false;
		else if (strcasecmp(lpszParam,"HOST")==0)
		{
			// lobby host
			_DPlay.UIPlayerType=PLAYER_HOST;
		}
		else if (strcasecmp(lpszParam,"GUEST")==0)
		{
			// lobby guest
			_DPlay.UIPlayerType=PLAYER_GUEST;
		}
		else CCommandLineInfo::ParseParam(lpszParam,bFlag,bLast);
	}
	else CCommandLineInfo::ParseParam(lpszParam,bFlag,bLast);
}


/////////////////////////////////////////////////////////////////////////////
// CMIGApp initialization

CFont* (g_AllFonts[MAXFONTS][4])={{NULL}};
CDC g_OffScreenDC;
static int __stdcall EnumFontFamProc(const LOGFONT *lpelf,const TEXTMETRIC *lpntm,ULong FontType,LPARAM lParam )
{
	*(int*)lParam=true;
	return 1;//0;
}
 

void CreatePointFont(int ind, int point,const char* name,bool all4)
{
	//input is 10ths of a point
	//a point is 1/72 inches
	//units are 1/720 inches
	//96 pixels per inch
	//multiply by 96/720=6*16/3*3*5*16=2/15
	enum	{POINT2PIXMUL=2,POINT2PIXDIV=15};
	point*=-POINT2PIXMUL;
	
	g_AllFonts[ind][0]=new CFont;
	g_AllFonts[ind][0]->CreateFont(point/POINT2PIXDIV,0,	0,0,0,	0,0,0,
						DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_CHARACTER_PRECIS,
						DEFAULT_QUALITY,DEFAULT_PITCH,
						name);
	g_AllFonts[ind][3]=new CFont;
	g_AllFonts[ind][3]->CreateFont(point*2/POINT2PIXDIV,0,	0,0,0,	0,0,0,
						DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_CHARACTER_PRECIS,
						DEFAULT_QUALITY,DEFAULT_PITCH,
						name);
	if (all4)
	{
		g_AllFonts[ind][1]=new CFont;
		g_AllFonts[ind][1]->CreateFont((point*4)/(3*POINT2PIXDIV),0,	0,0,0,	0,0,0,
						DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_CHARACTER_PRECIS,
						DEFAULT_QUALITY,DEFAULT_PITCH,
						name);
		g_AllFonts[ind][2]=new CFont;
		g_AllFonts[ind][2]->CreateFont((point*5)/(3*POINT2PIXDIV),0,	0,0,0,	0,0,0,
						DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_CHARACTER_PRECIS,
						DEFAULT_QUALITY,DEFAULT_PITCH,
						name);
	}
}

__inline HWND FindMSWheel( 
      PUINT puiMsh_MsgMouseWheel,
      PUINT puiMsh_Msg3DSupport=NULL,
      PUINT puiMsh_MsgScrollLines=NULL,
      PBOOL pf3DSupport=NULL,
      PINT  piScrollLines=NULL
)
{
   HWND hdlMsWheel;

   hdlMsWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
   if (puiMsh_MsgMouseWheel)
	   *puiMsh_MsgMouseWheel=0;
   if (hdlMsWheel)
   {
	   int	Msh_MsgMouseWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);
	   int	Msh_Msg3DSupport = RegisterWindowMessage(MSH_WHEELSUPPORT);
	   int	Msh_MsgScrollLines = RegisterWindowMessage(MSH_SCROLL_LINES);

	   if (pf3DSupport)
	   if (*puiMsh_Msg3DSupport)
		  *pf3DSupport = (BOOL)SendMessage(hdlMsWheel, Msh_Msg3DSupport, 0, 0);
	   else
		  *pf3DSupport = FALSE;  // default to FALSE

	   if (piScrollLines)
	   if (*puiMsh_MsgScrollLines )
		  *piScrollLines = (int)SendMessage(hdlMsWheel, Msh_MsgScrollLines, 0, 0);
	   else
		  *piScrollLines = 3;  // default
	   if (puiMsh_MsgMouseWheel)
		   *puiMsh_MsgMouseWheel=Msh_MsgMouseWheel;
	   if (puiMsh_Msg3DSupport)
		   *puiMsh_Msg3DSupport=Msh_Msg3DSupport;
	   if (puiMsh_MsgScrollLines)
		   *puiMsh_MsgScrollLines=Msh_MsgScrollLines;
   }
   return(hdlMsWheel);
}

extern SWord winmode_bpp;
extern SWord	winmode_w, winmode_h; //RERUN
BOOL CMIGApp::InitInstance()
{
	INT		sysnames[]={COLOR_SCROLLBAR,COLOR_3DDKSHADOW};
	DWORD	sysvals[]={0x0000ff00,0x00ff0000};

	TRACE0("InitInstance starting...\n");

	FindMSWheel(&wheelmessagenum);

	if (!RDialog::actscrw)										//RJS 08Sep98
	{	//1st time in read registry...
#if 0 // RERUN, no reading in windows registry
		HKEY k;
		DWORD type;
		unsigned char buff[10];
		DWORD size=9;
		RegOpenKeyEx( 
				HKEY_CURRENT_USER,
				"Control Panel\\desktop\\WindowMetrics",
				0, KEY_ALL_ACCESS, &k);
		RegQueryValueEx(k,"ScrollWidth",NULL,&type,buff,&size);
		if (type==REG_SZ)
			RDialog::actscrw=atoi((char*)buff)/12;
		else
			RDialog::actscrw=*(int*)buff/12;
		size=9;
		RegQueryValueEx(k,"ScrollHeight",NULL,&type,buff,&size);
		if (type==REG_SZ)
			RDialog::actscrh=atoi((char*)buff)/12;
		else
			RDialog::actscrh=*(int*)buff/12;
		size=9;
		RegQueryValueEx(k,"ScrollWidth",NULL,&type,buff,&size);
		if (type==REG_SZ)
			RDialog::actscrw=atoi((char*)buff)/12;
		else
			RDialog::actscrw=*(int*)buff/12;
		size=9;
		RegQueryValueEx(k,"BorderWidth",NULL,&type,buff,&size);
		if (type==REG_SZ)
			RDialog::borderwidth=atoi((char*)buff)/15;
		else
			RDialog::borderwidth=*(int*)buff/15;
		RDialog::borderwidth=2-RDialog::borderwidth;
		RegCloseKey(k);
#else // RERUN No reading in windows registry
		RDialog::actscrw=16; //Something typical
		RDialog::actscrh=16; //Something typical
		RDialog::borderwidth=1; // 1 pixel
#endif // RERUN No reading in windows registry
	}															//RJS 08Sep98
	RDialog::fontdpi=RDialog::GetFontScaling();


    // one of the first things in the init code
	//Load alternate language resources
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
//RERUN	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

#if 0 // RERUN no registry
	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Rowan Software Ltd"));				
	//This is not being used as we still have a settings.mig :(
	//I think for BoB we should progress to using the registry ...
	//however - I will use this to indicate that the program is running

	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)
#endif // RERUN no registry

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMIGDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMIGView));
	pDocTemplate->SetContainerInfo(IDR_CNTR_INPLACE);
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	CWinApp::InitInstance(); //RERUN added call to base implementation to activate what is setup with AddDocTemplate

	// Parse command line for standard shell commands, DDE, file open
	//CCommandLineInfo cmdInfo;
	RCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	gameSettings=cmdInfo.m_gameSettings;

	// RERUN: if the frame window failed to create (e.g. SDL_CreateWindow
	// failed), m_pMainWnd is garbage — bail instead of dereferencing it.
	if (!m_pMainWnd)
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ModifyStyle(m_pMainWnd->GetStyle(),WS_THICKFRAME); // Bye bye title bar
//	m_pMainWnd->ModifyStyle(m_pMainWnd->GetStyle(),NULL); // Bye bye title bar and edges
	m_pMainWnd->SetMenu(NULL); // Bye bye menu  Status bar removed in MainFrm.cpp
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);

	Master_3d.Init(m_hInstance,m_pMainWnd->GetHwnd());

//#ifndef	NDEBUG
//In normal development, we don't want to load this DLL because of the risk of orphan DLLs
//It is too late in MiG Alley for this sort of garbage;
//If you do get a orphan DLL you will have to exit DevStudio
#if 0 // RERUN not needed
	char buffer[80];
 	resourceInst = LoadLibrary(File_Man.NameNumberedFile(FIL_LANGRESOURCEDLL,buffer));//"c:\\mig\\src\\english\\debug\\miglang.dll");
	if (resourceInst != NULL)
		AfxSetResourceHandle(resourceInst);
#endif // RERUN not needed

	oldhelppath=m_pszHelpFilePath ;
	if (File_Man.existnumberedfile(FIL_LANGRESOURCEHELPTABSANDPANELS))
	{
		m_pszHelpFilePath =  File_Man.NameNumberedFile(FIL_LANGRESOURCEHELPTABSANDPANELS,helpfilepath);
		((CMainFrame*)(m_pMainWnd))->disablehelp=0;
	}
	else
	if (File_Man.existnumberedfile(FIL_LANGRESOURCEHELPPANELS))
	{
		m_pszHelpFilePath =  File_Man.NameNumberedFile(FIL_LANGRESOURCEHELPPANELS,helpfilepath);
		((CMainFrame*)(m_pMainWnd))->disablehelp=1;
	}
	else
	if (File_Man.existnumberedfile(FIL_LANGRESOURCEHELPSEPARATETABS)) 
	{
		m_pszHelpFilePath =  File_Man.NameNumberedFile(FIL_LANGRESOURCEHELPSEPARATETABS,helpfilepath);
		((CMainFrame*)(m_pMainWnd))->disablehelp=2;
	}
	else
	if (File_Man.existnumberedfile(FIL_LANGRESOURCEHELP)) 
	{
		m_pszHelpFilePath =  File_Man.NameNumberedFile(FIL_LANGRESOURCEHELP,helpfilepath);
		((CMainFrame*)(m_pMainWnd))->disablehelp=3;

	}

	CDC* pdc=m_pMainWnd->GetDC();
	g_OffScreenDC.CreateCompatibleDC(pdc);
	g_OffScreenDC.SetMapMode(MM_TEXT);
	m_pMainWnd->ReleaseDC(pdc);
	const char* curlyfont="?????????"; //RERUN
	const char* straightfont="?????????????"; //RERUN
	int	gotfont=false;
	const char*	myfont="Intel";										//DAW 29Oct99

#if 0 // RERUN Windows only
	EnumFontFamilies(GetDC(m_pMainWnd->m_hWnd),curlyfont,EnumFontFamProc, (LPARAM)&gotfont);
	if (gotfont)
	{	//must be doing japanese!
		myfont=curlyfont;
	}
	else
	{
		straightfont="Arial";
		curlyfont="Arial Italic";
		EnumFontFamilies(GetDC(m_pMainWnd->m_hWnd),myfont,EnumFontFamProc, (LPARAM)&gotfont);
 		if (!gotfont)
			myfont="Times New Roman Bold";
		EnumFontFamilies(GetDC(m_pMainWnd->m_hWnd),myfont,EnumFontFamProc, (LPARAM)&gotfont);
 		if (!gotfont)
			myfont="MS Serif";
	}
#else // RERUN Windows only
	straightfont="Arial";
	curlyfont="Arial Italic";
#endif // RERUN Windows only

	CreatePointFont(0,90,curlyfont,FALSE); // data font (small frontend and map)
	CreatePointFont(1,200,myfont,FALSE); // title bar font (map only)
	CreatePointFont(2,104,straightfont,FALSE); // Arial font (small frontend and map) was 110
	CreatePointFont(3,200,curlyfont,FALSE); // data font (large frontend) was 220
	CreatePointFont(4,200,straightfont,FALSE); // the default listbox Arial font was 220
	CreatePointFont(5,200,myfont,FALSE); // Head font (large frontend) was 220
	CreatePointFont(6,286,myfont,FALSE); // the default title screen font
	CreatePointFont(7,110,curlyfont,FALSE); // data font (small frontend and map) (again)
	CreatePointFont(8,120,straightfont,FALSE); // Arial font (small frontend and map) was 110
	CreatePointFont(9,140,curlyfont,FALSE); // data font (small frontend and map) (again)
	CreatePointFont(10,150,myfont,TRUE); // front screen listbox font

	((CMainFrame*)(m_pMainWnd))->InitialiseSafe();
	m_pMainWnd->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	//CWinApp::InitInstance(); // call base implementation;

	return TRUE;
}

int CMIGApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CWinApp::ExitInstance();
}
CMIGApp::~CMIGApp()
{
	m_pszHelpFilePath= oldhelppath;
#if 0 // RERUN Not needed
    if (resourceInst != NULL)
        FreeLibrary(resourceInst);
#endif // RERUN Not needed
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMIGApp::OnAppAbout()
{
	RDEmptyD aboutDlg;
//	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMIGApp commands

int CMIGApp::Run() 
{
	TRACE0("MessageLoop starting...\n");
	// TODO: Add your specialized code here and/or call the base class
	for (int i=0;i<Master_3d.NUM_EVENTS;i++)
		Master_3d.htable[i]=CreateEvent(0,0,0,0);

	// Culled from CWinApp::Run() and CWinTread::Run()
	// The following bit is called once in CWinApp::Run()
	if (m_pMainWnd == NULL && AfxOleGetUserCtrl())
	{
		// Not launched /Embedding or /Automation, but has no main window!
		TRACE0("Warning: m_pMainWnd is NULL in CWinApp::Run - quitting application.\n");
		AfxPostQuitMessage(0);
	}


	// Add your specialized code here and/or call the base class
	//
	// The following is adapted from CWinThread::Run()
	// OnIdle()			is called when there are no more messages
	// PumpMessage()	is called when there is a windows message
	//						-returns FALSE after WM_QUIT message.
	//
	// I need to add new code for the message pump when in the 3d.
	// At present, I will just cheat.
	//

 	ASSERT_VALID(this);
	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		if (Rtestsh1::tmpview)
		{
			Rtestsh1::tmpview->drawloop(Rtestsh1::tmpview);
		}
		int result;
		ULong t;
		if (Inst3d::InThe3D()||!bIdle)
			result=MsgWaitForMultipleObjects(Mast3d::NUM_EVENTS,Master_3d.htable,FALSE,INFINITE,QS_ALLINPUT);//don't wait - ever!
		else
			result=MsgWaitForMultipleObjects(Mast3d::NUM_EVENTS,Master_3d.htable,FALSE,0,QS_ALLINPUT);//don't wait - ever!

		switch (result)
		{
		case WAIT_OBJECT_0+Mast3d::EVENT_KEYS:
			break;
		case WAIT_OBJECT_0+Mast3d::EVENT_MOUSE:
			break;

		case WAIT_OBJECT_0+Mast3d::EVENT_AGGREGATOR:
			break;

#if 1 //RERUN     _MSC_VER >= 1300
#define AFXATTR AfxGetThreadState()->m_msgCur
#else
#define AFXATTR m_msgCur
#endif

		case WAIT_OBJECT_0+Mast3d::NUM_EVENTS:
			while (::PeekMessage(&AFXATTR, NULL, 0, 0, PM_NOREMOVE))
			{
				bool fIn3d=Inst3d::InThe3D();
				if ((Rtestsh1::tempblockkeys || fIn3d) && AFXATTR.message >= WM_KEYFIRST &&  AFXATTR.message <= WM_KEYLAST)
				{
					while (::PeekMessage(&AFXATTR, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
					{}
				}
				else
					if ((Rtestsh1::tempblockkeys || fIn3d) && wheelmessagenum && AFXATTR.message == wheelmessagenum)
				{
					while (::PeekMessage(&AFXATTR, NULL, wheelmessagenum, wheelmessagenum, PM_REMOVE))
					{}
				}
				else
				{
					if (!PumpMessage())
					{
						TRACE0("MessageLoop exitting...\n");
						return ExitInstance();
					}
				}
			}
			if (IsIdleMessage(&AFXATTR))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}
			break;
		default:
			RDialog::m_moving=false;
			if (!OnIdle(lIdleCount++))
				bIdle = FALSE; // assume "no idle" state
		}
	}

	ASSERT(FALSE);  // not reachable
}
