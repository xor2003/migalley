//------------------------------------------------------------------------------
//Filename       WinMain.h
//System         
//Author         Andrew McRae
//Date           Wed 2 Jul 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	main_Included
#define	main_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#include	"GETSET.H"
#include	"SCREENXY.H"

#define APP_NAME    "MiG Alley"
#define APP_TITLE   "MIG ALLEY"

#define	ID_TIMER	1

#define	NOGUID	(LPGUID)-1

typedef ULong myDPID;
typedef ULong* myLPDPID;

class	Main : public	GetSet
{
	public:

		void	MainInit(void);
		void	QuitGame(void );
		void	RowanMessage();
		void	DoScreen(MigWindow *);						//JIM 12Dec95

 		Bool	InitFileSystem(InitDisplayDef&);

		// Windows Params
		HWND hWnd;

		// Message Loop Control + task switching bits'n'bobs
		ScreenRez	prevMode;
		UINT 		AppActive;
		Bool 		Minimized;
		Bool 		GameEnd;
		Bool 		DDrawInitialised;

		// Timer Stuff
		TIMECAPS    tc;
		UINT        wTimerRes;
		UINT		uTimerID;

		// Misc
		char MyString [256];

		// Palette management stuff

	private:

	protected:

};

	extern Main _Main;		//For 'EmitSysErr'			//JIM 21Sep95

#pragma pack(pop)
#endif


//RERUN long FAR PASCAL DLLExport WndProc (HWND, UINT, UINT, LONG);

VOID MainLoopThread (PVOID pvoid);
VOID TimerThread (PVOID pvoid);
void CALLBACK TimeProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

Bool GetCDFile (FileNum, char* Name, ULong Offset);
void ThreadFunction (void * ptr);
