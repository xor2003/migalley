//------------------------------------------------------------------------------
// Unit tests for the native MiG Alley port (32-bit).
//
// Assertion-style, no framework (keeps the tree dependency-free and the
// tests readable next to the 1998 code they pin down).
//
// Run via ctest or directly: ./unit_tests
//------------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstddef>

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
#include "DISPLAY.H"

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
// Time/phase handlers. ViewPoint::TimeOfDay() is a 3-hop chain in the port:
//   this+0x52 -> inst, inst+0x50 -> next, next+0x1d -> timeofday
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
    *(void**)(fake_viewpoint + 0x52) = fake_instmid;
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

    // noxframes=0: guarded -> stepx=frameno, stepy=0.
    nv->noxframes = 0;
    nv->factor = 1;
    anim[2] = 2;
    ip = stream;
    shape_donianimverts(ip);
    CHECK_EQ(shape_shpco[1].ix, 2 * 32 + 3);
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
    test_dotimerphase();
    test_dofadeenvelope();
    test_donianimverts();
    test_donsubs();
    test_doifpiloted();
    test_drawstation_whiteout();

    test_win32_events();
    test_win32_semaphore();
    test_win32_mutex();
    test_win32_timing();

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
