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
// Tiny harness
//------------------------------------------------------------------------------
static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++g_checks;                                                        \
        if (!(cond)) {                                                     \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                  \
    } while (0)

#define CHECK_EQ(a, b)                                                     \
    do {                                                                   \
        ++g_checks;                                                        \
        long long _va = (long long)(a), _vb = (long long)(b);             \
        if (_va != _vb) {                                                  \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s (=%lld) != %s (=%lld)\n",          \
                        __FILE__, __LINE__, #a, _va, #b, _vb);             \
        }                                                                  \
    } while (0)

#define CHECK_FEQ(a, b)                                                    \
    do {                                                                   \
        ++g_checks;                                                        \
        double _va = (double)(a), _vb = (double)(b);                      \
        double _d = _va - _vb;                                             \
        if (_d < -1e-6 || _d > 1e-6) {                                     \
            ++g_failures;                                                  \
            std::printf("FAIL %s:%d: %s (=%g) != %s (=%g)\n",              \
                        __FILE__, __LINE__, #a, _va, #b, _vb);             \
        }                                                                  \
    } while (0)

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
    test_ftoitexture();
    test_select_palette();

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
