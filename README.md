# Mig Alley Linux Port

A complete native Linux port of Rowan’s **Mig Alley** (1999).  
All original Windows dependencies have been removed or replaced with modern, cross‑platform components.  
The objective is long‑term preservation, maintainability, and future expansion of the dynamic campaign engine.

## Overview

This project contains a full platform migration of Mig Alley from Windows to Linux.  
The codebase no longer depends on Win32, DirectX, DirectInput, or Miles Sound System.  
All required subsystems have been rewritten or adapted to use portable libraries and modern APIs.

## Major Changes

### Platform and System Layer
- Replacement of the MFC layer with a custom implementation compatible with the original message routing model  
- Reimplementation of required Win32 API functions  
- Removal of all Windows‑specific code paths  
- Native Linux build using standard toolchains

### Graphics
- 2D rendering implemented with SDL2  
- UI rendering implemented with SDL2  
- 3D engine rewritten to use Vulkan
- All legacy Direct3D 5/6 fixed‑function code replaced with a modern rendering backend

### Input
- DirectInput replaced with SDL input  
- Support for mouse, keyboard, and joystick through SDL’s unified input system  
- Axis, button, and hat mappings aligned with original behavior

### Audio
- Miles Sound System API ported to use SDL_mixer  
- Channel management and playback timing matched to original logic

### Dependencies
The Linux version depends only on:
- SDL2, SDL_mixer and SDL_ttf  
- Vulkan loader and driver
- nlohmann C++ json support

No Windows DLLs or compatibility layers are required.

## Build Instructions

### Requirements
- C++ compiler with C++17 support  
- CMake  
- SDL2 development packages  
- SDL_mixer development packages  
- Vulkan SDK or system Vulkan headers and loader

### Build
```
mkdir build
cd build
cmake ..
make
```

## Current Status

The game is fully playable on Linux.  
All subsystems have been ported.  
The campaign, AI, UI, and mission logic behave as in the original release.

## Roadmap

### Short Term
- Codebase cleanup  
- Validation of campaign logic  
- Regression testing against the Windows version

### Medium Term
- Reactive multiplayer
- Improvements to the dynamic campaign engine  
- More autonomous ground and air operations  
- Better integration between strategic and tactical layers

### Long Term
- Tools for mission analysis and debugging

## 64-bit Port Audit (prepared, not started)

`-m64 -fsyntax-only` census: ~2555 errors across 235 TUs, ~90% mechanical.
The `#pragma pack` regions mark where layout is contractual — they are the
audit surface, but pack only controls padding, not member sizes.

### Category A — serialized/file-format layout (must stay 4-byte world)

| Site | Contents | Action |
|---|---|---|
| `H/DOSDEFS.H:57` | `pack(1)` file-wide, covers `SLong`/`ULong`/`Long = long` typedefs (:117-118) | retag to `int32_t`/`uint32_t` — the single root fix; every shape/savegame struct inherits it |
| `Graphics/GRAFPRIM.CPP:455` | `colourdata` + `.lbm` palette structs | already fixed-width bytes — verify only |
| `Bfields/SAVEGAME.CPP` | `memcpy` of `AirStruc`/save records into buffers | audit after retag; savegame compat breaks anyway |
| `Files/FILEMAN.CPP` | raw `fread` into shape blobs; struct layout overlaid later | covered transitively by DOSDEFS retag |

### Category B — dead-API compat structs (layout can float, pointer members still fragile)

| Site | Risky members | Notes |
|---|---|---|
| `H/direct_3d.h` pack(1) | ~297 | D3D retained-mode compat; counterparty is gone (Vulkan now) — sizes may float, but pointer members must not be truncated |
| `H/WIN3D.H` pack(1) | ~62 | `D3DAppDDDriver` contains real `ULong vidMem` |
| `H/ddraw_stub.h` pack(1) | ~50 | DirectDraw surface/compat structs |
| `H/MSSW.H` + `H/AIL.H` | ~285+144 | Miles Sound SDK headers — still compiled via `Hardware/MILES.CPP`/`SFONTS.CPP` |
| `H/SFMAN.H` pack(2) | ~25 | contains `LPSTR m_Buffer` — pointer inside packed struct |

### Category C — asm interop (already clean)

- `H/3DDEFS.H:393` `Vertex` pack(4) — already `int32_t` throughout (MASM DD comment; asm dead).

### No-pragma hazard class (separate sweep — nothing marks these)

| Site | Issue | Count |
|---|---|---|
| `H/WIN32_COMPAT.H:676` `LRESULT = int32_t` | carries pointers (SendMessage etc.) | ~2100 of census errors — change to `intptr_t` eliminates 82% |
| `H/WIN32_COMPAT.H:163` `LONG = long` | 8 bytes on LP64 | retag `int32_t` |
| `animptr` union | pointer punned into size/flag word fields | rework encoding |
| `ULong(this)` / pointer-in-DWORD casts | HWND/HANDLE/`void*` stored in int fields | ~950 cast sites repo-wide |
| asm remnants | `GRAFPASM.ASM`/`GRAFJIM.ASM`/`HARDPASM.H`/`MATHASM.H` | ~82 refs, mostly stubbed already |

### Migration order

1. `LRESULT`/`LONG`/`SLong`/`ULong` typedef retag (kills ~85% of errors)
2. `-m64 -fsyntax-only` loop until clean — mechanical cast fixes
3. UBSan → ASan → TSan (the prize: races only diagnosable on 64-bit)
4. Savegame/dplay serialization audit (format breaks regardless)

## License

This project uses the original license provided by Rowan Software when the source code was released.

## Acknowledgments

Original game by Rowan Software.  
This project aims to preserve and extend the technical and historical value of the simulation.
