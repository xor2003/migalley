#!/usr/bin/env bash
# Regenerate tests/unit/test_abi.cpp.
#
# Extracts every typedef'd struct name from the wire-format headers,
# compiles a probe TU with the same flags as unit_tests, runs it on the
# 32-bit build, and emits static_assert()s pinning each sizeof() to the
# golden i386 value. On a 64-bit compile any layout that drifts (the
# SLong/ULong = long hazard) breaks the build at the exact line that
# names the affected struct — the migration fix-list, for free.
#
# Re-run after adding/removing a shape instruction or wire struct:
#   tests/unit/gen_abi.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
UNIT_DIR="$ROOT/tests/unit"
OUT="$UNIT_DIR/test_abi.cpp"
PROBE="$(mktemp -d)/abi_probe"
trap 'rm -rf "$(dirname "$PROBE")"' EXIT

# 1) typedef'd names from the wire headers (pattern: } NAME, / } NAME; )
TYPES_FILE="$(mktemp)"
trap 'rm -rf "$(dirname "$PROBE")" "$TYPES_FILE"' EXIT
for hdr in SHPINSTR.H; do
    awk '/^}/{getline; if ($0 ~ /^[A-Za-z_][A-Za-z0-9_]*,? *$/ && $0 !~ /\*/) print $1}' \
        "$ROOT/H/$hdr" | tr -d ',;'
done | sort -u > "$TYPES_FILE"

# 2) probe program: prints one static_assert per type
{
    cat <<'EOF'
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include "DOSDEFS.H"
#include "MYANGLES.H"
#include "3DDEFS.H"
#include "VERTEX.H"
#include "MATRIX.H"
#include "SHPINSTR.H"
#include "ANIMPTR.H"
#include "MODVEC.H"
#define P(T) std::printf("static_assert(sizeof(%s) == %d, \"sizeof(%s) drifted from i386 wire layout\");\n", #T, (int)sizeof(T), #T);
EOF
    echo "int main() {"
    echo "    P(COORDS3D) P(fpCOORDS3D) P(IFShare) P(VERTEX) P(FPMATRIX) \\"
    echo "        P(MATRIX) P(ANGLES) P(DoPointStruc) P(FCRD) P(FCRDlong) P(FORI)"
    while read -r t; do echo "    P($t)"; done < "$TYPES_FILE"
    echo "    return 0;"
    echo "}"
} > "$PROBE.cpp"

g++ -m32 -fms-extensions -fpermissive -w \
    -I"$ROOT/H" -I"$ROOT/H/Stub3D" -I"$ROOT/MFC" \
    -o "$PROBE" "$PROBE.cpp"
GOLDEN="$("$PROBE")"

# 3) emit the test TU
{
    cat <<'EOF'
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
EOF
    echo "$GOLDEN"
    cat <<'EOF'

// Intentionally NOT pinned (in-memory only, size may float on 64-bit):
//   animptr / *P typedefs  - contain real pointers
//   FPMATRIX_PTR, FILE, *_PTR typedefs
// Anything serialized to disk, the shape stream, or shared via memcpy
// MUST appear in the generated block above.
EOF
} > "$OUT"
echo "wrote $OUT ($(grep -c static_assert "$OUT") asserts)"
