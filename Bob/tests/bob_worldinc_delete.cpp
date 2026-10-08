// bob_worldinc_delete.cpp — regression test for the sized-delete mismatch in
// h/worldinc.h's custom operator delete implementations.
//
// The legacy pattern was `void operator delete(void* obj) { ::delete(P) obj; }`
// where P was a *smaller base* type (e.g. MovingItemPtr inside mobileitem).
// Deleting a derived object through the base ran the base dtor twice and
// freed sizeof(base) bytes for a sizeof(derived) allocation — ASan reported
// new-delete-type-mismatch every frame in 3D (59-byte TransientItem freed as
// 50-byte MovingItem).  Fixed to `::operator delete(obj)` — the same pattern
// MiG Alley's sibling header already used.
//
// The defect only fires under ASan (a wrong-size free is silent otherwise),
// but this test also pins that every patched class still constructs and
// deallocates through its own static type without aborting.

#include "WIN32_COMPAT.H"
#include "dosdefs.h"
#include <stdio.h>
#include "worldinc.h"

// Static storage the inline ctors touch but the TU never defines otherwise.
int TransientItem::transcount;
// Model hooks live in the game binary; the delete path never calls them.
Bool NewModel (AirStrucPtr const)  { return TRUE; }
void DeleteModel (AirStrucPtr const) {}

#include "../tests/unit/harness.h"
extern int g_checks;
extern int g_failures;

void test_worldinc_delete()
{
    // The scenario ASan caught: a derived item deleted through mobileitem*
    // hit mobileitem::operator delete, which sized-freed it as MovingItem.
    CHECK(sizeof(TransientItem) > sizeof(MovingItem));

    mobileitem* m = new TransientItem;
    CHECK(m->Status.size >= MobileSize);
    delete m;                          // mobileitem::operator delete

    mobileitem* f = new formationitem;
    delete f;                          // formationitem size > MobileSize

    rotitem* r = new mobileitem;       // rotitem::operator delete path
    delete r;

    hdgitem* h = new hpitem;           // hpitem via hdgitem static type
    delete h;

    WayPoint* w = new WayPoint;        // WayPoint's own operator delete
    CHECK(w->Status.size == WayPointSize);
    delete w;

    AirStruc* a = new AirStruc;        // the largest patched class
    delete a;
}
