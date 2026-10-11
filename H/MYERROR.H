//------------------------------------------------------------------------------
//Filename       error.h
//System         
//Author         Martin Alderton
//Date           Fri 6 Oct 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	error_Included
#define	error_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#include <cassert> // RERUN for e.g. in FILEMAN.H

class	 Error  
{	//can't allow this to be half commented out!
	FILE	*logfile;										//JIM 01Aug96
	char m_errorString[1024];

	public:
		const char* exitmsg;
		Error(void );
		~Error(void );

		Error& ExitMode();
		Error& Say(const char *, ...);
		Error& SayAndQuit(const char *, ...);
		void Quit();
#ifdef __WATCOMC__
#pragma	aux Quit			aborts;
#pragma	aux SayAndQuit		aborts;
#endif
		#if	DEBUGGING
			void OpenLog(void );
			void WriteToLog(const char *,...);
			void CloseLog(void );
			void KeyTrap(void );								//pd 01Nov95
			static	void	dummyproc(const char*)	{;}
			static	void	dummyproc(const char*,int)	{;}
			static	void	dummyproc(const char*,int,int)	{;}
			static	void	dummyproc(const char*,int,int,const char*)	{;}
			static	void	dummyproc(const char*,int,int,int)	{;}
			static	void	dummyproc(const char*,int,int,int,int)	{;}
			static	void	dummyproc(const char*,int,int,int,const char*,int)	{;}
			static	void	dummyproc(const char*,const char*)	{;}
			static	void	dummyproc(const char*,const char*,const char*)	{;}
			static	void	dummyproc(const char*,const char*,int,int)	{;}
			static	void	dummyproc(const char*,const char*,int)	{;}
			static	void	dummyproc(const char*,int,const char*)	{;}
		#endif
			static	void	ReportLastRunError();
			static void	PleaseReport(bool x)		{pleasereport=x;}
			static void	PleaseLog(bool x)			{pleaselog=x;}
			static void TerminateCondition(char* x=NULL)	
			{	if (x)
				{
					stackedterminatecondition=terminatecondition;
					terminatecondition=x;
				}
				else
				{
					terminatecondition=stackedterminatecondition;
					stackedterminatecondition=NULL;
				}
			}
	private:
		static const char*	terminatecondition;
		static const char*	stackedterminatecondition;
		static bool	pleasereport;
		static bool	pleaselog;
		static void terminate(); 
	protected:

};
extern	Error	_Error;

// RERUN: The NDEBUG version of EmitSysErr was discarding the error message.
// This change ensures the message is always processed, prepending file and line info.
#define EmitSysErr ExitMode().Say("%s:%d: ", __FILE__, __LINE__).SayAndQuit
#define ReallyEmitSysErr ExitMode().Say("%s:%d: ", __FILE__, __LINE__).SayAndQuit

#pragma pack(pop)
#endif
