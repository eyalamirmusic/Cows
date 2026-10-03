# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Git Rules

Claude must never commit or push without explicit permission from the user in
the current conversation.

## Project Overview

Cows In Love exists to stress-test [eacp](https://github.com/eyalamirmusic/eacp),
our app framework: can we make awesome 3D games with eacp and ship them on every
serious platform (macOS, Windows, iOS, Android, Steam, the app stores)? Every
gap it exposes is an eacp fix first and a workaround second.

Cows is a 3D GPU recreation of the "cows in love" terminal animation
(`ssh ssh.cowsinlove.com`), built on [eacp](https://github.com/eyalamirmusic/eacp)
and its shader EDSL. Game-independent code lives in static libraries under
`Lib/`; each app under `Apps/` links them. Today there is one app target, `Cows`,
under `Apps/CowsInLove/`. See `docs/structure.md` for the layering.

- `Lib/cowsinlove_Engine` — knows nothing about cows. Include with the
  subdirectory: `#include "Render/Mesh.h"`.
  - `Render/Common.h` — `using namespace eacp` and `eacp::GPU`, included by all
    graphics code
  - `Render/Palette.h` — the original's colours, and sRGB → linear
  - `Render/Shaders` / `Shading` — the generic `ShaderProgram`s (sky, surface,
    shadow caster, glow), and the shared shading functions (lighting, PCF
    shadows, haze, tone curve, value noise)
  - `Render/Lighting.h` — the light, and the uniforms lit shaders share
  - `Render/Mesh` — procedural meshes: sphere, lathe shapes, box, wedge
  - `Render/Instances` — per-instance data and per-mesh batches
  - `Render/ShadowMap` — the key light's depth target
  - `Camera/OrbitCamera` — drag to orbit, scroll to zoom, idle drift
  - `UI/Hud` — draws the HUD at the end of the scene's own pass: discs and
    rings through eacp's `UI::ShapeBatch`, text through `Text::TextRenderer`
    (Menlo), on every platform; `CowsView` owns one and calls `drawHud`
  - `UI/Overlay` — `Footer` (the footer text, drawn through `Hud`), and
    `RootView` (q / Esc to quit)
  - `UI/TouchControls` — on-screen stick, Moo / Jump / Again, drag to look,
    pinch to zoom; reports everything through one `onControl` callback as a
    `ControlEvent` (`UI/ControlEvent.h`), and shows Again when `showAgain`;
    a `View` for input only, drawn by `draw(Hud&)`; eacp's `View` touch
    events feed it multi-touch and its safe area places it. `touchScreen`
    (iOS, Android) decides whether it is on screen at all
- `Lib/cowsinlove_AudioEngine/SamplePlayer` — plays a mono float sample through
  the output device, mixed as panned, pitched, muffled voices; the device is
  opened with MakeASound (`CMake/FindMakeASound.cmake`, miniaudio underneath),
  output only, on every platform
- `Lib/cowsinlove_Actors` — the things in the world, each with its model,
  collision shape and behaviour. Links Engine and AudioEngine.
  - `Animation/Choreography` — every timing of the original (sway/kiss, hops,
    heartbeat, sun pulse, title wave)
  - `Cow/Cow` — the cow model (parts on bones) and its animation;
    `Cow/HeartMesh` — puffy heart; `Cow/KissHearts` — the heart burst at each
    kiss (stateless); `Cow/Moo` — the recorded moo and her answer, embedded
    from `Resources/moo.f32` (see `Resources/CREDITS.md`)
  - `Sky/SkyDecor` — sun, clouds, hills
  - `Props/Collision` — `Collider` and `Block`; `Props/Scenery` — the batch,
    colliders and blocks props are added to; `Props/Props` — seeded draws,
    `matte`, `addBlock`
  - `Props/Barn`, `Tree`, `Hedge`, `Bale`, `Crate`, `Rock`, `Log`, `Fence`,
    `Bridge` — one add function each, taking a `Scenery&` and, where the prop
    is random, a `std::mt19937&`
  - `Props/Mover` — a barrel rolling to and fro on a closed-form timeline
    (`positionAt`, `modelAt`) with a collider that follows it;
    `makeRollingBale` in `Props/Bale`
- `Lib/cowsinlove_World` — how actors are laid out and collided with. Links
  Actors.
  - `Level` — a `Scenery` plus the hideout, `start` / `startHeading`, `gaps`
    (holes in the ground with a floor depth), `movers` (their `moving` batch
    rebuilt by `update(seconds)`), `killDepth` and `criticalPath`;
    `pushedOut` / `floorAt` / `hasGround` / `overGap` / `isFree`; `stepUp`
  - `Levels/` — level generation (see `docs/structure.md`): `Region` (an xz
    rectangle), `Layout` (the seeded draws, hiding places, `hasRoom`,
    `chooseHideout`), `Biome` (`meadowBiome`, `populate`), `Segments/`
    (`Segment` and `MeadowSegment`, `JumpLineSegment`, `RavineSegment`,
    `MoverSpawner`), `LevelTemplate`, `LevelGenerator` (`generate(template,
    seed)`)
  - `Terrain/Ground` — the ground mesh with the level's gaps cut out
    (`makeGround`, the plain plane when there are none) and the chasms' rock
    (`makeChasms`); `Terrain/Grass` — instanced blades (`makeBlade`,
    `makeGrassTile()` for one tile, `GrassField` for the tile with blades over
    gaps cut out); `Terrain/TerrainShaders` — the ground and grass shaders
- `Lib/cowsinlove_Game` — the rules of this game; nothing that owns GPU passes.
  Links World.
  - `Game` — state machine, player movement, found test, moo cooldown, the moo
    hint, the level's clock (`seconds`, driving its movers), the checkpoint and
    respawn below `killDepth` (`sinceFell`), and `footerText`; `reset(seed)` builds the level through its
    `makeLevel` hook, which the app sets (the library never names a level)
  - `Ending` — the ending's numbers (title rise, kiss point, camera settle) and
    `titlePlacement`, `loops` (start again after the title)
  - `Input` — held keys (wasd / hjkl / arrows, space) and the touch stick,
    summed into walk ahead / turn
  - `Title/TitleFont` — the tube-font title; `Title/TitleShader` — its shader
- `Apps/CowsInLove/Source/Main.cpp` — runs `CowsApp`. On Android eacp sets the
  COWS_* settings before `main()` in a debug build, from the launch intent's
  `--es` extras and the `debug.com.cowsinlove.cows.env` property
- `Apps/CowsInLove/Source/Templates` — the level templates (`meadowTemplate`,
  `meadowRavineTemplate`: the segment lists and lengths)
- `Apps/CowsInLove/Source/Stages` — the content: the ordered level templates
  (meadow, then meadow → ravine → meadow), `advance()` to the next after the
  ending (wrapping; nothing is saved), `level()` as a `LevelMaker`, where the
  seed comes from (`COWS_SEED`, else the clock) and the first stage
  (`COWS_STAGE`, else 0). `r` retries the stage with a fresh seed
- `Apps/CowsInLove/Source/CowsApp` — the window: scene, footer, touch controls,
  root view; wires them the same on every platform and ends by attaching the
  platform (the app has no platform directories or branches)
- `Apps/CowsInLove/Source/Scene/CowsView` — the `GPUView`: gathers instances,
  shadow pass, main pass (ending with the `Hud`), camera steering, mouse;
  forwards keys to `Input`
- `tools/Art` — `CowsArt`, a macOS tool that renders the icon, key art, logo
  and store screenshots from the game's own views; `tools/store-art.sh` and
  `tools/screenshots.sh` drive it. `tools/release-*.sh`, `release-msix.ps1`
  and `steam-upload.sh` build and package each store; `Deploy/README.md` is
  the runbook
- `Tests/Engine`, `Tests/AudioEngine`, `Tests/Actors`, `Tests/World`,
  `Tests/Game` — one NanoTest executable per library, and `Tests/CowsInLove`
  for the app's content; run with `ctest --test-dir build` or `just test`.
  `Tests/Support` holds the shared `TestMain` and `SnapshotView`, which draws a
  `SurfaceBatch` (plus anything in `drawOpaque`) off-screen and saves
  `docs/shots/tests/<name>.png`; snapshot tests skip without a GPU

`COWS_TIME=<seconds>` starts the clock there and `COWS_FREEZE=1` stops it, for
screenshots; `COWS_SEED=<n>` fixes the level, `COWS_STAGE=<n>` starts on
stage n (1 is the ravine) and `COWS_FOUND=1` starts beside her (run the binary in `build/Apps/CowsInLove/Cows.app/Contents/MacOS/` directly).

## Build Commands

eacp is fetched by CPM (`CMake/Findeacp.cmake`; on this branch
`eyalamirmusic/eacp#develop`, on `main` `eyalamirmusic/eacp#main`).
To build against the local checkout instead (usually ahead of `main`):

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug -DCPM_eacp_SOURCE=$HOME/Code/eacp
cmake --build build
open build/Apps/CowsInLove/Cows.app
```

Android: `cmake --preset android` (`CMakePresets.json`) configures
`build-android` (Release, arm64-v8a, API 33) and writes the Android Studio
project, a Gradle project with a `Cows` module, to `build-android/AndroidStudio`;
`just studio-android` does that and opens it. From a terminal there,
`./gradlew :Cows:installDebug` (with `JAVA_HOME` at a JDK 17+, e.g. Studio's
JBR) does what Run does, for arm64-v8a and x86_64 (`EACP_ANDROID_ABIS`).
`tools/android.sh sim [shot.png]` (`just sim-android`) builds, boots the `cows`
emulator and runs it. The preset's toolchain, `CMake/AndroidToolchain.cmake`,
builds with the NDK `-DCOWS_NDK=<version>` locks to, else the one
`$ANDROID_NDK_HOME` names, else the newest in the SDK ($ANDROID_HOME, else
Studio's), as eacp's own toolchain does.

Use `$HOME`, not `~`: CMake does not expand `~`.

iOS: `tools/ios.sh sim [shot.png]` builds `build-ios/` and runs it on the "Cows iPhone"
simulator; `tools/ios.sh device` signs with `COWS_TEAM` (default: Jamie's Personal Team) and runs it on the phone.

Shortcuts in the `justfile`: `just macos`, `just sim-ios [shot.png]`,
`just ios [udid]` (defaults to pond), `just shot`, `just devices`, `just test`.

## Rendering Rule

Everything on screen goes through eacp's GPU path: `GPUView` for the scene,
`GPUWidgets` paths for shapes, the `Text` glyph atlas and `TextRenderer` for
text. Never add a CPU-rasterised tier (a software 2D context, an overlay painted
into an image and uploaded as a texture) to get a platform working; that is a
regression, not a port. A platform without a glyph rasteriser gets a
`GlyphRasterizer-<Platform>` in eacp over the platform's own text engine
(CoreText, DirectWrite, `android.graphics` over JNI), not a new dependency such
as FreeType. Gaps found this way are eacp fixes first; a Cows-side workaround is
the last resort and is called out as one. This is today's position, not
dogma: eacp may grow a CPU tier one day, but for now `GPUView` is the way
because it performs better and ports further.

## Code Style

Follows eacp's style:

- Modern C++20 and RAII. `auto` for variables; never for function return types.
- PascalCase types, camelCase functions and members, no member prefixes.
- Structs marked `final` where nothing derives from them; data members at the
  bottom.
- File-local helpers live in an anonymous namespace. Graphics code includes
  `Render/Common.h`, which brings `eacp` and `eacp::GPU` in globally, so nothing
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
