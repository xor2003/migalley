// bob_analogue_axis.cpp — regression test for Analogue::TransAxis bounds.
//
// DirectInput device enumeration calls TransAxis() with sentinel axis slots
// (AU_UNUSED == -1); the original code indexed tune[startaxis] before
// validating, so AU_UNUSED read tune[-1] — UBSan index-out-of-bounds at
// runtime (invisible to ASan: the member array sits inside the object, so
// no redzone separates it).  Fix: return the sentinel unchanged before any
// indexing.
//
// The implementation TU is included directly (same seam as the keystub
// test); TransAxis(-1)/(AU_MAX) return before touching members, so an
// unconstructed Analogue backing store exercises exactly the guarded code.

#include "WIN32_COMPAT.H"
#include "dosdefs.h"
#include <stdio.h>
#include "analogue.h"
#include "analogue.cpp"

// Defined in the build's analogue globals TU; the test only needs the
// terminator so TransAxis's alias scan exits immediately.
Analogue::AliassingOption Analogue::Aliassing_List[] = { {(AllowAliasing)0, AU_ILLEGAL} };
Analogue::AliassingOption* Analogue::Aliassing_Table[] = { NULL };

// PollPosition's key-injection calls; the test never invokes it but the
// sanitizer build keeps the section live (gc-sections doesn't drop it).
void Inst3d::OnKeyDown(int) {}
void Inst3d::OnKeyUp(int) {}

#include "../tests/unit/harness.h"
extern int g_checks;
extern int g_failures;

void test_analogue_transaxis_bounds()
{
    alignas(Analogue) unsigned char buf[sizeof(Analogue)];
    Analogue* a = reinterpret_cast<Analogue*>(buf);

    // Sentinels must pass straight through without indexing tune[].
    CHECK_EQ(a->TransAxis(AU_UNUSED), AU_UNUSED);       // -1: tune[-1] before
    CHECK_EQ(a->TransAxis(AU_MAX),  AU_MAX);            // tune[AU_MAX]: tune has AU_MAX entries
    CHECK_EQ(a->TransAxis(-128),    -128);

    // A valid axis on a zeroed map returns itself (no aliasing configured).
    memset(buf, 0, sizeof buf);
    CHECK_EQ(a->TransAxis(AU_MIN), AU_MIN);
    CHECK_EQ(a->TransAxis(10),     10);
}
