// bob_port shim: crtdbg.h -> DEBUG_NEW only (RERUN)
// Real crtdbg provides CRT debug alloc hooks; BoB only needs DEBUG_NEW
// for "#define new DEBUG_NEW" blocks. No-op impl keeps call sites compiling.
#ifndef _CRTDBG_H_
#define _CRTDBG_H_
#include <assert.h>
#ifndef DEBUG_NEW
#define DEBUG_NEW new
#endif
#ifndef _ASSERTE
#define _ASSERTE(expr) assert(expr)
#endif
typedef int _CrtMemState;
inline void _CrtMemCheckpoint(_CrtMemState*) {}
inline int _CrtSetDbgFlag(int f) { return f; }
#endif
