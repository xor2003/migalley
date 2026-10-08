// bob_unit_tests.cpp — regression tests for defect fixes found under
// ASan/UBSan in the live BoB code paths.  Each case maps to a real fault
// report; where the original defect was an out-of-object byte access that
// wrote/reads back unchanged values (invisible to value checks), the test
// places the object flush against an mprotect PROT_NONE page so the old
// code faults deterministically even in a non-sanitized build.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// Shared check macros (same as the migalley unit suite).
#include "../tests/unit/harness.h"
int g_checks;
int g_failures;

// BoB-side headers resolve through the generated overlay dir first.
#include "dosdefs.h"
#include "mathasm.h"

// lib3d.h needs the rowan DX compat types in scope first (same as the
// bob_lib3d_smoke TU does).
#include <d3dtypes.h>
#include <d3d.h>
#include "lib3d.h"

//------------------------------------------------------------------------------
// mathasm bit ops on narrow storage — CSQuick1::nonplayer is a 2-byte
// MakeField; the original x86 `bts mem, reg` did a 32-bit RMW at the base,
// touching 2 bytes past the field (ASan global-buffer-overflow).  Fixed to
// byte-granular access.  A value check cannot see the old bug (bts writes
// back the same bytes), so the field sits at the end of a mapped page with
// a PROT_NONE page after it: the 32-bit write faults, the byte write doesn't.
//------------------------------------------------------------------------------
static void test_bitops_narrow_guardpage()
{
    long ps = sysconf(_SC_PAGESIZE);
    char* r = (char*)mmap(NULL, 2 * ps, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(r != MAP_FAILED);
    if (r == MAP_FAILED) return;
    CHECK_EQ(mprotect(r + ps, ps, PROT_NONE), 0);

    UWord* f = (UWord*)(r + ps - 2);   // last 2 bytes before the guard page
    *f = 0;

    CHECK_EQ(BITSET(f, 0), 0);
    CHECK_EQ(BITSET(f, 15), 0);        // top bit of the 2-byte field
    CHECK_EQ(*f, 0x8001);
    CHECK_EQ(BITSET(f, 15), 1);        // reports previous state

    CHECK_EQ(BITTEST(f, 0), 1);
    CHECK_EQ(BITTEST(f, 7), 0);
    CHECK_EQ(BITTEST(f, 15), 1);

    CHECK_EQ(BITCOMP(f, 0), 1);        // was set -> clears
    CHECK_EQ(*f, 0x8000);
    CHECK_EQ(BITRESET(f, 15), 1);
    CHECK_EQ(*f, 0);
    CHECK_EQ(BITRESET(f, 15), 0);      // already clear

    munmap(r, 2 * ps);
}

// Unbounded bit-string semantics must still work: bit indices >= 32 walk
// the byte stream exactly like the CPU bit-string addressing did.
static void test_bitops_wide_string()
{
    unsigned char buf[9];
    memset(buf, 0, sizeof buf);

    CHECK_EQ(BITSET(buf, 40), 0);      // byte 5, bit 0
    CHECK_EQ(buf[5], 0x01);
    CHECK_EQ(BITTEST(buf, 40), 1);
    CHECK_EQ(BITTEST(buf, 39), 0);
    CHECK_EQ(BITCOMP(buf, 63), 0);     // byte 7, bit 7
    CHECK_EQ(buf[7], 0x80);
    CHECK_EQ(BITRESET(buf, 40), 1);
    CHECK_EQ(buf[5], 0x00);
}

//------------------------------------------------------------------------------
// R3DMATRIX::operator== — the loop counter was never decremented, so equal
// matrices never returned true and the scan ran past both operands (ASan
// global-buffer-overflow on IDENTITY).  Two checks: semantics, and a
// guard-page pair that faults if the loop walks past 64 bytes again.
//------------------------------------------------------------------------------
static void test_r3dmatrix_eq()
{
    R3DMATRIX a, b;
    memset(&a, 0, sizeof a);
    memset(&b, 0, sizeof b);

    CHECK(a == b);                     // the old loop never reached here

    for (int i = 0; i < 16; ++i) {
        ((ULong*)&b)[i] ^= 0x40000000UL;   // flip one element's top bits
        CHECK(!(a == b));
        ((ULong*)&b)[i] ^= 0x40000000UL;
    }
    CHECK(a == b);
}

static void test_r3dmatrix_eq_guardpage()
{
    long ps = sysconf(_SC_PAGESIZE);
    char* r1 = (char*)mmap(NULL, 2 * ps, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char* r2 = (char*)mmap(NULL, 2 * ps, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(r1 != MAP_FAILED && r2 != MAP_FAILED);
    if (r1 == MAP_FAILED || r2 == MAP_FAILED) return;
    mprotect(r1 + ps, ps, PROT_NONE);
    mprotect(r2 + ps, ps, PROT_NONE);

    // Both operands end exactly at their guard page; a scan past the last
    // element faults instead of silently reading the neighbour.
    R3DMATRIX* a = (R3DMATRIX*)(r1 + ps - sizeof(R3DMATRIX));
    R3DMATRIX* b = (R3DMATRIX*)(r2 + ps - sizeof(R3DMATRIX));
    memset(a, 0, sizeof *a);
    memset(b, 0, sizeof *b);
    CHECK(*a == *b);

    ((ULong*)b)[15] = 1;               // differ only in the last element
    CHECK(!(*a == *b));

    munmap(r1, 2 * ps);
    munmap(r2, 2 * ps);
}

// Declared in bob_dinput_focus.cpp — drives the real SDL event watch in
// Hardware/dinput_stub.cpp.
void test_dinput_focus_release();
void test_dinput_guid_identity();
// Declared in bob_keystub_conv.cpp — keymap terminator bound at the last
// record of keyb3d.bin.
void test_keystub_reg3dconv_bound();
// Declared in bob_worldinc_delete.cpp — sized-delete mismatch on the
// item-hierarchy operator delete overrides.
void test_worldinc_delete();
// Declared in bob_analogue_axis.cpp — TransAxis sentinel/upper bound.
void test_analogue_transaxis_bounds();

int main()
{
    test_bitops_narrow_guardpage();
    test_bitops_wide_string();
    test_r3dmatrix_eq();
    test_r3dmatrix_eq_guardpage();
    test_dinput_focus_release();
    test_dinput_guid_identity();
    test_keystub_reg3dconv_bound();
    test_worldinc_delete();
    test_analogue_transaxis_bounds();

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
