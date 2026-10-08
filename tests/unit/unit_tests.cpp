//------------------------------------------------------------------------------
// Unit tests for the native MiG Alley port (32-bit).
//
// Assertion-style, no framework (keeps the tree dependency-free and the
// tests readable next to the 1998 code they pin down).
//
// Run via ctest or directly: ./unit_tests
//
// Shape-interpreter coverage boundary
// -----------------------------------
// Every handler that can run on synthetic streams + fake display/window
// chains is pinned below. The rest split into three buckets:
//
//   ERROR-EXIT - the body is _Error.EmitSysErr -> SayAndQuit -> exit(0).
//     They exist only as malformed-shape traps and would kill the runner:
//     do3dbreak, dodepthcolour, dodepthpoly, doend, dogroup, domappoly,
//     donextvec, dorepos, dosetcolourall, dosetcolourh, dosetmapmap,
//     dotrifan, dotrizag, dotrizagflat.
//
//   INTEGRATION-ONLY - need a live rasterizer (POLYGON/current_screen->Do*),
//     Image_Map data, the full item/Trans_Obj world, or recursive
//     SHAPE.draw_shape. Unit-faking these would test the fakes, not the
//     code; they are exercised by smoke_xvfb instead:
//     all do*poly render ops (dopolygon, dorelpoly, doquikpoly,
//     doquikrelpoly, doquiksmoothpoly, dosmoothpoly, dosmthrelpoly,
//     dodrawipoly, dodrawipolys, dodrawopoly, dodrawrpoly,
//     dodrawreflectpoly, dotrifanflat), doline, doblobline, doincln,
//     doheathaze, dodrawsun, dotransparenton, dosetmipmap (SetMipMap is
//     hardware), dosetcolour, dosphere/doisphere/doosphere, docylinder,
//     doicylinder, docreateipoly, docreaterpoly, dodigitdial, dodial,
//     dowindowdial, dogunsight, doattitude, dolight,
//     dodot, domorphsphere*, domorphnsphrs*, dosmokepnt, dosmktrail,
//     dowheelspray, doeffect, docollision, docallshape (DrawSubShape ->
//     SHAPE.draw_shape), dodamage, dowheeldamage, and the launch path of
//     dobitsofffx.
//
//   PARTIALLY COVERED - safe paths pinned, heavy paths noted at the test:
//     dotransformlight (!IsSubShape path needs ItemPtr/Three_Dee/
//     Land_Scape), dolighttimer (PERIODIC+nonzero needs view3dwin),
//     dodrawstation (station>0 needs station shape data),
//     dobitsofffx (triggered+matrix path needs Trans_Obj).
//
// Note: SHPINSTR structs are compiled under DOSDEFS.H #pragma pack(1) -
// the on-disk layout differs from the natural C layout. Tests write
// named struct fields (compiler applies packing); raw stream[] offsets
// must use the packed offsets (see test_doanimation).
//
// Non-shape coverage: test_mathlib_* (trig/distance/calendar/rnd/a2iend),
// test_fileman (fake-dir naming, exist/open/read wrappers,
// translatedirlist), test_mathasm_* (the asm->GNU port: bit ops,
// fixed-point mul/div, sign helpers, x87 cw), test_bitcount_macros,
// test_matrix_int (fixed-point MATRIX path), test_malloc_wrappers.
//------------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <sys/mman.h>
#include <unistd.h>

// The game headers use MSVC-era keywords; ProjectDefaults supplies -m32 and
// -fms-extensions so the same headers compile here unchanged.
#include "DOSDEFS.H"
#include "MYANGLES.H"
#include "3DDEFS.H"
#include "VERTEX.H"
#include "MATRIX.H"
#include "SHPINSTR.H"
#include "ANIMPTR.H"
#include "FTOI.H"
#include "MODVEC.H"
#include "CURVES.H"
#include "MYMATH.H"
#include "WIN32_COMPAT.H"
#include "FILES.H"
#include <sys/stat.h>
// dirfakeblock/direntries are fileman-private; access control is
// compile-time only, so expose them for this TU alone. Same for
// Graphic's static IFF readers (SkipRow/SearchIFFHunk*/Read*) and
// LBM's hdr/body/pal members.
#define private public
#define protected public
#include "FILEMAN.H"
#include "DISPLAY.H"
#include "LBM.H"
#undef protected
#undef private
#include "BITCOUNT.H"
#include "ANIMDATA.H"
#include "SAVEGAME.H"
#include "CBUFFER.H"
#include "BSTREAM.H"
#include "MigCstring.h"
// Last: BITFIELD.H defines assert() away (legacy choice, errorcheck is a
// no-op here as in the game build).
#include "BITFIELD.H"

// shape:: statics + instruction handlers live in libMy3D but the class needs
// the whole world include chain. Access control is compile-time only, so the
// tests bind the exported symbols directly.
extern DoPointStruc*  shape_newco            __asm__("_ZN5shape5newcoE");
extern DoPointStruc   shape_shpco[]          __asm__("_ZN5shape5shpcoE");
extern FPMATRIX_PTR   shape_fpobject_matrix  __asm__("_ZN5shape15fpobject_matrixE");
extern void*          shape_object_obj3d     __asm__("_ZN5shape12object_obj3dE");
// InterpTable stores InterpProc = void(UByte*&): the instruction handlers
// are this-less, so the alias signature is a single reference arg.
extern void           shape_dopoint2x(UByte*& ip)
                                               __asm__("_ZN5shape9dopoint2xERPh");
extern void           shape_dopoint(UByte*& ip)
                                               __asm__("_ZN5shape7dopointERPh");
extern void           shape_donpoints(UByte*& ip)
                                               __asm__("_ZN5shape9donpointsERPh");
extern void           shape_dogoto(UByte*& ip) __asm__("_ZN5shape6dogotoERPh");
extern void           shape_dogosub(UByte*& ip) __asm__("_ZN5shape7dogosubERPh");
extern void           shape_doret(UByte*& ip)   __asm__("_ZN5shape5doretERPh");
extern void           shape_doifeq(UByte*& ip)  __asm__("_ZN5shape6doifeqERPh");
extern void           shape_doifne(UByte*& ip)  __asm__("_ZN5shape6doifneERPh");
extern void           shape_doswitch(UByte*& ip) __asm__("_ZN5shape8doswitchERPh");
extern void           shape_docaserange(UByte*& ip)
                                             __asm__("_ZN5shape11docaserangeERPh");
extern void           shape_doplumepnt(UByte*& ip)
                                             __asm__("_ZN5shape10doplumepntERPh");
extern void           shape_donincpnts(UByte*& ip)
                                             __asm__("_ZN5shape10donincpntsERPh");
extern void           shape_dontpoints(UByte*& ip)
                                             __asm__("_ZN5shape10dontpointsERPh");
extern void           shape_domorphnpoints(UByte*& ip)
                                             __asm__("_ZN5shape14domorphnpointsERPh");
extern void           shape_domorphpoint(UByte*& ip)
                                             __asm__("_ZN5shape12domorphpointERPh");
extern void           shape_doifbright(UByte*& ip)
                                             __asm__("_ZN5shape10doifbrightERPh");
extern void           shape_doifcross(UByte*& ip)
                                             __asm__("_ZN5shape9doifcrossERPh");
extern void           shape_doflipvector(UByte*& ip)
                                             __asm__("_ZN5shape12doflipvectorERPh");
extern void           shape_doflipnvec(UByte*& ip)
                                             __asm__("_ZN5shape10doflipnvecERPh");

extern void           shape_doifcase(UByte*& ip)
                                             __asm__("_ZN5shape8doifcaseERPh");
extern void           shape_dosetcolour256(UByte*& ip)
                                             __asm__("_ZN5shape14dosetcolour256ERPh");

// Interpreter state the handlers read/write through.
extern animptr        shape_GlobalAdptr  __asm__("_ZN5shape11GlobalAdptrE");
extern void         (**shape_InterpTable)(UByte*&)
                                               __asm__("_ZN5shape11InterpTableE");
extern void*          shape_current_screen
                                             __asm__("_ZN5shape14current_screenE");
extern int            shape_colour     __asm__("_ZN5shape6colourE");
extern short          shape_range      __asm__("_ZN5shape5rangeE");
extern int            shape_image      __asm__("_ZN5shape5imageE");
extern SLong          shape_object_dist __asm__("_ZN5shape11object_distE");
extern SLong          shape_fade_start  __asm__("_ZN5shape10fade_startE");
extern void           shape_donop(UByte*& ip)    __asm__("_ZN5shape5donopERPh");
extern void           shape_donormal(UByte*& ip) __asm__("_ZN5shape8donormalERPh");
extern void           shape_dosetlc(UByte*& ip)  __asm__("_ZN5shape7dosetlcERPh");
extern void           shape_do4cmpnt(UByte*& ip) __asm__("_ZN5shape8do4cmpntERPh");
extern void           shape_do4cmpt2x(UByte*& ip) __asm__("_ZN5shape9do4cmpt2xERPh");
extern void           shape_dooffsetpnt(UByte*& ip)
                                             __asm__("_ZN5shape11dooffsetpntERPh");
extern void           shape_dosetmapoff(UByte*& ip)
                                             __asm__("_ZN5shape11dosetmapoffERPh");
extern void           shape_doifhard3d(UByte*& ip)
                                             __asm__("_ZN5shape10doifhard3dERPh");
extern void           shape_don4cmpnts(UByte*& ip)
                                             __asm__("_ZN5shape10don4cmpntsERPh");
extern void           shape_dosmokedon(UByte*& ip)
                                             __asm__("_ZN5shape10dosmokedonERPh");
extern void           shape_dosmokedoff(UByte*& ip)
                                             __asm__("_ZN5shape11dosmokedoffERPh");
extern void           shape_dotransparentoff(UByte*& ip)
                                             __asm__("_ZN5shape16dotransparentoffERPh");
extern void           shape_doresetanim(UByte*& ip)
                                             __asm__("_ZN5shape11doresetanimERPh");
extern void           shape_doscalesize(UByte*& ip)
                                             __asm__("_ZN5shape11doscalesizeERPh");
extern void           shape_dobitsoff(UByte*& ip)
                                             __asm__("_ZN5shape9dobitsoffERPh");
extern void           shape_doniverts(UByte*& ip)
                                             __asm__("_ZN5shape9donivertsERPh");
extern void           shape_dosetluminosity(UByte*& ip)
                                             __asm__("_ZN5shape15dosetluminosityERPh");
extern void           shape_dolshadeon(UByte*& ip)
                                             __asm__("_ZN5shape10dolshadeonERPh");
extern void           shape_dostretchpoint(UByte*& ip)
                                             __asm__("_ZN5shape14dostretchpointERPh");
extern void           shape_dostretchmap(UByte*& ip)
                                             __asm__("_ZN5shape12dostretchmapERPh");
extern void           shape_donpoint2x(UByte*& ip)
                                             __asm__("_ZN5shape10donpoint2xERPh");
extern void           shape_dondeltapoints(UByte*& ip)
                                             __asm__("_ZN5shape14dondeltapointsERPh");
extern void*          shape_View_Point __asm__("_ZN5shape10View_PointE");
extern bool           shape_doingHW3D  __asm__("_ZN5shape9doingHW3DE");
extern void           shape_dotimerphase(UByte*& ip)
                                             __asm__("_ZN5shape12dotimerphaseERPh");
extern void           shape_dofadeenvelope(UByte*& ip)
                                             __asm__("_ZN5shape14dofadeenvelopeERPh");
extern void           shape_donianimverts(UByte*& ip)
                                             __asm__("_ZN5shape13donianimvertsERPh");
extern void           shape_donsubs(UByte*& ip)
                                             __asm__("_ZN5shape7donsubsERPh");
extern void           shape_doifpiloted(UByte*& ip)
                                             __asm__("_ZN5shape11doifpilotedERPh");
extern void           shape_dodrawstation(UByte*& ip)
                                             __asm__("_ZN5shape13dodrawstationERPh");
extern void           shape_dowhiteout(UByte*& ip)
                                             __asm__("_ZN5shape10dowhiteoutERPh");
extern void*          BoxCol_Col_Shooter __asm__("_ZN6BoxCol11Col_ShooterE");
extern UByte          Manual_Pilot_bytes[] __asm__("Manual_Pilot");
extern void           shape_donvec(UByte*& ip)
                                             __asm__("_ZN5shape6donvecERPh");
extern void           shape_dondupvec(UByte*& ip)
                                             __asm__("_ZN5shape9dondupvecERPh");
extern void           shape_dotransformlight(UByte*& ip)
                                             __asm__("_ZN5shape16dotransformlightERPh");
extern LightVec       shape_TransLightVector
                                             __asm__("_ZN5shape16TransLightVectorE");
extern LightVec       shape_TransViewVector
                                             __asm__("_ZN5shape15TransViewVectorE");
extern Bool           shape_specularEnabled
                                             __asm__("_ZN5shape15specularEnabledE");
extern Bool           shape_IsSubShape       __asm__("_ZN5shape10IsSubShapeE");
extern FPMATRIX_PTR   shape_fprealobject_matrix
                                             __asm__("_ZN5shape19fprealobject_matrixE");
extern void           shape_dovector(UByte*& ip)
                                             __asm__("_ZN5shape8dovectorERPh");
extern void           shape_docopyivert(UByte*& ip)
                                             __asm__("_ZN5shape11docopyivertERPh");
extern void           shape_dobitsoffcock(UByte*& ip)
                                             __asm__("_ZN5shape13dobitsoffcockERPh");
extern void           shape_doondamaged(UByte*& ip)
                                             __asm__("_ZN5shape11doondamagedERPh");
extern void           shape_doweaponoff(UByte*& ip)
                                             __asm__("_ZN5shape11doweaponoffERPh");
extern void           shape_douserealtime(UByte*& ip)
                                             __asm__("_ZN5shape13douserealtimeERPh");
extern void           shape_dosetglassrange(UByte*& ip)
                                             __asm__("_ZN5shape15dosetglassrangeERPh");
extern void           shape_doiswitch(UByte*& ip)
                                             __asm__("_ZN5shape9doiswitchERPh");
extern void           shape_docopybvert(UByte*& ip)
                                             __asm__("_ZN5shape11docopybvertERPh");
extern void           shape_docreateivert(UByte*& ip)
                                             __asm__("_ZN5shape13docreateivertERPh");
extern void           shape_dobitsofffx(UByte*& ip)
                                             __asm__("_ZN5shape11dobitsofffxERPh");
extern void           shape_dosetmappingplaner(UByte*& ip)
                                             __asm__("_ZN5shape18dosetmappingplanerERPh");
extern void           shape_dosetmappingtan(UByte*& ip)
                                             __asm__("_ZN5shape15dosetmappingtanERPh");
extern void           shape_dospin(UByte*& ip)
                                             __asm__("_ZN5shape6dospinERPh");
extern void           shape_dolighttimer(UByte*& ip)
                                             __asm__("_ZN5shape12dolighttimerERPh");
extern void           shape_doanimation(UByte*& ip)
                                             __asm__("_ZN5shape11doanimationERPh");
extern void           shape_docompass(UByte*& ip)
                                             __asm__("_ZN5shape9docompassERPh");
extern void           shape_docreatebpoly(UByte*& ip)
                                             __asm__("_ZN5shape13docreatebpolyERPh");
extern void           shape_dodrawbpoly(UByte*& ip)
                                             __asm__("_ZN5shape11dodrawbpolyERPh");
extern void           shape_doimagemap(UByte*& ip)
                                             __asm__("_ZN5shape10doimagemapERPh");
extern void           shape_dolauncher(UByte*& ip)
                                             __asm__("_ZN5shape10dolauncherERPh");
extern void           shape_donspheres(UByte*& ip)
                                             __asm__("_ZN5shape10donspheresERPh");
extern void           shape_donspheresimapd(UByte*& ip)
                                             __asm__("_ZN5shape15donspheresimapdERPh");
extern UByte          Three_Dee_bytes[]      __asm__("Three_Dee");

// MODVEC.CPP - FP/FCRD/FORI signatures come from MODVEC.H.
extern void mv_NullVec(FCRD& v) __asm__("_Z7NullVecR5_fcrd");
extern void mv_CopyVec(FCRD& s, FCRD& d) __asm__("_Z7CopyVecR5_fcrdS0_");
extern void mv_AddVec(FCRD& d, FCRD& a, FCRD& b) __asm__("_Z6AddVecR5_fcrdS0_S0_");
extern void mv_SubVec(FCRD& d, FCRD& a, FCRD& b) __asm__("_Z6SubVecR5_fcrdS0_S0_");
extern FP   mv_VecLen(FCRD& v) __asm__("_Z6VecLenR5_fcrd");
extern Bool mv_NrmVec2D(FP& x, FP& y) __asm__("_Z8NrmVec2DRfS_");
extern void mv_RotateVec2D(FP& x, FP& y, FP a) __asm__("_Z11RotateVec2DRfS_f");
extern void mv_RotVecXSC(FCRD& s, FCRD& d, FP sn, FP cs)
                                             __asm__("_Z9RotVecXSCR5_fcrdS0_ff");
extern void mv_RotVecYSC(FCRD& s, FCRD& d, FP sn, FP cs)
                                             __asm__("_Z9RotVecYSCR5_fcrdS0_ff");
extern void mv_RotVecZSC(FCRD& s, FCRD& d, FP sn, FP cs)
                                             __asm__("_Z9RotVecZSCR5_fcrdS0_ff");
extern void mv_TnsPnt(FCRD& s, FCRD& d, FORI& o) __asm__("_Z6TnsPntR5_fcrdS0_R5_fori");
extern void mv_SetOri(FORI& o, FP x, FP y, FP z) __asm__("_Z6SetOriR5_forifff");
extern FP   mv_CalcAngle(FP x, FP y) __asm__("_Z9CalcAngleff");

// Rasterizer colour state from GRAFPRIM.CPP (extern "C", unmangled).
extern "C" {
struct TestColourData {
    uint32_t imageptr, alphaptr, imagexmask, imageymask,
             aliastblptr, lumtblptr;
    uint8_t  col, imageyshift, pad2, pad3;
};
extern TestColourData colour_data;
}

// Debug globals SHAPES.CPP expects from the exe (defined in MigAlley.cpp).
int   g_shp_last_num = -1;
void* g_shp_last_fb = 0;
void* g_shp_last_data = 0;

// Graphics globals (GRAFPRIM.CPP) - extern linkage, checked directly.
extern uint16_t palette_table[256];
extern uint16_t landscape_palette[256];
extern uint16_t palette_buffer[256 * 8];

// Graphic:: palette methods (WRAPPER.CPP/GRAFPRIM.CPP) - they touch only
// globals, so a dummy `this` is safe.
extern void   graphic_SelectPalette(void* self, SWord n)
                                          __asm__("_ZN7Graphic13SelectPaletteEs");
extern UWord  graphic_GetPaletteEntry(void* self, UWord c)
                                          __asm__("_ZN7Graphic15GetPaletteEntryEt");
extern void   graphic_SetPaletteEntry(void* self, UWord c, UWord v)
                                          __asm__("_ZN7Graphic15SetPaletteEntryEtt");

//------------------------------------------------------------------------------
// Tiny harness (shared with test_compat.cpp)
//------------------------------------------------------------------------------
#include "harness.h"
int g_checks = 0;
int g_failures = 0;

// Compat-layer tests live in test_compat.cpp (WIN32_COMPAT.H collides with
// the game headers included here).
void test_win32_events();
void test_win32_semaphore();
void test_win32_mutex();
void test_win32_timing();
void test_win32_files();

//------------------------------------------------------------------------------
// Type + union layout invariants the instruction stream and fixed-point
// paths depend on. These pin the i386 ABI the port was validated against.
//------------------------------------------------------------------------------
static void test_type_layout()
{
    CHECK_EQ(sizeof(SByte), 1);
    CHECK_EQ(sizeof(UByte), 1);
    CHECK_EQ(sizeof(SWord), 2);
    CHECK_EQ(sizeof(UWord), 2);
    CHECK_EQ(sizeof(SLong), 4);
    CHECK_EQ(sizeof(ULong), 4);
    CHECK_EQ(sizeof(Float), 8);          // Float is double
    CHECK_EQ(sizeof(void*), 4);          // 32-bit build
}

static void test_ifshare()
{
    // IFShare overlays a 32-bit int with a double: writers use .f (double),
    // scanline readers use .i (low 32 bits). FtoITexture performs the
    // conversion; this test documents the contract.
    IFShare s;
    s.f = 5.0;
    CHECK_EQ(s.i, 0);                    // low dword of 5.0 is 0, NOT 5
    s.i = 12345;
    CHECK(s.f == s.f);                   // readable; value is garbage-double
    CHECK_EQ(sizeof(IFShare), 8);
    CHECK_EQ(offsetof(IFShare, i), offsetof(IFShare, f));

    // .i aliases the LOW 32 bits of the double (little-endian). Doubles
    // with zero low-mantissa read 0 even with a sign/exponent set:
    s.f = -0.0;                          // 0x80000000_00000000
    CHECK_EQ(s.i, 0);                    // sign lives in the HIGH dword
    s.f = 1.1;                           // 0x3FF19999_9999999A
    CHECK_EQ(s.i, (SLong)0x9999999A);    // raw low mantissa, not a float cast
}

static void test_vertex_layout()
{
    // vertex (VERTEX.H): IFShare fields then intensity/bigsx/clipFlags.
    CHECK_EQ(offsetof(vertex, by), offsetof(vertex, bx) + sizeof(IFShare));
    CHECK_EQ(offsetof(vertex, bz), offsetof(vertex, by) + sizeof(IFShare));
    CHECK_EQ(offsetof(vertex, ix), offsetof(vertex, sy) + sizeof(IFShare));
    CHECK_EQ(offsetof(vertex, iy), offsetof(vertex, ix) + sizeof(IFShare));
}

static void test_dopointstruc_layout()
{
    CHECK_EQ(offsetof(DoPointStruc, bodyy),
             offsetof(DoPointStruc, bodyx) + sizeof(IFShare));
    CHECK_EQ(offsetof(DoPointStruc, screenx),
             offsetof(DoPointStruc, bodyz) + sizeof(IFShare));
    CHECK_EQ(offsetof(DoPointStruc, clipFlags),
             offsetof(DoPointStruc, specFlip) + sizeof(SWord));
    // mist/snow/ni/nj/nk ride the tail - written by the land poly path.
    CHECK_EQ(offsetof(DoPointStruc, ni),
             offsetof(DoPointStruc, snow) + sizeof(UByte));
}

//------------------------------------------------------------------------------
// Shape instruction-stream structs must match the 1998 on-disk byte layout
// (natural MSVC packing: UWord members 2-aligned). Any drift desyncs the
// interpreter - this is the bug class the artifact hunt chased.
//------------------------------------------------------------------------------
static void test_shape_instr_layout()
{
    CHECK_EQ(sizeof(DOPOINT), 8);
    CHECK_EQ(sizeof(DOLINE), 4);
    CHECK_EQ(sizeof(DOPOLYGON), 1);
    CHECK_EQ(sizeof(DOSETCOLOUR), 4);
    CHECK_EQ(sizeof(DOIFCROSS), 8);
    CHECK_EQ(sizeof(NNEXT), 6);

    // DOSDEFS.H sets #pragma pack(1) globally: the instruction stream is
    // byte-packed, so members are MISALIGNED by design. These sizes are the
    // wire format - a pack(4) regression would desync the interpreter.
    CHECK_EQ(sizeof(DONPOINTS), 3);
    CHECK_EQ(offsetof(DONPOINTS, start_vertex), 1);

    CHECK_EQ(sizeof(DOPOINT2X), 10);
    CHECK_EQ(offsetof(DOPOINT2X, next_vertex_offset), 2);
    CHECK_EQ(offsetof(DOPOINT2X, xcoord), 4);

    CHECK_EQ(sizeof(DOCASERANGE), 5);
    CHECK_EQ(offsetof(DOCASERANGE, failjump), 3);
}

//------------------------------------------------------------------------------
// matrix::SetClipFlags - the exact predicates the renderer relies on.
//------------------------------------------------------------------------------
static void test_clip_flags()
{
    matrix m;
    m.fpMaximumZ = 1600000.0;            // RANGE_AIRCRAFT

    DoPointStruc dp;
    dp.bodyx.f = 0;
    dp.bodyy.f = 0;

    // On-axis visible point -> no flags.
    dp.bodyz.f = 5000;  dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK_EQ(dp.clipFlags, CF3D_NULL);

    // Behind near plane (z < 1).
    dp.bodyz.f = 0.5;   dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK_EQ(dp.clipFlags, CF3D_BEHINDNEARZ);

    // Past far plane.
    dp.bodyz.f = 2000000.0; dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK_EQ(dp.clipFlags, CF3D_PASTFARZ);

    // Off left/right: |x| beyond the view cone at this z.
    dp.bodyz.f = 1000; dp.clipFlags = CF3D_NULL;
    dp.bodyx.f = -5000;
    m.SetClipFlags(dp);
    CHECK(dp.clipFlags & CF3D_OFFLEFT);
    dp.bodyx.f = 5000; dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK(dp.clipFlags & CF3D_OFFRIGHT);

    // Off top/bottom.
    dp.bodyx.f = 0; dp.bodyy.f = 5000; dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK(dp.clipFlags & CF3D_OFFTOP);
    dp.bodyy.f = -5000; dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK(dp.clipFlags & CF3D_OFFBOTTOM);
}

// The values logged from live shape 289 (CMIG15) during the artifact hunt:
// marker-line verts sit ~700000 units deep. They must NOT be far-clipped,
// which is why the distant-MiG line fan legitimately renders as specks.
static void test_distant_marker_clip()
{
    matrix m;
    m.fpMaximumZ = 1600000.0;
    DoPointStruc dp;
    dp.bodyx.f = -323551; dp.bodyy.f = -385596; dp.bodyz.f = 715233;
    dp.clipFlags = CF3D_NULL;
    m.SetClipFlags(dp);
    CHECK_EQ(dp.clipFlags, CF3D_NULL);   // inside frustum by design
}

//------------------------------------------------------------------------------
// matrix::transformNC - integer local coords -> float body coords, and it
// must reset clipFlags (the NC variant does not flag).
//------------------------------------------------------------------------------
static void zero_fpmatrix(FPMATRIX& m)
{
    m.L11 = m.L12 = m.L13 =
    m.L21 = m.L22 = m.L23 =
    m.L31 = m.L32 = m.L33 = 0.0;
}

static void test_transform_nc()
{
    matrix m;
    FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    DoPointStruc dp;
    dp.bodyx.i = 100; dp.bodyy.i = 50; dp.bodyz.i = 200;
    dp.clipFlags = CF3D_OFFTOP;          // must be cleared
    m.transformNC(&ident, &dp);
    CHECK_FEQ(dp.bodyx.f, 100.0);
    CHECK_FEQ(dp.bodyy.f, 50.0);
    CHECK_FEQ(dp.bodyz.f, 200.0);
    CHECK_EQ(dp.clipFlags, CF3D_NULL);

    // 90-degree rotation about Y: x' = z, z' = -x (right-handed).
    FPMATRIX rot;
    zero_fpmatrix(rot);
    rot.L13 = 1.0; rot.L22 = 1.0; rot.L31 = -1.0;
    dp.bodyx.i = 10; dp.bodyy.i = 0; dp.bodyz.i = 5;
    m.transformNC(&rot, &dp);
    CHECK_FEQ(dp.bodyx.f, 5.0);
    CHECK_FEQ(dp.bodyz.f, -10.0);
}

//------------------------------------------------------------------------------
// shape::dopoint2x - writes a mirrored x pair into newco[v1] and
// newco[v1+off]. This is the instruction that writes the "mystery" slots
// 7,8,11,12 in CMIG15's marker fan; pin its contract directly.
//------------------------------------------------------------------------------
// Obj_3D (3DCODE.H) - replicated here so the test avoids the world include
// chain. Layout: Base_Obj3D base + ptr + animptr + fpCOORDS3D + bitfield +
// 3 ANGLES; offsets verified against the real struct members dopoint2x reads.
struct TestObj3D : Base_Obj3D
{
    void*       ItemPtr;
    animptr     AnimPtr;
    fpCOORDS3D  Body;
    UWord       Shape:15,
                IsTransient:1;
    ANGLES      AngH,
                AngC,
                AngR;
};

static void test_dopoint2x()
{
    TestObj3D obj = TestObj3D();
    obj.Body.X.f = 1000.0;
    obj.Body.Y.f = -50.0;
    obj.Body.Z.f = 4000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    shape_newco = shape_shpco;
    shape_fpobject_matrix = &ident;
    shape_object_obj3d = &obj;
    _matrix.fpMaximumZ = 1600000.0;

    // Instruction: start_vertex=7, next_vertex_offset=1, (100,50,200).
    DOPOINT2X ins;
    ins.start_vertex = 7;
    ins.next_vertex_offset = 1;
    ins.xcoord = 100; ins.ycoord = 50; ins.zcoord = 200;

    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    UByte* ip = (UByte*)&ins;
    shape_dopoint2x(ip);

    CHECK_EQ(ip - (UByte*)&ins, (long)sizeof(DOPOINT2X));

    // Slot 7: identity transform + object body.
    CHECK_FEQ(shape_shpco[7].bodyx.f, 1100.0);
    CHECK_FEQ(shape_shpco[7].bodyy.f, 0.0);
    CHECK_FEQ(shape_shpco[7].bodyz.f, 4200.0);

    // Slot 8: mirrored x only (the 2x contract).
    CHECK_FEQ(shape_shpco[8].bodyx.f, 900.0);
    CHECK_FEQ(shape_shpco[8].bodyy.f, 0.0);
    CHECK_FEQ(shape_shpco[8].bodyz.f, 4200.0);

    // Both get clip flags + specular init.
    CHECK_EQ(shape_shpco[7].specular, -1);
    CHECK_EQ(shape_shpco[8].specular, -1);
}

//------------------------------------------------------------------------------
// FtoITexture (FTOI.H, verbatim from POLYGON.CPP) - converts the float
// u,v written by createvert into the fixed-point .i the scanline fillers
// read. Without this step u,v read as the low dword of a double, which is
// the garbage-texture bug class the violet terrain came from.
//------------------------------------------------------------------------------
static void test_ftoitexture()
{
    vertex v[3];
    for (int i = 0; i < 3; ++i)
    {
        v[i].ix.f = 0.0; v[i].iy.f = 0.0;
    }
    v[0].ix.f = 37.5;  v[0].iy.f = 12.25;
    v[1].ix.f = -4.9;  v[1].iy.f = 255.99;
    v[2].ix.f = 0.0;   v[2].iy.f = 0.0;

    VERTEX_PTR ptrs[4] = { &v[0], &v[1], &v[2], NULL };
    FtoITexture(ptrs);

    CHECK_EQ(v[0].ix.i, 37);             // SLong(double) truncates
    CHECK_EQ(v[0].iy.i, 12);
    CHECK_EQ(v[1].ix.i, -4);
    CHECK_EQ(v[1].iy.i, 255);
    CHECK_EQ(v[2].ix.i, 0);

    // Edge: truncation is C-style toward zero on both signs; the loop
    // stops at the NULL terminator (v[2] is still covered above).
    vertex w;
    w.ix.f = 256.9;  w.iy.f = -255.99;
    VERTEX_PTR p2[2] = { &w, NULL };
    FtoITexture(p2);
    CHECK_EQ(w.ix.i, 256);
    CHECK_EQ(w.iy.i, -255);              // truncates toward zero, not -256

    VERTEX_PTR empty[1] = { NULL };      // empty list: no-op, no deref
    FtoITexture(empty);
}

//------------------------------------------------------------------------------
// Palette selection - regression for the violet-terrain bug:
// SelectPalette(-1) must copy landscape_palette, not palette_buffer[-256]
// (which under GCC lands on palette_table and no-ops).
//------------------------------------------------------------------------------
static void test_select_palette()
{
    static char dummy_graphic[64];
    void* g = dummy_graphic;

    for (int i = 0; i < 256; ++i) landscape_palette[i] = (uint16_t)(0x4000 + i);
    for (int i = 0; i < 256; ++i) palette_buffer[256 + i] = (uint16_t)(0x8000 + i);

    graphic_SelectPalette(g, -1);
    CHECK_EQ(graphic_GetPaletteEntry(g, 0), 0x4000);
    CHECK_EQ(graphic_GetPaletteEntry(g, 255), 0x40FF);
    CHECK_EQ(palette_table[17], landscape_palette[17]);

    graphic_SelectPalette(g, 1);
    CHECK_EQ(graphic_GetPaletteEntry(g, 0), 0x8000);
    CHECK_EQ(palette_table[200], palette_buffer[256 + 200]);

    // SetPaletteEntry round-trips through palette_table.
    graphic_SetPaletteEntry(g, 7, 0x1234);
    CHECK_EQ(graphic_GetPaletteEntry(g, 7), 0x1234);
    CHECK_EQ(palette_table[7], 0x1234);
}

//------------------------------------------------------------------------------
// animptr (ANIMPTR.H, NANIMDEBUG build) - thin void* wrapper the whole
// animation system indexes through. operator[] is the cast lint-fixed to
// ((UByteP)ptr)[a]; pin it plus the byte-diff/offset contract.
//------------------------------------------------------------------------------
static void test_animptr()
{
    UByte buf[8] = {10, 11, 12, 13, 14, 15, 16, 17};

    animptr a;
    a = buf;                                     // operator=(UByteP)
    CHECK_EQ(a[0], 10);                          // operator[](int)
    CHECK_EQ(a[3], 13);
    CHECK_EQ(a[(UWord)5], 15);                   // operator[](UWord)
    CHECK_EQ(a[(SWord)6], 16);                   // operator[](SWord)
    a[2] = 99;                                   // lvalue through []
    CHECK_EQ(buf[2], 99);

    CHECK(a == (void*)buf);
    CHECK(a != (void*)(buf + 1));

    animptr b; b = buf;
    animptr c; c = buf + 4;
    CHECK_EQ(c - b, 4);                          // operator- = byte distance
    CHECK_EQ(b.Offset(buf + 6), 6);              // Offset(void*)

    c += 2;
    CHECK_EQ(c - b, 6);
    ++b;
    CHECK_EQ(b - a, 1);                          // ++ advances one byte

    // operator& returns the raw UByteP (overloaded unary &).
    animptr d; d = buf;
    UByteP raw = &d;
    CHECK_EQ((void*)raw, (void*)buf);

    // NANIMDEBUG is raw pointer math: negative indices read before the
    // base, and Offset() backward is a wrapped byte distance. The debug
    // assertions that would catch these are compiled out - the wrap is
    // the live contract.
    CHECK_EQ(c[-4], buf[2]);             // c = buf+6; [-4] -> buf[2] = 99
    CHECK_EQ(c.Offset(buf + 2), (ULong)-4); // -4 wrapped to ULong
}

//------------------------------------------------------------------------------
// MODVEC.CPP angle conversions - the Rowan angle unit (2^16 per revolution)
// every orientation in the sim is stored in. Degs2Rowan/Rads2Rowan truncate
// to SLong then wrap via &0xffff; the inverse functions divide.
//------------------------------------------------------------------------------
static void test_modvec_angles()
{
    // NB: 182.04444444 sits just below 65536/360, so (SLong) truncation
    // lands 1 LSB low on round inputs - 1998 behavior, pinned as-is.
    CHECK_EQ(Degs2Rowan(0), 0);
    CHECK_EQ(Degs2Rowan(90), 16383);             // quarter turn minus 1 LSB
    CHECK_EQ(Degs2Rowan(360), -1);               // 65535 -> 0xFFFF wraps
    CHECK_EQ(Degs2Rowan(180), 32767);            // 32767, not -32768

    CHECK_NEAR(Rowan2Rads(16384), 1.5707963, 1e-4);   // quarter turn = pi/2
    CHECK_NEAR(Rowan2Degs(16384), 90.0, 0.01);
    CHECK_NEAR(Rowan2Rads(0), 0.0, 1e-9);

    // Rads->Rowan truncates (SLong cast) before masking: pi -> 0x7FFF..0x8000.
    SWord r = Rads2Rowan(3.14159265f);
    CHECK(r == 32767 || r == -32768 || r == -32767);

    // Roundtrip: degrees -> Rowan -> degrees.
    CHECK_NEAR(Rowan2Degs((UWord)Degs2Rowan(45)), 45.0, 0.1);

    CHECK_NEAR(Degs2Rads(180), 3.14159265, 1e-5);
    CHECK_NEAR(Rads2Degs(3.14159265), 180.0, 1e-4);
}

//------------------------------------------------------------------------------
// MODVEC.CPP vector ops - portable replacements for the dead x86 asm.
// Conventions: RotVecXSC rotates y/z (x' = x), RotVecYSC uses
// x' = x.cos + z.sin / z' = z.cos - x.sin, RotVecZSC uses
// x' = x.cos - y.sin / y' = y.cos + x.sin.
//------------------------------------------------------------------------------
static void test_modvec_vectors()
{
    FCRD v34 = {3.0f, 4.0f, 0.0f};
    CHECK_NEAR(VecLen(v34), 5.0, 1e-6);
    CHECK_NEAR(VecLen2D(3.0f, 4.0f), 5.0, 1e-6);

    // NrmVec always writes dest (the len==1.0 early-out must still copy:
    // DotPrd consumes the dest of two NrmVec calls).
    FCRD unit = {1.0f, 0.0f, 0.0f}, dst = {9.0f, 9.0f, 9.0f};
    CHECK(NrmVec(unit, dst) != BOOL_FALSE);
    CHECK_NEAR(dst.x, 1.0, 1e-9);
    FCRD nv;
    CHECK(NrmVec(v34, nv) != BOOL_FALSE);
    CHECK_NEAR(VecLen(nv), 1.0, 1e-6);
    FCRD zero = {0.0f, 0.0f, 0.0f};
    CHECK(NrmVec(zero, nv) == BOOL_FALSE);       // zero-length -> FALSE

    // DotPrd normalises BOTH operands first: it is a cosine, not a raw
    // dot product. (v1,v2,v3) ops all take dest FIRST: CPrd(v1,v2,v3) is
    // v1 = v2 x v3.
    FCRD two = {2, 0, 0}, three = {0, 3, 0};
    CHECK_NEAR(DotPrd(two, three), 0.0, 1e-6);   // orthogonal
    FCRD four = {4, 0, 0};
    CHECK_NEAR(DotPrd(two, four), 1.0, 1e-6);    // parallel
    CHECK_NEAR(DotPrd(two, two), 1.0, 1e-6);

    FCRD ex = {1, 0, 0}, ey = {0, 1, 0}, r;
    CPrd(r, ex, ey);                             // r = x cross y = z
    CHECK_NEAR(r.x, 0.0, 1e-9);
    CHECK_NEAR(r.y, 0.0, 1e-9);
    CHECK_NEAR(r.z, 1.0, 1e-9);
    CPrd(r, ey, ex);                             // anti-commutes
    CHECK_NEAR(r.z, -1.0, 1e-9);

    AddVec(r, ex, ey);
    CHECK_NEAR(r.x, 1.0, 1e-9); CHECK_NEAR(r.y, 1.0, 1e-9);
    SubVec(r, ex, ey);
    CHECK_NEAR(r.x, 1.0, 1e-9); CHECK_NEAR(r.y, -1.0, 1e-9);

    // sin=1, cos=0 -> quarter turn.
    RotVecZSC(ex, r, 1.0f, 0.0f);                // (1,0,0) -> (0,1,0)
    CHECK_NEAR(r.x, 0.0, 1e-6); CHECK_NEAR(r.y, 1.0, 1e-6);
    RotVecXSC(ey, r, 1.0f, 0.0f);                // (0,1,0) -> (0,0,1)
    CHECK_NEAR(r.y, 0.0, 1e-6); CHECK_NEAR(r.z, 1.0, 1e-6);
    RotVecYSC(ex, r, 1.0f, 0.0f);                // (1,0,0) -> (0,0,-1)
    CHECK_NEAR(r.x, 0.0, 1e-6); CHECK_NEAR(r.z, -1.0, 1e-6);

    // 2D rotation: (1,0) by pi/2 -> (0,1).
    FP rx = 1.0f, ry = 0.0f;
    RotateVec2D(rx, ry, 1.5707963f);
    CHECK_NEAR(rx, 0.0, 1e-5); CHECK_NEAR(ry, 1.0, 1e-5);

    // SetOri(0,0,0) is identity; TnsPnt passes the vector through.
    FORI ori;
    SetOri(ori, 0.0f, 0.0f, 0.0f);
    FCRD vin = {7.0f, -2.0f, 3.0f}, vout;
    TnsPnt(vin, vout, ori);
    CHECK_NEAR(vout.x, 7.0, 1e-5);
    CHECK_NEAR(vout.y, -2.0, 1e-5);
    CHECK_NEAR(vout.z, 3.0, 1e-5);
}

//------------------------------------------------------------------------------
// shape::dopoint - integer-coords path: bodyx.i = xcoord then transformNC.
// Same frame setup as test_dopoint2x.
//------------------------------------------------------------------------------
static void test_dopoint()
{
    TestObj3D obj = TestObj3D();
    obj.Body.X.f = 1000.0;
    obj.Body.Y.f = -50.0;
    obj.Body.Z.f = 4000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    shape_newco = shape_shpco;
    shape_fpobject_matrix = &ident;
    shape_object_obj3d = &obj;
    _matrix.fpMaximumZ = 1600000.0;

    DOPOINT ins;
    ins.vertex = 5;
    ins.xcoord = 100; ins.ycoord = 50; ins.zcoord = 200;

    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    UByte* ip = (UByte*)&ins;
    shape_dopoint(ip);

    CHECK_EQ(ip - (UByte*)&ins, (long)sizeof(DOPOINT));
    CHECK_FEQ(shape_shpco[5].bodyx.f, 1100.0);
    CHECK_FEQ(shape_shpco[5].bodyy.f, 0.0);
    CHECK_FEQ(shape_shpco[5].bodyz.f, 4200.0);
    CHECK_EQ(shape_shpco[5].specular, -1);
}

//------------------------------------------------------------------------------
// shape::donpoints - float-path batch writer: DONPOINTS{count,start} then
// count COORDS records. Each vertex gets f=matrix*local+Body, clipFlags
// from double-bit compares, specular=-1; instr_ptr advances past all data.
//------------------------------------------------------------------------------
static void test_donpoints()
{
    TestObj3D obj = TestObj3D();
    obj.Body.X.f = 1000.0;
    obj.Body.Y.f = -50.0;
    obj.Body.Z.f = 4000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    shape_newco = shape_shpco;
    shape_fpobject_matrix = &ident;
    shape_object_obj3d = &obj;
    _matrix.fpMaximumZ = 1600000.0;

    // Wire layout: DONPOINTS(3B) + 2 x COORDS(6B).
    UByte stream[sizeof(DONPOINTS) + 2 * sizeof(COORDS)];
    DONPOINTS* hdr = (DONPOINTS*)stream;
    hdr->vertex_count = 2;
    hdr->start_vertex = 3;
    COORDS* c = (COORDS*)(stream + sizeof(DONPOINTS));
    c[0].xcoord = 10;  c[0].ycoord = 20;  c[0].zcoord = 30;
    c[1].xcoord = 40;  c[1].ycoord = 50;  c[1].zcoord = 60;

    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    UByte* ip = stream;
    shape_donpoints(ip);

    CHECK_EQ(ip - stream, (long)sizeof(stream));
    CHECK_FEQ(shape_shpco[3].bodyx.f, 1010.0);
    CHECK_FEQ(shape_shpco[3].bodyy.f, -30.0);
    CHECK_FEQ(shape_shpco[3].bodyz.f, 4030.0);
    CHECK_FEQ(shape_shpco[4].bodyx.f, 1040.0);
    CHECK_FEQ(shape_shpco[4].bodyy.f, 0.0);
    CHECK_FEQ(shape_shpco[4].bodyz.f, 4060.0);
    CHECK_EQ(shape_shpco[3].specular, -1);
    CHECK_EQ(shape_shpco[4].specular, -1);
    CHECK_EQ(shape_shpco[3].clipFlags, CF3D_NULL);   // inside frustum
}

//------------------------------------------------------------------------------
// matrix::transform (MATRIX.CPP) - unlike transformNC it runs SetClipFlags,
// so a vertex past the far plane comes back flagged.
//------------------------------------------------------------------------------
static void test_transform()
{
    matrix m;
    m.fpMaximumZ = 1600000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    DoPointStruc dp;
    dp.bodyx.i = 100; dp.bodyy.i = 50; dp.bodyz.i = 200;
    m.transform(&ident, dp);
    CHECK_FEQ(dp.bodyx.f, 100.0);
    CHECK_FEQ(dp.bodyz.f, 200.0);
    CHECK_EQ(dp.clipFlags, CF3D_NULL);           // in frustum

    dp.bodyx.i = 0; dp.bodyy.i = 0; dp.bodyz.i = 2000000;
    m.transform(&ident, dp);
    CHECK_EQ(dp.clipFlags & CF3D_PASTFARZ, CF3D_PASTFARZ);
}

//------------------------------------------------------------------------------
// Shape-stream flow control - pure instr_ptr manipulation. A wrong jump
// desyncs the stream and produces every artifact class downstream, so the
// jump math is pinned directly.
//------------------------------------------------------------------------------
static void test_flow_control()
{
    // dogoto: instr_ptr += *(SWord*)instr_ptr (signed 16-bit offset)
    {
        UByte stream[8] = {0};
        *(SWord*)stream = 5;
        UByte* ip = stream;
        shape_dogoto(ip);
        CHECK_EQ(ip - stream, 5);
        *(SWord*)(stream + 6) = -3;                 // backward jump
        ip = stream + 6;
        shape_dogoto(ip);
        CHECK_EQ(ip - stream, 3);
    }
    // doret: marker only - InterpLoop terminates on the opcode itself;
    // the handler must leave instr_ptr untouched.
    {
        UByte stream[4] = {0};
        UByte* ip = stream;
        shape_doret(ip);
        CHECK_EQ((void*)ip, (void*)stream);
    }
    // doifeq/doifne: advance-only ops (their operand words are consumed
    // by the compiler's stream layout, not evaluated at runtime).
    {
        UByte stream[8] = {0};
        UByte* ip = stream;
        shape_doifeq(ip);
        CHECK_EQ(ip - stream, (long)sizeof(DOIFEQ));
        ip = stream;
        shape_doifne(ip);
        CHECK_EQ(ip - stream, (long)sizeof(DOIFNE));
    }
}

//------------------------------------------------------------------------------
// shape::doswitch - reads GlobalAdptr[animoff]; on condition fail it
// rewinds one byte so the next loop read lands inside the instruction
// (the 1998 conditional-skip idiom). Both modes pinned: single-bit test
// (nobits) and scaled compare (condition + animscale).
//------------------------------------------------------------------------------
static void test_doswitch()
{
    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;                    // animptr::operator=(UByteP)

    DOSWITCH ins;
    std::memset(&ins, 0, sizeof(ins));
    ins.animscale = 2;
    ins.animoff = 1;
    UByte* base = (UByte*)&ins;

    // nobits mode: flag = (byte >> bitoffset) & 1
    anim[1] = 0x02;
    ins.nobits = 1; ins.bitoffset = 1;
    ins.value = 1;
    UByte* ip = base;
    shape_doswitch(ip);
    CHECK_EQ(ip - base, (long)sizeof(DOSWITCH)); // flag==value -> continue
    ins.value = 0;
    ip = base;
    shape_doswitch(ip);
    CHECK_EQ(ip - base, (long)sizeof(DOSWITCH) - 1); // mismatch -> rewind

    // scaled mode: flag = byte / animscale; GREATER_THAN skips when
    // flag <= value, LESS_THAN skips when flag >= value.
    ins.nobits = 0;
    anim[1] = 10;                                // flag = 10/2 = 5
    ins.condition = 0;                           // GREATER_THAN
    ins.value = 4;
    ip = base;
    shape_doswitch(ip);
    CHECK_EQ(ip - base, (long)sizeof(DOSWITCH)); // 5 > 4 -> continue
    ins.value = 5;
    ip = base;
    shape_doswitch(ip);
    CHECK_EQ(ip - base, (long)sizeof(DOSWITCH) - 1); // 5 <= 5 -> rewind
    ins.condition = 1;                           // LESS_THAN
    ins.value = 6;
    ip = base;
    shape_doswitch(ip);
    CHECK_EQ(ip - base, (long)sizeof(DOSWITCH)); // 5 < 6 -> continue
    ins.value = 5;
    ip = base;
    shape_doswitch(ip);
    CHECK_EQ(ip - base, (long)sizeof(DOSWITCH) - 1); // 5 >= 5 -> rewind
}

//------------------------------------------------------------------------------
// shape::docaserange - walks (range,jump) SWord pairs and takes the jump
// of the LAST range <= flag (or failjump). Byte flag scaled by factor;
// useword reads a SWord from the anim block instead.
//------------------------------------------------------------------------------
static void test_docaserange()
{
    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;

    UByte stream[32] = {0};
    DOCASERANGE* p = (DOCASERANGE*)stream;
    p->flag = 1;
    p->nofields = 2;
    p->useword = 0;
    p->factor = 2;
    p->failjump = 50;
    SWord* pairs = (SWord*)(stream + sizeof(DOCASERANGE));
    pairs[0] = 3;  pairs[1] = 20;               // flag>=3 -> +20
    pairs[2] = 7;  pairs[3] = 30;               // flag>=7 -> +30

    anim[1] = 10;                                // flag = 10/2 = 5 -> range3
    UByte* ip = stream;
    shape_docaserange(ip);
    CHECK_EQ(ip - stream, 20);

    anim[1] = 16;                                // flag = 8 -> range7
    ip = stream;
    shape_docaserange(ip);
    CHECK_EQ(ip - stream, 30);

    anim[1] = 2;                                 // flag = 1 < 3 -> failjump
    ip = stream;
    shape_docaserange(ip);
    CHECK_EQ(ip - stream, 50);

    // useword: SWord read at anim[flag].
    *(SWord*)(anim + 1) = 9;
    p->useword = 1;
    p->factor = 2;                               // unused on the word path
    ip = stream;
    shape_docaserange(ip);
    CHECK_EQ(ip - stream, 30);                   // flag = 9 -> range7
}

//------------------------------------------------------------------------------
// shape::dogosub + InterpLoop - dogosub jumps by a signed offset, runs the
// subroutine through InterpLoop until doretno, then resumes after its own
// 2-byte operand. InterpTable is a shape:: static: point it at a local
// table with a marker handler so the dispatch is verifiable.
//------------------------------------------------------------------------------
// InterpLoop consumes the opcode byte (instr_ptr++) before dispatching, so
// the operand pointer handed to a handler already sits past the opcode -
// a no-operand handler must NOT advance it.
static int g_marker;
static void marker_op(UByte*&) { ++g_marker; }

static void test_dogosub_interploop()
{
    static void (*tab[dosetglassrangeno + 1])(UByte*&) = {};
    void (**saved)(UByte*&) = shape_InterpTable; // ~shape() deletes this
    shape_InterpTable = tab;                     // - restore it after the test
    const int MARKOP = dosetglassrangeno;        // last valid opcode index
    tab[MARKOP] = &marker_op;

    g_marker = 0;
    // [SWord offset=8][pad][sub at +8: MARKOP, doretno]
    UByte stream[16] = {0};
    *(SWord*)stream = 8;
    stream[8] = (UByte)MARKOP;
    stream[9] = (UByte)doretno;

    UByte* ip = stream;
    shape_dogosub(ip);
    shape_InterpTable = saved;
    CHECK_EQ(g_marker, 1);                       // sub dispatched our handler
    CHECK_EQ(ip - stream, 2);                    // resumed after the offset
}

//------------------------------------------------------------------------------
// shape::doifcase - jump-table select: offset = anim[flag]/factor (or
// signed-bidirectional with shiftup), clamped to count-1, picks the UWord
// table entry at header+offset*2, then jumps forward by that entry's value.
//------------------------------------------------------------------------------
static void test_doifcase()
{
    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;

    UByte stream[sizeof(DOIFCASE) + 3 * 2] = {0};
    DOIFCASE* p = (DOIFCASE*)stream;
    p->flag = 1;
    p->count = 3;                                   // realcnt = count-1 = 2
    p->bidirectional = 0;
    p->factor = 4;
    UWord* tbl = (UWord*)(stream + sizeof(DOIFCASE));
    tbl[0] = 100; tbl[1] = 200; tbl[2] = 300;
    const long entry2 = (long)(sizeof(DOIFCASE) + 2 * sizeof(UWord)) + 300;

    anim[1] = 10;                                   // offset = 10/4 = 2
    UByte* ip = stream;
    shape_doifcase(ip);
    CHECK_EQ(ip - stream, entry2);

    anim[1] = 200;                                  // offset 50 -> clamped to 2
    ip = stream;
    shape_doifcase(ip);
    CHECK_EQ(ip - stream, entry2);

    anim[1] = 3;                                    // offset 0 -> entry 0
    ip = stream;
    shape_doifcase(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFCASE) + 100);

    // bidirectional: SByte anim value; nonzero offset += shiftup, clamped.
    p->bidirectional = 1;
    p->shiftup = 4;
    *(SByte*)(anim + 1) = -4;                       // -4/4 = -1; +4 -> 3 -> 2
    ip = stream;
    shape_doifcase(ip);
    CHECK_EQ(ip - stream, entry2);
}

//------------------------------------------------------------------------------
// shape::dosetcolour256 - updates shape::colour/range/image statics, then
// fires current_screen->SetColour. MigWindow::SetColour forwards to
// Graphic::SetColour (member writes + ASM_SetColour into colour_data -
// the imageptr member is stored, never dereferenced), so a zeroed dummy
// window is a valid screen. imap=0x0FF keeps the GetImageMapPtr branch
// out (it needs a real Image_Map table).
//------------------------------------------------------------------------------
static void test_dosetcolour256()
{
    static UByte fake_screen[16384];                // >> sizeof(MigWindow)
    shape_current_screen = fake_screen;
    shape_object_dist = 0;
    shape_fade_start = 0;

    UByte stream[sizeof(DOSETCOLOUR256)] = {0};
    DOSETCOLOUR256* p = (DOSETCOLOUR256*)stream;
    p->basecolour = 0x0100;                         // >>1 -> base 0x80
    p->spread = 5;
    p->imap = 0x0FF;

    UByte* ip = stream;
    shape_dosetcolour256(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETCOLOUR256));
    CHECK_EQ(shape_colour, 0x80);                   // basecolour >> 1
    CHECK_EQ(shape_range, 5);
    CHECK_EQ(shape_image, 0x0FF);
    // SetColour(base, range) -> ASM_SetColour(0x80,4,4,NULL):
    // col written, x/y masks = ((1<<2)-1)<<16, yshift = 16-2 = 14.
    CHECK_EQ(colour_data.col, 0x80);
    CHECK_EQ(colour_data.imagexmask, 0x00030000u);
    CHECK_EQ(colour_data.imageymask, 0x00030000u);
    CHECK_EQ(colour_data.imageyshift, 14);
    CHECK_EQ(colour_data.imageptr, 0u);

    // spread is a UByte: 0xFF can never mean range -1, so the
    // 'if (range == -1)' single-colour path is dead on this instruction.
    p->spread = 0xFF;
    ip = stream;
    shape_dosetcolour256(ip);
    CHECK_EQ(shape_range, 0xFF);                    // NOT -1 - two-arg call
    CHECK_EQ(colour_data.col, 0x80);
}

//------------------------------------------------------------------------------
// Skip-only writers: doplumepnt (fully dead - every line commented) and
// donincpnts (skips count*(DOINIT+DOINC) + header). They still advance the
// stream, so desync = corruption downstream; pin the advance formula.
//------------------------------------------------------------------------------
static void test_skip_writers()
{
    UByte stream[64] = {0};
    UByte* ip = stream;
    shape_doplumepnt(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOPLUMEPNT));

    DONINCPNTS* n = (DONINCPNTS*)stream;
    n->count = 3;
    ip = stream;
    shape_donincpnts(ip);
    CHECK_EQ(ip - stream,
        (long)(3 * (sizeof(DOINIT) + sizeof(DOINC)) + sizeof(DONINCPNTS)));
}

//------------------------------------------------------------------------------
// shape::dontpoints - rotating-prop writer: builds a rotation matrix from
// an RPM-derived Rowan angle (anim word, byte*257, or fixedrpm), applies it
// about (base_x,base_y,base_z), then the normal object transform. With
// notrpm the angle is used directly; hpr_data selects the axis.
//------------------------------------------------------------------------------
static void test_dontpoints()
{
    TestObj3D obj = TestObj3D();
    obj.Body.X.f = 1000.0;
    obj.Body.Y.f = -50.0;
    obj.Body.Z.f = 4000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    shape_newco = shape_shpco;
    shape_fpobject_matrix = &ident;
    shape_object_obj3d = &obj;
    _matrix.fpMaximumZ = 1600000.0;

    UByte anim[8] = {0};
    *(UWord*)(anim + 2) = 0x4000;                // ANGLES_90Deg in Rowan
    shape_GlobalAdptr = anim;

    UByte stream[sizeof(DONTPOINTS) + sizeof(NEXTT)] = {0};
    DONTPOINTS* p = (DONTPOINTS*)stream;
    p->count = 1;
    p->start_vertex = 4;
    p->notrpm = 1;                               // angle = rpm verbatim
    p->isWord = 1;                               // UWord at anim[flag]
    p->flag = 2;
    p->hpr_data = 2;                             // pitch axis
    NEXTT* nt = (NEXTT*)(stream + sizeof(DONTPOINTS));
    nt->xcoord = 0; nt->ycoord = 100; nt->zcoord = 0;

    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    UByte* ip = stream;
    shape_dontpoints(ip);

    CHECK_EQ(ip - stream,
             (long)(sizeof(DONTPOINTS) + sizeof(NEXTT)));
    // 90 deg pitch: (0,100,0) -> (0,0,-100); +Body -> (1000,-50,3900)
    CHECK_FEQ(shape_shpco[4].bodyx.f, 1000.0);
    CHECK_FEQ(shape_shpco[4].bodyy.f, -50.0);
    CHECK_FEQ(shape_shpco[4].bodyz.f, 3900.0);
    CHECK_EQ(shape_shpco[4].specular, -1);
}

//------------------------------------------------------------------------------
// shape::domorphnpoints - morph interpolation: verts lerp toward their
// morph targets by timefrac = (timedelta<<13)/growtime, a 13-bit fraction
// read from the anim block. This is the exact arithmetic the "exploded
// vertex" artifact hunt suspected; pin it at 0%, 50% and 100% morph.
//------------------------------------------------------------------------------
static void test_domorphnpoints()
{
    TestObj3D obj = TestObj3D();
    obj.Body.X.f = 1000.0;
    obj.Body.Y.f = -50.0;
    obj.Body.Z.f = 4000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    shape_newco = shape_shpco;
    shape_fpobject_matrix = &ident;
    shape_object_obj3d = &obj;
    _matrix.fpMaximumZ = 1600000.0;

    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;

    UByte stream[sizeof(DOMORPHNPOINTS) + sizeof(MORPHNNEXT)] = {0};
    DOMORPHNPOINTS* p = (DOMORPHNPOINTS*)stream;
    p->framecntoffset = 4;                        // timedelta at anim+4
    p->startvertex = 6;
    p->growtime = 8192;
    p->nopoints = 1;
    MORPHNNEXT* mn = (MORPHNNEXT*)(stream + sizeof(DOMORPHNPOINTS));
    mn->xcoord = 100; mn->ycoord = 0; mn->zcoord = 0;
    mn->mxcoord = 300; mn->mycoord = 0; mn->mzcoord = 0;

    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    // 0%: delta=(0*(300-100))>>13 = 0 -> vert = x = 100
    *(UWord*)(anim + 4) = 0;
    UByte* ip = stream;
    shape_domorphnpoints(ip);
    CHECK_FEQ(shape_shpco[6].bodyx.f, 1100.0);

    // 50%: timefrac = (4096<<13)/8192 = 4096; delta = (4096*200)>>13 = 100
    *(UWord*)(anim + 4) = 4096;
    ip = stream;
    shape_domorphnpoints(ip);
    CHECK_FEQ(shape_shpco[6].bodyx.f, 1200.0);

    // 100%: timefrac = 8192; delta = 200 -> vert = mx = 300
    *(UWord*)(anim + 4) = 8192;
    ip = stream;
    shape_domorphnpoints(ip);
    CHECK_FEQ(shape_shpco[6].bodyx.f, 1300.0);
    CHECK_EQ(ip - stream,
             (long)(sizeof(DOMORPHNPOINTS) + sizeof(MORPHNNEXT)));
}

//------------------------------------------------------------------------------
// matrix::generate2 / fptrans - the 3-angle rotation builder and the
// fp transform that returns clip flags. Convention check at pitch=90deg:
// (x,y,z) -> (x,z,-y).
//------------------------------------------------------------------------------
static void test_matrix_generate2_fptrans()
{
    matrix m;
    m.fpMaximumZ = 1600000.0;

    static FPMATRIX rot;
    zero_fpmatrix(rot);
    m.generate2(ANGLES_0Deg, ANGLES_90Deg, ANGLES_0Deg, &rot);
    CHECK_FEQ(rot.L11, 1.0);
    CHECK_FEQ(rot.L23, 1.0);
    CHECK_FEQ(rot.L32, -1.0);
    CHECK_FEQ(rot.L22, 0.0);

    IFShare x, y, z;
    x.f = 0.0; y.f = 100.0; z.f = 0.0;
    UWord cf = m.fptrans(&rot, x, y, z);
    CHECK_FEQ(x.f, 0.0);
    CHECK_FEQ(y.f, 0.0);
    CHECK_FEQ(z.f, -100.0);
    // Flags are an OR-union, not exclusive: behind-near-z also trips
    // OFFRIGHT/OFFTOP because _clipLR/_clipTB compare x/y against the
    // (negative) z. 25 = 0x01|0x08|0x10 - the real 1998 bitmask.
    CHECK_EQ(cf, CF3D_BEHINDNEARZ | CF3D_OFFRIGHT | CF3D_OFFTOP);

    // Identity passes through and clips nothing inside the frustum.
    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;
    x.f = 10.0; y.f = 5.0; z.f = 200.0;
    cf = m.fptrans(&ident, x, y, z);
    CHECK_FEQ(x.f, 10.0);
    CHECK_EQ(cf, CF3D_NULL);
}

//------------------------------------------------------------------------------
// matrix::Generate2 + multiply - Generate2 is the public scaled builder
// (all nine elements x `scale` - the per-object size contract; aspectRatio
// scaling in matrix::Generate sits behind private members + SetViewParams).
// multiply(t,sip) computes t <- sip*t, so rot^2 doubles the angle.
//------------------------------------------------------------------------------
static void test_matrix_generate_multiply()
{
    matrix m;

    static FPMATRIX rot, scaled;
    m.Generate2(ANGLES_0Deg, ANGLES_90Deg, ANGLES_0Deg, 1.0, &rot);
    m.Generate2(ANGLES_0Deg, ANGLES_90Deg, ANGLES_0Deg, 2.0, &scaled);
    CHECK_FEQ(rot.L11, 1.0);                        // (x,y,z)->(x,z,-y)
    CHECK_FEQ(rot.L23, 1.0);
    CHECK_FEQ(rot.L32, -1.0);
    CHECK_FEQ(scaled.L11, 2.0);
    CHECK_FEQ(scaled.L23, 2.0);
    CHECK_FEQ(scaled.L32, -2.0);

    static FPMATRIX sq;
    sq = rot;
    m.multiply(&sq, &rot);                          // sq = rot*sq = rot^2
    CHECK_FEQ(sq.L11, 1.0);                         // pitch 180: (x,-y,-z)
    CHECK_FEQ(sq.L22, -1.0);
    CHECK_FEQ(sq.L33, -1.0);
    CHECK_FEQ(sq.L23, 0.0);
}

//------------------------------------------------------------------------------
// shape::domorphpoint - single-vertex morph, then delegates to dopoint.
// Delta formula differs from domorphnpoints: (timedelta*(m-x))/growtime,
// no <<13 fraction. growtime=0 is file data -> growt=1 (div-zero guard),
// which morphs instantly to the target.
//------------------------------------------------------------------------------
static void test_domorphpoint()
{
    TestObj3D obj = TestObj3D();
    obj.Body.X.f = 1000.0;
    obj.Body.Y.f = -50.0;
    obj.Body.Z.f = 4000.0;

    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;

    shape_newco = shape_shpco;
    shape_fpobject_matrix = &ident;
    shape_object_obj3d = &obj;
    _matrix.fpMaximumZ = 1600000.0;

    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;

    UByte stream[sizeof(DOMORPHPOINT)] = {0};
    DOMORPHPOINT* p = (DOMORPHPOINT*)stream;
    p->framecntoffset = 4;
    p->vertex = 6;
    p->haswind = 0;
    p->xcoord = 100; p->ycoord = 0; p->zcoord = 0;
    p->mxcoord = 300; p->mycoord = 0; p->mzcoord = 0;
    p->growtime = 8192;

    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    // 50%: delta = (4096*200)/8192 = 100 -> vert = 200
    *(UWord*)(anim + 4) = 4096;
    UByte* ip = stream;
    shape_domorphpoint(ip);
    CHECK_FEQ(shape_shpco[6].bodyx.f, 1200.0);      // 1000 + 200
    CHECK_EQ(ip - stream, (long)sizeof(DOMORPHPOINT));

    // growtime=0 (malformed file): growt=1, delta = timedelta*200.
    // timedelta=1 -> vert lands exactly on the morph target.
    p->growtime = 0;
    *(UWord*)(anim + 4) = 1;
    ip = stream;
    shape_domorphpoint(ip);
    CHECK_FEQ(shape_shpco[6].bodyx.f, 1300.0);      // 1000 + 300
}

//------------------------------------------------------------------------------
// Div-zero guards - every divisor below is raw shape-file data (animscale,
// factor, growtime, MrphGrowtime). The guards leave the raw dividend in
// place when the divisor is 0 rather than SIGFPE-ing on malformed data.
// These tests would have crashed before the RERUN guards went in.
//------------------------------------------------------------------------------
static void test_divzero_guards()
{
    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;

    // doswitch animscale=0: flag stays the raw byte (10 > 4 -> continue).
    {
        DOSWITCH ins;
        std::memset(&ins, 0, sizeof(ins));
        ins.nobits = 0;
        ins.animscale = 0;
        ins.animoff = 1;
        ins.condition = 0;                          // GREATER_THAN
        ins.value = 4;
        anim[1] = 10;
        UByte* base = (UByte*)&ins;
        UByte* ip = base;
        shape_doswitch(ip);
        CHECK_EQ(ip - base, (long)sizeof(DOSWITCH));
    }

    // docaserange factor=0: flag = raw byte 10 -> last range <= 10.
    {
        UByte stream[32] = {0};
        DOCASERANGE* p = (DOCASERANGE*)stream;
        p->flag = 1;
        p->nofields = 2;
        p->useword = 0;
        p->factor = 0;
        p->failjump = 50;
        SWord* pairs = (SWord*)(stream + sizeof(DOCASERANGE));
        pairs[0] = 3;  pairs[1] = 20;
        pairs[2] = 7;  pairs[3] = 30;
        anim[1] = 10;
        UByte* ip = stream;
        shape_docaserange(ip);
        CHECK_EQ(ip - stream, 30);                  // flag=10 -> range7
    }

    // doifcase factor=0: offset = raw byte 10 -> clamped to count-1=2.
    {
        UByte stream[sizeof(DOIFCASE) + 3 * 2] = {0};
        DOIFCASE* p = (DOIFCASE*)stream;
        p->flag = 1;
        p->count = 3;
        p->bidirectional = 0;
        p->factor = 0;
        UWord* tbl = (UWord*)(stream + sizeof(DOIFCASE));
        tbl[0] = 100; tbl[1] = 200; tbl[2] = 300;
        anim[1] = 10;
        UByte* ip = stream;
        shape_doifcase(ip);
        CHECK_EQ(ip - stream,
                 (long)(sizeof(DOIFCASE) + 2 * sizeof(UWord)) + 300);
    }

    // domorphnpoints growtime=0: growt=1 -> timefrac=timedelta<<13,
    // so one tick completes the morph (vert lands on target).
    {
        TestObj3D obj = TestObj3D();
        obj.Body.X.f = 0.0; obj.Body.Y.f = 0.0; obj.Body.Z.f = 0.0;
        static FPMATRIX ident2;
        zero_fpmatrix(ident2);
        ident2.L11 = ident2.L22 = ident2.L33 = 1.0;
        shape_newco = shape_shpco;
        shape_fpobject_matrix = &ident2;
        shape_object_obj3d = &obj;
        _matrix.fpMaximumZ = 1600000.0;

        UByte stream[sizeof(DOMORPHNPOINTS) + sizeof(MORPHNNEXT)] = {0};
        DOMORPHNPOINTS* mp = (DOMORPHNPOINTS*)stream;
        mp->framecntoffset = 4;
        mp->startvertex = 6;
        mp->growtime = 0;
        mp->nopoints = 1;
        MORPHNNEXT* mn = (MORPHNNEXT*)(stream + sizeof(DOMORPHNPOINTS));
        mn->xcoord = 100; mn->ycoord = 0; mn->zcoord = 0;
        mn->mxcoord = 300; mn->mycoord = 0; mn->mzcoord = 0;

        for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
        *(UWord*)(anim + 4) = 1;
        UByte* ip = stream;
        shape_domorphnpoints(ip);
        CHECK_FEQ(shape_shpco[6].bodyx.f, 300.0);   // instant morph
    }
}

//------------------------------------------------------------------------------
// Boundary semantics - instruction-stream and clip-flag edges.
//------------------------------------------------------------------------------
static void test_edge_boundaries()
{
    UByte anim[8] = {0};
    shape_GlobalAdptr = anim;

    // dogoto offset=0: lands on itself (documented hang on bad data).
    {
        UByte stream[4] = {0};
        *(SWord*)stream = 0;
        UByte* ip = stream;
        shape_dogoto(ip);
        CHECK_EQ(ip - stream, 0);
    }

    // doswitch bit tests at bit 0 and bit 7 (field extremes).
    {
        DOSWITCH ins;
        std::memset(&ins, 0, sizeof(ins));
        ins.nobits = 1;
        ins.animoff = 1;
        UByte* base = (UByte*)&ins;
        anim[1] = 0x81;
        ins.bitoffset = 0; ins.value = 1;
        UByte* ip = base;
        shape_doswitch(ip);
        CHECK_EQ(ip - base, (long)sizeof(DOSWITCH));   // bit0 = 1
        ins.bitoffset = 7;
        ip = base;
        shape_doswitch(ip);
        CHECK_EQ(ip - base, (long)sizeof(DOSWITCH));   // bit7 = 1
        ins.bitoffset = 6;
        ip = base;
        shape_doswitch(ip);
        CHECK_EQ(ip - base, (long)sizeof(DOSWITCH) - 1); // bit6 = 0 -> rewind
    }

    // docaserange nofields=0: loop skipped -> failjump verbatim.
    {
        UByte stream[32] = {0};
        DOCASERANGE* p = (DOCASERANGE*)stream;
        p->flag = 1;
        p->nofields = 0;
        p->factor = 1;
        p->failjump = 42;
        anim[1] = 10;
        UByte* ip = stream;
        shape_docaserange(ip);
        CHECK_EQ(ip - stream, 42);
    }

    // doifcase count=1: realcnt=0 clamps every offset to entry 0.
    {
        UByte stream[sizeof(DOIFCASE) + 2] = {0};
        DOIFCASE* p = (DOIFCASE*)stream;
        p->flag = 1;
        p->count = 1;
        p->factor = 1;
        *(UWord*)(stream + sizeof(DOIFCASE)) = 77;
        anim[1] = 250;
        UByte* ip = stream;
        shape_doifcase(ip);
        CHECK_EQ(ip - stream, (long)sizeof(DOIFCASE) + 77);
    }

    // domorphnpoints extrapolation: timedelta=2*growtime -> vert passes
    // the target (x + 2*(mx-x)) - the unclamped overshoot contract.
    {
        TestObj3D obj = TestObj3D();
        obj.Body.X.f = 0.0; obj.Body.Y.f = 0.0; obj.Body.Z.f = 0.0;
        static FPMATRIX ident2;
        zero_fpmatrix(ident2);
        ident2.L11 = ident2.L22 = ident2.L33 = 1.0;
        shape_newco = shape_shpco;
        shape_fpobject_matrix = &ident2;
        shape_object_obj3d = &obj;
        _matrix.fpMaximumZ = 1600000.0;

        UByte stream[sizeof(DOMORPHNPOINTS) + sizeof(MORPHNNEXT)] = {0};
        DOMORPHNPOINTS* mp = (DOMORPHNPOINTS*)stream;
        mp->framecntoffset = 4;
        mp->startvertex = 6;
        mp->growtime = 8192;
        mp->nopoints = 1;
        MORPHNNEXT* mn = (MORPHNNEXT*)(stream + sizeof(DOMORPHNPOINTS));
        mn->xcoord = 100; mn->ycoord = 0; mn->zcoord = 0;
        mn->mxcoord = 300; mn->mycoord = 0; mn->mzcoord = 0;

        for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
        *(UWord*)(anim + 4) = 16384;                // 2*growtime
        UByte* ip = stream;
        shape_domorphnpoints(ip);
        CHECK_FEQ(shape_shpco[6].bodyx.f, 500.0);   // 100 + 2*200
    }

    // dontpoints count=0: header advance only, no verts touched.
    {
        UByte stream[sizeof(DONTPOINTS)] = {0};
        DONTPOINTS* p = (DONTPOINTS*)stream;
        p->count = 0;
        UByte* ip = stream;
        shape_dontpoints(ip);
        CHECK_EQ(ip - stream, (long)sizeof(DONTPOINTS));
    }

    // fptrans clip boundaries: far-z flag trips on z > farz (strict).
    {
        matrix m;
        m.fpMaximumZ = 1600000.0;
        static FPMATRIX ident;
        zero_fpmatrix(ident);
        ident.L11 = ident.L22 = ident.L33 = 1.0;
        IFShare x, y, z;

        x.f = 0.0; y.f = 0.0; z.f = 1600000.0;      // z == farz
        UWord cf = m.fptrans(&ident, x, y, z);
        CHECK_EQ(cf, CF3D_NULL);                    // not past far yet

        x.f = 0.0; y.f = 0.0; z.f = 1600001.0;      // z = farz+1
        cf = m.fptrans(&ident, x, y, z);
        CHECK_EQ(cf, CF3D_PASTFARZ);

        // x == z is NOT off-right (test is strict >).
        x.f = 200.0; y.f = 0.0; z.f = 200.0;
        cf = m.fptrans(&ident, x, y, z);
        CHECK_EQ(cf & CF3D_OFFRIGHT, 0);

        // x = z+1 trips OFFRIGHT; mirrored for OFFLEFT.
        x.f = 201.0; y.f = 0.0; z.f = 200.0;
        cf = m.fptrans(&ident, x, y, z);
        CHECK_EQ(cf & CF3D_OFFRIGHT, CF3D_OFFRIGHT);
        x.f = -201.0;
        cf = m.fptrans(&ident, x, y, z);
        CHECK_EQ(cf & CF3D_OFFLEFT, CF3D_OFFLEFT);
    }
}

//------------------------------------------------------------------------------
// MODVEC edge cases - NrmVec zero vector returns FALSE and copies source;
// DotPrd (a cosine) hits -1 on antiparallel vectors.
//------------------------------------------------------------------------------
static void test_modvec_edges()
{
    FCRD zero_v = {0.0, 0.0, 0.0};
    FCRD dest = {-9.0, -9.0, -9.0};
    CHECK_EQ(NrmVec(zero_v, dest), FALSE);
    CHECK_FEQ(dest.x, 0.0);                          // dest = srce copy
    CHECK_FEQ(dest.y, 0.0);
    CHECK_FEQ(dest.z, 0.0);

    FCRD ex  = {2.0, 0.0, 0.0};
    FCRD nex = {-3.0, 0.0, 0.0};
    FCRD ey  = {0.0, 4.0, 0.0};
    CHECK_FEQ(DotPrd(ex, nex), -1.0);                // antiparallel
    CHECK_FEQ(DotPrd(ex, ey), 0.0);                  // orthogonal

    // CPrd of parallel vectors is the zero vector.
    FCRD r = {1.0, 1.0, 1.0};
    CPrd(r, ex, nex);
    CHECK_FEQ(r.x, 0.0);
    CHECK_FEQ(r.y, 0.0);
    CHECK_FEQ(r.z, 0.0);
}

//------------------------------------------------------------------------------
// shape::doifbright - averages (256 - intensity) over a vertex byte list,
// then either skips forward by offset or stays to interpret the stream
// after the list (bright path -> DoSetGlobalAlpha through current_screen).
// That call walks MigWindow -> Master() -> MigDisplay::SetGlobalAlpha,
// which reads the member DD.lpDirect3D. `master` is protected in Graphic,
// so the dummy window's pointer slots are all filled with the display
// address: wherever master sits it resolves to the zeroed MigDisplay
// (lpDirect3D == NULL -> returns the alpha value unchanged).
// Edge guards pinned: nopoints=0 and threshold=256 take the offset path.
//------------------------------------------------------------------------------
static void test_doifbright()
{
    static UByte fake_screen[16384];
    static UByte fake_display[16384];
    std::memset(fake_display, 0, sizeof(fake_display));
    for (size_t i = 0; i < sizeof(fake_screen) / sizeof(void*); ++i)
        ((void**)fake_screen)[i] = fake_display;
    shape_current_screen = fake_screen;
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    // [DOIFBRIGHT][v0][v1] - vertex indices are the stream bytes after hdr
    UByte stream[16] = {0};
    DOIFBRIGHT* p = (DOIFBRIGHT*)stream;
    p->threshold = 100;
    p->offset = 40;
    stream[sizeof(DOIFBRIGHT)] = 4;                 // vertex index 4
    stream[sizeof(DOIFBRIGHT) + 1] = 5;             // vertex index 5

    // nopoints=0: avg=0, 0 > 100 false -> +offset (no /0, no alpha call)
    p->nopoints = 0;
    UByte* ip = stream;
    shape_doifbright(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFBRIGHT) + 40);

    // dim: intensity 250 each -> avg 6 <= 100 -> +offset; consumed verts
    // are reset to intensity/specular -1.
    p->nopoints = 2;
    shape_shpco[4].intensity = 250;
    shape_shpco[5].intensity = 250;
    ip = stream;
    shape_doifbright(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFBRIGHT) + 40);
    CHECK_EQ(shape_shpco[4].intensity, -1);
    CHECK_EQ(shape_shpco[4].specular, -1);

    // boundary: avg == threshold is NOT bright (strict >).
    shape_shpco[4].intensity = 156;                 // 256-156 = 100 avg
    shape_shpco[5].intensity = 156;
    ip = stream;
    shape_doifbright(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFBRIGHT) + 40);

    // bright: intensity 0 each -> avg 256 > 100 -> alpha set (255) and
    // instr_ptr stays at the byte after the vertex list.
    shape_shpco[4].intensity = 0;
    shape_shpco[5].intensity = 0;
    ip = stream;
    shape_doifbright(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFBRIGHT) + 2);

    // threshold=256: even max brightness takes offset (256 < 256 false).
    p->threshold = 256;
    shape_shpco[4].intensity = 0;
    shape_shpco[5].intensity = 0;
    ip = stream;
    shape_doifbright(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFBRIGHT) + 40);
}

//------------------------------------------------------------------------------
// shape::doifcross - backface/winding test through _matrix.crossproduct
// (projects bodyx/bodyz, bodyy/bodyz; temp <= 0 -> clockwise -> continue,
// else +offset). Pins both windings plus the temp=0 collinear boundary.
//------------------------------------------------------------------------------
static void test_doifcross()
{
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    shape_shpco[0].bodyx.f = 0.0;
    shape_shpco[0].bodyy.f = 0.0;
    shape_shpco[0].bodyz.f = 100.0;
    shape_shpco[1].bodyx.f = 100.0;                 // -> (1,0)
    shape_shpco[1].bodyy.f = 0.0;
    shape_shpco[1].bodyz.f = 100.0;
    shape_shpco[2].bodyx.f = 0.0;                   // -> (0,1)
    shape_shpco[2].bodyy.f = 100.0;
    shape_shpco[2].bodyz.f = 100.0;

    UByte stream[8] = {0};
    DOIFCROSS* p = (DOIFCROSS*)stream;
    p->vertex1 = 0;
    p->vertex2 = 1;
    p->vertex3 = 2;
    p->offset = 60;

    // (0,0),(1,0),(0,1): temp = 1 > 0 -> anti-clockwise -> +offset.
    UByte* ip = stream;
    shape_doifcross(ip);
    CHECK_EQ(ip - stream, 60);

    // Swap v2/v3: temp = -1 <= 0 -> clockwise -> continue past header.
    p->vertex2 = 2;
    p->vertex3 = 1;
    ip = stream;
    shape_doifcross(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFCROSS));

    // All three verts identical -> temp = 0 -> clockwise (degenerate).
    p->vertex2 = 0;
    p->vertex3 = 0;
    ip = stream;
    shape_doifcross(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFCROSS));
}

//------------------------------------------------------------------------------
// shape::doflipvector / doflipnvec - face-flip fixup: intensity -> 256-i
// (clamped at 0), specular <-> specFlip swap. doflipnvec loops nopoints
// verts starting at ptr->vertex.
//------------------------------------------------------------------------------
static void test_flip_writers()
{
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    UByte stream[8] = {0};
    DOFLIPVECTOR* p = (DOFLIPVECTOR*)stream;
    p->vertex = 2;

    shape_shpco[2].intensity = 100;
    shape_shpco[2].specular = 10;
    shape_shpco[2].specFlip = 20;
    UByte* ip = stream;
    shape_doflipvector(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOFLIPVECTOR));
    CHECK_EQ(shape_shpco[2].intensity, 156);        // 256 - 100
    CHECK_EQ(shape_shpco[2].specular, 20);          // swapped
    CHECK_EQ(shape_shpco[2].specFlip, 10);

    // Edge: intensity > 256 underflows negative -> clamps to 0.
    shape_shpco[2].intensity = 300;
    ip = stream;
    shape_doflipvector(ip);
    CHECK_EQ(shape_shpco[2].intensity, 0);

    // Edge: intensity = 0 -> 256 (full flip, no clamp).
    shape_shpco[2].intensity = 0;
    ip = stream;
    shape_doflipvector(ip);
    CHECK_EQ(shape_shpco[2].intensity, 256);

    // doflipnvec: nopoints=2 covers verts 3,4; vert 5 untouched.
    DOFLIPNVEC* np = (DOFLIPNVEC*)stream;
    np->vertex = 3;
    np->nopoints = 2;
    shape_shpco[3].intensity = 100;
    shape_shpco[3].specular = 1;
    shape_shpco[3].specFlip = 2;
    shape_shpco[4].intensity = 0;
    shape_shpco[5].intensity = 50;
    ip = stream;
    shape_doflipnvec(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOFLIPNVEC));
    CHECK_EQ(shape_shpco[3].intensity, 156);
    CHECK_EQ(shape_shpco[3].specular, 2);
    CHECK_EQ(shape_shpco[3].specFlip, 1);
    CHECK_EQ(shape_shpco[4].intensity, 256);
    CHECK_EQ(shape_shpco[5].intensity, 50);         // outside range
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Advance-only handlers: pin the exact instruction-pointer movement. A wrong
// skip length desyncs the interpreter for everything downstream.
//------------------------------------------------------------------------------
static void test_stream_advance()
{
    UByte stream[64] = {0};

    // DONOP is an empty struct: the opcode still carries a 1-byte operand.
    UByte* ip = stream;
    shape_donop(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONOP));

    ip = stream;
    shape_donormal(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONORMAL));

    ip = stream;
    shape_dosetlc(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETLC));

    ip = stream;
    shape_do4cmpnt(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DO4CMPNT));

    ip = stream;
    shape_do4cmpt2x(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DO4CMPT2X));

    ip = stream;
    shape_dooffsetpnt(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOOFFSETPNT));

    // dosetmapoff is a dead no-op: it does NOT advance at all.
    ip = stream;
    shape_dosetmapoff(ip);
    CHECK_EQ(ip - stream, 0);

    // doifhard3d's doingHW3D test is commented out - it always jumps +offset.
    DOIFHARD3D* hd = (DOIFHARD3D*)stream;
    hd->offset = 20;
    ip = stream;
    shape_doifhard3d(ip);
    CHECK_EQ(ip - stream, 20);

    // don4cmpnts skips its own header plus vertex_count COORDS records.
    DON4CMPNTS* n4 = (DON4CMPNTS*)stream;
    n4->vertex_count = 3;
    ip = stream;
    shape_don4cmpnts(ip);
    CHECK_EQ(ip - stream, (long)(sizeof(DON4CMPNTS) + 3 * sizeof(COORDS)));

    n4->vertex_count = 0;
    ip = stream;
    shape_don4cmpnts(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DON4CMPNTS));
}

//------------------------------------------------------------------------------
// Animation-data handlers: GlobalAdptr is UByte-wide, so scaled results
// truncate to a byte (the real 1998 behavior).
//------------------------------------------------------------------------------
static void test_anim_data_ops()
{
    static UByte anim[64];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;

    // doresetanim: 16-bit controlframe==0 writes resetval into animoffframe.
    UByte stream[16] = {0};
    DORESETANIM* ra = (DORESETANIM*)stream;
    ra->animofftimer = 4;
    ra->animoffframe = 8;
    ra->resetval = 0xAB;

    anim[4] = anim[5] = 0;                       // controlframe == 0
    anim[8] = 0x11;
    UByte* ip = stream;
    shape_doresetanim(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DORESETANIM));
    CHECK_EQ(anim[8], 0xAB);

    anim[5] = 1;                                 // controlframe == 0x0100
    anim[8] = 0x11;
    ip = stream;
    shape_doresetanim(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DORESETANIM));
    CHECK_EQ(anim[8], 0x11);                     // untouched

    // doscalesize: dest = src*255/animscale truncated to a byte.
    DOSCALESIZE* ss = (DOSCALESIZE*)stream;
    ss->animoffsrc = 2;
    ss->animoffdest = 3;
    ss->animscale = 5;
    anim[2] = 10;
    anim[3] = 0;
    ip = stream;
    shape_doscalesize(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSCALESIZE));
    CHECK_EQ(anim[3], (UByte)((10 * 255 / 5) & 0xFF));   // 510 -> 254

    ss->animscale = 0;                           // guarded: raw *255 kept
    anim[3] = 0;
    ip = stream;
    shape_doscalesize(ip);
    CHECK_EQ(anim[3], (UByte)((10 * 255) & 0xFF));       // 2550 -> 246

    // dobitsoff: damage bands + the transient deadoff=-1 quirk. Instruction
    // always consumes sizeof(DOBITSOFF), then adds a band offset.
    TestObj3D obj = TestObj3D();
    shape_object_obj3d = &obj;
    UByte stream2[32] = {0};
    DOBITSOFF* db = (DOBITSOFF*)stream2;
    db->animoff = 6;
    db->dam1offset = 100;
    db->dam2offset = 200;
    db->deadoffset = 50;

    anim[6] = 40;                                // below BS_DAMLV1 (85)
    ip = stream2;
    shape_dobitsoff(ip);
    CHECK_EQ(ip - stream2, (long)sizeof(DOBITSOFF));

    anim[6] = 90;                                // 85..169 -> dam1
    ip = stream2;
    shape_dobitsoff(ip);
    CHECK_EQ(ip - stream2, (long)sizeof(DOBITSOFF) + 100);

    anim[6] = 200;                               // 170..254 -> dam2
    ip = stream2;
    shape_dobitsoff(ip);
    CHECK_EQ(ip - stream2, (long)sizeof(DOBITSOFF) + 200);

    anim[6] = 255;                               // BS_DEAD -> deadoffset
    ip = stream2;
    shape_dobitsoff(ip);
    CHECK_EQ(ip - stream2, (long)sizeof(DOBITSOFF) + 50);

    // transient + deadoffset==0 rewinds 1 byte so the next opcode is re-read.
    obj.IsTransient = 1;
    db->deadoffset = 0;
    ip = stream2;
    shape_dobitsoff(ip);
    CHECK_EQ(ip - stream2, (long)sizeof(DOBITSOFF) - 1);
    obj.IsTransient = 0;

    // doniverts writes ix/iy from count NEXTMAP records and advances vertex.
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    UByte stream3[32] = {0};
    DONIVERTS* iv = (DONIVERTS*)stream3;
    iv->vertex = 2;
    iv->count = 2;
    NEXTMAP* nm = (NEXTMAP*)(stream3 + sizeof(DONIVERTS));
    nm[0].ix = 640;  nm[0].iy = 480;
    nm[1].ix = 11;   nm[1].iy = 22;

    ip = stream3;
    shape_doniverts(ip);
    CHECK_EQ(ip - stream3,
             (long)(sizeof(DONIVERTS) + 2 * sizeof(NEXTMAP)));
    CHECK_EQ(shape_shpco[2].ix, 640);
    CHECK_EQ(shape_shpco[2].iy, 480);
    CHECK_EQ(shape_shpco[3].ix, 11);
    CHECK_EQ(shape_shpco[3].iy, 22);

    iv->count = 0;                               // no records consumed
    ip = stream3;
    shape_doniverts(ip);
    CHECK_EQ(ip - stream3, (long)sizeof(DONIVERTS));
}

//------------------------------------------------------------------------------
// Display-state setters that survive on a zeroed dummy MigWindow+MigDisplay
// (SetLuminosity/SetTransparency are no-op stubs; DoSetGlobalAlpha returns
// early when DD.lpDirect3D == NULL).
//------------------------------------------------------------------------------
static void test_state_setters()
{
    static UByte fake_screen[16384];
    static UByte fake_display[16384];
    static UByte fake_viewpoint[4096];
    std::memset(fake_display, 0, sizeof(fake_display));
    std::memset(fake_viewpoint, 0, sizeof(fake_viewpoint));
    for (size_t i = 0; i < sizeof(fake_screen) / sizeof(void*); ++i)
        ((void**)fake_screen)[i] = fake_display;
    shape_current_screen = fake_screen;

    UByte stream[16] = {0};

    // dosetluminosity: animoff==0 uses brightness, clamped to LUM_MAX (8).
    DOSETLUMINOSITY* sl = (DOSETLUMINOSITY*)stream;
    sl->animoff = 0;
    sl->brightness = 3;
    UByte* ip = stream;
    shape_dosetluminosity(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETLUMINOSITY));

    sl->brightness = 200;                        // > LUM_MAX clamps to 8
    ip = stream;
    shape_dosetluminosity(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETLUMINOSITY));

    // anim path: depth = GlobalAdptr[animoff] / animscale (guarded).
    static UByte anim[8];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;
    anim[5] = 64;
    sl->animoff = 5;
    sl->animscale = 4;                           // 64/4 = 16 -> clamp 8
    ip = stream;
    shape_dosetluminosity(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETLUMINOSITY));

    sl->animscale = 0;                           // zero scale: raw 64 kept
    ip = stream;
    shape_dosetluminosity(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETLUMINOSITY));

    // dolshadeon: detail bit clear -> else path, isLightShaded=FALSE. The
    // struct carries only a surfacetype byte; ip advances sizeof(DOLSHADEON).
    shape_doingHW3D = false;
    shape_View_Point = fake_viewpoint;
    ip = stream;
    shape_dolshadeon(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOLSHADEON));

    // Opcode-only state setters: no operand, instr_ptr does not move.
    ip = stream;
    shape_dosmokedon(ip);
    CHECK_EQ(ip - stream, 0);
    ip = stream;
    shape_dosmokedoff(ip);
    CHECK_EQ(ip - stream, 0);

    // dotransparentoff restores oldAlphaSwitch; zeroed SHAPE -> 0 != -1, so
    // the DoSetGlobalAlpha path fires through the dummy Master() chain.
    ip = stream;
    shape_dotransparentoff(ip);
    CHECK_EQ(ip - stream, 0);
}

//------------------------------------------------------------------------------
// Stretch writers: the 1998 code applies killfrac to Y but killdiff to X/Z -
// an asymmetric quirk pinned here deliberately.
//------------------------------------------------------------------------------
static void test_stretch_writers()
{
    static UByte anim[16];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    TestObj3D obj = TestObj3D();
    shape_object_obj3d = &obj;
    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;
    shape_fpobject_matrix = &ident;
    _matrix.fpMaximumZ = 1600000.0;

    // dostretchpoint calls body2screen: mat_win must survive
    // DoingHardware3D() so the POLYGON.viewdata path is taken.
    static UByte fake_win[16384];
    static UByte fake_disp[16384];
    std::memset(fake_disp, 0, sizeof(fake_disp));
    for (size_t i = 0; i < sizeof(fake_win) / sizeof(void*); ++i)
        ((void**)fake_win)[i] = fake_disp;
    _matrix.SetWin((MigWindow*)fake_win);

    UByte stream[16] = {0};
    DOSTRETCHPOINT* sp = (DOSTRETCHPOINT*)stream;
    sp->killflag = 1;
    sp->minpoint = 0;
    sp->maxpoint = 1;
    sp->outpoint = 2;

    // nokills=0: out is a straight copy of min.
    anim[1] = 0;
    shape_shpco[0].bodyx.f = 7.0;  shape_shpco[0].bodyy.f = 8.0;
    shape_shpco[0].bodyz.f = 9.0;
    UByte* ip = stream;
    shape_dostretchpoint(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSTRETCHPOINT));
    CHECK_FEQ(shape_shpco[2].bodyx.f, 7.0);
    CHECK_FEQ(shape_shpco[2].bodyz.f, 9.0);

    // nokills=32 -> killfrac=2, killdiff=0: x/z get (max-min)/16*1, y gets
    // (max-min)/16*3. The Y-vs-X/Z asymmetry is the original formula.
    anim[1] = 32;
    shape_shpco[0].bodyx.f = 0.0;  shape_shpco[0].bodyy.f = 0.0;
    shape_shpco[0].bodyz.f = 0.0;
    shape_shpco[1].bodyx.f = 160.0; shape_shpco[1].bodyy.f = 320.0;
    shape_shpco[1].bodyz.f = 480.0;
    ip = stream;
    shape_dostretchpoint(ip);
    CHECK_FEQ(shape_shpco[2].bodyx.f, 10.0);     // 160/16 * (0+1)
    CHECK_FEQ(shape_shpco[2].bodyy.f, 60.0);     // 320/16 * (2+1)
    CHECK_FEQ(shape_shpco[2].bodyz.f, 30.0);     // 480/16 * (0+1)

    // dostretchmap: same formula on ix/iy (SWord fields, float diff added).
    DOSTRETCHMAP* sm = (DOSTRETCHMAP*)stream;
    sm->killflag = 1;
    sm->minpoint = 0;
    sm->maxpoint = 1;
    sm->outpoint = 2;
    anim[1] = 32;
    shape_shpco[0].ix = 5;   shape_shpco[0].iy = 7;
    shape_shpco[1].ix = 165; shape_shpco[1].iy = 327;
    ip = stream;
    shape_dostretchmap(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSTRETCHMAP));
    CHECK_EQ(shape_shpco[2].ix, 15);             // 5 + 160/16*(0+1)
    CHECK_EQ(shape_shpco[2].iy, 67);             // 7 + 320/16*(2+1)

    anim[1] = 0;                                 // nokills=0: pure copy
    shape_shpco[0].ix = 40;  shape_shpco[0].iy = 41;
    ip = stream;
    shape_dostretchmap(ip);
    CHECK_EQ(shape_shpco[2].ix, 40);
    CHECK_EQ(shape_shpco[2].iy, 41);
}

//------------------------------------------------------------------------------
// Mirrored and delta vertex writers.
//------------------------------------------------------------------------------
static void test_delta_mirror_writers()
{
    shape_newco = shape_shpco;
    for (int i = 0; i < 32; ++i) shape_shpco[i] = DoPointStruc();
    TestObj3D obj = TestObj3D();
    shape_object_obj3d = &obj;
    static FPMATRIX ident;
    zero_fpmatrix(ident);
    ident.L11 = ident.L22 = ident.L33 = 1.0;
    shape_fpobject_matrix = &ident;
    _matrix.fpMaximumZ = 1600000.0;

    // dondeltapoints calls body2screen: see test_stretch_writers.
    static UByte fake_win[16384];
    static UByte fake_disp[16384];
    std::memset(fake_disp, 0, sizeof(fake_disp));
    for (size_t i = 0; i < sizeof(fake_win) / sizeof(void*); ++i)
        ((void**)fake_win)[i] = fake_disp;
    _matrix.SetWin((MigWindow*)fake_win);

    // donpoint2x: each NNEXT2X writes vertex1 and a mirrored partner at
    // vertex1+vertex with negated x; both end intensity/specular -1.
    UByte stream[64] = {0};
    DONPOINT2X* n2 = (DONPOINT2X*)stream;
    n2->start_vertex = 5;
    n2->count = 2;
    NNEXT2X* recs = (NNEXT2X*)(stream + sizeof(DONPOINT2X));
    recs[0].vertex = 10;  recs[0].xcoord = 30; recs[0].ycoord = 40;
    recs[0].zcoord = 50;
    recs[1].vertex = 10;  recs[1].xcoord = 60; recs[1].ycoord = 70;
    recs[1].zcoord = 80;

    UByte* ip = stream;
    shape_donpoint2x(ip);
    CHECK_EQ(ip - stream,
             (long)(sizeof(DONPOINT2X) + 2 * sizeof(NNEXT2X)));
    CHECK_FEQ(shape_shpco[5].bodyx.f, 30.0);
    CHECK_FEQ(shape_shpco[15].bodyx.f, -30.0);   // mirrored partner
    CHECK_FEQ(shape_shpco[15].bodyy.f, 40.0);
    CHECK_FEQ(shape_shpco[6].bodyx.f, 60.0);
    CHECK_FEQ(shape_shpco[16].bodyx.f, -60.0);
    CHECK_EQ(shape_shpco[5].intensity, -1);
    CHECK_EQ(shape_shpco[15].specular, -1);

    // dondeltapoints: bases live at instr_ptr+offset, deltas follow the
    // header; bodyx/z add deltas, bodyy SUBTRACTS (>>8 fixed point).
    UByte stream2[64] = {0};
    DONDELTAPOINTS* dp = (DONDELTAPOINTS*)stream2;
    dp->count = 2;
    dp->vertex = 3;
    dp->mask = DMASK_X_Y_Z;
    dp->scale = 256;                             // delta*256>>8 == delta
    dp->offset = sizeof(DONDELTAPOINTS) + 2 * sizeof(NDNEXT3);

    NDNEXT3* deltas = (NDNEXT3*)(stream2 + sizeof(DONDELTAPOINTS));
    deltas[0].delta[0] = 10;  deltas[0].delta[1] = -20; deltas[0].delta[2] = 30;
    deltas[1].delta[0] = 1;   deltas[1].delta[1] = 2;   deltas[1].delta[2] = 3;

    NNEXT* bases = (NNEXT*)(stream2 + dp->offset);
    bases[0].xcoord = 100;  bases[0].ycoord = 200;  bases[0].zcoord = 300;
    bases[1].xcoord = 400;  bases[1].ycoord = 500;  bases[1].zcoord = 600;

    ip = stream2;
    shape_dondeltapoints(ip);
    CHECK_EQ(ip - stream2,
             (long)(sizeof(DONDELTAPOINTS) + 2 * sizeof(NDNEXT3)));
    CHECK_FEQ(shape_shpco[3].bodyx.f, 110.0);    // 100 + 10
    CHECK_FEQ(shape_shpco[3].bodyy.f, 220.0);    // 200 - (-20)
    CHECK_FEQ(shape_shpco[3].bodyz.f, 330.0);    // 300 + 30
    CHECK_FEQ(shape_shpco[4].bodyx.f, 401.0);
    CHECK_FEQ(shape_shpco[4].bodyy.f, 498.0);    // 500 - 2
    CHECK_FEQ(shape_shpco[4].bodyz.f, 603.0);
    CHECK_EQ(shape_shpco[3].intensity, -1);
    CHECK_EQ(shape_shpco[4].specFlip, -1);
}

//------------------------------------------------------------------------------
// MODVEC primitives: dest-FIRST argument order, Rowan rotation conventions,
// and zero-vector edge behavior.
//------------------------------------------------------------------------------
static void test_modvec_full()
{
    FCRD a, b, c;
    a.x = 1; a.y = 2; a.z = 3;
    b.x = 4; b.y = 5; b.z = 6;

    mv_NullVec(c);
    CHECK_FEQ(c.x, 0.0); CHECK_FEQ(c.y, 0.0); CHECK_FEQ(c.z, 0.0);

    mv_CopyVec(a, c);
    CHECK_FEQ(c.x, 1.0); CHECK_FEQ(c.y, 2.0); CHECK_FEQ(c.z, 3.0);

    mv_AddVec(c, a, b);                          // c = a + b
    CHECK_FEQ(c.x, 5.0); CHECK_FEQ(c.y, 7.0); CHECK_FEQ(c.z, 9.0);

    mv_SubVec(c, b, a);                          // c = b - a
    CHECK_FEQ(c.x, 3.0); CHECK_FEQ(c.y, 3.0); CHECK_FEQ(c.z, 3.0);

    a.x = 3; a.y = 4; a.z = 12;
    CHECK_FEQ(mv_VecLen(a), 13.0);               // 3-4-12 = 13

    FP x = 3, y = 4;
    CHECK(mv_NrmVec2D(x, y) == TRUE);
    CHECK_FEQ(x, 0.6); CHECK_FEQ(y, 0.8);
    x = 0; y = 0;
    CHECK(mv_NrmVec2D(x, y) == FALSE);           // leaves args unchanged
    CHECK_FEQ(x, 0.0); CHECK_FEQ(y, 0.0);

    // RotateVec2D 90 deg: (x,y) -> (-y,x).
    x = 1; y = 0;
    mv_RotateVec2D(x, y, F1PIE2);
    CHECK_FEQ(x, 0.0); CHECK_FEQ(y, 1.0);

    // RotVec*SC with sin=1,cos=0 = 90 deg about each axis.
    a.x = 1; a.y = 2; a.z = 3;
    mv_RotVecXSC(a, c, 1.0, 0.0);                // y'=-z, z'=y
    CHECK_FEQ(c.x, 1.0); CHECK_FEQ(c.y, -3.0); CHECK_FEQ(c.z, 2.0);
    mv_RotVecYSC(a, c, 1.0, 0.0);                // x'=z, z'=-x
    CHECK_FEQ(c.x, 3.0); CHECK_FEQ(c.y, 2.0); CHECK_FEQ(c.z, -1.0);
    mv_RotVecZSC(a, c, 1.0, 0.0);                // x'=-y, y'=x
    CHECK_FEQ(c.x, -2.0); CHECK_FEQ(c.y, 1.0); CHECK_FEQ(c.z, 3.0);

    // SetOri(0,0,0) yields the identity orientation; TnsPnt through it
    // copies the vector.
    FORI ori;
    mv_SetOri(ori, 0.0, 0.0, 0.0);
    CHECK_FEQ(ori.x.x, 1.0); CHECK_FEQ(ori.y.y, 1.0);
    CHECK_FEQ(ori.z.z, 1.0); CHECK_FEQ(ori.x.y, 0.0);
    mv_TnsPnt(a, c, ori);
    CHECK_FEQ(c.x, 1.0); CHECK_FEQ(c.y, 2.0); CHECK_FEQ(c.z, 3.0);

    // CalcAngle = atan2 wrapped to [0, 2pi).
    CHECK_FEQ(mv_CalcAngle(1, 0), 0.0);
    CHECK_FEQ(mv_CalcAngle(0, 1), (FP)1.5707963);
    CHECK_FEQ(mv_CalcAngle(-1, 0), (FP)3.1415927);
    CHECK_FEQ(mv_CalcAngle(0, -1), (FP)4.7123890);
    CHECK_FEQ(mv_CalcAngle(1, 1), (FP)0.7853982);
}

//------------------------------------------------------------------------------
// MODVEC residual coverage: orientation helpers (NullOri/CopyOri, CPrdX/Y/Z
// basis completion, RotOri*Vec in-place axis rotation), TnsAxs (the
// transpose of TnsPnt), the FCRDlong overloads, the rotational-rate
// conversions, and CalcAngle's eight interior octants.
//------------------------------------------------------------------------------
static void test_modvec2()
{
    // NullOri/CopyOri: all nine fields.
    FORI ori;
    NullOri(ori);
    CHECK_FEQ(ori.x.x, 0.0); CHECK_FEQ(ori.y.y, 0.0); CHECK_FEQ(ori.z.z, 0.0);
    FORI ident;
    mv_SetOri(ident, 0.0, 0.0, 0.0);
    CopyOri(ident, ori);
    CHECK_FEQ(ori.x.x, 1.0); CHECK_FEQ(ori.y.y, 1.0); CHECK_FEQ(ori.z.z, 1.0);
    CHECK_FEQ(ori.x.y, 0.0); CHECK_FEQ(ori.y.z, 0.0); CHECK_FEQ(ori.z.x, 0.0);

    // CPrdX/Y/Z complete the third basis vector from the other two:
    // x = y x z, y = z x x, z = x x y. On identity each must rebuild the
    // missing axis.
    NullOri(ori); ori.y.y = 1; ori.z.z = 1;
    CPrdX(ori);
    CHECK_FEQ(ori.x.x, 1.0); CHECK_FEQ(ori.x.y, 0.0); CHECK_FEQ(ori.x.z, 0.0);
    NullOri(ori); ori.x.x = 1; ori.z.z = 1;
    CPrdY(ori);
    CHECK_FEQ(ori.y.x, 0.0); CHECK_FEQ(ori.y.y, 1.0); CHECK_FEQ(ori.y.z, 0.0);
    NullOri(ori); ori.x.x = 1; ori.y.y = 1;
    CPrdZ(ori);
    CHECK_FEQ(ori.z.x, 0.0); CHECK_FEQ(ori.z.y, 0.0); CHECK_FEQ(ori.z.z, 1.0);

    // RotOri*Vec rotate the two non-named columns: X->(z'=y, y'=-z),
    // Y->(x'=z, z'=-x), Z->(y'=x, x'=-y) at +90deg on identity. The named
    // column is untouched. ang==0 early-returns without change.
    CopyOri(ident, ori);
    RotOriXVec(ori, 0.0f);
    CHECK_FEQ(ori.y.y, 1.0); CHECK_FEQ(ori.z.z, 1.0);   // unchanged
    RotOriXVec(ori, (FP)F1PIE2);
    CHECK_NEAR(ori.z.y, 1.0, 1e-4); CHECK_NEAR(ori.y.z, -1.0, 1e-4);
    CHECK_NEAR(ori.x.x, 1.0, 1e-4);
    CopyOri(ident, ori);
    RotOriYVec(ori, (FP)F1PIE2);
    CHECK_NEAR(ori.x.z, 1.0, 1e-4); CHECK_NEAR(ori.z.x, -1.0, 1e-4);
    CHECK_NEAR(ori.y.y, 1.0, 1e-4);
    CopyOri(ident, ori);
    RotOriZVec(ori, (FP)F1PIE2);
    CHECK_NEAR(ori.y.x, 1.0, 1e-4); CHECK_NEAR(ori.x.y, -1.0, 1e-4);
    CHECK_NEAR(ori.z.z, 1.0, 1e-4);

    // SetOri heading-only quarter turn: forward axis x -> +z.
    FORI hdg;
    mv_SetOri(hdg, 0.0, (FP)F1PIE2, 0.0);
    CHECK_NEAR(hdg.x.x, 0.0, 1e-4); CHECK_NEAR(hdg.x.z, 1.0, 1e-4);
    CHECK_NEAR(hdg.y.y, 1.0, 1e-4); CHECK_NEAR(hdg.z.x, -1.0, 1e-4);

    // TnsAxs is the transpose of TnsPnt; for an orthonormal orientation
    // they are inverses, so TnsPnt . TnsAxs is identity on any vector.
    FORI o2;
    mv_SetOri(o2, 0.3f, 0.7f, -0.2f);
    FCRD v = {5.0f, -3.0f, 2.0f}, t1, t2;
    TnsAxs(v, t1, o2);
    mv_TnsPnt(t1, t2, o2);
    CHECK_NEAR(t2.x, 5.0, 1e-4); CHECK_NEAR(t2.y, -3.0, 1e-4);
    CHECK_NEAR(t2.z, 2.0, 1e-4);
    // And through the identity orientation TnsAxs passes the vector.
    TnsAxs(v, t1, ident);
    CHECK_NEAR(t1.x, 5.0, 1e-6); CHECK_NEAR(t1.y, -3.0, 1e-6);
    CHECK_NEAR(t1.z, 2.0, 1e-6);

    // FCRDlong (double) overloads.
    FCRDlong la = {1.0, 2.0, 3.0}, lb = {4.0, 5.0, 6.0}, lc = {9.9, 9.9, 9.9};
    NullVec(lc);
    CHECK(lc.x == 0.0 && lc.y == 0.0 && lc.z == 0.0);
    AddVec(lc, la, lb);
    CHECK(lc.x == 5.0 && lc.y == 7.0 && lc.z == 9.0);
    SubVec(lc, lb, la);
    CHECK(lc.x == 3.0 && lc.y == 3.0 && lc.z == 3.0);

    // Rotational-rate conversions. 6000rpm = 2pi rad/s (the scale factor
    // is rpm * 2pi / 6000). RadPerCSec2RowanPerMin(1.0) = 955*65536 ->
    // lands exactly on the 16-bit wrap boundary and yields 0.
    CHECK_NEAR(Rpm2RadsPerCSec(6000.0f), 6.283185307, 1e-4);
    CHECK_NEAR(Rpm2RadsPerCSec(0.0f), 0.0, 1e-9);
    CHECK_EQ(RadPerCSec2RowanPerMin(1.0f), 0);
    CHECK_EQ(RadPerCSec2DegsPerMin(1.0f), 16120);   // 343800 & 0xFFFF
    CHECK_EQ(RadPerCSec2DegsPerMin(0.0f), 0);

    // CalcAngle interior octants: atan(0.5) = 0.4636476 swept through all
    // eight branches plus the (0,0) edge which collapses into octant 8.
    const FP a5 = 0.4636476f;
    CHECK_NEAR(mv_CalcAngle(1.0f, 0.5f), a5, 1e-5);             // oct 8
    CHECK_NEAR(mv_CalcAngle(0.5f, 1.0f), F1PIE2 - a5, 1e-5);    // oct 7
    CHECK_NEAR(mv_CalcAngle(-0.5f, 1.0f), F1PIE2 + a5, 1e-5);   // oct 6
    CHECK_NEAR(mv_CalcAngle(-1.0f, 0.5f), FPIE - a5, 1e-5);     // oct 5
    CHECK_NEAR(mv_CalcAngle(-1.0f, -0.5f), FPIE + a5, 1e-5);    // oct 4
    CHECK_NEAR(mv_CalcAngle(-0.5f, -1.0f), F3PIE2 - a5, 1e-5);  // oct 3
    CHECK_NEAR(mv_CalcAngle(0.5f, -1.0f), F3PIE2 + a5, 1e-5);   // oct 2
    CHECK_NEAR(mv_CalcAngle(1.0f, -0.5f), F2PIE - a5, 1e-5);    // oct 1
    CHECK_NEAR(mv_CalcAngle(0.0f, 0.0f), 0.0, 1e-9);            // (0,0) edge
}

//------------------------------------------------------------------------------
// CURVES.CPP piecewise-linear curve lookups. GetValue interpolates with
// IDT_LIMIT clamp or IDT_WRAP modular indices; GetIndex inverts (with a
// >PI index normalisation quirk); GetClIndex walks back past the stall
// peak then scans rising segments; GetMaxValue picks the last maximum.
//------------------------------------------------------------------------------
static void test_curves()
{
    // --- GetValue / GetIndex on an IDT_LIMIT ramp ---
    CURVEPNT ramp[3] = {{0, 0}, {5, 50}, {10, 100}};
    {
        Curve lim("TESTAC", "LIMC", 3, 0.0f, 10.0f, IDT_LIMIT, ramp);
        CHECK_FEQ(lim.GetValue(0), 0.0f);
        CHECK_FEQ(lim.GetValue(5), 50.0f);
        CHECK_FEQ(lim.GetValue(10), 100.0f);
        CHECK_FEQ(lim.GetValue(2.5f), 25.0f);
        CHECK_FEQ(lim.GetValue(7.5f), 75.0f);
        CHECK_FEQ(lim.GetValue(-3.0f), 0.0f);        // clamps to MinIndex
        CHECK_FEQ(lim.GetValue(13.0f), 100.0f);      // clamps to MaxIndex

        FP r = -99;
        // Quirk pinned: EVERY bracket index > PI is normalised by -2PI
        // before interpolating (written for angle-domain curves, but it
        // fires on these degree-range indices too: 5 -> -1.2832,
        // 10 -> +3.7168). Results land in -PI..PI units, not the
        // original index domain.
        CHECK(lim.GetIndex(25.0f, r) != BOOL_FALSE);
        CHECK_NEAR(r, -0.641593, 1e-4);              // 2.5-domain shifted
        CHECK(lim.GetIndex(50.0f, r) != BOOL_FALSE);
        CHECK_NEAR(r, -1.28319, 1e-4);               // 5 - 2PI
        CHECK(lim.GetIndex(0.0f, r) != BOOL_FALSE);  // exact min value fits
        CHECK_NEAR(r, 0.0, 1e-6);
        // Pinned: value exactly AT the max never fits (p2->value > value
        // is strict) -> FALSE; below min or above max also FALSE.
        CHECK(lim.GetIndex(100.0f, r) == BOOL_FALSE);
        CHECK(lim.GetIndex(-1.0f, r) == BOOL_FALSE);
        CHECK(lim.GetIndex(150.0f, r) == BOOL_FALSE);

        // FindCurve hits by exact name, and by prefix (strncmp is bounded
        // by the QUERY length - "LI" matches "LIMC").
        CHECK(_CurveRsc.FindCurve("TESTAC", "LIMC") == &lim);
        CHECK(_CurveRsc.FindCurve("TESTAC", "LI") == &lim);

        // GetMaxValue returns the LAST knot on ties.
        FP mv = -1, mi = -1;
        lim.GetMaxValue(mv, mi);
        CHECK_FEQ(mv, 100.0f); CHECK_FEQ(mi, 10.0f);

        // Inactive curve: GetValue 0, the index lookups FALSE.
        lim.Active = FALSE;
        CHECK_FEQ(lim.GetValue(5.0f), 0.0f);
        CHECK(lim.GetIndex(50.0f, r) == BOOL_FALSE);
        CHECK(lim.GetClIndex(50.0f, r) == BOOL_FALSE);
        lim.Active = TRUE;
    }
    // lim detached at scope end - a later FindCurve must not see it.

    // --- GetValue sparse-knot bracket boundaries (previously read
    // uninitialised c1/c2/i1/i2; now clamped to the boundary segment) ---
    {
        // Knots end before HalfIndex: query below Half but above the last
        // knot extrapolates the final segment.
        CURVEPNT lo[3] = {{0, 0}, {2, 2}, {4, 4}};
        Curve loEnd("T2", "LO", 3, 0.0f, 10.0f, IDT_LIMIT, lo);
        CHECK_NEAR(loEnd.GetValue(4.5f), 4.5, 1e-5);
        // Knots start after HalfIndex: query at/above Half but below the
        // first knot extrapolates the first segment.
        CURVEPNT hi[3] = {{6, 60}, {8, 80}, {10, 100}};
        Curve hiStart("T2", "HI", 3, 0.0f, 10.0f, IDT_LIMIT, hi);
        CHECK_NEAR(hiStart.GetValue(5.0f), 50.0, 1e-5);
    }

    // --- GetValue IDT_WRAP: modular index + wrap segment ---
    {
        // Knots {0->0, 180->1, 340->0.5} over [0,360): the wrap segment
        // interpolates 340 -> (360+)0 between the last and first knots.
        CURVEPNT w[3] = {{0, 0}, {180, 1}, {340, 0.5f}};
        Curve wrp("T3", "W", 3, 0.0f, 360.0f, IDT_WRAP, w);
        CHECK_FEQ(wrp.GetValue(0.0f), 0.0f);
        CHECK_NEAR(wrp.GetValue(90.0f), 0.5, 1e-5);
        CHECK_NEAR(wrp.GetValue(180.0f), 1.0, 1e-5);
        CHECK_NEAR(wrp.GetValue(260.0f), 0.75, 1e-5);
        // Wrap segment 340..360(+0): midpoint of the 20-index span.
        CHECK_NEAR(wrp.GetValue(350.0f), 0.25, 1e-5);
        // Index exactly ON the last knot returns that knot's value -
        // the >= fix (a raw > extrapolated ~18x the segment).
        CHECK_NEAR(wrp.GetValue(340.0f), 0.5, 1e-5);
        // Out-of-range indices wrap into the domain.
        CHECK_NEAR(wrp.GetValue(-10.0f), 0.25, 1e-5);   // = 350
        CHECK_NEAR(wrp.GetValue(370.0f), 10.0f / 180.0f, 1e-5);
        // MaxIndex itself lands back on knot 0.
        CHECK_NEAR(wrp.GetValue(360.0f), wrp.GetValue(0.0f), 1e-6);
    }

    // --- GetIndex radian normalisation: indices > PI are shifted by -2PI
    // before interpolating (angle-domain curves store 0..2PI) ---
    {
        CURVEPNT rad[2] = {{4, 0}, {5, 10}};
        Curve rc("T4", "RAD", 2, 4.0f, 5.0f, IDT_LIMIT, rad);
        FP r = 0;
        CHECK(rc.GetIndex(5.0f, r) != BOOL_FALSE);
        CHECK_NEAR(r, 4.0 - 6.283185307 + 0.5, 1e-4);   // -1.7832
    }

    // --- GetClIndex: scans rising segments only, stops at the stall ---
    {
        CURVEPNT cl[4] = {{0, 0}, {5, 10}, {10, 15}, {15, 8}};
        Curve clc("T5", "CL", 4, 0.0f, 15.0f, IDT_LIMIT, cl);
        FP r = -99;
        // Same >PI index normalisation as GetIndex: results are in the
        // shifted domain (5 -> -1.2832, 10 -> +3.7168).
        CHECK(clc.GetClIndex(5.0f, r) != BOOL_FALSE);    // seg (0,5)
        CHECK_NEAR(r, -0.641593, 1e-4);
        CHECK(clc.GetClIndex(12.0f, r) != BOOL_FALSE);   // seg (5,10)
        CHECK_NEAR(r, 0.716815, 1e-4);
        // Value in the stalled tail / above the peak -> FALSE.
        CHECK(clc.GetClIndex(20.0f, r) == BOOL_FALSE);
        // Exactly the peak value is not "<" any bracket -> FALSE.
        CHECK(clc.GetClIndex(15.0f, r) == BOOL_FALSE);
        // Flat curve: backward walk hits the base, forward scan cycles
        // without a rising fit -> bounded FALSE (used to read OOB/loop).
        CURVEPNT flat[3] = {{0, 5}, {5, 5}, {10, 5}};
        Curve flc("T6", "FL", 3, 0.0f, 10.0f, IDT_LIMIT, flat);
        CHECK(flc.GetClIndex(5.0f, r) == BOOL_FALSE);
    }

    // --- GetMaxValue: >= comparison keeps the LAST maximum ---
    {
        CURVEPNT mx[3] = {{0, 10}, {5, 30}, {10, 30}};
        Curve mxc("T7", "MX", 3, 0.0f, 10.0f, IDT_LIMIT, mx);
        FP mv = 0, mi = 0;
        mxc.GetMaxValue(mv, mi);
        CHECK_FEQ(mv, 30.0f); CHECK_FEQ(mi, 10.0f);
    }
}

//------------------------------------------------------------------------------
// Time/phase handlers. ViewPoint::TimeOfDay() is a 3-hop chain in the port:
//   this+0x3c -> view3dwin, view3dwin+0x50 -> inst, inst+0x1d -> timeofday
// (inst sits at an unaligned offset, so a uniform pointer-fill can't fake
// it - the chain is built at the exact offsets). Setting timeofday makes
// the returned time controllable instead of just zero.
//------------------------------------------------------------------------------
static UByte fake_viewpoint[4096];
static UByte fake_instmid[4096];
static UByte fake_inst[4096];
static void init_fake_viewpoint(int tod)
{
    std::memset(fake_viewpoint, 0, sizeof(fake_viewpoint));
    std::memset(fake_instmid, 0, sizeof(fake_instmid));
    std::memset(fake_inst, 0, sizeof(fake_inst));
    *(void**)(fake_viewpoint + 0x3c) = fake_instmid;
    *(void**)(fake_instmid + 0x50) = fake_inst;
    *(int*)(fake_inst + 0x1d) = tod;
    shape_View_Point = fake_viewpoint;
}

static void test_dotimerphase()
{
    init_fake_viewpoint(0);                      // TimeOfDay() == 0

    static UByte anim[16];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;

    // [DOTIMERPHASE][TPHASESTEP x2]: btime=0 -> tdelta=0.
    UByte stream[32] = {0};
    DOTIMERPHASE* tp = (DOTIMERPHASE*)stream;
    tp->birthtimeoffset = 0;
    tp->deltatimeoffset = 4;
    tp->nophases = 2;
    TPHASESTEP* ph = (TPHASESTEP*)(stream + sizeof(DOTIMERPHASE));
    ph[0].timedelta = 100;   ph[0].objjump = 20;
    ph[1].timedelta = 300;   ph[1].objjump = 30;

    // tdelta=0 lands in phase 0: dtime=0, ip += objjump.
    UByte* ip = stream;
    shape_dotimerphase(ip);
    CHECK_EQ(ip - stream, 20);                   // phase-0 jump from instr start
    CHECK_EQ(*(UWord*)(anim + 4), 0);            // dtime = tdelta - lowtime

    // btime so large that tdelta exceeds every phase delta -> no phase
    // matches -> ip ends after all phase records.
    *(ULong*)(anim + 0) = 0xFFFFF000;            // tdelta = 0x1000 > 300
    ip = stream;
    shape_dotimerphase(ip);
    CHECK_EQ(ip - stream,
             (long)(sizeof(DOTIMERPHASE) + 2 * sizeof(TPHASESTEP)));

    // timedelta==65535 is the "open" phase: tdelta is forced to 65534 so
    // the open phase always matches.
    *(ULong*)(anim + 0) = 0;                     // tdelta=0 again? no: force
    ph[0].timedelta = 50;
    ph[1].timedelta = 65535; ph[1].objjump = 44;
    *(ULong*)(anim + 0) = 0xFFFFFFFF;            // tdelta = 0 - 0xFFFFFFFF = 1
    ip = stream;
    shape_dotimerphase(ip);
    // tdelta=1: lowtime=0,phase0 delta 50 -> match phase0, jump 20.
    CHECK_EQ(ip - stream, 20);

    // tdelta beyond phase0 but under the open phase -> phase1 jump.
    *(ULong*)(anim + 0) = 0xFFFFFF00;            // tdelta = 0 - 0xFFFFFF00 = 256
    ph[0].timedelta = 100;                       // 256 >= 100
    ip = stream;
    shape_dotimerphase(ip);
    // 256 >= lowtime=100 && 256 < 65535 after forcing? No: timedelta==65535
    // forces tdelta=65534, which is >= lowtime=100 and < 65535 -> phase1.
    CHECK_EQ(ip - stream, 44);
    CHECK_EQ(*(UWord*)(anim + 4), 65534 - 100);  // dtime = tdelta - lowtime
}

static void test_dofadeenvelope()
{
    init_fake_viewpoint(0);                      // TimeOfDay() == 0

    static UByte anim[32];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;

    // [DOFADEENVELOPE]: btime = TimeOfDay - SLong@birthtimeoffset.
    UByte stream[16] = {0};
    DOFADEENVELOPE* fe = (DOFADEENVELOPE*)stream;
    fe->birthtimeoffset = 0;
    fe->animfadeoffset = 8;
    fe->attacktime = 100;
    fe->decaytime = 50;
    fe->sustaintime = 200;
    fe->releasetime = 60;
    fe->attackval = 200;
    fe->decayval = 100;
    fe->sustainval = 80;
    fe->releaseval = 0;

    // btime=0 <= attacktime? no: 0 > 100 false -> else branch ->
    // totval = ((decayval-attackval)*0)/100 + attackval = 200.
    *(SLong*)(anim + 0) = 0;
    UByte* ip = stream;
    shape_dofadeenvelope(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOFADEENVELOPE));
    CHECK_EQ(anim[8], 200);                      // attackval at btime 0

    // attacktime=0 + btime=0 -> else with attacktime guard false:
    // totval stays attackval.
    fe->attacktime = 0;
    anim[8] = 0;
    ip = stream;
    shape_dofadeenvelope(ip);
    CHECK_EQ(anim[8], 200);

    // mid-attack: btime=50, attacktime=100 -> totval =
    // (decayval-attackval)*50/100 + attackval = -50+200 = 150.
    fe->attacktime = 100;
    *(SLong*)(anim + 0) = -50;                   // TimeOfDay(0) - (-50) = 50
    anim[8] = 0;
    ip = stream;
    shape_dofadeenvelope(ip);
    CHECK_EQ(anim[8], 150);

    // full release: btime > a+d+s+r -> totval = releaseval = 0.
    *(SLong*)(anim + 0) = -1000;
    anim[8] = 0;
    ip = stream;
    shape_dofadeenvelope(ip);
    CHECK_EQ(anim[8], 0);
}

//------------------------------------------------------------------------------
// donianimverts: frame-grid positioning (stepx/stepy * framew/framewx) then
// NEXTMAP offsets - doniverts plus an animation-driven window origin.
//------------------------------------------------------------------------------
static void test_donianimverts()
{
    static UByte anim[16];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    // DONIANIMVERTS{vertex,count,animoff:9/noxframes:6/isthresh:1,
    //               factor,framewx,framewy,mapscale}
    UByte stream[32] = {0};
    DONIANIMVERTS* nv = (DONIANIMVERTS*)stream;
    nv->vertex = 1;
    nv->count = 1;
    nv->animoff = 2;
    nv->noxframes = 4;                           // 4 frames across
    nv->isthresh = 0;
    nv->factor = 2;
    nv->framewx = 32;
    nv->framewy = 16;
    nv->mapscale = 0;
    NEXTMAP* nm = (NEXTMAP*)(stream + sizeof(DONIANIMVERTS));
    nm->ix = 3;
    nm->iy = 5;

    // frameno = anim[2]/factor = 10/2 = 5 -> stepy = 5/4 = 1, stepx = 1.
    anim[2] = 10;
    UByte* ip = stream;
    shape_donianimverts(ip);
    CHECK_EQ(ip - stream,
             (long)(sizeof(DONIANIMVERTS) + sizeof(NEXTMAP)));
    CHECK_EQ(shape_shpco[1].ix, 32 * 1 + 3);     // minx=stepx*framewx
    CHECK_EQ(shape_shpco[1].iy, 16 * 1 + 5);     // miny=stepy*framewy

    // isthresh: frameno > factor -> 1 else 0 (no division at all).
    nv->isthresh = 1;
    nv->factor = 7;
    anim[2] = 20;                                // 20 > 7 -> frameno 1
    ip = stream;
    shape_donianimverts(ip);
    CHECK_EQ(shape_shpco[1].ix, 32 + 3);         // stepx=1,stepy=0
    CHECK_EQ(shape_shpco[1].iy, 0 + 5);

    anim[2] = 5;                                 // 5 <= 7 -> frameno 0
    ip = stream;
    shape_donianimverts(ip);
    CHECK_EQ(shape_shpco[1].ix, 3);
    CHECK_EQ(shape_shpco[1].iy, 5);

    // factor=0 + isthresh=0: guarded divide keeps raw frameno.
    nv->isthresh = 0;
    nv->factor = 0;
    anim[2] = 4;                                 // stepy=1,stepx=0
    ip = stream;
    shape_donianimverts(ip);
    CHECK_EQ(shape_shpco[1].ix, 0 + 3);
    CHECK_EQ(shape_shpco[1].iy, 16 + 5);

    // noxframes=0: guarded divides -> stepy=0 AND stepx=0 (frame stays
    // in-bounds instead of escaping via stepx=frameno).
    nv->noxframes = 0;
    nv->factor = 1;
    anim[2] = 2;
    ip = stream;
    shape_donianimverts(ip);
    CHECK_EQ(shape_shpco[1].ix, 0 + 3);
    CHECK_EQ(shape_shpco[1].iy, 0 + 5);
}

//------------------------------------------------------------------------------
// donsubs: offset-table sub-shape calls through the real InterpLoop.
//------------------------------------------------------------------------------
static void test_donsubs()
{
    static void (*tab[dosetglassrangeno + 1])(UByte*&) = {};
    void (**saved)(UByte*&) = shape_InterpTable; // ~shape() deletes this
    shape_InterpTable = tab;                     // - restore it after the test
    const int MARKOP = dosetglassrangeno;
    tab[MARKOP] = &marker_op;

    g_marker = 0;
    // [DONSUBS{count=2}][off1][off2][pad pad][sub1][sub2]
    // off@P jumps P+off; each sub is [MARKOP][doretno].
    UByte stream[16] = {0};
    *(UWord*)(stream + 0) = 2;                   // DONSUBS.count
    *(UWord*)(stream + 2) = 6;                   // off1: 2+6 = 8
    *(UWord*)(stream + 4) = 6;                   // off2: 4+6 = 10
    stream[8]  = (UByte)MARKOP;
    stream[9]  = (UByte)doretno;
    stream[10] = (UByte)MARKOP;
    stream[11] = (UByte)doretno;

    UByte* ip = stream;
    shape_donsubs(ip);
    shape_InterpTable = saved;
    CHECK_EQ(g_marker, 2);                       // both subs dispatched
    CHECK_EQ(ip - stream, 6);                    // resumed after off2+2
}

//------------------------------------------------------------------------------
// doifpiloted: BoxCol::Col_Shooter vs Manual_Pilot.ControlledAC2
// (ManualPilot layout: ViewPoint*, WorldStuff*, CONTROLMODE, ControlledAC2
// -> pointer at offset 12).
//------------------------------------------------------------------------------
static void test_doifpiloted()
{
    UByte stream[16] = {0};
    DOIFPILOTED* pp = (DOIFPILOTED*)stream;
    pp->offset = 40;

    void* ac = (void*)0x1234;
    *(void**)(Manual_Pilot_bytes + 12) = ac;     // ControlledAC2

    BoxCol_Col_Shooter = ac;                     // match -> +sizeof
    UByte* ip = stream;
    shape_doifpiloted(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOIFPILOTED));

    BoxCol_Col_Shooter = (void*)0x5678;          // mismatch -> +offset
    ip = stream;
    shape_doifpiloted(ip);
    CHECK_EQ(ip - stream, 40);
}

//------------------------------------------------------------------------------
// dodrawstation (empty station -> advance only) and dowhiteout (in/out of
// the fade box; DoWhiteFade itself is a shape member we can't observe
// portably, so the pin is ip movement + no crash).
//------------------------------------------------------------------------------
static void test_drawstation_whiteout()
{
    static UByte anim[256];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;                    // all stationshape fields 0

    UByte stream[16] = {0};
    DODRAWSTATION* ds = (DODRAWSTATION*)stream;
    ds->stationno = 0;
    UByte* ip = stream;
    shape_dodrawstation(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DODRAWSTATION));

    TestObj3D obj = TestObj3D();
    shape_object_obj3d = &obj;
    DOWHITEOUT* wo = (DOWHITEOUT*)stream;
    wo->fadedist = 100;

    obj.Body.X.f = 1000000;                      // way outside -> no fade
    obj.Body.Y.f = 0;
    obj.Body.Z.f = 0;
    ip = stream;
    shape_dowhiteout(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOWHITEOUT));

    obj.Body.X.f = 10;                           // inside box -> fade path
    obj.Body.Y.f = 20;
    obj.Body.Z.f = 30;
    ip = stream;
    shape_dowhiteout(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOWHITEOUT));
}

//------------------------------------------------------------------------------
// Lighting/vector handlers: donvec and dondupvec gate everything on
// View_Point->isLightShaded (Bool at ViewPoint+0x1e4, per dolshadeon's store).
// Shaded path normalises each NEXTVEC record, dots it with TransLightVector,
// maps to intensity, and optionally runs calcSpecular against TransViewVector.
//------------------------------------------------------------------------------
static void test_donvec()
{
    static UByte fake_viewpoint[4096];
    std::memset(fake_viewpoint, 0, sizeof(fake_viewpoint));
    shape_View_Point = fake_viewpoint;
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    // not light-shaded: header + count*NEXTVEC skipped, nothing written.
    UByte stream[16] = {0};
    DONVEC* hdr = (DONVEC*)stream;
    hdr->vertex = 2;
    hdr->count = 2;
    NEXTVEC* v0 = (NEXTVEC*)(stream + sizeof(DONVEC));
    v0[0].an = 127; v0[0].bn = 0; v0[0].cn = 0;
    v0[1].an = -128; v0[1].bn = 0; v0[1].cn = 0;

    shape_shpco[2].intensity = 0x7777;
    shape_shpco[2].specular = 0x7777;
    shape_shpco[2].specFlip = 0x7777;
    UByte* ip = stream;
    shape_donvec(ip);
    CHECK_EQ(ip - stream, (long)(sizeof(DONVEC) + 2 * sizeof(NEXTVEC)));
    CHECK_EQ(shape_shpco[2].intensity, 0x7777);

    // light-shaded: light = +X. Facing normal -> lit (intensity 1);
    // back-facing -> clamps at 256.
    *(int*)(fake_viewpoint + 0x1e4) = 1;   // isLightShaded = TRUE
    shape_TransLightVector.ni.f = 1.0;
    shape_TransLightVector.nj.f = 0.0;
    shape_TransLightVector.nk.f = 0.0;
    shape_specularEnabled = FALSE;

    ip = stream;
    shape_donvec(ip);
    CHECK_EQ(ip - stream, (long)(sizeof(DONVEC) + 2 * sizeof(NEXTVEC)));
    // an=(1,0,0): lightDot=1 -> (32385+32385)/232=279 -> 280-279=1
    CHECK_EQ(shape_shpco[2].intensity, 1);
    CHECK_EQ(shape_shpco[2].specular, -1);
    CHECK_EQ(shape_shpco[2].specFlip, -1);
    // an=(-1,0,0): lightDot=-1 -> 0/232=0 -> 280 -> clamp 256
    CHECK_EQ(shape_shpco[3].intensity, 256);
    CHECK_EQ(shape_shpco[3].specular, -1);
    CHECK_EQ(shape_shpco[3].specFlip, -1);

    // specular path: view = +X as well -> specDot=lightDot=1 -> delta 0
    // -> specFlip=specMax(95); the +pi-folded block then yields specular=0.
    shape_TransViewVector.ni.f = 1.0;
    shape_TransViewVector.nj.f = 0.0;
    shape_TransViewVector.nk.f = 0.0;
    shape_specularEnabled = TRUE;
    hdr->count = 1;
    ip = stream;
    shape_donvec(ip);
    CHECK_EQ(shape_shpco[2].intensity, 1);
    CHECK_EQ(shape_shpco[2].specular, 0);
    CHECK_EQ(shape_shpco[2].specFlip, 95);

    // specular glow needs the normal facing AWAY from both vectors:
    // light = view = -X, normal +X -> folded angles 0 -> specular=95,
    // specFlip=0; intensity = the 256-clamped back-face value.
    shape_TransLightVector.ni.f = -1.0;
    shape_TransViewVector.ni.f = -1.0;
    ip = stream;
    shape_donvec(ip);
    CHECK_EQ(shape_shpco[2].intensity, 256);
    // Build-dependent: when acos(-1) is constant-folded the folded delta
    // is exactly 0 -> specular = specMax(95); when it runs through the
    // x87 80-bit path a tiny epsilon survives -> truncation gives 94.
    CHECK(shape_shpco[2].specular == 94 || shape_shpco[2].specular == 95);
    CHECK_EQ(shape_shpco[2].specFlip, 0);

    // count = 0: advances past the header only, writes nothing.
    shape_specularEnabled = FALSE;
    shape_shpco[2].intensity = 0x7777;
    hdr->count = 0;
    ip = stream;
    shape_donvec(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONVEC));
    CHECK_EQ(shape_shpco[2].intensity, 0x7777);

    *(int*)(fake_viewpoint + 0x1e4) = 0;
    shape_specularEnabled = FALSE;
}

static void test_dondupvec()
{
    static UByte fake_viewpoint[4096];
    std::memset(fake_viewpoint, 0, sizeof(fake_viewpoint));
    shape_View_Point = fake_viewpoint;
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    // not light-shaded: sizeof(DONDUPVEC)=5, nothing written.
    UByte stream[16] = {0};
    DONDUPVEC* hdr = (DONDUPVEC*)stream;
    hdr->vertex = 2;
    hdr->count = 3;
    hdr->ambientfiddle = 0;
    hdr->an = 127; hdr->bn = 0; hdr->cn = 0;

    shape_shpco[2].intensity = 0x7777;
    UByte* ip = stream;
    shape_dondupvec(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONDUPVEC));
    CHECK_EQ(shape_shpco[2].intensity, 0x7777);

    *(int*)(fake_viewpoint + 0x1e4) = 1;   // isLightShaded = TRUE
    shape_TransLightVector.ni.f = 1.0;
    shape_TransLightVector.nj.f = 0.0;
    shape_TransLightVector.nk.f = 0.0;
    shape_specularEnabled = FALSE;

    // ambientfiddle=0: intensity = (64770 >> 8)=253 -> 256-253=3, written
    // to count consecutive vertices.
    ip = stream;
    shape_dondupvec(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONDUPVEC));
    CHECK_EQ(shape_shpco[2].intensity, 3);
    CHECK_EQ(shape_shpco[3].intensity, 3);
    CHECK_EQ(shape_shpco[4].intensity, 3);
    CHECK_EQ(shape_shpco[2].specular, -1);
    CHECK_EQ(shape_shpco[2].specFlip, -1);
    // vertex 5 is outside the count range - untouched.
    CHECK_EQ(shape_shpco[5].intensity, 0);

    // ambientfiddle=1: (64770 / 232)=279 -> 280-279=1.
    hdr->count = 2;
    hdr->ambientfiddle = 1;
    ip = stream;
    shape_dondupvec(ip);
    CHECK_EQ(shape_shpco[2].intensity, 1);
    CHECK_EQ(shape_shpco[3].intensity, 1);
    CHECK_EQ(shape_shpco[4].intensity, 3);   // outside new count range

    // back-facing normal + ambientfiddle=0: intensity base 0 -> 256-0=256.
    hdr->count = 1;
    hdr->ambientfiddle = 0;
    hdr->an = -128;
    ip = stream;
    shape_dondupvec(ip);
    CHECK_EQ(shape_shpco[2].intensity, 256);

    // count = 0 with shading on: header advance only, nothing written.
    shape_shpco[2].intensity = 0x7777;
    hdr->count = 0;
    ip = stream;
    shape_dondupvec(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONDUPVEC));
    CHECK_EQ(shape_shpco[2].intensity, 0x7777);

    *(int*)(fake_viewpoint + 0x1e4) = 0;
    shape_specularEnabled = FALSE;
}

//------------------------------------------------------------------------------
// dotransformlight: the IsSubShape guard must return before touching any
// lighting state (the outer path needs ItemPtr/Three_Dee/Land_Scape and is
// integration-only).
//------------------------------------------------------------------------------
static void test_dotransformlight()
{
    shape_IsSubShape = TRUE;
    shape_specularEnabled = FALSE;
    shape_TransLightVector.ni.f = 0.0;
    shape_TransLightVector.nj.f = 0.0;
    shape_TransLightVector.nk.f = 0.0;

    UByte stream[8] = {0};
    UByte* ip = stream;
    shape_dotransformlight(ip);

    CHECK_EQ(shape_specularEnabled, FALSE);
    CHECK_EQ(shape_TransLightVector.ni.f, 0.0);
    CHECK_EQ(shape_TransLightVector.nj.f, 0.0);
    CHECK_EQ(shape_TransLightVector.nk.f, 0.0);

    shape_IsSubShape = FALSE;
}

//------------------------------------------------------------------------------
// dovector: single-vertex variant of the dondupvec lighting calc - same
// ambientfiddle split (>>8 vs /232), always +sizeof(DOVECTOR).
//------------------------------------------------------------------------------
static void test_dovector()
{
    static UByte fake_viewpoint[4096];
    std::memset(fake_viewpoint, 0, sizeof(fake_viewpoint));
    shape_View_Point = fake_viewpoint;
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();

    UByte stream[8] = {0};
    DOVECTOR* hdr = (DOVECTOR*)stream;
    hdr->an = 127;
    hdr->vertex = 3;
    hdr->ambientfiddle = 0;
    hdr->bn = 0;
    hdr->cn = 0;

    // not light-shaded: nothing written, still advances.
    shape_shpco[3].intensity = 0x7777;
    UByte* ip = stream;
    shape_dovector(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOVECTOR));
    CHECK_EQ(shape_shpco[3].intensity, 0x7777);

    *(int*)(fake_viewpoint + 0x1e4) = 1;   // isLightShaded = TRUE
    shape_TransLightVector.ni.f = 1.0;
    shape_TransLightVector.nj.f = 0.0;
    shape_TransLightVector.nk.f = 0.0;
    shape_specularEnabled = FALSE;

    // ambientfiddle=0, facing normal: 64770>>8=253 -> 256-253=3.
    ip = stream;
    shape_dovector(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOVECTOR));
    CHECK_EQ(shape_shpco[3].intensity, 3);
    CHECK_EQ(shape_shpco[3].specular, -1);
    CHECK_EQ(shape_shpco[3].specFlip, -1);

    // ambientfiddle=1: 64770/232=279 -> 280-279=1.
    hdr->ambientfiddle = 1;
    ip = stream;
    shape_dovector(ip);
    CHECK_EQ(shape_shpco[3].intensity, 1);

    // back-facing normal: 256-0=256.
    hdr->ambientfiddle = 0;
    hdr->an = -128;
    ip = stream;
    shape_dovector(ip);
    CHECK_EQ(shape_shpco[3].intensity, 256);

    *(int*)(fake_viewpoint + 0x1e4) = 0;
    shape_specularEnabled = FALSE;
}

//------------------------------------------------------------------------------
// Small GlobalAdptr-driven handlers: conditional advances and rewinds that
// re-run or skip the following instruction.
//------------------------------------------------------------------------------
static void test_anim_conditionals2()
{
    static UByte anim[64];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;

    // docopyivert: copies image coords into the vertex record.
    shape_newco = shape_shpco;
    for (int i = 0; i < 16; ++i) shape_shpco[i] = DoPointStruc();
    UByte stream[16] = {0};
    DOCOPYIVERT* ci = (DOCOPYIVERT*)stream;
    ci->vertex = 4;
    ci->image_x = 320;
    ci->image_y = 240;
    UByte* ip = stream;
    shape_docopyivert(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOCOPYIVERT));
    CHECK_EQ(shape_shpco[4].ix, 320);
    CHECK_EQ(shape_shpco[4].iy, 240);
    // UWord 0xFFFF narrows to SWord -1.
    ci->image_x = 0xFFFF;
    ip = stream;
    shape_docopyivert(ip);
    CHECK_EQ(shape_shpco[4].ix, -1);

    // dobitsoffcock: bit clear -> +offset; bit set -> +sizeof.
    DOBITSOFFCOCK* bc = (DOBITSOFFCOCK*)stream;
    bc->bitflag = 3;
    bc->animflag = 10;
    bc->offset = 40;
    *(ULong*)(anim + 10) = 0x8;              // bit 3 set
    ip = stream;
    shape_dobitsoffcock(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOBITSOFFCOCK));
    *(ULong*)(anim + 10) = 0x4;              // bit 3 clear
    ip = stream;
    shape_dobitsoffcock(ip);
    CHECK_EQ(ip - stream, 40);
    // bitflag beyond the flag's bits -> always clear.
    bc->bitflag = 31;
    ip = stream;
    shape_dobitsoffcock(ip);
    CHECK_EQ(ip - stream, 40);

    // doondamaged: rewind when damval outside [thresh, topthresh).
    DOONDAMAGED* od = (DOONDAMAGED*)stream;
    od->topthresh = 200;
    od->animoff = 20;
    od->thresh = 100;
    anim[20] = 50;                            // below thresh -> rewind 1
    ip = stream;
    shape_doondamaged(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOONDAMAGED) - 1);
    anim[20] = 100;                           // == thresh -> in range
    ip = stream;
    shape_doondamaged(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOONDAMAGED));
    anim[20] = 199;                           // topthresh-1 -> in range
    ip = stream;
    shape_doondamaged(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOONDAMAGED));
    anim[20] = 200;                           // == topthresh -> out
    ip = stream;
    shape_doondamaged(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOONDAMAGED) - 1);
    // topthresh=0 means 256: 255 stays in range.
    od->topthresh = 0;
    anim[20] = 255;
    ip = stream;
    shape_doondamaged(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOONDAMAGED));

    // doweaponoff: rewind unless launchertype matches AND stores remain.
    DOWEAPONOFF* wo = (DOWEAPONOFF*)stream;
    wo->launchertype = 7;
    wo->storesoffset = 30;
    wo->launchoffset = 32;
    anim[32] = 7;                             // launcher matches
    *(SWord*)(anim + 30) = 0;                 // no stores -> rewind
    ip = stream;
    shape_doweaponoff(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOWEAPONOFF) - 1);
    *(SWord*)(anim + 30) = 3;                 // stores remain -> keep
    ip = stream;
    shape_doweaponoff(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOWEAPONOFF));
    anim[32] = 9;                             // different launcher -> rewind
    ip = stream;
    shape_doweaponoff(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOWEAPONOFF) - 1);
}

//------------------------------------------------------------------------------
// douserealtime seeds a zero birth-time slot with View_Point->TimeOfDay();
// a non-zero slot is left alone.
//------------------------------------------------------------------------------
static void test_douserealtime()
{
    static UByte anim[64];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;
    init_fake_viewpoint(4242);

    UByte stream[8] = {0};
    DOUSEREALTIME* rt = (DOUSEREALTIME*)stream;
    rt->birthtimeoffset = 16;

    UByte* ip = stream;
    shape_douserealtime(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOUSEREALTIME));
    CHECK_EQ(*(ULong*)(anim + 16), (ULong)4242);

    // already seeded -> not overwritten.
    *(ULong*)(anim + 20) = 777;
    rt->birthtimeoffset = 20;
    ip = stream;
    shape_douserealtime(ip);
    CHECK_EQ(*(ULong*)(anim + 20), (ULong)777);

    shape_View_Point = NULL;
}

//------------------------------------------------------------------------------
// Advance-only and no-op handlers: empty bodies or dead local writes.
// dobitsofffx only advances when the effect is not triggered (the launch
// path needs fprealobject_matrix + Trans_Obj -> integration-only).
//------------------------------------------------------------------------------
static void test_advance_only2()
{
    UByte stream[16] = {0};
    static UByte anim[64];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;

    UByte* ip = stream;
    shape_doiswitch(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOISWITCH));
    ip = stream;
    shape_docopybvert(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOCOPYBVERT));
    ip = stream;
    shape_docreateivert(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOCREATEIVERT));
    ip = stream;
    shape_dosetglassrange(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSETGLASSRANGE));

    // dead no-ops: pointer must not move.
    ip = stream;
    shape_dosetmappingplaner(ip);
    CHECK_EQ(ip - stream, 0);
    ip = stream;
    shape_dosetmappingtan(ip);
    CHECK_EQ(ip - stream, 0);
    ip = stream;
    shape_dodrawbpoly(ip);
    CHECK_EQ(ip - stream, 0);
    ip = stream;
    shape_doimagemap(ip);
    CHECK_EQ(ip - stream, 0);

    // advance-only stubs (declared locals are dead).
    ip = stream;
    shape_docompass(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOCOMPASS));
    ip = stream;
    shape_docreatebpoly(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOCREATEBUMPPOLY));
    ip = stream;
    shape_dolauncher(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOLAUNCHER));
    ip = stream;
    shape_donspheres(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONSPHERES));
    ip = stream;
    shape_donspheresimapd(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DONSPHERESIMAPD));

    // dobitsofffx: damval <= threshold -> no effect.
    DOBITSOFFFX* fx = (DOBITSOFFFX*)stream;
    fx->animoff = 8;
    fx->threshold = 100;
    anim[8] = 100;                            // == threshold -> not triggered
    ip = stream;
    shape_dobitsofffx(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOBITSOFFFX));
    anim[8] = 101;                            // triggered but no matrix ->
    ip = stream;                              // inner block skipped
    shape_fprealobject_matrix = NULL;
    shape_dobitsofffx(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOBITSOFFFX));
    fx->threshold = 0xFFFF;                   // max UWord -> never triggers
    anim[8] = 255;
    ip = stream;
    shape_dobitsofffx(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOBITSOFFFX));
}

//------------------------------------------------------------------------------
// dospin: anim-slot spin speed. Nonzero speed adds speed*FrameTime()/100
// (FrameTime()=0 on the fake display -> angle frozen). Zero speed re-arms
// from minspeed + rnd(diff), or pins angle to minspeed when diff==0.
//------------------------------------------------------------------------------
static void test_dospin()
{
    static UByte anim[64];
    static UByte fake_screen[16384];
    static UByte fake_display[16384];
    std::memset(anim, 0, sizeof(anim));
    std::memset(fake_display, 0, sizeof(fake_display));
    for (size_t i = 0; i < sizeof(fake_screen) / sizeof(void*); ++i)
        ((void**)fake_screen)[i] = fake_display;
    shape_GlobalAdptr = anim;
    shape_current_screen = fake_screen;

    UByte stream[8] = {0};
    DOSPIN* sp = (DOSPIN*)stream;
    sp->animoff = 10;                          // UWord angle slot
    sp->speedoff = 12;                         // UWord speed slot
    sp->minspeed = 100;
    sp->maxspeed = 300;

    // running spin with FrameTime()==0: angle does not move.
    *(UWord*)(anim + 10) = 500;
    *(UWord*)(anim + 12) = 50;
    UByte* ip = stream;
    shape_dospin(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSPIN));
    CHECK_EQ(*(UWord*)(anim + 10), (UWord)500);
    CHECK_EQ(*(UWord*)(anim + 12), (UWord)50);

    // zero speed + diff!=0: re-arms speed into [minspeed, minspeed+diff),
    // angle untouched.
    *(UWord*)(anim + 12) = 0;
    ip = stream;
    shape_dospin(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSPIN));
    UWord armed = *(UWord*)(anim + 12);
    CHECK(armed >= 100 && armed < 300);
    CHECK_EQ(*(UWord*)(anim + 10), (UWord)500);

    // zero speed + diff==0: "crap fix" pins angle to minspeed.
    *(UWord*)(anim + 12) = 0;
    sp->maxspeed = 100;
    ip = stream;
    shape_dospin(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOSPIN));
    CHECK_EQ(*(UWord*)(anim + 10), (UWord)100);
    CHECK_EQ(*(UWord*)(anim + 12), (UWord)0);
}

//------------------------------------------------------------------------------
// dolighttimer: word/byte anim slots driven by light timer types.
// Paths that call View_Point->FrameTime (PERIODIC with nonzero frametime)
// need a real view3dwin -> integration-only; everything else is pinned here.
// Three_Dee.lightson sits at offset 0x2f0 (per dolighttimer codegen).
//------------------------------------------------------------------------------
static void test_dolighttimer()
{
    static UByte anim[64];
    std::memset(anim, 0, sizeof(anim));
    shape_GlobalAdptr = anim;
    *(Bool*)(Three_Dee_bytes + 0x2f0) = FALSE; // lightson

    UByte stream[8] = {0};
    DOLIGHTTIMER* lt = (DOLIGHTTIMER*)stream;
    lt->animoff = 10;
    lt->duration = 1000;
    lt->isword = 0;

    // ONLY_DARK while lights off -> frametime forced to 0, byte slot.
    lt->timertype = 4;                          // LGT_ONLY_DARK
    anim[10] = 77;
    UByte* ip = stream;
    shape_dolighttimer(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOLIGHTTIMER));
    CHECK_EQ(anim[10], 0);

    // same in word mode -> writes UWord 0.
    lt->isword = 1;
    *(UWord*)(anim + 10) = 77;
    ip = stream;
    shape_dolighttimer(ip);
    CHECK_EQ(*(UWord*)(anim + 10), (UWord)0);

    // USER type (no ONLY_DARK, no PERIODIC): frametime read + written
    // straight back - value preserved, byte and word modes.
    lt->timertype = 1;                          // LGT_USER
    lt->isword = 0;
    anim[10] = 55;
    ip = stream;
    shape_dolighttimer(ip);
    CHECK_EQ(anim[10], 55);
    lt->isword = 1;
    *(UWord*)(anim + 10) = 55;
    ip = stream;
    shape_dolighttimer(ip);
    CHECK_EQ(*(UWord*)(anim + 10), (UWord)55);

    // PERIODIC but frametime==0: the && short-circuit skips
    // View_Point->FrameTime; still writes 0 back.
    lt->timertype = 2;                          // LGT_PERIODIC
    lt->isword = 0;
    anim[10] = 0;
    ip = stream;
    shape_dolighttimer(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOLIGHTTIMER));
    CHECK_EQ(anim[10], 0);

    // ONLY_DARK with lights ON -> else branch runs: frametime read
    // (USER-like), non-periodic so value preserved.
    *(Bool*)(Three_Dee_bytes + 0x2f0) = TRUE;
    lt->timertype = 4;
    lt->isword = 0;
    anim[10] = 33;
    ip = stream;
    shape_dolighttimer(ip);
    CHECK_EQ(anim[10], 33);

    *(Bool*)(Three_Dee_bytes + 0x2f0) = FALSE;
}

//------------------------------------------------------------------------------
// doanimation: the anim-slot state machine. On the fake display
// FrameTime()==0, so cntinc only comes from the timeroffset slot - which
// makes frame stepping fully deterministic. activatenow is matched
// against MinAnimData.itemstate (bits 5-6 of GlobalAdptr[0]).
//------------------------------------------------------------------------------
static void test_doanimation()
{
    static UByte anim[256];
    static UByte fake_screen[16384];
    static UByte fake_display[16384];
    std::memset(anim, 0, sizeof(anim));
    std::memset(fake_display, 0, sizeof(fake_display));
    for (size_t i = 0; i < sizeof(fake_screen) / sizeof(void*); ++i)
        ((void**)fake_screen)[i] = fake_display;
    shape_GlobalAdptr = anim;
    shape_current_screen = fake_screen;

    UByte stream[32] = {0};
    DOANIMATION* an = (DOANIMATION*)stream;
    an->maxframes = 100;
    an->flagoffset = 20;                       // animtimer slot
    an->timeroffset = 40;                      // cntinc injection
    stream[6] = 0x20;                          // maxaction=STAY, inc=PER_FRAME,
                                             // activatenow=1
    anim[0] = 0x20;                            // MinAnimData.itemstate=1
    anim[40] = 10;
    anim[20] = 50;

    // basic increment: 50 + 10 = 60.
    UByte* ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(ip - stream, (long)sizeof(DOANIMATION));
    CHECK_EQ(anim[20], 60);

    // over the max: MAX_STAY clamps to themax.
    anim[20] = 95;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 100);

    // MAX_RESET_ZERO: non-cheat trigger resets to 0...
    stream[6] = 0x21;                          // maxaction=RESET_ZERO
    anim[20] = 95;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 0);
    // ...but via toggleoffset any nonzero byte is a "cheat" trigger,
    // which resets to cntinc instead.
    an->toggleoffset = 30;
    anim[30] = 1;
    anim[20] = 95;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 10);
    CHECK_EQ(anim[30], 1);                     // toggleflag written back

    // negative toggle drives the reverse path: 5 + (10 * -1) = -5
    // -> MAX_RESET_ZERO on the <0 side writes themax.
    anim[30] = 0xFF;                           // SByte -1
    anim[20] = 5;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 100);
    CHECK_EQ(anim[30], 0xFF);
    an->toggleoffset = 0;

    // state mismatch but nonzero framecounter -> cheat-resume forces doit;
    // 250+10 wraps past themax and MAX_STAY clamps to 100.
    stream[6] = 0x20;                          // STAY
    anim[0] = 0;                               // itemstate=0 != 1
    anim[20] = 250;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 100);

    // nonzero framecounter alone re-triggers the anim (cheat resume):
    // condition fails but framecounter!=0 forces doit.
    anim[20] = 50;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 60);

    // damagetoggle: ThisState = GlobalAdptr[damageoffset]/96.
    stream[7] = 8; stream[8] = 0x80;           // damageoffset=8,damagetoggle=1
    anim[8] = 96;                              // -> state 1 -> activates
    anim[20] = 50;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 60);
    anim[8] = 95;                              // -> state 0 -> no activate,
    anim[20] = 0;                              // framecounter 0 -> frozen
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 0);
    anim[8] = 192;                             // -> state 2: 2&1 == 0 -> no
    anim[20] = 0;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[20], 0);
    stream[7] = 0; stream[8] = 0;              // damagetoggle off
    anim[8] = 0;

    // fade path: fadeoffset + nonzero fadedepth reroutes animtimer to
    // the fade slot and writes fadedepth = frame*254/themax + 1.
    anim[0] = 0x20;
    an->fadeoffset = 50;
    an->maxfadeframes = 100;
    an->fadedepth = 51;
    anim[51] = 1;
    anim[50] = 0;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[50], 10);                    // framecounter advanced
    CHECK_EQ(anim[51], 26);                    // (10*254)/100 + 1

    // maxfadeframes=0 with a live fade -> guarded divide, no SIGFPE.
    // themax=0 also makes MAX_STAY clamp the counter to 0.
    an->maxfadeframes = 0;
    anim[51] = 9;
    anim[50] = 5;
    ip = stream;
    shape_doanimation(ip);
    CHECK_EQ(anim[50], 0);
    CHECK_EQ(anim[51], 9);

    // clean up the slots this test shares with nothing else
    std::memset(anim, 0, sizeof(anim));
}

//------------------------------------------------------------------------------
// DeadStream::DeadBlockItterator packs dead-element records into 1KB
// blocks. SetWorldDead runs inside ~Inst3d — on a partially torn-down
// world the anim-derived payload size (diplc) can be garbage: a negative
// count used to become a wild memcpy and an oversized count overflowed
// data[maxblocksize]. Pin the guard behaviour plus the normal paths.
//------------------------------------------------------------------------------
static void test_deadstream_iterator()
{
    char payload[2048];
    std::memset(payload, 0x7E, sizeof payload);

    DeadStream::DeadBlockPtr base = NULL;
    {
        DeadStream::DeadBlockItterator it(base);
        CHECK(base != NULL);

        // Ordinary writes land inside the block.
        CHECK_EQ(it.PutInfo(payload, 16), false);
        CHECK_EQ(base->dataused, 16);
        CHECK(std::memcmp(base->data, payload, 16) == 0);

        // count+off over the block -> spill into a fresh block.
        CHECK_EQ(it.PutInfo(payload, 1024), true);
        CHECK(base->nextblock != NULL);
        CHECK_EQ(base->nextblock->dataused, 1024);

        // Impossible counts are skipped, not written (pre-fix the oversize
        // one overflowed the block, the negative one crashed memcpy).
        CHECK_EQ(it.PutInfo(payload, 2048), false);
        CHECK_EQ(it.PutInfo(payload, -9), false);
        CHECK(base->nextblock->nextblock == NULL);
    }
    while (base) { DeadStream::DeadBlockPtr n = base->nextblock; delete base; base = n; }

    // SetNextDeadElt with a sane payload: flag set, then word count + data.
    base = NULL;
    {
        DeadStream::DeadBlockItterator it(base);
        char elt[10];
        std::memset(elt, 0x11, sizeof elt);
        it.SetNextDeadElt((char)0x01, 10, elt);
        CHECK(((MinAnimData&)base->data[0]).IsInvisible);
        CHECK_EQ(base->dataused, 1 + 2 + 10);
        CHECK_EQ(*(UWord*)(base->data + 1), 10);
        CHECK(std::memcmp(base->data + 3, elt, 10) == 0);
    }
    while (base) { DeadStream::DeadBlockPtr n = base->nextblock; delete base; base = n; }

    // Garbage diplc (torn-down world anim): flag-less byte only — the
    // reader keys payload presence off IsInvisible so the stream stays
    // consistent.
    base = NULL;
    {
        DeadStream::DeadBlockItterator it(base);
        it.SetNextDeadElt((char)0xFF, -3, payload);
        CHECK(!((MinAnimData&)base->data[0]).IsInvisible);
        CHECK_EQ(base->dataused, 1);
        it.SetNextDeadElt((char)0xFF, 5000, payload);
        CHECK_EQ(base->dataused, 2);
        CHECK(!((MinAnimData&)base->data[1]).IsInvisible);
        // diplc == 0 behaves exactly like a bad count.
        it.SetNextDeadElt((char)0xFF, 0, payload);
        CHECK_EQ(base->dataused, 3);
        CHECK(!((MinAnimData&)base->data[2]).IsInvisible);
    }
    while (base) { DeadStream::DeadBlockPtr n = base->nextblock; delete base; base = n; }
}

//------------------------------------------------------------------------------
// MathLib trigonometry - pins the 10-bit sincos_table (scale 32767, not
// 32768) and 8.8-fixed tan_table. high_sin_cos interpolates within each
// 64-angle step; sin_cos truncates.
//
// Fixed defects (were pinned as quirks before the bug-fix pass):
//  - arcsin/arccos were exact-match linear scans returning the table
//    INDEX and uninitialized garbage on misses; both now delegate to
//    high_arc_sin/high_arc_cos (real math, same input scale).
//  - hightan's QII/QIV branch assigned tan0 twice and never set tan1
//    ("DAW 27Sep00 from bob" copy-paste), indexed tan_table[-1] at the
//    180deg boundary, and fed the quadrant bit into the interpolation
//    fraction. All three are fixed; upper-half angles now interpolate.
//------------------------------------------------------------------------------
static void test_mathlib_trig()
{
    SWord s, c;

    // Cardinal points - raw table reads.
    Math_Lib.sin_cos(ANGLES_0Deg, s, c);
    CHECK_EQ(s, 0);          CHECK_EQ(c, 32767);
    Math_Lib.sin_cos(ANGLES_90Deg, s, c);
    CHECK_EQ(s, 32767);      CHECK_EQ(c, 0);
    Math_Lib.sin_cos(ANGLES_180Deg, s, c);
    CHECK_EQ(s, 0);          CHECK_EQ(c, -32767);
    Math_Lib.sin_cos(ANGLES_270Deg, s, c);
    CHECK_EQ(s, -32767);     CHECK_EQ(c, 0);
    Math_Lib.sin_cos(ANGLES_45Deg, s, c);
    CHECK_EQ(s, 23169);      CHECK_EQ(c, 23169);

    // Truncation: angle 96 sits 32/64 through step 1 - sin_cos ignores the
    // fraction, high_sin_cos interpolates (sin 201->300, cos stays 32766).
    Math_Lib.sin_cos((Angles)96, s, c);
    CHECK_EQ(s, 201);        CHECK_EQ(c, 32766);
    Math_Lib.high_sin_cos((Angles)96, s, c);
    CHECK_EQ(s, 300);        CHECK_EQ(c, 32766);
    Math_Lib.high_sin_cos((Angles)0x2001, s, c);
    CHECK_EQ(s, 23171);      CHECK_EQ(c, 23167);
    // Wrap: 0xFFFF interpolates back toward sin 0.
    Math_Lib.high_sin_cos((Angles)0xFFFF, s, c);
    CHECK_EQ(s, -5);         CHECK_EQ(c, 32767);

    // tan_table is 8.8 fixed: tan(45deg)=256. QII mirrors with sign flip.
    CHECK_EQ(Math_Lib.tan(ANGLES_0Deg), 0);
    CHECK_EQ(Math_Lib.tan(ANGLES_45Deg), 256);
    CHECK_EQ(Math_Lib.tan(ANGLES_135Deg), -252);
    CHECK_EQ(Math_Lib.tan((Angles)0x1000), 105);   // 22.5deg

    // hightan is 16.16 fixed: hightan(45deg)=65536. Upper-half angles
    // mirror through -tan_table[255-i] (off-by-one vs the ideal 256-i
    // mirror is the original BoB convention, kept).
    CHECK_EQ(Math_Lib.hightan(ANGLES_0Deg), 0);
    CHECK_EQ(Math_Lib.hightan(ANGLES_45Deg), 65536);
    CHECK_EQ(Math_Lib.hightan((Angles)0x1000), 26880);
    SLong ht135 = Math_Lib.hightan(ANGLES_135Deg);
    CHECK(ht135 < 0 && ht135 < -60000 && ht135 > -70000);  // ~-1.0*65536
    SLong ht225 = Math_Lib.hightan((Angles)0xA000);        // 225deg: tan=+1
    CHECK(ht225 > 0 && ht225 > 60000 && ht225 < 70000);
    CHECK_EQ(Math_Lib.hightan(ANGLES_180Deg), 0);
    Math_Lib.hightan((Angles)0x7FFF);                      // boundary, no OOB
    Math_Lib.hightan((Angles)0xFFFF);

    // arctan(dx,dy) = 10430.387*atan2(dx,dy) - quadrant-exact.
    CHECK_EQ((SWord)Math_Lib.arctan(0, 1000), 0);
    CHECK_EQ((SWord)Math_Lib.arctan(1000, 0), ANGLES_90Deg);
    CHECK_EQ((SWord)Math_Lib.arctan(1000, 1000), ANGLES_45Deg);
    CHECK_EQ((SWord)Math_Lib.arctan(-1000, 1000), (SWord)-8192);
    CHECK_EQ((SWord)Math_Lib.arctan(0, -1000), (SWord)ANGLES_180Deg);
    CHECK_EQ((SWord)Math_Lib.arctan(1000, -1000), ANGLES_135Deg);
    CHECK_EQ((SWord)Math_Lib.arctan(0, 0), 0);

    // HighArcTan agrees at cardinals and interpolates via matan[].
    CHECK_EQ((SWord)Math_Lib.HighArcTan(0, 1000), 0);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(1000, 0), ANGLES_90Deg);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(-1000, 0), (SWord)-16384);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(1000, 1000), ANGLES_45Deg);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(-1000, 1000), (SWord)-8192);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(0, -1000), (SWord)ANGLES_180Deg);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(-1000, -1000), (SWord)-24576);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(1000, -1000), ANGLES_135Deg);
    CHECK_EQ((SWord)Math_Lib.HighArcTan(0, 0), 0);

    // arcsin/arccos now delegate to high_arc_sin/high_arc_cos: real
    // angles out, deterministic for every input (was uninit garbage).
    CHECK_EQ((SWord)Math_Lib.arcsin(0), 0);
    CHECK_EQ((SWord)Math_Lib.arcsin(16384), 5461);    // sin^-1(0.5) -> 30deg
    CHECK_EQ((SWord)Math_Lib.arcsin(32767), 16302);   // ~89.6deg
    CHECK_EQ((SWord)Math_Lib.arcsin(-16384), -5461);
    CHECK_EQ((SWord)Math_Lib.arccos(0), ANGLES_90Deg);
    CHECK_EQ((SWord)Math_Lib.arccos(32767), (SWord)Math_Lib.high_arc_cos(32767));
    CHECK_EQ((SWord)Math_Lib.arccos(-16384),
             (SWord)(ANGLES_180Deg - (SWord)Math_Lib.arccos(16384)));

    // high-precision variants use real FP math (correct values).
    CHECK_EQ((SWord)Math_Lib.high_arc_sin(0), 0);
    CHECK_EQ((SWord)Math_Lib.high_arc_sin(32767), 16302);   // ~89.6deg
    CHECK_EQ((SWord)Math_Lib.high_arc_sin(-32767), -16302);
    CHECK_EQ((SWord)Math_Lib.high_arc_sin(16384), 5461);    // ~30deg
    CHECK_EQ((SWord)Math_Lib.high_arc_cos(0), ANGLES_90Deg);
    CHECK_EQ((SWord)Math_Lib.high_arc_cos(32767), 81);
    CHECK_EQ((SWord)Math_Lib.high_arc_cos(-32767), 32687);
}

//------------------------------------------------------------------------------
// MathLib distance/intercept - distance3d is exact sqrt; Distance2d/
// Distance_Unsigned use the max+frac approximation (4993 vs 5000), with a
// >>15 range modifier for large inputs.
//------------------------------------------------------------------------------
static void test_mathlib_distance()
{
    CHECK_EQ(Math_Lib.distance3d(3000, 4000, 0), 5000);
    CHECK_EQ(Math_Lib.distance3d(0, 0, 0), 0);
    CHECK_EQ(Math_Lib.distance3d(-3000, -4000, 12000), 13000);
    CHECK_EQ(Math_Lib.distance3d(0, 0, 70000), 70000);

    // Approximate 2D distance: ~0.14% under for the 3-4-5 triangle.
    CHECK_EQ(Math_Lib.Distance_Unsigned(3000, 4000), 4993);
    CHECK_EQ(Math_Lib.Distance_Unsigned(0, 0), 0);
    CHECK_EQ(Math_Lib.Distance_Unsigned(5000, 0), 5000);
    CHECK_EQ(Math_Lib.Distance2d(3000, 4000), 4993);
    CHECK_EQ(Math_Lib.Distance2d(-3000, -4000), 4993);

    CHECK_EQ(Math_Lib.DistAbsSum(3, -4, 0), 7);
    CHECK_EQ(Math_Lib.DistAbsSum(-1, -2, -3, -4), 10);
    CHECK_EQ(Math_Lib.DistAbsSum(0, 0, 0, 0), 0);

    // Intercept: Range is exact 3D, heading is arctan(dx,dz).
    SLong rng; int hd, pt;
    Math_Lib.Intercept(100000, 0, 200000, rng, hd, pt);
    CHECK_EQ(rng, 223607);                       // sqrt(1e10+4e10)
    CHECK_EQ(hd, 4836);                          // atan2(1,2) = 26.57deg
    CHECK_EQ(pt, 0);
    Math_Lib.Intercept(0, 0, -50000, rng, hd, pt);
    CHECK_EQ(rng, 50000);
    CHECK_EQ(hd, 32768);                         // directly behind
    CHECK_EQ(pt, 0);

    // HighIntercept shares the heading but approximates range via the
    // lookup sin/cos - 2 units off on this vector.
    Math_Lib.HighIntercept(100000, 0, 200000, rng, hd, pt);
    CHECK_EQ(rng, 223609);
    CHECK_EQ(hd, 4836);
    CHECK_EQ(pt, 0);

    // InterceptHdg: approx 2D distance + HighArcTan heading.
    ULong d2; UWord h2;
    Math_Lib.InterceptHdg(1000, 2000, 3000, 5000, d2, h2);
    CHECK_EQ(d2, 3597);                          // sqrt(13e6) = 3605.5
    CHECK_EQ(h2, 6133);                          // atan2(2000,3000)=33.7deg
}

//------------------------------------------------------------------------------
// MathLib calendar/time - days are 1-based in MonthFromDays (day 31 is
// still January); DateFromSecs counts from day 1/month 0/year 0 (year 0
// is a leap year: 1461 days -> year 4).
//------------------------------------------------------------------------------
static void test_mathlib_datetime()
{
    SWord dm;
    CHECK_EQ(Math_Lib.MonthFromDays(0, dm, 0), 0);
    CHECK_EQ(Math_Lib.MonthFromDays(31, dm, 0), 0);     // Jan 31
    CHECK_EQ(Math_Lib.MonthFromDays(32, dm, 0), 1);     // Feb 1
    CHECK_EQ(Math_Lib.MonthFromDays(59, dm, 0), 1);     // Feb 28 (normal)
    CHECK_EQ(Math_Lib.MonthFromDays(60, dm, 0), 2);     // Mar 1 (normal)
    CHECK_EQ(Math_Lib.MonthFromDays(60, dm, 1), 1);     // Feb 29 (leap)
    CHECK_EQ(Math_Lib.MonthFromDays(334, dm, 0), 10);   // Nov 30
    CHECK_EQ(Math_Lib.MonthFromDays(335, dm, 0), 11);   // Dec 1
    CHECK_EQ(Math_Lib.MonthFromDays(365, dm, 0), 11);   // Dec 31
    Math_Lib.MonthFromDays(60, dm, 0);
    CHECK_EQ(dm, 59);                                   // days in full months

    SWord dy, mo, yr;
    Math_Lib.DateFromSecs(0, dy, mo, yr);
    CHECK_EQ(dy, 1); CHECK_EQ(mo, 0); CHECK_EQ(yr, 0);  // Jan 1, year 0
    Math_Lib.DateFromSecs(86400L * 59, dy, mo, yr);
    CHECK_EQ(dy, 1); CHECK_EQ(mo, 2); CHECK_EQ(yr, 0);  // Mar 1, year 0
    Math_Lib.DateFromSecs(86400L * 365, dy, mo, yr);
    CHECK_EQ(dy, 1); CHECK_EQ(mo, 0); CHECK_EQ(yr, 1);
    Math_Lib.DateFromSecs(86400L * 1461, dy, mo, yr);
    CHECK_EQ(dy, 1); CHECK_EQ(mo, 0); CHECK_EQ(yr, 4);  // 4-year leap cycle
    Math_Lib.DateFromSecs(86400L * (365 * 31 + 59), dy, mo, yr);
    CHECK_EQ(dy, 22); CHECK_EQ(mo, 1); CHECK_EQ(yr, 31);

    SWord hr, mn;
    Math_Lib.TimeFromSecs(0, hr, mn);
    CHECK_EQ(hr, 0); CHECK_EQ(mn, 0);
    Math_Lib.TimeFromSecs(3661, hr, mn);
    CHECK_EQ(hr, 1); CHECK_EQ(mn, 1);                   // drops seconds
    Math_Lib.TimeFromSecs(86399, hr, mn);
    CHECK_EQ(hr, 23); CHECK_EQ(mn, 59);

    SWord dfy, lyr;
    CHECK_EQ(Math_Lib.YearFromDays(0, dfy, lyr), 0);
    CHECK_EQ(Math_Lib.YearFromDays(365, dfy, lyr), 0);
    CHECK_EQ(Math_Lib.YearFromDays(1461, dfy, lyr), 4);
    CHECK_EQ(Math_Lib.YearFromSecs(0), 0);
    CHECK_EQ(Math_Lib.YearFromSecs(86400L * 365), 0);
    CHECK_EQ(Math_Lib.YearFromSecs(86400L * 11000), 30);

    // Sun angle: -90deg at midnight, +90deg at noon.
    ANGLES sun;
    Math_Lib.SunPosFromSecs(0, sun);
    CHECK_EQ((SWord)sun, -16384);
    Math_Lib.SunPosFromSecs(43200, sun);
    CHECK_EQ((SWord)sun, 16384);

    CHECK_EQ(Math_Lib.DofCampFromSecs(0, 0), 0);
    CHECK_EQ(Math_Lib.DofCampFromSecs(86400L * 100, 0), 100);
    CHECK_EQ(Math_Lib.DofCampFromSecs(86400L * 100, 86400L * 40), 60);
}

//------------------------------------------------------------------------------
// MathLib misc - a2iend parses FORWARD from the pointer, advancing it
// past consumed digits (name is misleading); the two-arg form decrements
// lengthdec per digit. rnd() is a table-driven generator seeded by
// statics bval=23/cval=54 - deterministic given identical state.
//------------------------------------------------------------------------------
static void test_mathlib_misc()
{
    // Forward parse, pointer stops on first non-digit.
    char s1[] = "42abc"; char *p = s1;
    CHECK_EQ(Math_Lib.a2iend(p), 42);
    CHECK_EQ(*p, 'a');
    char s2[] = "-42"; char *q = s2;
    CHECK_EQ(Math_Lib.a2iend(q), 0);                    // '-' is not a digit
    CHECK_EQ(q, s2);                                    // pointer unmoved
    char s3[] = "7x"; char *r = s3;
    CHECK_EQ(Math_Lib.a2iend(r), 7);
    char s4[] = ""; char *t = s4;
    CHECK_EQ(Math_Lib.a2iend(t), 0);

    // Two-arg form: lengthdec decremented per consumed digit.
    char s5[] = "15x3"; char *u = s5; ULong len = 5;
    CHECK_EQ(Math_Lib.a2iend(u, len), 15);
    CHECK_EQ(len, 3);
    CHECK_EQ(*u, 'x');
    char s6[] = "12345"; char *v = s6; ULong len2 = 5;
    CHECK_EQ(Math_Lib.a2iend(v, len2), 12345);
    CHECK_EQ(len2, 0);

    // Overflow wraps mod 2^32 (99999999999 = 0x174876E7FF -> 0x4876E7FF).
    char s7[] = "99999999999"; char *w = s7;
    CHECK_EQ(Math_Lib.a2iend(w), 1215752191UL);

    // rnd: deterministic given restored state. Snapshot the lookup table
    // + bval/cval, run a sequence, restore, verify identical repeat.
    UWord savedtab[MathLib::MAX_RND];
    for (int i = 0; i < MathLib::MAX_RND; i++) savedtab[i] = Math_Lib.GetRndLookUp(i);
    UWord svb = Math_Lib.Getbval(), svc = Math_Lib.Getcval();

    Math_Lib.Setbval(23); Math_Lib.Setcval(54); Math_Lib.ResetRndCount();
    SWord seq[8];
    for (int i = 0; i < 8; i++) seq[i] = (SWord)Math_Lib.rnd();
    // First warmup draw is rndlookup[23] + rndlookup[54].
    CHECK_EQ((SWord)((savedtab[23] + savedtab[54]) & 0xFFFF), seq[0]);

    // Warmup writes back into rndlookup (rndlookup[cval++]=b), so the
    // table itself must be restored for an identical replay.
    for (int i = 0; i < MathLib::MAX_RND; i++) Math_Lib.SetRndLookUp(i, savedtab[i]);
    Math_Lib.Setbval(23); Math_Lib.Setcval(54); Math_Lib.ResetRndCount();
    for (int i = 0; i < 8; i++)
        CHECK_EQ((SWord)Math_Lib.rnd(), seq[i]);        // same state, same seq

    // Restore caller's state.
    for (int i = 0; i < MathLib::MAX_RND; i++) Math_Lib.SetRndLookUp(i, savedtab[i]);
    Math_Lib.Setbval(svb); Math_Lib.Setcval(svc); Math_Lib.ResetRndCount();

    // rnd(M) is a signed 16.16 multiply-shift of the RndVal: result stays
    // strictly inside (-M, M) for positive M.
    for (int i = 0; i < 200; i++) {
        SLong v = (SLong)Math_Lib.rnd(1000);
        CHECK(v > -1000 && v < 1000);
    }
}

//------------------------------------------------------------------------------
// fileman/FileMan - the fake-dir mechanism lets callers register a real
// host path under a directory number (dirfakeblock+RUNTIME holds the dir
// name, namedirdir+128 the file name); namenumberedfile then composes
// "dir/file" inside namedirdir. fakefile always returns dirnum+8.
// translatedirlist parses "dirnum parentnum dirname" lines in place,
// compacting the name strings to the front of the buffer.
//------------------------------------------------------------------------------
static void test_fileman()
{
    // Real host dir + file the fake entries will point at.
    mkdir("/tmp/migfmtest", 0755);
    FILE* mk = fopen("/tmp/migfmtest/testfile.bin", "wb");
    fputs("HELLO12345", mk);
    fclose(mk);

    static char fakebuf[512];
    memset(fakebuf, 0, sizeof fakebuf);
    void* savedfake = File_Man.dirfakeblock;
    int savedassume = File_Man.assumefakedir;
    File_Man.dirfakeblock = fakebuf;

    File_Man.fakedir((FileNum)(5 << 8), (char*)"/tmp/migfmtest");
    CHECK_EQ((int)File_Man.direntries[5].driverfile, RCH_DIRBASE);
    CHECK_EQ((int)File_Man.direntries[5].parentdir, RAMCACHEHANDLEDIR);
    CHECK_EQ((int)File_Man.direntries[5].dirnameind, 200);   // RUNTIME
    CHECK_EQ(strcmp(fakebuf + 200, "/tmp/migfmtest"), 0);

    FileNum f = File_Man.fakefile((FileNum)(5 << 8), "testfile.bin");
    CHECK_EQ((int)f, 0x508);                                // dirnum(5) + 8
    CHECK_EQ(File_Man.assumefakedir, 5);

    string p = File_Man.namenumberedfile(f);
    CHECK_EQ(strcmp(p, "/tmp/migfmtest/testfile.bin"), 0);
    string p2 = File_Man.namenumberedfilelessfail(f);
    CHECK_EQ(strcmp(p2, "/tmp/migfmtest/testfile.bin"), 0);
    CHECK(File_Man.existnumberedfile(f) == TRUE);

    // opennumberedfile + raw I/O wrappers.
    FILE* fh = File_Man.opennumberedfile(f);
    CHECK(fh != NULL);
    CHECK_EQ((long)File_Man.getfilesize(fh), 10);
    File_Man.seekfilepos(fh, 5);
    char buf[16] = {0};
    CHECK_EQ((long)File_Man.readfileblock(fh, buf, 5), 5);
    CHECK_EQ(strcmp(buf, "12345"), 0);
    File_Man.closefile(fh);

    // Missing file in the same fake dir: naming works, exist fails.
    FileNum f2 = File_Man.fakefile((FileNum)(5 << 8), "missing.bin");
    CHECK_EQ((int)f2, 0x508);
    CHECK(File_Man.existnumberedfile(f2) == FALSE);
    string pm = File_Man.namenumberedfilelessfail(f2);
    CHECK_EQ(strcmp(pm, "/tmp/migfmtest/missing.bin"), 0);

    // Untouched dir (200): driverfile INVALIDFILENUM -> lessfail NULL,
    // exist FALSE.
    CHECK(File_Man.namenumberedfilelessfail((FileNum)(200 << 8)) == NULL);
    CHECK(File_Man.existnumberedfile((FileNum)(200 << 8)) == FALSE);

    // translatedirlist: "dirnum parentnum dirname". dir 8 self-parents
    // (-> RAMCACHEHANDLEDIR, allowed for dirnum<=16); dir 9 hangs off 8.
    // Name strings are compacted to the start of the same buffer.
    static char dbuf[128];
    strcpy(dbuf, "8 8 /tmp/tdir\n9 8 sub\n");
    void* dp = dbuf; ULong dl = strlen(dbuf);
    FILEMAN.currfilenum = (FileNum)0x9999;
    FileMan::translatedirlist(dp, dl);
    CHECK_EQ((int)FILEMAN.direntries[8].parentdir, RAMCACHEHANDLEDIR);
    CHECK_EQ((int)FILEMAN.direntries[8].driverfile, 0x9999);
    CHECK_EQ(strcmp(dbuf + (int)FILEMAN.direntries[8].dirnameind, "/tmp/tdir"), 0);
    CHECK_EQ((int)FILEMAN.direntries[9].parentdir, 8);
    CHECK_EQ(strcmp(dbuf + (int)FILEMAN.direntries[9].dirnameind, "sub"), 0);

    // Restore shared state.
    File_Man.direntries[5].driverfile = File_Man.direntries[8].driverfile =
        File_Man.direntries[9].driverfile = INVALIDFILENUM;
    File_Man.dirfakeblock = savedfake;
    File_Man.assumefakedir = savedassume;
    remove("/tmp/migfmtest/testfile.bin");
    rmdir("/tmp/migfmtest");
}

//------------------------------------------------------------------------------
// MATHASM bit ops - GNU replacements for the original bts/btr/btc/bsf/bsr
// inline asm. BITSET/BITRESET/BITCOMP return the PREVIOUS bit value (the
// carry flag), and mutate the operand in place.
//------------------------------------------------------------------------------
static void test_mathasm_bits()
{
    ULong w[2] = {0, 0};

    // bts returns old bit, sets new.
    CHECK_EQ(BITSET(w, 5), 0);
    CHECK_EQ(w[0], 0x20);
    CHECK_EQ(BITSET(w, 5), 1);              // already set -> reports 1
    CHECK_EQ(w[0], 0x20);
    CHECK_EQ(BITSET(w, 63), 0);             // crosses into w[1]
    CHECK_EQ(w[1], 0x80000000UL);
    CHECK_EQ(w[0], 0x20);

    // btr returns old bit, clears.
    CHECK_EQ(BITRESET(w, 5), 1);
    CHECK_EQ(w[0], 0);
    CHECK_EQ(BITRESET(w, 5), 0);            // already clear
    CHECK_EQ(BITRESET(w, 63), 1);
    CHECK_EQ(w[1], 0);

    // bt (read-only).
    w[0] = 0x21;
    CHECK_EQ(BITTEST(w, 0), 1);
    CHECK_EQ(BITTEST(w, 5), 1);
    CHECK_EQ(BITTEST(w, 1), 0);
    CHECK_EQ(BITTEST(w, 32), 0);

    // btc returns old bit, toggles.
    CHECK_EQ(BITCOMP(w, 3), 0);             // was clear -> sets
    CHECK_EQ(w[0], 0x29);
    CHECK_EQ(BITCOMP(w, 3), 1);             // was set -> clears
    CHECK_EQ(w[0], 0x21);

    // Immediate-value variants (no memory operand).
    CHECK_EQ(BITSETI(0, 7), 0x80);
    CHECK_EQ(BITRESETI(0xFF, 3), 0xF7);
    CHECK_EQ(BITCOMPI(0xFF, 3), 0xF7);
    CHECK_EQ(BITTESTI(0xFF, 3), 1);
    CHECK_EQ(BITTESTI(0xF7, 3), 0);
    // Index masks to 5 bits like the CPU shifter.
    CHECK_EQ(BITSETI(0, 39), 0x80);         // 39 & 31 = 7

    // bsf/bsr; zero input returns errcode (CPU leaves dest undefined).
    CHECK_EQ(BITSCANLOWEST(0, 99), 99);
    CHECK_EQ(BITSCANLOWEST(0x28, 9), 3);
    CHECK_EQ(BITSCANHIGHEST(0, 77), 77);
    CHECK_EQ(BITSCANHIGHEST(0x28, 9), 5);
    CHECK_EQ(BITSCANHIGHEST(0x80000000UL, 0), 31);
}

// Narrow MakeField storage: BITSET & friends are used on bitfields as small
// as 2 bytes (e.g. BoB QFDField).  A 32-bit bt* RMW writes 2 bytes past such
// a field — invisible to value checks because it writes back what it read.
// The field sits flush against a PROT_NONE page so a word-wide access
// faults instead of silently clobbering the neighbour.
static void test_mathasm_bits_narrow()
{
    long ps = sysconf(_SC_PAGESIZE);
    char* r = (char*)mmap(NULL, 2 * ps, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(r != MAP_FAILED);
    if (r == MAP_FAILED) return;
    CHECK_EQ(mprotect(r + ps, ps, PROT_NONE), 0);

    UWord* f = (UWord*)(r + ps - 2);
    *f = 0;

    CHECK_EQ(BITSET(f, 0), 0);
    CHECK_EQ(BITSET(f, 15), 0);
    CHECK_EQ(*f, 0x8001);
    CHECK_EQ(BITTEST(f, 15), 1);
    CHECK_EQ(BITTEST(f, 14), 0);
    CHECK_EQ(BITCOMP(f, 0), 1);
    CHECK_EQ(*f, 0x8000);
    CHECK_EQ(BITRESET(f, 15), 1);
    CHECK_EQ(*f, 0);

    munmap(r, 2 * ps);
}

//------------------------------------------------------------------------------
// MATHASM fixed-point mul/div - 64-bit intermediates like the original
// mul/imul + shrd/div sequences. Quirk pinned: MULSHSIN shifts the 64-bit
// product LOGICALLY (matches shrd semantics - high zeros shift in, not
// sign bits), and SHDIVSIN casts the divisor to int32 so divisors with
// bit31 set go negative vs the original unsigned div.
//------------------------------------------------------------------------------
static void test_mathasm_muldiv()
{
    CHECK_EQ(MULSHUNS(0x8000, 0x8000, 16), 0x4000UL);
    CHECK_EQ(MULSHUNS(0x40000, 0x40000, 16), 0x100000UL);
    CHECK_EQ((ULong)MULSHSIN(-0x8000, 0x8000, 16), 0xFFFFC000UL);
    // Negative product, extreme shift: 0xFFFFFFFFFFFF0000 >> 31 keeps
    // 0xFFFFFFFF - logical shift of the 64-bit product (shrd semantics).
    CHECK_EQ((ULong)MULSHSIN(-1, 65536, 31), 0xFFFFFFFFUL);

    CHECK_EQ(SHDIVUNS(1, 16, 2), 0x8000UL);
    CHECK_EQ(SHDIVUNS(1, 16, 0), 0);                    // div0 -> 0
    CHECK_EQ(SHDIVUNS(65536, 0, 65536), 1);
    CHECK_EQ((ULong)SHDIVSIN(-65536, 0, 2), (ULong)-32768);
    CHECK_EQ(SHDIVSIN(1, 16, 0), 0);                    // div0 -> 0
    // int32 divisor cast: 0xC0000000 becomes -1073741824, so
    // -2147483648 / -1073741824 = 2 (unsigned div would give 0).
    CHECK_EQ(SHDIVSIN((SLong)0x80000000, 0, 0xC0000000UL), 2);

    CHECK_EQ(MULDIVUNS(6, 7, 2), 21);
    CHECK_EQ(MULDIVUNS(0x8000, 0x8000, 3), 0x15555555UL); // 2^30/3
    CHECK_EQ(MULDIVSIN(-6, 7, 2), -21);
    CHECK_EQ(MULDIVSIN(6, -7, 2), -21);
    CHECK_EQ(MULDIVSIN(-6, -7, 2), 21);
}

//------------------------------------------------------------------------------
// MATHASM misc - sign extract/apply (the Pos/UseSign/AbsSign plumbing),
// repmovsd block copy, and the real i386 x87 control-word accessors.
//------------------------------------------------------------------------------
static void test_mathasm_misc()
{
    CHECK_EQ(mathlib_w_getsign(-5), -1);
    CHECK_EQ(mathlib_w_getsign(0), 0);
    CHECK_EQ(mathlib_w_getsign(5), 0);
    CHECK_EQ(mathlib_w_applysign(7, -1), -7);
    CHECK_EQ(mathlib_w_applysign(7, 0), 7);
    CHECK_EQ(mathlib_w_applysign(7, 1), 7);
    CHECK_EQ(mathlib_l_getsign(-9), -1);
    CHECK_EQ(mathlib_l_getsign(9), 0);
    CHECK_EQ(mathlib_l_applysign(7, -1), -7);
    CHECK_EQ(mathlib_l_applysign(-7, -1), 7);   // (a^s)-s restores abs

    ULong src[4] = {1, 2, 3, 4}, dst[4] = {0, 0, 0, 0};
    repmovsd(src, dst, 4);
    CHECK_EQ(dst[0], 1); CHECK_EQ(dst[3], 4);

    // x87 control word is real on i386 (0x37f = default extended prec).
    UWord saved = GETFPCW();
    CHECK_EQ(saved, 0x37F);
    CHECK_EQ(GETPREC(), 3);                     // bits 8-9: 64-bit ext prec
    SETPREC(2); CHECK_EQ(GETPREC(), 2);         // 53-bit double
    SETPREC(3); CHECK_EQ(GETPREC(), 3);         // restore
    SETFPCW(saved);
}

//------------------------------------------------------------------------------
// BITCOUNT.H flag macros - ONLYFIELD wraps a value with a typed proxy;
// the BEGIN_/FIRST_/BITFIELD/LAST_/END_BITFIELD_STRUCT family (the live
// one - BOOLFIELDS is dead: BOOLFIELD is never defined) unions `value`
// with one proxy per field, so fields alias the raw word like real
// bitfields (was: independent storage - 4x size, value disconnected -
// which bloated every serialized struct containing them).
//------------------------------------------------------------------------------
struct BitfieldBox {
    BEGIN_BITFIELD_STRUCT(Test16, UWord)
    FIRST_BITFIELD(UByte, low3, 2)          // proxy stores 3 bits
    BITFIELD(UByte, mid4, 3, 6)             // proxy stores 4 bits
    LAST_BITFIELD(UByte, top2, 7, 8)        // proxy stores 2 bits
    END_BITFIELD_STRUCT(Test16, UWord)
};

static void test_bitcount_macros()
{
    // ONLYFIELD: typed assign/read wrapper.
    struct OnlyBox { ONLYFIELD(UByte, int, flag); };
    OnlyBox o = {};
    o.flag = 7;
    CHECK_EQ((int)o.flag, 7);
    CHECK_EQ(o.flag.value, 7);

    BitfieldBox box = {};
    // All three proxies union over the single UWord `value`.
    CHECK_EQ(sizeof(box.Test16), 2);

    // Field proxies mask to their declared bit width.
    box.Test16.low3 = 0xFF;                 // 3-bit field
    CHECK_EQ((int)box.Test16.low3, 7);
    box.Test16.mid4 = 0x1F;                 // 4-bit field
    CHECK_EQ((int)box.Test16.mid4, 0xF);
    box.Test16.top2 = 0x7;                  // 2-bit field
    CHECK_EQ((int)box.Test16.top2, 3);

    // Fields alias `value` both directions - real bitfield semantics.
    CHECK_EQ(box.Test16.value, (7 | (0xF << 3) | (3 << 7)));   // 0x01FF
    box.Test16.value = 0xA5;                // raw write -> field reads
    CHECK_EQ((int)box.Test16.low3, 5);
    CHECK_EQ((int)box.Test16.mid4, 4);
    CHECK_EQ((int)box.Test16.top2, 1);
    box.Test16 = UWord(0xABCD);             // storage_t assign
    CHECK_EQ(box.Test16.value, 0xABCD);
    box.Test16.low3 = 0;                    // field write preserves others
    CHECK_EQ(box.Test16.value, 0xABC8);
}

//------------------------------------------------------------------------------
// matrix fixed-point path - MATRIX elements are SWord 2.15 (1.0 = 32766
// after the >>15 product quantization). generate uses interpolated
// high_sin_cos; generateh/p/r single-axis builders use raw 32767.
//------------------------------------------------------------------------------
static void test_matrix_int()
{
    matrix m;
    static MATRIX im;

    // Identity via the full generate path: diagonal quantizes to 32766.
    m.generate(ANGLES_0Deg, ANGLES_0Deg, ANGLES_0Deg, &im);
    CHECK_EQ(im.L11, 32766); CHECK_EQ(im.L22, 32766); CHECK_EQ(im.L33, 32766);
    CHECK_EQ(im.L12, 0); CHECK_EQ(im.L21, 0); CHECK_EQ(im.L31, 0);

    // Heading 90: forward (+z) maps to +x.
    m.generate(ANGLES_90Deg, ANGLES_0Deg, ANGLES_0Deg, &im);
    CHECK_EQ(im.L11, 0); CHECK_EQ(im.L31, -32766); CHECK_EQ(im.L13, 32766);
    CHECK_EQ(im.L22, 32766); CHECK_EQ(im.L33, 0);

    // Pitch 90 / roll 90 spot rows.
    m.generate(ANGLES_0Deg, ANGLES_90Deg, ANGLES_0Deg, &im);
    CHECK_EQ(im.L32, -32767); CHECK_EQ(im.L23, 32765);
    m.generate(ANGLES_0Deg, ANGLES_0Deg, ANGLES_90Deg, &im);
    CHECK_EQ(im.L12, 32766); CHECK_EQ(im.L21, -32766);

    // Single-axis builders keep the unquantized 32767.
    m.generateh(ANGLES_90Deg, &im);
    CHECK_EQ(im.L31, -32767); CHECK_EQ(im.L13, 32767); CHECK_EQ(im.L22, 32767);
    m.generatep(ANGLES_90Deg, &im);
    CHECK_EQ(im.L32, -32767); CHECK_EQ(im.L23, 32767); CHECK_EQ(im.L11, 32767);
    m.generater(ANGLES_90Deg, &im);
    CHECK_EQ(im.L12, 32767); CHECK_EQ(im.L21, -32767); CHECK_EQ(im.L33, 32767);

    // rotate: (0,0,32768) under h90 -> +x.
    m.generate(ANGLES_90Deg, ANGLES_0Deg, ANGLES_0Deg, &im);
    SLong x = 0, y = 0, z = 32768;
    m.rotate(&im, x, y, z);
    CHECK_EQ(x, 32766); CHECK_EQ(y, 0); CHECK_EQ(z, 0);

    // transform: identity maps v -> v*32766*2 (ASMTransform scaling),
    // clip flags 0. Large inputs shift right to fit 16 bits, cf != 0.
    m.generate(ANGLES_0Deg, ANGLES_0Deg, ANGLES_0Deg, &im);
    x = 100; y = 200; z = 300;
    CHECK_EQ(m.transform(&im, x, y, z), 0);
    CHECK_EQ(x, 6553200); CHECK_EQ(y, 13106400); CHECK_EQ(z, 19659600);
    x = 70000; y = 0; z = 0;
    CHECK_EQ(m.transform(&im, x, y, z), 2);
    CHECK_EQ(x, 1146810000);

    // scaleto16bit: bsr(max|v|)-14, returns shift applied to all three.
    x = 100; y = 200; z = 300;
    CHECK_EQ(m.scaleto16bit(x, y, z), 0);
    CHECK_EQ(x, 100); CHECK_EQ(z, 300);
    x = 70000; y = 200; z = 300;
    CHECK_EQ(m.scaleto16bit(x, y, z), 2);   // 70000 needs >>2 for <32768
    CHECK_EQ(x, 17500); CHECK_EQ(y, 50); CHECK_EQ(z, 75);
    x = 0; y = 0; z = 0;
    CHECK_EQ(m.scaleto16bit(x, y, z), 0);   // bsr(0) -> no shift

    // multiply: h90 squared = h180 (L11/L33 ~= -1).
    static MATRIX a;
    m.generate(ANGLES_90Deg, ANGLES_0Deg, ANGLES_0Deg, &a);
    static MATRIX b;
    b = a;
    m.multiply(&b, &a);                     // b = a * b
    CHECK_EQ(b.L11, -32765); CHECK_EQ(b.L33, -32765);
    CHECK_EQ(b.L31, 0); CHECK_EQ(b.L13, 0);

    // inverse of h90 == h270 (transposed signs on L31/L13).
    static MATRIX inv;
    m.inverse(ANGLES_90Deg, ANGLES_0Deg, ANGLES_0Deg, &inv);
    CHECK_EQ(inv.L31, 32766); CHECK_EQ(inv.L13, -32767);
}

//------------------------------------------------------------------------------
// GENERAL malloc wrappers - ailmalloc/radmalloc forward to malloc with
// the RAD/Miles PTR4 conventions; failure paths return NULL.
//------------------------------------------------------------------------------
void* ailmalloc(size_t numbytes);           // ailfree/rad* are commented out
static void test_malloc_wrappers()
{
    CHECK_EQ(ailmalloc(0), (void*)NULL);    // size 0 -> NULL, no EmitSysErr
    void* p = ailmalloc(128);
    CHECK(p != NULL);
    if (p) {
        memset(p, 0xAB, 128);               // usable memory
        delete[] (char*)p;
    }
}

//------------------------------------------------------------------------------
// CBuffer<T,size> - comms circular buffer. curr is the consume cursor,
// next the write cursor, temp a scratch walker. All three wrap at
// maxentries; entries counts unread records.
//------------------------------------------------------------------------------
static void test_cbuffer()
{
    CBuffer<int, 4> b;
    CHECK_EQ(b.NumEntries(), 0);
    CHECK(b.GetCurr() == b.GetNext());          // both at slot 0

    // Fill all four slots; next wraps back to slot 0.
    *b.GetNext() = 10; b.AddEntryAndUpdateNext();
    *b.GetNext() = 20; b.AddEntryAndUpdateNext();
    *b.GetNext() = 30; b.AddEntryAndUpdateNext();
    *b.GetNext() = 40; b.AddEntryAndUpdateNext();
    CHECK_EQ(b.NumEntries(), 4);
    CHECK(b.GetNext() == b.GetCurr());          // next wrapped to slot 0

    // Consume in FIFO order.
    CHECK_EQ(*b.GetCurr(), 10);
    b.UpdateCurr();
    CHECK_EQ(b.NumEntries(), 3);
    CHECK_EQ(*b.GetCurr(), 20);

    // Wrapped next overwrites the consumed slot.
    *b.GetNext() = 50; b.AddEntryAndUpdateNext();
    CHECK_EQ(b.NumEntries(), 4);
    b.UpdateCurr();                             // 20
    b.UpdateCurr();                             // 30
    b.UpdateCurr();                             // 40
    CHECK_EQ(*b.GetCurr(), 50);
    b.UpdateCurr();
    CHECK_EQ(b.NumEntries(), 0);

    // InitBuffer: drops pending entries, next re-joins curr.
    *b.GetNext() = 99; b.AddEntryAndUpdateNext();
    b.InitBuffer();
    CHECK_EQ(b.NumEntries(), 0);
    CHECK(b.GetNext() == b.GetCurr());

    // temp walker: forward wrap and TempPrev under-wrap.
    CBuffer<int, 4> w;
    w.AddEntryAndUpdateNext();                  // next=1
    w.AddEntryAndUpdateNext();                  // next=2
    w.AddEntryAndUpdateNext();                  // next=3
    int* slot3 = w.GetNext();
    w.AddEntryAndUpdateNext();                  // next wraps -> slot 0
    CHECK(w.GetNext() == w.GetCurr());

    w.SetTempCurr();                            // temp at slot 0
    w.TempPrev();                               // under-wrap -> slot 3
    CHECK(w.GetTemp() == slot3);
    w.UpdateTemp();                             // back to slot 0
    CHECK(w.GetTemp() == w.GetCurr());

    // SetTempNext follows the write cursor.
    w.SetTempNext();
    CHECK(w.GetTemp() == w.GetNext());
}

//------------------------------------------------------------------------------
// Angles fixed-point ops: (i + ANGLES_FRACT/2) >> ANGLES_SHIFT, i.e.
// round-to-nearest division by 32768. int/SLong/ULong overloads share
// the shift semantics; negative results rely on arithmetic >>.
//------------------------------------------------------------------------------
static void test_angles()
{
    CHECK_EQ(ANGLES_FRACT, 32768);
    CHECK_EQ(ANGLES_SHIFT, 15);
    CHECK_EQ(ANGLES_SHIFT_TWICE, 30);
    CHECK_EQ((int)ANGLES_90Deg, 0x4000);
    CHECK_EQ((int)ANGLES_180Deg, 0x8000);

    CHECK_EQ(32768 >> ANGLES_SHIFT, 1);         // 49152>>15 = 1
    CHECK_EQ(49152 / ANGLES_FRACT, 2);
    CHECK_EQ(16383 / ANGLES_FRACT, 0);          // .4999 -> 0
    CHECK_EQ(16385 / ANGLES_FRACT, 1);          // .5000x rounds up
    CHECK_EQ(16384 / ANGLES_FRACT, 1);          // exactly .5 -> 1

    // Negatives: the +16384 bias means -16384 is the round-to-zero
    // crossover; below it arithmetic >> goes to -1.
    CHECK_EQ(-16384 / ANGLES_FRACT, 0);
    CHECK_EQ(-16385 / ANGLES_FRACT, -1);
    CHECK_EQ(-32768 / ANGLES_FRACT, -1);        // (-16384)>>15
    CHECK_EQ(-32769 / ANGLES_FRACT, -1);        // (-16385)>>15 -> -1
    CHECK_EQ(-49153 / ANGLES_FRACT, -2);        // (-32769)>>15 -> -2

    // SLong / ULong variants apply the same bias+shift.
    SLong sl = 100000;
    ULong ul = 100000;
    CHECK_EQ(sl / ANGLES_FRACT, (100000 + 16384) >> 15);
    CHECK_EQ(sl >> ANGLES_SHIFT, (100000 + 16384) >> 15);
    CHECK_EQ(ul / ANGLES_FRACT, (ULong)((100000 + 16384) >> 15));
}

//------------------------------------------------------------------------------
// MAKEFIELD bitfield classes - used by SAVEGAME/IMAGEMAP/SHAPES for
// compact flag sets. |= set, %= clear, ^= toggle, [] test, unary +
// "any set", field |=/&=/^=/-= bulk ops, >= containment, =(0|-1) fill.
// Indices are offset by MIN; errorcheck is a compiled-out assert.
//------------------------------------------------------------------------------
enum TstBit16 { TB_MIN = 0, TB_MAX = 15 };
MAKEFIELD(TstBit16, TB_MIN, TB_MAX);
enum TstOff { TO_MIN = 5, TO_MAX = 20 };
MAKEFIELD(TstOff, TO_MIN, TO_MAX);

static void test_bitfield()
{
    TstBit16Field f;
    CHECK(!+f);                                 // empty: no bits set

    f |= TB_MIN;
    f |= TB_MAX;                                // bit 15 - second byte
    CHECK(+f);
    CHECK(f[TB_MIN]);
    CHECK(f[TB_MAX]);
    CHECK(!f[(TstBit16)1]);

    f ^= TB_MIN;                                // toggle set -> clear
    CHECK(!f[TB_MIN]);
    f ^= TB_MIN;                                // toggle clear -> set
    CHECK(f[TB_MIN]);
    f %= TB_MIN;                                // clear
    CHECK(!f[TB_MIN]);
    f %= TB_MIN;                                // clear again - no-op
    CHECK(!f[TB_MIN]);

    // Field-vs-field ops.
    TstBit16Field g;
    g |= TB_MAX;
    CHECK(f >= g);                              // f contains g's bits
    CHECK(g >= f);                              // equal sets contain each other
    g |= (TstBit16)3;
    CHECK(!(f >= g));                           // g now has a bit f lacks
    CHECK(g >= f);                              // ...while still containing f

    TstBit16Field h = f;                        // raw-copy copy ctor
    h &= g;                                     // {15} & {15,3} = {15}
    CHECK(h[TB_MAX]);
    CHECK(!h[(TstBit16)3]);
    h -= g;                                     // {15} - {15,3} = {}
    CHECK(!+h);

    // Fill via =(SLong): only 0 and -1 are legal.
    f = -1;
    CHECK(f[TB_MIN]);
    CHECK(f[(TstBit16)8]);
    CHECK(f[TB_MAX]);
    f = 0;
    CHECK(!+f);

    // Range ctor sets minset..maxset inclusive.
    TstBit16Field rf((TstBit16)3, (TstBit16)5);
    CHECK(!rf[(TstBit16)2]);
    CHECK(rf[(TstBit16)3]);
    CHECK(rf[(TstBit16)4]);
    CHECK(rf[(TstBit16)5]);
    CHECK(!rf[(TstBit16)6]);

    // Equality and <=.
    TstBit16Field e1; e1 |= (TstBit16)9;
    TstBit16Field e2; e2 |= (TstBit16)9;
    CHECK(e1 == e2);
    e2 |= (TstBit16)10;
    CHECK(!(e1 == e2));
    CHECK(e1 <= e2);                            // e2 contains e1
    CHECK(!(e2 <= e1));

    // MIN-offset field: index s-MIN maps onto bit 0.
    TstOffField of;
    of |= TO_MIN;                               // bit 0
    of |= TO_MAX;                               // bit 15
    CHECK(of[TO_MIN]);
    CHECK(of[TO_MAX]);
    CHECK(!of[(TstOff)(TO_MIN + 1)]);
    CHECK_EQ((int)TstOffField::RANGE, 16);
    CHECK_EQ((int)TstOffField::BYTES, 2);
}

//------------------------------------------------------------------------------
// CString shim (MigCstring.h) - single char* payload standing in for
// MFC CString. Covers construction, concat (incl. self-append), Mid/
// Left/Right edge counts, case/trim/replace/find, GetBuffer, Part-*
// splitting, Escape/UnEscape round-trip + malformed input, CSprintf.
//------------------------------------------------------------------------------
static void test_cstring()
{
    // Construction / assignment.
    CString s("Hello");
    CHECK_EQ(s.GetLength(), 5);
    CHECK(!s.IsEmpty());
    CString e;
    CHECK(e.IsEmpty());
    CHECK_EQ(strcmp(e.c_str(), ""), 0);
    CString r('x', 4);
    CHECK_EQ(strcmp(r.c_str(), "xxxx"), 0);
    CString r0('x', 0);
    CHECK(r0.IsEmpty());
    CString si(42);
    CHECK(si == "42");
    CString sn(-7);
    CHECK(sn == "-7");
    CString nul((const char*)NULL);             // NULL -> empty
    CHECK(nul.IsEmpty());
    s = s;                                      // self-assign safe
    CHECK(s == "Hello");

    // Concatenation.
    s += " World";
    CHECK(s == "Hello World");
    s += '!';
    CHECK_EQ(s.GetLength(), 12);
    s += s;                                     // self-append doubles
    CHECK(s == "Hello World!Hello World!");
    CString t = CString("a") + "b" + 'c';
    CHECK(t == "abc");
    CString t2 = "x" + CString("y");
    CHECK(t2 == "xy");
    CString nn("a"); nn += (const char*)NULL;   // NULL += no-op
    CHECK(nn == "a");

    // Comparison.
    CHECK(CString("abc") < CString("abd"));
    CHECK_EQ(CString("AbC").CompareNoCase("ABC"), 0);
    CHECK(CString("abc") == "abc");
    CHECK(CString("abc") != "abd");
    CHECK_EQ(CString("x").Compare("x"), 0);

    // GetAt/SetAt.
    CString ga("ab");
    CHECK_EQ(ga.GetAt(0), 'a');
    CHECK_EQ(ga.GetAt(1), 'b');
    ga.SetAt(1, 'Z');
    CHECK(ga == "aZ");

    // Mid/Left/Right - normal and edge counts.
    CString m("HelloWorld");
    CHECK(m.Mid(0, 5) == "Hello");
    CHECK(m.Mid(5) == "World");
    CHECK(m.Mid(2, 3) == "llo");
    CHECK(m.Mid(5, 0) == "");
    CHECK(m.Mid(50) == "");                     // first past end
    CHECK(m.Mid(-3, 4) == "Hell");              // first<0 clamps to 0
    CHECK(m.Mid(0, -1) == "HelloWorld");        // count<0 -> rest
    CHECK(m.Mid(7, -1) == "rld");
    CHECK(m.Mid(0, 99) == "HelloWorld");        // count clamps to len
    CHECK(m.Left(4) == "Hell");
    CHECK(m.Left(50) == "HelloWorld");
    CHECK(m.Left(-2) == "");                    // MFC: negative -> empty
    CHECK(m.Left(0) == "");
    CHECK(m.Right(5) == "World");
    CHECK(m.Right(50) == "HelloWorld");
    CHECK(m.Right(-2) == "");                   // MFC: negative -> empty
    CHECK(m.Right(0) == "");

    // Case / reverse.
    CString u("aBc");
    u.MakeUpper();  CHECK(u == "ABC");
    u.MakeLower();  CHECK(u == "abc");
    CString rv("abc");
    rv.MakeReverse(); CHECK(rv == "cba");
    CString rv0;
    rv0.MakeReverse(); CHECK(rv0.IsEmpty());    // empty-safe

    // Trim.
    CString tr("  hi \t\n");  tr.Trim();       CHECK(tr == "hi");
    CString trl("   x");      trl.TrimLeft();  CHECK(trl == "x");
    CString trr("x   ");      trr.TrimRight(); CHECK(trr == "x");
    CString tall("   \t");    tall.Trim();     CHECK(tall.IsEmpty());
    CString tempty;           tempty.Trim();   CHECK(tempty.IsEmpty());
    CString tmid("a b");      tmid.Trim();     CHECK(tmid == "a b");

    // Replace / Remove.
    CString rp("a-b-a");
    CHECK_EQ(rp.Replace('a', 'X'), 2);
    CHECK(rp == "X-b-X");
    CHECK_EQ(rp.Replace('q', 'z'), 0);
    CString rp2("aa.bb.aa");
    CHECK_EQ(rp2.Replace("aa", "c"), 2);
    CHECK(rp2 == "c.bb.c");
    CHECK_EQ(rp2.Replace("zz", "q"), 0);
    CString rp3("aaa");
    CHECK_EQ(rp3.Replace("aa", "b"), 1);        // non-overlapping
    CHECK(rp3 == "ba");
    CString rm("a.b.c");
    CHECK_EQ(rm.Remove('.'), 2);
    CHECK(rm == "abc");
    CHECK_EQ(rm.Remove('x'), 0);

    // Find / ReverseFind.
    CString fnd("a.b.c.b");
    CHECK_EQ(fnd.Find('.'), 1);
    CHECK_EQ(fnd.Find('.', 2), 3);
    CHECK_EQ(fnd.Find('.', 4), 5);
    CHECK_EQ(fnd.Find('.', 6), -1);             // past last
    CHECK_EQ(fnd.Find('z'), -1);
    CHECK_EQ(fnd.ReverseFind('.'), 5);
    CHECK_EQ(fnd.ReverseFind('z'), -1);
    CHECK_EQ(fnd.Find("b.c"), 2);               // "a.b.c.b": match at 2
    CHECK_EQ(fnd.Find("b.c", 4), -1);
    CHECK_EQ(fnd.Find("zz"), -1);
    CHECK_EQ(CString("").Find('x'), -1);

    // GetBuffer/ReleaseBuffer - grow then publish.
    CString gb("hi");
    char* bp = gb.GetBuffer(16);
    strcpy(bp, "changed");
    gb.ReleaseBuffer();
    CHECK(gb == "changed");
    CString gb2("hi");
    gb2.GetBuffer();                            // no-grow path
    CHECK(gb2 == "hi");
    CString gb3("abc");
    char* bp3 = gb3.GetBuffer(8);
    bp3[0] = 'Z';
    gb3.ReleaseBuffer(1);                       // explicit new length
    CHECK(gb3 == "Z");

    // Part/PartCount/PartBegin - comma-split helpers.
    CString p1("a,b,c");
    CHECK_EQ(p1.PartCount(','), 3);
    CHECK(p1.Part(',', 0) == "a");
    CHECK(p1.Part(',', 1) == "b");
    CHECK(p1.Part(',', 2) == "c");
    CHECK(p1.Part(',', 3).IsEmpty());           // out of range
    CHECK_EQ(p1.PartBegin(',', 0), 0);
    CHECK_EQ(p1.PartBegin(',', 1), 2);
    CHECK_EQ(p1.PartBegin(',', 2), 4);
    CHECK_EQ(p1.PartBegin(',', 3), -1);
    CHECK_EQ(CString("").PartCount(','), 0);
    CHECK_EQ(CString("abc").PartCount(','), 1); // no delimiter
    CString p2("a,");
    CHECK_EQ(p2.PartCount(','), 2);
    CHECK(p2.Part(',', 1).IsEmpty());           // trailing empty part
    CString p3(",a");
    CHECK(p3.Part(',', 0).IsEmpty());           // leading empty part
    CHECK(p3.Part(',', 1) == "a");

    // Escape/UnEscape round-trip + malformed input passes through.
    CString es("a b&c~d");
    es.Escape();
    CHECK(es == "a%20b%26c~d");
    es.UnEscape();
    CHECK(es == "a b&c~d");
    CString e41("%41");   e41.UnEscape();   CHECK(e41 == "A");
    CString e25("100%25"); e25.UnEscape();  CHECK(e25 == "100%");
    CString b1("%2G");    b1.UnEscape();    CHECK(b1 == "%2G");
    CString b2("%zz");    b2.UnEscape();    CHECK(b2 == "%zz");
    CString b3("%");      b3.UnEscape();    CHECK(b3 == "%");
    CString b4("%4");     b4.UnEscape();    CHECK(b4 == "%4");
    CString b5("%%41");   b5.UnEscape();    CHECK(b5 == "%A");
    CString b6("a%b");    b6.UnEscape();    CHECK(b6 == "a%b");

    // Format + CSprintf (CString args are unwrapped to const char*).
    CString fm;
    fm.Format("%d-%s", 7, "x");
    CHECK(fm == "7-x");
    CString sp = CSprintf("%s-%d", CString("x"), 5);
    CHECK(sp == "x-5");
    CString sp2 = CSprintf("%03d|%s", 7, "ab");
    CHECK(sp2 == "007|ab");
}

//------------------------------------------------------------------------------
// BIStream/BOStream - binary file streams where << and >> are remapped
// to raw read/write (used by the savegame formats). char* transfers use
// strlen, so a read needs a preloaded buffer of the right length.
//------------------------------------------------------------------------------
static void test_bstream()
{
    static char fn[] = "/tmp/migbstream.bin";

    {
        BOStream o(fn);
        SLong a = -12345;
        UWord w = 0xBEEF;
        char c = 'Q';
        o << a << w << c;
        o << "PAYLOAD";                          // strlen bytes, no NUL
        short arr[3] = {1, -2, 3};
        o.write(arr, 3);                         // element count
        UByte ub[4] = {9, 8, 7, 6};
        o.write(ub, 4);
    }

    {
        BIStream i(fn);
        CHECK(i.is_open());
        SLong a = 0; UWord w = 0; char c = 0;
        i >> a >> w >> c;
        CHECK_EQ(a, -12345);
        CHECK_EQ(w, 0xBEEF);
        CHECK_EQ(c, 'Q');

        // >> (char*) reads strlen(preloaded) bytes.
        char buf[8] = "1234567";                 // 7-char preload
        i >> buf;
        CHECK_EQ(strcmp(buf, "PAYLOAD"), 0);

        short arr[3] = {0, 0, 0};
        i.read(arr, 3);                          // reads 3*sizeof(short)
        CHECK_EQ(arr[0], 1);
        CHECK_EQ(arr[1], -2);
        CHECK_EQ(arr[2], 3);

        UByte ub[4] = {0, 0, 0, 0};
        i.read(ub, 4);                           // raw byte count
        CHECK(ub[0] == 9 && ub[3] == 6);

        // Reading past EOF sets fail.
        char extra = 0;
        i >> extra;
        CHECK(i.fail());
    }

    // Missing file -> stream not open.
    {
        BIStream missing("/tmp/migbstream-absent.bin");
        CHECK(!missing.is_open());
    }

    // BSTREAM's own DeleteFile(char*) -> Bool TRUE on success.
    CHECK(DeleteFile(fn) == TRUE);
    CHECK(DeleteFile(fn) == FALSE);              // already gone
}

//-------------------- LBM / IFF helpers (Graphics/LBM.CPP) --------------------
// LBM::MakeBody encodes each row as PackBits-style chunks: header byte
// >0x80 = run of 1-(SByte)b copies of the next byte, <0x80 = literal of
// 1+b bytes, 0x80 = transparent run (SkipRow-only; the encoder never
// emits it). Decode here mirrors Graphic::SkipRow's accounting.

// Free functions in LBM.CPP with no header declaration.
extern void Put68KWord(FILE* fp, UWord size);
extern void Put68KLong(FILE* fp, int size);

static const UByte* lbm_decode_row(const UByte* c, UByte* out, SLong width)
{
    SLong got = 0;
    while (got < width) {
        UByte ub = *c++;
        if (ub > 0x80) {
            SLong n = 1 - (SByte)ub;
            UByte v = *c++;
            for (SLong i = 0; i < n; i++) out[got++] = v;
        } else if (ub < 0x80) {
            SLong n = 1 + (SByte)ub;
            for (SLong i = 0; i < n; i++) out[got++] = *c++;
        } else {
            SLong n = 1 + *c++;                  // transparent run
            got += n;
        }
    }
    return c;
}

static void lbm_check_roundtrip(const UByte* pixels, SLong w, SLong h, SLong stride)
{
    LBM l;
    l.MakeHead((short)w, (short)h);
    ULong encLen = l.MakeBody((LogicalPtr)pixels, (int)stride);
    CHECK(encLen > 0);
    CHECK(encLen <= (ULong)w * (ULong)h * 2);    // body buffer is w*h*2

    // Structural: SkipRow must consume exactly width pixels per row and
    // land exactly on encLen — a run leaking into the next row shows up
    // as a stride error here.
    const UByte* p = l.body;
    for (SLong y = 0; y < h; y++) {
        const UByte* next = Graphic::SkipRow((UByte*)p, (SWord)w);
        CHECK(next > p);
        p = next;
    }
    CHECK_EQ((SLong)(p - l.body), (SLong)encLen);

    // Pixel-exact decode of every row.
    static UByte dec[128 * 128];
    CHECK(w * h <= (SLong)sizeof dec);
    p = l.body;
    for (SLong y = 0; y < h; y++)
        p = lbm_decode_row(p, &dec[(size_t)y * w], w);
    for (SLong y = 0; y < h; y++)
        CHECK(memcmp(&dec[(size_t)y * w], &pixels[(size_t)y * stride], w) == 0);
}

static void test_lbm()
{
    // 1) Uniform image: each row is a single max-length run chain
    //    (127-run cap + remainder).
    {
        static UByte img[8 * 8];
        memset(img, 0x5A, sizeof img);
        lbm_check_roundtrip(img, 8, 8, 8);

        LBM l; l.MakeHead(8, 8);
        ULong n = l.MakeBody(img, 8);
        CHECK_EQ(n, (ULong)(8 * 2));             // 8 rows * (hdr + value)
        CHECK_EQ(l.body[0], (UByte)249);         // run of 8: 257-8
        CHECK_EQ(l.body[1], (UByte)0x5A);
    }

    // 2) Alternating bytes: no runs at all — worst-case literal stream.
    {
        static UByte img[16 * 4];
        for (int i = 0; i < (int)sizeof img; i++) img[i] = (UByte)(i & 1);
        lbm_check_roundtrip(img, 16, 4, 16);
    }

    // 3) Row-boundary trap: last byte of a row equals the first byte of
    //    the next row. The old code probed [x+1] at x==width-1 — reading
    //    the next row (harmless mid-buffer, OOB on the last row). Encode
    //    must not start a cross-row run.
    {
        static UByte img[4 * 3] = {
            9, 9, 9, 7,     // row0 ends 7
            7, 2, 2, 2,     // row1 starts 7 — same byte, NOT same row
            5, 5, 5, 7,     // row2 ends 7 — [x+1] is past the buffer
        };
        lbm_check_roundtrip(img, 4, 3, 4);
    }

    // 4) Runs wider than the 127 cap split into chained run blocks.
    {
        static UByte img[130 * 2];
        memset(img, 0x11, sizeof img);
        lbm_check_roundtrip(img, 130, 2, 130);

        LBM l; l.MakeHead(130, 1);
        ULong n = l.MakeBody(img, 130);
        // row = 127-run + 3-run -> 4 bytes
        CHECK_EQ(n, (ULong)4);
        CHECK_EQ(l.body[0], (UByte)130);         // 257-127
        CHECK_EQ(l.body[2], (UByte)254);         // 257-3
        CHECK_EQ(l.body[3], (UByte)0x11);
    }

    // 5) Degenerate 1xN: no adjacent pair ever — every pixel a 1-literal.
    {
        static UByte img[1 * 5] = {3, 3, 3, 3, 3};
        lbm_check_roundtrip(img, 1, 5, 1);

        LBM l; l.MakeHead(1, 5);
        ULong n = l.MakeBody(img, 5);
        CHECK_EQ(n, (ULong)(5 * 2));
        CHECK_EQ(l.body[0], (UByte)0);           // literal of 1
        CHECK_EQ(l.body[1], (UByte)3);
    }

    // 6) Stride > width: row data must come from the strided source,
    //    not packed contiguity.
    {
        static UByte img[6 * 3];
        memset(img, 0xFF, sizeof img);           // padding bytes
        for (int y = 0; y < 3; y++)
            memset(&img[y * 6], (UByte)(0x20 + y), 4);
        lbm_check_roundtrip(img, 4, 3, 6);
    }

    // 7) Big-endian field readers on a synthetic buffer.
    {
        UByte buf[] = {0x12, 0x34, 0xAB, 0xCD, 0xEF, 0x01};
        SWord w = 0; SLong v = 0;
        UByte* p = Graphic::ReadWord(buf, w);
        CHECK_EQ(w, (SWord)0x1234);
        CHECK(p == buf + 2);
        p = Graphic::ReadLong(p, v);
        CHECK_EQ(v, (SLong)0xABCDEF01);
        CHECK(p == buf + 6);
    }

    // 8) IFF chunk walkers: FORM > CMAP + BMHD + BODY. Chunk stride is
    //    len+8 with no even-padding — pinned, that is the shipped quirk.
    {
        static UByte iff[] = {
            'F','O','R','M', 0, 0, 0, 38,
            'P','B','M',' ',
            'C','M','A','P', 0, 0, 0, 6, 1, 2, 3, 4, 5, 6,
            'B','M','H','D', 0, 0, 0, 4, 9, 9, 9, 9,
            'B','O','D','Y', 0, 0, 0, 2, 7, 7,
        };
        UByte* bmhd = Graphic::SearchIFFHunk((UByte*)"BMHD", iff);
        CHECK(bmhd == iff + 34);                 // 8+4(type)+12+8
        CHECK_EQ(bmhd[0], (UByte)9);
        CHECK(Graphic::SearchIFFHunk((UByte*)"BODY", iff) == iff + 46);
        CHECK(Graphic::SearchIFFHunk((UByte*)"XXXX", iff) == 0);
        // Not a FORM buffer at all -> fail-safe null.
        CHECK(Graphic::SearchIFFHunk((UByte*)"BMHD", (UByte*)"NOPE") == 0);

        IFFHunkSearch s[2];
        s[0].searchVal = *(ULong*)"CMAP"; s[0].hunkPtr = nullptr;
        s[1].searchVal = *(ULong*)"BMHD"; s[1].hunkPtr = nullptr;
        CHECK_EQ(Graphic::SearchIFFHunks(2, s, iff), (SLong)2);
        CHECK(s[0].hunkPtr == iff + 20);         // CMAP data
        CHECK(s[1].hunkPtr == iff + 34);         // BMHD data

        // Absent hunk -> found count stops short.
        s[0].hunkPtr = nullptr; s[1].hunkPtr = nullptr;
        s[1].searchVal = *(ULong*)"NOPE";
        CHECK_EQ(Graphic::SearchIFFHunks(2, s, iff), (SLong)1);
        CHECK(s[0].hunkPtr == iff + 20);
        CHECK(s[1].hunkPtr == nullptr);
    }

    // 9) Put68KWord/Long write big-endian to the file.
    {
        FILE* f = tmpfile();
        CHECK(f != nullptr);
        if (!f) return;
        Put68KWord(f, 0xABCD);
        Put68KLong(f, 0x01020304);
        rewind(f);
        CHECK_EQ(fgetc(f), 0xAB);
        CHECK_EQ(fgetc(f), 0xCD);
        CHECK_EQ(fgetc(f), 0x01);
        CHECK_EQ(fgetc(f), 0x02);
        CHECK_EQ(fgetc(f), 0x03);
        CHECK_EQ(fgetc(f), 0x04);
        fclose(f);
    }
}

//-------------------- CD-file seek state machine (Files/WINFILE.CPP) ----------
// loadCDfile emulates CD-ROM seek latency: a request first posts a seek
// (seekingtoposition countdown), then a pending-read flag on currindex,
// then advances the head in SeekStep-sized skip reads until
// deltaseekpos <= SeekStep, finally returning the sector-rounded block.
// readblockbuffer is the global landing zone (read itself is sector-
// rounded to 2048).
extern char readblockbufferbase[65536];
extern char* readblockbuffer;

static void test_cdfile()
{
    // A real 16 KB payload file standing in for the .dat image.
    mkdir("/tmp/migfmtest", 0755);
    static char cfn[] = "/tmp/migfmtest/cd0.dat";
    FILE* mk = fopen(cfn, "wb");
    CHECK(mk != nullptr);
    if (!mk) return;
    for (int i = 0; i < 16384; i++) fputc(i & 0xFF, mk);
    fclose(mk);

    int fd = open(cfn, O_RDONLY);
    CHECK(fd >= 0);
    if (fd < 0) return;
    HANDLE wh = (HANDLE)(intptr_t)fd;

    // Wire the cd table: entry 0 = our file, entry 1 unused.
    FileNum F = (FileNum)0x600;
    File_Man.cdfiles[0].number = F;
    File_Man.cdfiles[0].winhandle = wh;
    File_Man.cdfiles[0].maxfilesize = 16384;
    File_Man.cdfiles[0].handle = nullptr;
    File_Man.cdfiles[1].number = INVALIDFILENUM;
    File_Man.cdfiles[1].winhandle = INVALID_HANDLE_VALUE;
    File_Man.cdfile->number = INVALIDFILENUM;
    File_Man.cdfile->winhandle = INVALID_HANDLE_VALUE;
    File_Man.cdfile->currindex = 0;
    File_Man.cdfile->actualindex = -1;
    File_Man.cdfile->seekingtoposition = 0;
    File_Man.driveletter = 'C';                 // nonzero: resetCDfile path
    SWord sav_delay = Save_Data.SeekingDelay, sav_step = Save_Data.SeekStep;
    Save_Data.SeekingDelay = 2;
    Save_Data.SeekStep = 2048;

    // --- forward seek, from a cold table ---------------------------------
    CHECK(File_Man.loadCDfile(F, 100, 0x400, TRUE) == nullptr);
    CHECK_EQ(File_Man.cdfile->seekingtoposition, 2);      // seek posted
    CHECK_EQ(File_Man.cdfile->currindex, 0x400);
    CHECK_EQ(File_Man.cdfile->actualindex, 0x400);
    CHECK_EQ(File_Man.cdfile->number, F);                 // adopted entry

    CHECK(File_Man.loadCDfile(F, 100, 0x400, TRUE) == nullptr);
    CHECK_EQ(File_Man.cdfile->seekingtoposition, 1);      // countdown
    CHECK(File_Man.loadCDfile(F, 100, 0x400, TRUE) == nullptr);
    CHECK_EQ(File_Man.cdfile->seekingtoposition, 0);

    // Delay over -> posts the pending-read flag on currindex.
    CHECK(File_Man.loadCDfile(F, 100, 0x400, TRUE) == nullptr);
    CHECK(File_Man.cdfile->currindex & 0x80000000);
    CHECK_EQ(File_Man.cdfile->currindex & 0x7FFFFFFF, 0x400);

    // deltaseekpos == 0 <= SeekStep -> completes immediately.
    UByte* got = (UByte*)File_Man.loadCDfile(F, 100, 0x400, TRUE);
    CHECK(got == (UByte*)readblockbuffer);
    CHECK_EQ(got[0], (UByte)0x00);                        // file[0x400] = 0
    CHECK_EQ(got[4], (UByte)0x04);
    CHECK_EQ(got[255], (UByte)0xFF);
    // "nobody cares" quirk: head stays at the block start, not +length.
    CHECK_EQ(File_Man.cdfile->currindex, 0x400);
    CHECK_EQ(File_Man.cdfile->actualindex, 0x400);

    // --- stepped skip-read seek ------------------------------------------
    // offset 0x2400: delta 0x2000 > SeekStep -> skip reads until within
    // one step, then complete at the requested index.
    CHECK(File_Man.loadCDfile(F, 50, 0x2400, TRUE) == nullptr);
    CHECK(File_Man.cdfile->currindex & 0x80000000);
    int polls = 0;
    UByte* g2 = nullptr;
    for (; polls < 32; polls++) {
        g2 = (UByte*)File_Man.loadCDfile(F, 50, 0x2400, TRUE);
        if (g2) break;
    }
    CHECK(g2 == (UByte*)readblockbuffer);
    CHECK(polls > 0);                                    // needed skip steps
    CHECK(polls <= 16);                                  // bounded walk
    CHECK_EQ(g2[0], (UByte)0x00);                        // file[0x2400]
    CHECK_EQ(File_Man.cdfile->currindex, 0x2400);        // completes AT index

    // --- backward seek re-arms the whole machine --------------------------
    CHECK(File_Man.loadCDfile(F, 20, 0x100, TRUE) == nullptr);
    CHECK_EQ(File_Man.cdfile->seekingtoposition, 2);      // fresh seek
    CHECK_EQ(File_Man.cdfile->actualindex, 0x100);
    File_Man.cdfile->seekingtoposition = 0;               // fast-forward delay
    CHECK(File_Man.loadCDfile(F, 20, 0x100, TRUE) == nullptr);
    UByte* g3 = (UByte*)File_Man.loadCDfile(F, 20, 0x100, TRUE);
    CHECK(g3 == (UByte*)readblockbuffer);
    CHECK_EQ(g3[0], (UByte)0x00);                        // file[0x100]

    // --- pingCD: housekeeping honours seekingtoposition + PingStep ------
    SWord sav_pingd = Save_Data.PingDelay, sav_pingstep = Save_Data.PingStep;
    Save_Data.PingDelay = 0;                   // pingcount++ always passes
    Save_Data.PingStep = 2048;
    File_Man.cdfile->seekingtoposition = 1;
    File_Man.cdfile->actualindex = 0;
    File_Man.cdfile->maxfilesize = 16384;
    File_Man.cdfile->winhandle = wh;
    File_Man.cdfile->number = F;

    File_Man.pingCD();
    CHECK_EQ(File_Man.cdfile->seekingtoposition, 0);   // countdown wins
    CHECK_EQ(File_Man.cdfile->actualindex, 0);         // no read yet
    File_Man.pingCD();
    CHECK_EQ(File_Man.cdfile->actualindex, 2048);      // += PingStep per ping
    File_Man.pingCD();
    CHECK_EQ(File_Man.cdfile->actualindex, 4096);

    // Past maxfilesize -> wraps the head back ~1 MB and re-arms the seek.
    File_Man.cdfile->actualindex = 16384;
    File_Man.pingCD();
    CHECK_EQ(File_Man.cdfile->actualindex, 16384 - 1024 * 1024);
    CHECK_EQ(File_Man.cdfile->seekingtoposition, Save_Data.SeekingDelay);

    Save_Data.PingDelay = sav_pingd;
    Save_Data.PingStep = sav_pingstep;

    // --- skipread=FALSE: immediate synchronous read -----------------------
    // (the seekingtoposition countdown still gates this path — drain it)
    File_Man.cdfile->seekingtoposition = 0;
    UByte* g4 = (UByte*)File_Man.loadCDfile(F, 20, 0x800, FALSE);
    CHECK(g4 == (UByte*)readblockbuffer);
    CHECK_EQ(g4[0], (UByte)0x00);                        // file[0x800]
    // synchronous path advances the head past the block (unlike quirk).
    CHECK_EQ(File_Man.cdfile->currindex, 0x800 + 20);
    CHECK_EQ(File_Man.cdfile->actualindex, 0x800 + 20);

    close(fd);
    Save_Data.SeekingDelay = sav_delay;
    Save_Data.SeekStep = sav_step;
    File_Man.driveletter = 0;
    File_Man.cdfile->number = INVALIDFILENUM;
    File_Man.cdfiles[0].number = INVALIDFILENUM;
    File_Man.cdfiles[0].winhandle = INVALID_HANDLE_VALUE;
}

//---------------- GLOBREFS bitfield access (Bfields/GLOBREFS.CPP) ----------
// Persons2::SetLoc/GetLoc read and write masked bitfields inside the ULongs
// that BFieldGlobalTable references. The table's fieldsize/fieldshift are
// filled by the generated pass, so the test injects its own entry (the
// struct is mirrored - MAKEBF.H pulls in FLYINIT.H, too heavy here; both
// TUs are under DOSDEFS pack(1) so the layout is identical).
struct MirrorGlobalRef { void* ref; UWord fsize:5, fshift:5, pindx:5, spare:1; };
extern MirrorGlobalRef bf_globtable[] __asm__("BFieldGlobalTable");
extern void globref_SetLoc(int,int) __asm__("_ZN8Persons26SetLocEii");
extern int& globref_GetLoc(int)     __asm__("_ZN8Persons26GetLocEi");

static void test_globrefs()
{
    ULong target = 0;
    MirrorGlobalRef saved = bf_globtable[0];
    bf_globtable[0].ref = &target;

    // fieldsize N exposes N-1 value bits: imask = ((1<<N)>>1)-1.
    bf_globtable[0].fsize = 16; bf_globtable[0].fshift = 0;
    globref_SetLoc(0, 0x1234);
    CHECK_EQ(target, 0x1234UL);
    CHECK_EQ(globref_GetLoc(0), 0x1234);
    // Values truncate to the field width (15 bits for size 16).
    globref_SetLoc(0, 0x1FFFF);
    CHECK_EQ(target, 0x7FFFUL);
    CHECK_EQ(globref_GetLoc(0), 0x7FFF);
    // Bits outside the field are preserved.
    target = 0xFFFF0000;
    globref_SetLoc(0, 5);
    CHECK_EQ(target, 0xFFFF0005UL);

    // Shifted field: fsize 8 at bit 8 occupies bits 8..14.
    bf_globtable[0].fsize = 8; bf_globtable[0].fshift = 8;
    target = 0xFFFF00FF;
    globref_SetLoc(0, 0x55);
    CHECK_EQ(target, 0xFFFF55FFUL);
    CHECK_EQ(globref_GetLoc(0), 0x55);
    // Signed stores mask to the raw field: -5 in 7 bits reads back 123,
    // GetLoc does NOT sign-extend.
    globref_SetLoc(0, -5);
    CHECK_EQ(globref_GetLoc(0), 123);
    CHECK_EQ((target >> 8) & 0x7F, 123UL);

    // fsize 0 degenerates to a full 32-bit field (imask = 0xFFFFFFFF).
    bf_globtable[0].fsize = 0; bf_globtable[0].fshift = 0;
    globref_SetLoc(0, (int)0x89ABCDEF);
    CHECK_EQ(target, 0x89ABCDEFUL);
    CHECK_EQ((ULong)globref_GetLoc(0), 0x89ABCDEFUL);

    // fsize 1 -> imask 0 -> the write is a no-op, the read is always 0.
    bf_globtable[0].fsize = 1;
    target = 0xFFFFFFFF;
    globref_SetLoc(0, 99);
    CHECK_EQ(target, 0xFFFFFFFFUL);               // field width 0
    CHECK_EQ(globref_GetLoc(0), 0);

    // NULL reference -> GetLoc returns BAD_RV (0x80000000). SetLoc would
    // write through NULL - not exercised.
    bf_globtable[0].ref = NULL;
    CHECK_EQ(globref_GetLoc(0), (int)INT32_MIN);

    bf_globtable[0] = saved;
}

//---------------- FILEMAN residuals: dupandrepointtxt + retranslatedirlist -
// dupandrepointtxt strdup's into the caller's own variable (leak-by-
// contract: callers never free). retranslatedirlist re-parses the same
// "dirnum [parentnum] name" lines and compacts the name text to the
// buffer front without touching direntries.
extern string dupandrepointtxt(string&);
void       retranslatedirlist(void*&, ULong&);   // friend decl'd in FILEMAN.H

static void test_files_misc()
{
    char* p = (char*)"hello";
    char* orig = p;
    char* q = dupandrepointtxt(p);
    CHECK(q == p);                          // returns repointed src
    CHECK(p != orig);                       // caller var now owns a copy
    CHECK_EQ(strcmp(p, "hello"), 0);
    delete[] p;

    // Retranslate: names compacted to the buffer start, quoted names
    // lose their quotes.
    static char rbuf[128];
    strcpy(rbuf, "8 8 /tmp/tdir\n9 8 \"sub dir\"\n");
    void* rp = rbuf; ULong rl = strlen(rbuf);
    retranslatedirlist(rp, rl);
    CHECK_EQ(strcmp(rbuf, "/tmp/tdir"), 0);
    CHECK_EQ(strcmp(rbuf + strlen("/tmp/tdir") + 1, "sub dir"), 0);

    // Single field (no parentnum) and leading garbage skip.
    strcpy(rbuf, "junk\n8 sub\n");
    rp = rbuf; rl = strlen(rbuf);
    retranslatedirlist(rp, rl);
    CHECK_EQ(strcmp(rbuf, "sub"), 0);
}

// Graphic::CompOutCode / Comp3DOutCode - Cohen-Sutherland region codes
// against the Physical* clip rect. CompOutCode keys on Min/Max fields;
// Comp3DOutCode uses 0..PhysicalWidth/Height. Edge values count as
// inside; the bit layout is top=8 bottom=4 right=2 left=1.
static void test_compoutcode()
{
    Bool unp = FALSE;
    static IntensityIndex ii;
    void* pal = nullptr;
    FontPtr fp = nullptr, mp = nullptr;
    Graphic g(unp, &ii, pal, fp, mp);

    g.PhysicalMinX = 10; g.PhysicalMaxX = 100;
    g.PhysicalMinY = 20; g.PhysicalMaxY = 200;

    OutCode c = g.CompOutCode(50, 50);           // interior
    CHECK_EQ(c.all, 0u);
    c = g.CompOutCode(50, 201);                  // below -> top=8
    CHECK_EQ(c.top, 8u); CHECK_EQ(c.all, 8u);
    c = g.CompOutCode(50, 19);                   // above -> bottom=4
    CHECK_EQ(c.bottom, 4u); CHECK_EQ(c.all, 4u);
    c = g.CompOutCode(101, 50);                  // right=2
    CHECK_EQ(c.right, 2u); CHECK_EQ(c.all, 2u);
    c = g.CompOutCode(9, 50);                    // left=1
    CHECK_EQ(c.left, 1u); CHECK_EQ(c.all, 1u);
    c = g.CompOutCode(9, 201);                   // corner left+top = 9
    CHECK_EQ(c.all, 9u);
    c = g.CompOutCode(101, 19);                  // corner right+bottom = 6
    CHECK_EQ(c.all, 6u);
    // Edges are inside.
    c = g.CompOutCode(10, 20);  CHECK_EQ(c.all, 0u);
    c = g.CompOutCode(100, 200); CHECK_EQ(c.all, 0u);
    c = g.CompOutCode(10, 200); CHECK_EQ(c.all, 0u);

    // 3D variant: 0..PhysicalWidth/Height.
    g.PhysicalWidth = 640; g.PhysicalHeight = 480;
    c = g.Comp3DOutCode(320, 240); CHECK_EQ(c.all, 0u);
    c = g.Comp3DOutCode(641, 240); CHECK_EQ(c.right, 2u);
    c = g.Comp3DOutCode(-1, 240);  CHECK_EQ(c.left, 1u);
    c = g.Comp3DOutCode(320, 481); CHECK_EQ(c.top, 8u);
    c = g.Comp3DOutCode(320, -1);  CHECK_EQ(c.bottom, 4u);
    c = g.Comp3DOutCode(-1, -1);   CHECK_EQ(c.all, 5u);
    c = g.Comp3DOutCode(640, 480); CHECK_EQ(c.all, 0u);   // corner edge in
}

int main()
{
    test_type_layout();
    test_ifshare();
    test_vertex_layout();
    test_dopointstruc_layout();
    test_shape_instr_layout();
    test_clip_flags();
    test_distant_marker_clip();
    test_transform_nc();
    test_dopoint2x();
    test_dopoint();
    test_donpoints();
    test_transform();
    test_animptr();
    test_modvec_angles();
    test_modvec_vectors();
    test_flow_control();
    test_doswitch();
    test_docaserange();
    test_dogosub_interploop();
    test_doifcase();
    test_dosetcolour256();
    test_skip_writers();
    test_dontpoints();
    test_domorphnpoints();
    test_matrix_generate2_fptrans();
    test_matrix_generate_multiply();
    test_domorphpoint();
    test_divzero_guards();
    test_edge_boundaries();
    test_modvec_edges();
    test_doifbright();
    test_doifcross();
    test_flip_writers();
    test_ftoitexture();
    test_select_palette();
    test_stream_advance();
    test_anim_data_ops();
    test_state_setters();
    test_stretch_writers();
    test_delta_mirror_writers();
    test_modvec_full();
    test_modvec2();
    test_curves();
    test_dotimerphase();
    test_dofadeenvelope();
    test_donianimverts();
    test_donsubs();
    test_doifpiloted();
    test_drawstation_whiteout();
    test_donvec();
    test_dondupvec();
    test_dotransformlight();
    test_dovector();
    test_anim_conditionals2();
    test_douserealtime();
    test_advance_only2();
    test_dospin();
    test_dolighttimer();
    test_doanimation();
    test_deadstream_iterator();
    test_lbm();
    test_cdfile();
    test_globrefs();
    test_files_misc();
    test_compoutcode();

    test_mathlib_trig();
    test_mathlib_distance();
    test_mathlib_datetime();
    test_mathlib_misc();
    test_fileman();
    test_mathasm_bits();
    test_mathasm_bits_narrow();
    test_mathasm_muldiv();
    test_mathasm_misc();
    test_bitcount_macros();
    test_matrix_int();
    test_malloc_wrappers();
    test_cbuffer();
    test_angles();
    test_bitfield();
    test_cstring();
    test_bstream();

    test_win32_events();
    test_win32_semaphore();
    test_win32_mutex();
    test_win32_timing();
    test_win32_files();

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
