// bob_keystub_conv.cpp — regression test for keytests::Reg3dConv's
// terminator peek in input/keystub.cpp.
//
// The map copy loop advanced mapreq and then read mapreq->bitflag as the
// terminator check unconditionally — for a keyb3d.bin with no zero-bitflag
// sentinel (the shipped file is exactly 628 records) that read one record
// past the file block (ASan heap-buffer-overflow on 3D entry).  Fixed to
// stop on the record count before touching the next entry.
//
// The implementation TU is included directly so the test runs the real
// loop; the record array ends flush against a PROT_NONE page so the old
// peek faults deterministically without a sanitizer.

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "WIN32_COMPAT.H"
#include "dosdefs.h"

#include "../tests/unit/harness.h"
extern int g_checks;
extern int g_failures;

// Unused by this TU's tests — the real object lives in monotxt.cpp; a
// definition is required here only because keystub's ctor prints to it.
#include "monotxt.h"
void MonoText::ClsMono() {}
MonoText Mono_Text;

#include "keystub.cpp"

void test_keystub_reg3dconv_bound()
{
    keytests kt;
    KeyMap3d* km = (KeyMap3d*)kt.reftable3d.flat;
    CHECK(km != NULL);
    if (!km) return;

    long ps = sysconf(_SC_PAGESIZE);
    char* r = (char*)mmap(NULL, 2 * ps, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(r != MAP_FAILED);
    if (r == MAP_FAILED) return;
    mprotect(r + ps, ps, PROT_NONE);

    // Three records with no terminator, ending exactly at the guard page.
    // The pre-fix loop peeked at arr[3].bitflag — inside PROT_NONE.
    KeyMapping* arr = (KeyMapping*)(r + ps - 3 * sizeof(KeyMapping));
    memset(arr, 0, 3 * sizeof(KeyMapping));
    arr[0].scancode = 0x39; arr[0].shiftstate = 0; arr[0].bitflag = 112;
    arr[1].scancode = 0x19; arr[1].shiftstate = 0; arr[1].bitflag = 114;
    arr[2].scancode = 0x3C; arr[2].shiftstate = 0; arr[2].bitflag = 134;

    kt.Reg3dConv(arr, 3);

    CHECK_EQ(km->mappings[0x39][0], 112);
    CHECK_EQ(km->mappings[0x19][0], 114);
    CHECK_EQ(km->mappings[0x3C][0], 134);
    CHECK_EQ(km->active, TRUE);

    // A mid-table terminator still truncates, same as the shipped path.
    memset(arr, 0, 3 * sizeof(KeyMapping));
    arr[0].scancode = 0x2C; arr[0].shiftstate = 0; arr[0].bitflag = 300;
    arr[1].scancode = 0x01; arr[1].shiftstate = 0; arr[1].bitflag = 0;
    arr[2].scancode = 0x02; arr[2].shiftstate = 0; arr[2].bitflag = 555;

    kt.Reg3dConv(arr, 3);
    CHECK_EQ(km->mappings[0x2C][0], 300);
    CHECK_EQ(km->mappings[0x02][0], 0);    // cleared by ClearKeyTables, no leak

    munmap(r, 2 * ps);
}
