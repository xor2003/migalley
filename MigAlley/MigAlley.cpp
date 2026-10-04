// MigAlley.cpp : Defines the entry point for the console application.
//
#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"
#include "MIGVIEW.H"


/////////////////////////////////////////////////////////////////
// The one and only CMIGApp object
CMIGApp theApp;

#ifndef __MSVC__
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//RERUN debug: last GetShapePtr call state
int		g_shp_last_num = -1;
void*	g_shp_last_fb = 0;
void*	g_shp_last_data = 0;
static void segv_backtrace(int sig)
{
	void*	buf[32];
	int		n = backtrace(buf, 32);
	const char*	msg = "\n*** fatal signal — backtrace ***\n";
	write(STDERR_FILENO, msg, strlen(msg));
	char	dbg[160];
	int		dl = snprintf(dbg, sizeof dbg, "shpdbg: num=%d fb=%p data=%p\n",
						g_shp_last_num, g_shp_last_fb, g_shp_last_data);
	write(STDERR_FILENO, dbg, dl);
	backtrace_symbols_fd(buf, n, STDERR_FILENO);
	_exit(128 + sig);
}
#endif

int main(int argc, char** argv)
{
#ifndef __MSVC__
	struct sigaction sa = {0};
	sa.sa_handler = segv_backtrace;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESETHAND;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
#endif

    if (!theApp.InitInstance())
        return -1;

    return theApp.Run();
}

/////////////////////////////////////////////////////////////////
