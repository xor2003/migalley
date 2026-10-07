// bob_main.cpp — dedicated `bob` executable entry (REQ-LAUNCH-01/02).
//
// Mirrors MigAlley.cpp: shared-shell install gate first (REQ-DETECT-01/02/03),
// then straight into BoB's real MFC entry — CMIGApp theApp lives in
// bob-flight-sim's mfc/mig.cpp (bob_frontend).

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ucontext.h>
#include <execinfo.h>
#include "detect.h"   // rowan-engine shared shell

#include <SDL.h>
#include <SDL_ttf.h>

#include "stdafx.h"   // Bob/port — MFC compat context (afxwin etc.)
#include "mig.h"      // h/mig.h via bob_overlay — class CMIGApp

extern CMIGApp theApp;

// MFC_stub.cpp — loads the install's RC json (BOB_RC.json) into the shared
// string table and dialog-template store. BoB's CMIGApp::InitInstance never
// chains to CWinApp::InitInstance, so it must run here (after chdir).
extern void LoadDialogTemplates();

// Bob/port/dd_present.cpp — installs RowanDDPresentHook so DirectDraw
// Flip/Blt presents upload the primary surface to the SDL window (T123).
extern void BoBInstallDDPresent();

#ifndef __MSVC__
// Async-signal-safe crash reporter.  The old handler called backtrace(),
// which dlopen()s libgcc_s and mallocs — if the faulting thread already
// held the malloc arena lock (e.g. a SEGV inside _int_malloc) the handler
// deadlocked on the arena and hid the real fault.  This version writes
// only the fault address + faulting EIP using async-safe calls, then exits.
static char* segv_hex(char* out, unsigned long v)
{
	static const char* digs = "0123456789abcdef";
	if (v > 0xf) out = segv_hex(out, v >> 4);
	*out++ = digs[v & 0xf];
	return out;
}
static void segv_report(int sig, siginfo_t* si, void* uc)
{
	ucontext_t* u = (ucontext_t*)uc;
	char buf[128];
	char* p = buf;
	static const char head[] = "\n*** bob fatal signal ";
	memcpy(p, head, sizeof(head) - 1); p += sizeof(head) - 1;
	p = segv_hex(p, (unsigned long)sig);
	static const char mid[] = " — si_addr=0x";
	memcpy(p, mid, sizeof(mid) - 1); p += sizeof(mid) - 1;
	p = segv_hex(p, (unsigned long)(si ? si->si_addr : 0));
	static const char mid2[] = " eip=0x";
	memcpy(p, mid2, sizeof(mid2) - 1); p += sizeof(mid2) - 1;
	p = segv_hex(p, u ? (unsigned long)u->uc_mcontext.gregs[REG_EIP] : 0);
	static const char tail[] = " ***\n";
	memcpy(p, tail, sizeof(tail)); p += sizeof(tail) - 1;
	write(STDERR_FILENO, buf, (size_t)(p - buf));
	_exit(128 + sig);
}
#endif

int main(int argc, char** argv)
{
#ifndef __MSVC__
	struct sigaction sa = {0};
	sa.sa_sigaction = segv_report;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
#endif

	// Shell gate — unconditional, no bypass (REQ-DETECT-01/02/03).
	const char* dir = (argc > 1 && argv[1][0] != '-') ? argv[1] : ".";
	rowan_shell::Report rep = rowan_shell::detect_install(dir, "bob");
	if (rep.verdict != rowan_shell::Verdict::OK)
	{
		fprintf(stderr, "bob: %s\n", rep.detail.c_str());
		fprintf(stderr, "usage: bob [install-dir]  (default: .)\n");
		return 2;
	}
	if (chdir(dir) != 0)
	{
		fprintf(stderr, "bob: cannot enter %s: %s\n", dir, strerror(errno));
		return 2;
	}

	// Same subset as CWinApp::InitInstance minus doc-template creation —
	// BoB builds its own document/frame inside CMIGApp::InitInstance.
	LoadDialogTemplates();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK |
	             SDL_INIT_GAMECONTROLLER) != 0) {
		fprintf(stderr, "bob: SDL_Init failed: %s\n", SDL_GetError());
		return 2;
	}
	if (TTF_Init() == -1) {
		fprintf(stderr, "bob: TTF_Init failed: %s\n", TTF_GetError());
		return 2;
	}
	SDL_DisableScreenSaver();
	BoBInstallDDPresent();

	if (!theApp.InitInstance())
		return -1;

	return theApp.Run();
}
