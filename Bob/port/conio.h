// bob_port shim: DOS-era <conio.h>. Only used by winerror.cpp's KeyTrap()
// debug break — a no-op under the port (kbhit() false => getch() unreached).
#pragma once

static inline int kbhit(void) { return 0; }
static inline int getch(void) { return 0; }
