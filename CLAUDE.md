# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Git Rules

Claude must never commit or push without explicit permission from the user in
the current conversation.

## Project Overview

Cows is a 3D GPU recreation of the "cows in love" terminal animation
(`ssh ssh.cowsinlove.com`), built on [eacp](https://github.com/eyalamirmusic/eacp)
and its shader EDSL. It is one app target, `Cows`, under `App/`.

- `App/Source/Main.cpp` — runs `CowsApp`
- `App/Source/CowsApp` — the window: scene, footer overlay, root view; ends
  by attaching the platform
- `App/Source/macOS`, `App/Source/iOS` — one `Platform.h` each: window size, and
  on iOS the touch HUD and audio session; CMake puts the
  current platform's directory on the include path, so no platform macros
- `App/Source/Scene/Common.h` — `using namespace eacp` and `eacp::GPU`, included
  by all graphics code
- `App/Source/Scene/CowsView` — the `GPUView`: gathers instances, shadow pass,
  main pass, mouse input
- `App/Source/Scene/Choreography` — every timing of the original (sway/kiss,
  hops, heartbeat, sun pulse, title wave)
- `App/Source/Scene/Palette.h` — the original's colours, and sRGB → linear
- `App/Source/Scene/Shaders` / `Shading` — the `ShaderProgram`s, and the shared
  shading functions (lighting, PCF shadows, haze, tone curve, value noise)
- `App/Source/Scene/Lighting.h` — the light, and the uniforms lit shaders share
- `App/Source/Scene/Mesh`, `HeartMesh`, `TitleFont` — procedural meshes: sphere,
  lathe shapes, puffy heart, tube-font title
- `App/Source/Scene/Instances` — per-instance data and per-mesh batches
- `App/Source/Scene/Cow` — the cow model (parts on bones) and its animation
- `App/Source/Scene/KissHearts` — the heart burst at each kiss (stateless)
- `App/Source/Scene/SkyDecor` — sun, clouds, hills; `Grass` — instanced blades
- `App/Source/Scene/ShadowMap` — the key light's depth target
- `App/Source/Scene/Overlay` — footer text, and q / Esc to quit
- `App/Source/Scene/TouchControls` — on-screen stick, Moo / Jump / Again, drag
  to look, pinch to zoom; `iOS/TouchSurface.mm` feeds it multi-touch on iOS
- `App/Source/Scene/OrbitCamera` — drag to orbit, scroll to zoom, idle drift

`COWS_TIME=<seconds>` starts the clock there and `COWS_FREEZE=1` stops it, for
screenshots (run the binary in `build/App/Cows.app/Contents/MacOS/` directly).

## Build Commands

eacp is fetched by CPM (`CMake/Findeacp.cmake`, `eyalamirmusic/eacp#main`).
To build against the local checkout instead (usually ahead of `main`):

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug -DCPM_eacp_SOURCE=$HOME/Code/eacp
cmake --build build
open build/App/Cows.app
```

Use `$HOME`, not `~`: CMake does not expand `~`.

iOS: `tools/ios.sh sim [shot.png]` builds `build-ios/` and runs it on the "Cows iPhone"
simulator; `tools/ios.sh device` signs with `COWS_TEAM` (default: Jamie's Personal Team) and runs it on the phone.

Shortcuts in the `justfile`: `just macos`, `just sim-ios [shot.png]`,
`just ios [udid]` (defaults to pond), `just shot`, `just devices`.

## Code Style

Follows eacp's style:

- Modern C++20 and RAII. `auto` for variables; never for function return types.
- PascalCase types, camelCase functions and members, no member prefixes.
- Structs marked `final` where nothing derives from them; data members at the
  bottom.
- File-local helpers live in an anonymous namespace. Graphics code includes
  `Scene/Common.h`, which brings `eacp` and `eacp::GPU` in globally, so nothing
  spells `eacp::` or `eacp::GPU::` out; `.cpp` files add
  `using namespace Maths;` as needed.
- Use `eacp::Vector` and the `Maths` types (`Vec3`, `Mat4`) as eacp does.
- No comments unless absolutely needed; name functions to be self-documenting.
- Give `std::function` members a non-null default.
- Builds must stay warning-free (`set_default_target_setting` turns on
  `-Wall -Wextra -Wpedantic -Wshorten-64-to-32`).

Enforced via `.clang-format` (copied from eacp): Allman braces, 85 columns,
4-space indentation, left pointer alignment. Always run clang-format on edited
C++ files.
