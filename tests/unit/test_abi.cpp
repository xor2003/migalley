//------------------------------------------------------------------------------
// ABI pin tests — GENERATED FILE. Do not edit; regenerate with
// tests/unit/gen_abi.sh after changing a wire-format header.
//
// Every static_assert compiles clean on the 32-bit build and FAILS THE
// BUILD on x86-64 wherever a layout drifts — the assert text names the
// struct to fix. These sizes are the on-disk/in-stream contract; they
// must hold identically on both architectures.
//------------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include "DOSDEFS.H"
#include "MYANGLES.H"
#include "3DDEFS.H"
#include "VERTEX.H"
#include "MATRIX.H"
#include "SHPINSTR.H"
#include "ANIMPTR.H"
#include "MODVEC.H"

// --- Fundamental width contract (DOSDEFS.H) -------------------------------
// Root migration hazard: SLong/ULong are `long` = 4B on i386/Win32 but 8B
// on Linux LP64. Retagging them to int/unsigned int is THE fix; these
// asserts stay green only once they are fixed-width.
static_assert(sizeof(UByte) == 1 && sizeof(SByte) == 1, "byte types");
static_assert(sizeof(UWord) == 2 && sizeof(SWord) == 2, "word types");
static_assert(sizeof(ULong) == 4 && sizeof(SLong) == 4,
              "SLong/ULong must be 32-bit on every platform (retag long->int)");
static_assert(sizeof(Float) == 8, "Float is double");
// IFShare overlays .i (SLong) on .f (Float/double): the union is 8 bytes
// and .i must read only the LOW 4 bytes of the double — that truncation
// IS the engine's fixed-point conversion. Pinning member width, not
// union size: on LP64 an unfixed SLong makes .i read all 8 bytes and
// every .f-write/.i-read silently corrupts.
static_assert(sizeof(IFShare) == 8, "IFShare is union{SLong;double} = 8B");
static_assert(sizeof(((IFShare*)nullptr)->i) == 4,
              "IFShare.i must stay 4 bytes (SLong must be int, not long)");
// Enums are int-sized everywhere (MSVC and GCC agree); Bool included.
static_assert(sizeof(Bool) == 4, "Bool enum is int-sized");

// --- Pointer-carrier contract ----------------------------------------------
// These types may legitimately hold pointers; they must be >= void*.
// (SLong/ULong deliberately must NOT be used to carry pointers - see the
// (int)ptr truncation sites listed in README.md 64-bit audit.)
static_assert(sizeof(void*) >= 4, "pointers at least 32-bit");
static_assert(sizeof(std::uintptr_t) == sizeof(void*), "uintptr_t");

// --- Packed-layout spot checks ---------------------------------------------
static_assert(offsetof(DOPOINT, xcoord) == 2, "DOPOINT opcode+vertex lead-in");
static_assert(offsetof(DONPOINTS, start_vertex) == 1, "DONPOINTS lead-in");
static_assert(offsetof(DOCASERANGE, failjump) == 3, "DOCASERANGE layout");

// --- Generated golden sizes (i386) -----------------------------------------
static_assert(sizeof(COORDS3D) == 12, "sizeof(COORDS3D) drifted from i386 wire layout");
static_assert(sizeof(fpCOORDS3D) == 24, "sizeof(fpCOORDS3D) drifted from i386 wire layout");
static_assert(sizeof(IFShare) == 8, "sizeof(IFShare) drifted from i386 wire layout");
static_assert(sizeof(VERTEX) == 76, "sizeof(VERTEX) drifted from i386 wire layout");
static_assert(sizeof(FPMATRIX) == 72, "sizeof(FPMATRIX) drifted from i386 wire layout");
static_assert(sizeof(MATRIX) == 18, "sizeof(MATRIX) drifted from i386 wire layout");
static_assert(sizeof(ANGLES) == 4, "sizeof(ANGLES) drifted from i386 wire layout");
static_assert(sizeof(DoPointStruc) == 65, "sizeof(DoPointStruc) drifted from i386 wire layout");
static_assert(sizeof(FCRD) == 12, "sizeof(FCRD) drifted from i386 wire layout");
static_assert(sizeof(FCRDlong) == 24, "sizeof(FCRDlong) drifted from i386 wire layout");
static_assert(sizeof(FORI) == 36, "sizeof(FORI) drifted from i386 wire layout");
static_assert(sizeof(COORDS) == 6, "sizeof(COORDS) drifted from i386 wire layout");
static_assert(sizeof(DO4CMPNT) == 8, "sizeof(DO4CMPNT) drifted from i386 wire layout");
static_assert(sizeof(DO4CMPT2X) == 10, "sizeof(DO4CMPT2X) drifted from i386 wire layout");
static_assert(sizeof(DOANIMATION) == 12, "sizeof(DOANIMATION) drifted from i386 wire layout");
static_assert(sizeof(DOBITSOFF) == 10, "sizeof(DOBITSOFF) drifted from i386 wire layout");
static_assert(sizeof(DOBITSOFFCOCK) == 5, "sizeof(DOBITSOFFCOCK) drifted from i386 wire layout");
static_assert(sizeof(DOBLOBLINE) == 6, "sizeof(DOBLOBLINE) drifted from i386 wire layout");
static_assert(sizeof(DOCALLSHAPE) == 10, "sizeof(DOCALLSHAPE) drifted from i386 wire layout");
static_assert(sizeof(DOCASERANGE) == 5, "sizeof(DOCASERANGE) drifted from i386 wire layout");
static_assert(sizeof(DOCOLLISION) == 21, "sizeof(DOCOLLISION) drifted from i386 wire layout");
static_assert(sizeof(DOCOMPASS) == 14, "sizeof(DOCOMPASS) drifted from i386 wire layout");
static_assert(sizeof(DOCOPYBVERT) == 6, "sizeof(DOCOPYBVERT) drifted from i386 wire layout");
static_assert(sizeof(DOCOPYIVERT) == 5, "sizeof(DOCOPYIVERT) drifted from i386 wire layout");
static_assert(sizeof(DOCREATEBUMPPOLY) == 4, "sizeof(DOCREATEBUMPPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOCREATEIPOLY) == 2, "sizeof(DOCREATEIPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOCREATEIVERT) == 10, "sizeof(DOCREATEIVERT) drifted from i386 wire layout");
static_assert(sizeof(DOCREATERPOLY) == 5, "sizeof(DOCREATERPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOCYLINDER) == 4, "sizeof(DOCYLINDER) drifted from i386 wire layout");
static_assert(sizeof(DODAMAGE) == 9, "sizeof(DODAMAGE) drifted from i386 wire layout");
static_assert(sizeof(DODIAL) == 21, "sizeof(DODIAL) drifted from i386 wire layout");
static_assert(sizeof(DODIGITDIAL) == 15, "sizeof(DODIGITDIAL) drifted from i386 wire layout");
static_assert(sizeof(DODOT) == 7, "sizeof(DODOT) drifted from i386 wire layout");
static_assert(sizeof(DODRAWBPOLY) == 1, "sizeof(DODRAWBPOLY) drifted from i386 wire layout");
static_assert(sizeof(DODRAWCLOUD) == 4, "sizeof(DODRAWCLOUD) drifted from i386 wire layout");
static_assert(sizeof(DODRAWIPOLY) == 1, "sizeof(DODRAWIPOLY) drifted from i386 wire layout");
static_assert(sizeof(DODRAWIPOLYS) == 1, "sizeof(DODRAWIPOLYS) drifted from i386 wire layout");
static_assert(sizeof(DODRAWREFLECTPOLY) == 1, "sizeof(DODRAWREFLECTPOLY) drifted from i386 wire layout");
static_assert(sizeof(DODRAWRPOLY) == 3, "sizeof(DODRAWRPOLY) drifted from i386 wire layout");
static_assert(sizeof(DODRAWSTATION) == 1, "sizeof(DODRAWSTATION) drifted from i386 wire layout");
static_assert(sizeof(DODRAWSUN) == 19, "sizeof(DODRAWSUN) drifted from i386 wire layout");
static_assert(sizeof(DOEFFECT) == 9, "sizeof(DOEFFECT) drifted from i386 wire layout");
static_assert(sizeof(DOEND) == 1, "sizeof(DOEND) drifted from i386 wire layout");
static_assert(sizeof(DOFADEENVELOPE) == 14, "sizeof(DOFADEENVELOPE) drifted from i386 wire layout");
static_assert(sizeof(DOGOSUB) == 2, "sizeof(DOGOSUB) drifted from i386 wire layout");
static_assert(sizeof(DOGOTO) == 2, "sizeof(DOGOTO) drifted from i386 wire layout");
static_assert(sizeof(DOGROUP) == 28, "sizeof(DOGROUP) drifted from i386 wire layout");
static_assert(sizeof(DOGUNSIGHT) == 9, "sizeof(DOGUNSIGHT) drifted from i386 wire layout");
static_assert(sizeof(DOHEATHAZE) == 13, "sizeof(DOHEATHAZE) drifted from i386 wire layout");
static_assert(sizeof(DOICYLINDER) == 25, "sizeof(DOICYLINDER) drifted from i386 wire layout");
static_assert(sizeof(DOIFBRIGHT) == 5, "sizeof(DOIFBRIGHT) drifted from i386 wire layout");
static_assert(sizeof(DOIFCASE) == 5, "sizeof(DOIFCASE) drifted from i386 wire layout");
static_assert(sizeof(DOIFCROSS) == 8, "sizeof(DOIFCROSS) drifted from i386 wire layout");
static_assert(sizeof(DOIFEQ) == 6, "sizeof(DOIFEQ) drifted from i386 wire layout");
static_assert(sizeof(DOIFHARD3D) == 2, "sizeof(DOIFHARD3D) drifted from i386 wire layout");
static_assert(sizeof(DOIFNE) == 6, "sizeof(DOIFNE) drifted from i386 wire layout");
static_assert(sizeof(DOIFPILOTED) == 2, "sizeof(DOIFPILOTED) drifted from i386 wire layout");
static_assert(sizeof(DOINC) == 6, "sizeof(DOINC) drifted from i386 wire layout");
static_assert(sizeof(DOINCLN) == 15, "sizeof(DOINCLN) drifted from i386 wire layout");
static_assert(sizeof(DOINIT) == 6, "sizeof(DOINIT) drifted from i386 wire layout");
static_assert(sizeof(DOISWITCH) == 10, "sizeof(DOISWITCH) drifted from i386 wire layout");
static_assert(sizeof(DOLAUNCHER) == 13, "sizeof(DOLAUNCHER) drifted from i386 wire layout");
static_assert(sizeof(DOLIGHT) == 17, "sizeof(DOLIGHT) drifted from i386 wire layout");
static_assert(sizeof(DOLIGHTTIMER) == 5, "sizeof(DOLIGHTTIMER) drifted from i386 wire layout");
static_assert(sizeof(DOLINE) == 4, "sizeof(DOLINE) drifted from i386 wire layout");
static_assert(sizeof(DOLSHADEON) == 1, "sizeof(DOLSHADEON) drifted from i386 wire layout");
static_assert(sizeof(DOMORPHCYLINDERIMAPD) == 27, "sizeof(DOMORPHCYLINDERIMAPD) drifted from i386 wire layout");
static_assert(sizeof(DOMORPHNSPHRS) == 13, "sizeof(DOMORPHNSPHRS) drifted from i386 wire layout");
static_assert(sizeof(DOMORPHNSPHRSIMAPD) == 20, "sizeof(DOMORPHNSPHRSIMAPD) drifted from i386 wire layout");
static_assert(sizeof(DOMORPHSPHERE) == 16, "sizeof(DOMORPHSPHERE) drifted from i386 wire layout");
static_assert(sizeof(DON4CMPNTS) == 4, "sizeof(DON4CMPNTS) drifted from i386 wire layout");
static_assert(sizeof(DONDELTAPOINTS) == 7, "sizeof(DONDELTAPOINTS) drifted from i386 wire layout");
static_assert(sizeof(DONDUPVEC) == 5, "sizeof(DONDUPVEC) drifted from i386 wire layout");
static_assert(sizeof(DONIANIMVERTS) == 8, "sizeof(DONIANIMVERTS) drifted from i386 wire layout");
static_assert(sizeof(DONINCPNTS) == 6, "sizeof(DONINCPNTS) drifted from i386 wire layout");
static_assert(sizeof(DONIVERTS) == 2, "sizeof(DONIVERTS) drifted from i386 wire layout");
static_assert(sizeof(DONOP) == 1, "sizeof(DONOP) drifted from i386 wire layout");
static_assert(sizeof(DONORADAR) == 1, "sizeof(DONORADAR) drifted from i386 wire layout");
static_assert(sizeof(DONORMAL) == 4, "sizeof(DONORMAL) drifted from i386 wire layout");
static_assert(sizeof(DONPOINT2X) == 4, "sizeof(DONPOINT2X) drifted from i386 wire layout");
static_assert(sizeof(DONPOINTS) == 3, "sizeof(DONPOINTS) drifted from i386 wire layout");
static_assert(sizeof(DONSPHERES) == 7, "sizeof(DONSPHERES) drifted from i386 wire layout");
static_assert(sizeof(DONSPHERESIMAPD) == 12, "sizeof(DONSPHERESIMAPD) drifted from i386 wire layout");
static_assert(sizeof(DONSUBS) == 2, "sizeof(DONSUBS) drifted from i386 wire layout");
static_assert(sizeof(DONTPOINTS) == 13, "sizeof(DONTPOINTS) drifted from i386 wire layout");
static_assert(sizeof(DONVEC) == 4, "sizeof(DONVEC) drifted from i386 wire layout");
static_assert(sizeof(DOOFFSETPNT) == 17, "sizeof(DOOFFSETPNT) drifted from i386 wire layout");
static_assert(sizeof(DOONDAMAGED) == 7, "sizeof(DOONDAMAGED) drifted from i386 wire layout");
static_assert(sizeof(DOPLUMEPNT) == 5, "sizeof(DOPLUMEPNT) drifted from i386 wire layout");
static_assert(sizeof(DOPOINT) == 8, "sizeof(DOPOINT) drifted from i386 wire layout");
static_assert(sizeof(DOPOINT2X) == 10, "sizeof(DOPOINT2X) drifted from i386 wire layout");
static_assert(sizeof(DOPOLYGON) == 1, "sizeof(DOPOLYGON) drifted from i386 wire layout");
static_assert(sizeof(DOQUIKPOLY) == 5, "sizeof(DOQUIKPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOQUIKRELPOLY) == 7, "sizeof(DOQUIKRELPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOQUIKSMOOTHPOLY) == 5, "sizeof(DOQUIKSMOOTHPOLY) drifted from i386 wire layout");
static_assert(sizeof(DORADAR) == 1, "sizeof(DORADAR) drifted from i386 wire layout");
static_assert(sizeof(DORELPOLY) == 3, "sizeof(DORELPOLY) drifted from i386 wire layout");
static_assert(sizeof(DORESETANIM) == 3, "sizeof(DORESETANIM) drifted from i386 wire layout");
static_assert(sizeof(DORET) == 1, "sizeof(DORET) drifted from i386 wire layout");
static_assert(sizeof(DOSCALESIZE) == 4, "sizeof(DOSCALESIZE) drifted from i386 wire layout");
static_assert(sizeof(DOSETANIM) == 6, "sizeof(DOSETANIM) drifted from i386 wire layout");
static_assert(sizeof(DOSETCOLOUR) == 4, "sizeof(DOSETCOLOUR) drifted from i386 wire layout");
static_assert(sizeof(DOSETCOLOUR256) == 4, "sizeof(DOSETCOLOUR256) drifted from i386 wire layout");
static_assert(sizeof(DOSETCOLOURALL) == 4, "sizeof(DOSETCOLOURALL) drifted from i386 wire layout");
static_assert(sizeof(DOSETCOLOURH) == 2, "sizeof(DOSETCOLOURH) drifted from i386 wire layout");
static_assert(sizeof(DOSETGLASSRANGE) == 2, "sizeof(DOSETGLASSRANGE) drifted from i386 wire layout");
static_assert(sizeof(DOSETLC) == 2, "sizeof(DOSETLC) drifted from i386 wire layout");
static_assert(sizeof(DOSETLUMINOSITY) == 3, "sizeof(DOSETLUMINOSITY) drifted from i386 wire layout");
static_assert(sizeof(DOSETMAPPINGPLANER) == 1, "sizeof(DOSETMAPPINGPLANER) drifted from i386 wire layout");
static_assert(sizeof(DOSETMAPPINGTAN) == 1, "sizeof(DOSETMAPPINGTAN) drifted from i386 wire layout");
static_assert(sizeof(DOSMOKEDOFF) == 1, "sizeof(DOSMOKEDOFF) drifted from i386 wire layout");
static_assert(sizeof(DOSMOKEDON) == 1, "sizeof(DOSMOKEDON) drifted from i386 wire layout");
static_assert(sizeof(DOSMOKEPNT) == 10, "sizeof(DOSMOKEPNT) drifted from i386 wire layout");
static_assert(sizeof(DOSMOOTHPOLY) == 5, "sizeof(DOSMOOTHPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOSMTHRELPOLY) == 7, "sizeof(DOSMTHRELPOLY) drifted from i386 wire layout");
static_assert(sizeof(DOSPHERE) == 10, "sizeof(DOSPHERE) drifted from i386 wire layout");
static_assert(sizeof(DOSTRETCHMAP) == 4, "sizeof(DOSTRETCHMAP) drifted from i386 wire layout");
static_assert(sizeof(DOSTRETCHPOINT) == 4, "sizeof(DOSTRETCHPOINT) drifted from i386 wire layout");
static_assert(sizeof(DOTEXTUREOFF) == 1, "sizeof(DOTEXTUREOFF) drifted from i386 wire layout");
static_assert(sizeof(DOTRANSFORMLIGHT) == 1, "sizeof(DOTRANSFORMLIGHT) drifted from i386 wire layout");
static_assert(sizeof(DOTRANSPARENTOFF) == 1, "sizeof(DOTRANSPARENTOFF) drifted from i386 wire layout");
static_assert(sizeof(DOTRANSPARENTON) == 1, "sizeof(DOTRANSPARENTON) drifted from i386 wire layout");
static_assert(sizeof(DOTRIFAN) == 4, "sizeof(DOTRIFAN) drifted from i386 wire layout");
static_assert(sizeof(DOTRIFANFLAT) == 7, "sizeof(DOTRIFANFLAT) drifted from i386 wire layout");
static_assert(sizeof(DOTRIZAG) == 3, "sizeof(DOTRIZAG) drifted from i386 wire layout");
static_assert(sizeof(DOTRIZAGFLAT) == 6, "sizeof(DOTRIZAGFLAT) drifted from i386 wire layout");
static_assert(sizeof(DOVDOT) == 6, "sizeof(DOVDOT) drifted from i386 wire layout");
static_assert(sizeof(DOVDOT4) == 6, "sizeof(DOVDOT4) drifted from i386 wire layout");
static_assert(sizeof(DOVECTOR) == 5, "sizeof(DOVECTOR) drifted from i386 wire layout");
static_assert(sizeof(DOWEAPONOFF) == 5, "sizeof(DOWEAPONOFF) drifted from i386 wire layout");
static_assert(sizeof(DOWHEELDAMAGE) == 10, "sizeof(DOWHEELDAMAGE) drifted from i386 wire layout");
static_assert(sizeof(DOWHEELSPRAY) == 8, "sizeof(DOWHEELSPRAY) drifted from i386 wire layout");
static_assert(sizeof(DOWHITEOUT) == 5, "sizeof(DOWHITEOUT) drifted from i386 wire layout");
static_assert(sizeof(DOWINDOWDIAL) == 17, "sizeof(DOWINDOWDIAL) drifted from i386 wire layout");
static_assert(sizeof(LIGHTDATA) == 14, "sizeof(LIGHTDATA) drifted from i386 wire layout");
static_assert(sizeof(MORPHSPHNEXT) == 4, "sizeof(MORPHSPHNEXT) drifted from i386 wire layout");
static_assert(sizeof(NDNEXT1) == 1, "sizeof(NDNEXT1) drifted from i386 wire layout");
static_assert(sizeof(NDNEXT2) == 2, "sizeof(NDNEXT2) drifted from i386 wire layout");
static_assert(sizeof(NDNEXT3) == 3, "sizeof(NDNEXT3) drifted from i386 wire layout");
static_assert(sizeof(NEXTMAP) == 4, "sizeof(NEXTMAP) drifted from i386 wire layout");
static_assert(sizeof(NEXTT) == 12, "sizeof(NEXTT) drifted from i386 wire layout");
static_assert(sizeof(NEXTVEC) == 3, "sizeof(NEXTVEC) drifted from i386 wire layout");
static_assert(sizeof(NNEXT) == 6, "sizeof(NNEXT) drifted from i386 wire layout");
static_assert(sizeof(NNEXT2X) == 8, "sizeof(NNEXT2X) drifted from i386 wire layout");
static_assert(sizeof(SPHERENEXT) == 2, "sizeof(SPHERENEXT) drifted from i386 wire layout");

// Intentionally NOT pinned (in-memory only, size may float on 64-bit):
//   animptr / *P typedefs  - contain real pointers
//   FPMATRIX_PTR, FILE, *_PTR typedefs
// Anything serialized to disk, the shape stream, or shared via memcpy
// MUST appear in the generated block above.
