// bob_main.cpp — dedicated `bob` executable entry (REQ-LAUNCH-01/02).
//
// RERUN: embeds the rowan shared-shell detection gate exactly like
// MigAlley.cpp; the game module entry is wired incrementally as BoB's
// frontend TUs land on the compat layer (T116 inventory → waves 2-3).
// Until the module seam is connected the exe refuses to pretend: it
// passes detection, reports port status, and exits 3 (distinct from
// the gate's exit-2 so CI can tell "bad install" from "not yet wired").

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "detect.h"   // rowan-engine shared shell

int main(int argc, char** argv)
{
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

	// wave-2 seam: CMIGApp/theApp entry once mfc/ TUs compile on the
	// compat layer. Not a frontend claim — explicit non-launch status.
	fprintf(stderr, "bob: detected %s/%s — frontend port in progress (wave 2)\n",
			rep.game.c_str(), rep.edition.c_str());
	return 3;
}
