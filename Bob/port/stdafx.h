// bob_port/stdafx.h — replacement for BoB's h/stdafx.h precompiled header.
//
// Original pulled real MFC (<afxctl.h> <afxwin.h> ...) + dosdefs + messages +
// mfc/afxauto.h (every r* OCX wrapper + dialog header). Here the afx* names
// resolve to Bob/port/*.h shims -> this project's WIN32_COMPAT + MFC_stub
// compat layer, and the r* wrapper headers in BoB's h/ are forwarding
// headers onto the sibling port's CWnd-native controls (T118).
//
// Placed BEFORE BoB's h/ in -I order so mfc/*.cpp's #include "stdafx.h"
// (absent in mfc/) lands here. The include list mirrors the prefix of
// mfc/afxauto.h — the shared declaration surface the PCH gave every TU.
#pragma once

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
// BoB's h/mig.h PCH-checks this macro that real afxwin.h defined.
#ifndef __AFXWIN_H__
#define __AFXWIN_H__
#endif
#include "dosdefs.h"
#include "messages.h"

// value types + resource IDs needed by the widget/dialog headers below
#include "resource.h"
#include "globdefs.h"
#include "files.g"
#include "fileman.h"
#include "UniqueID.h"

// --- mfc/afxauto.h equivalent ------------------------------------------
// Generated list: every header the real PCH included, so every TU sees the
// same shared declaration surface it did under MSVC. (7 dead entries
// skipped — see afxauto_equiv.h header comment.)
#include "afxauto_equiv.h"
