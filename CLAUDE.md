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
  - `Render/Mesh` — procedural meshes: sphere, lathe shapes, box, wedge, and
    `makeBarrelPatch`, a piece of the barrel between latitudes, longitudes,
    two planes along its length and two across it, with an optional round hole and a rim turned in,
    for clothes cut from a body part; `Render/ShapeMeshes` — one
    mesh per `Shape`, the game making the ones the Engine has not (the heart,
    the pants), shared by `CowsView` and `SnapshotView`
  - `Render/Instances` — per-instance data and per-mesh batches
  - `Render/ShadowMap` — the key light's depth target
  - `Camera/OrbitCamera` — drag to orbit, scroll to zoom, idle drift;
    `CameraPose`, `blend` and `easeOut` for the menu's camera swing, `SwingClock`
    (the swing's step on the clock, past eacp's 0.1 s delta clamp)
  - `UI/Hud` — draws the HUD at the end of the scene's own pass: discs and
    rings through eacp's `UI::ShapeBatch`, text through `Text::TextRenderer`
    (Menlo), on every platform; `CowsView` owns one and calls `drawHud`;
    `strokeRoundedRect` for the menu's selection ring
  - `UI/Menu` — the start menu: `MenuItem`s in Comic Neue Bold (embedded
    from `Resources/ComicNeue-Bold.ttf`, OFL, registered as a memory font
    through `Text::registerMemoryFont`; see `Resources/CREDITS.md`), side by
    side when wide, stacked when tall (by aspect, not platform), a
    `titleArea` left at the top; hover and click, tap (targets at least
    64 pt), and a selection ring for keys and controllers; a disabled item is
    grey and never chosen; `opacity` fades it
  - `UI/Editor` — the cow editor's rows ("Hat  <  Top Hat  >", then Done) in
    the menu's look over the live scene, right of `sceneArea` when wide and
    below it when tall; hover and click, tap, a selection ring for keys and
    controllers; a drag or wheel off the rows goes out through `onControl` as
    Look / Zoom. `topClearance` keeps it under the macOS title bar, which the
    safe area does not count. `UI/MenuStyle` — the pink Comic Neue look the
    menu and the editor share
  - `UI/Overlay` — `Footer` (the footer text, drawn through `Hud`), and
    `RootView` (q quits; Esc goes to `onEscape` first, quitting when it
    declines)
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
  - `Cow/CowSkin` — what the player's cow wears: one enum per item class
    (`Hat`, `Pants`), saved through Miro as enumerator names in the app's
    `Settings` (`withNamedChoices` puts anything unnamed back to the
    default). `itemClasses()` lists the
    classes for the editor, so a new class (Glasses, Trousers) is an enum, a
    `CowSkin` field and one line there. `Cow/Wardrobe` — each hat as parts
    on the head bone (`addHat`), seated on the crown (the cow bucket hat a lathed
    crown and sloping brim, `Shape::BucketCrown` and `BucketBrim` from
    `makeCowMesh`, in her own spots, smaller through `CowPart::spotScale`; the
    wizard hat a lathed cone whose top third is bent over, a waved brim and
    flat stars, `Shape::WizardCone`, `WizardBrim` and `Star`); pants on the body
    (`addPants`) cut from the cow's own parts (`torsoPlacement`,
    `legPlacements`, `tailRoot`): the torso's barrel scaled out as the seat
    (everything behind the hips, a hole for the tail, a waistband round the
    body at the hips) for the back legs, or for both pairs the belly (the
    lower half, cut level at the girth with a band along it; `makePantsMesh`), and the legs' capsules
    widened into sleeves with a cuff above the hoof; `makeCowMesh` for
    `ShapeMeshes`, and `makeCowParts(skin)`
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
  - `Game` — state machine (`Menu` first, then `Searching` and `Found`;
    `start()` leaves the menu, `openMenu()` pauses into it, `playing()` is the
    state behind it; a `reset` from play skips the menu), player movement, found test, moo cooldown, the moo
    hint, the level's clock (`seconds`, driving its movers), the checkpoint and
    respawn below `killDepth` (`sinceFell`), `footerText` (for a `Hints`), and
    `inputOwner` (menu, editor or play takes touches; play from the moment Start
    swings the camera down); `reset(seed)` builds the level through its
    `makeLevel` hook, which the app sets (the library never names a level)
  - `Ending` — the ending's numbers (title rise, kiss point, camera settle) and
    `titlePlacement`, `loops` (start again after the title)
  - `Input` — held keys (wasd / hjkl / arrows, space), the touch stick and the
    controller (`setPad`, kept apart so neither clears the other), summed and
    clamped into walk ahead / turn
  - `Pad` — `readPad` turns eacp's `GameInputFrame` into `PadControls`: radial
    deadzone, turn and look curves, D-pad, button edges (South jump, West / East
    moo and back, North again, right stick click recentre), triggers as zoom; the largest
    stick of several controllers wins. `padHints` names the footer's `Hints`
    from the controller's family
  - `Title/TitleFont` — the tube-font title; `Title/TitleShader` — its shader;
    `Title/MenuTitle` — "cows in love" for the menu (one line wide, two tall),
    placed in front of the camera to fill the menu's `titleArea`
- `Apps/CowsInLove/Source/Main.cpp` — runs `CowsApp`. On Android eacp sets the
  COWS_* settings before `main()` in a debug build, from the launch intent's
  `--es` extras and the `debug.com.cowsinlove.play.env` property
- `Apps/CowsInLove/Source/Templates` — the level templates (`meadowTemplate`,
  `meadowRavineTemplate`: the segment lists and lengths)
- `Apps/CowsInLove/Source/Stages` — the content: the ordered level templates
  (meadow, then meadow → ravine → meadow), `advance()` to the next after the
  ending (wrapping; nothing is saved), `level()` as a `LevelMaker`, where the
  seed comes from (`COWS_SEED`, else the clock) and the first stage
  (`COWS_STAGE`, else 0). `r` retries the stage with a fresh seed
- `Apps/CowsInLove/Source/Settings` — everything kept between runs, one Miro
  document, `settings.json` in `FilePath::appSupportDirectory()`: the
  `CowSkin` and the `QualityPreference`; anything unreadable or unnamed loads
  as the default, and a new setting is one field. No other file is written
- `Apps/CowsInLove/Source/Scene/Quality` — `Quality` (Low, Medium, High) and
  `settingsFor`, the one place every tier's knobs are set (MSAA, render scale,
  mesh detail, grass tiles, density and blade segments, grass and ground noise,
  shadow map side and taps); `QualityChoice` (Auto or a tier) and
  `qualityToUse` (COWS_QUALITY, else the choice, else the measured tier)
- `Apps/CowsInLove/Source/CowsApp` — the window: scene, footer, touch controls,
  root view, and the `GameInput` the scene polls for controllers; wires them the
  same on every platform and ends by attaching the platform (the app has no
  platform directories or branches). The footer names the last-used input's
  controls; the touch controls hide while a controller was used last
- `Apps/CowsInLove/Source/Scene/CowsView` — the `GPUView`: gathers instances,
  shadow pass, main pass (ending with the `Hud`), camera steering, mouse;
  forwards keys to `Input`; polls `GameInput` each `update`, sends the
  controller's buttons through `control()` as touch does, orbits on the right
  stick (a look, mouse drag included, holds off the chase for `lookHold`), and
  tracks the last-used input as `hints`. In the menu it holds the camera in
  front of the cow (`menuPose`, drifting), draws the menu title, and takes
  Enter / Space / arrows / A D and the controller's South / Start / stick /
  D-pad; Start swings the camera behind the cow over 1.1 s with an ease-out
  while the menu fades and the title slides up, then play starts (`playPose`);
  Esc and the controller's Start swing back. Dress Your Cow and Settings swing
  round to the cow (`editorPose`) and hand keys and the controller to that
  `Editor` (`openEditor(which)`); the camera orbits freely there (drag, right
  stick, touch drag), and Done, Esc and the controller's East / Start swing
  back to the menu. `CowsApp` saves `Settings` on every change
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
stage n (1 is the ravine) and `COWS_FOUND=1` starts beside her, skipping the menu; `COWS_MENU=0` skips
the menu, `COWS_DRESS=1` opens the cow editor, `COWS_SETTINGS=1` opens
Settings and `COWS_START=<seconds>` presses Start after that long (run the binary in `build/Apps/CowsInLove/Cows In Love.app/Contents/MacOS/` directly).
`COWS_PROFILE=1` logs, once a second, the frame interval, the CPU time of each
part of a frame, the GPU time of the frame and of each pass (`shadows`,
`scene`) and the instances and triangles each pass drew; a profiling run
varies one of `COWS_BLADES=<n>` (blades drawn per grass tile), `COWS_MSAA=<n>`,
`COWS_SHADOW=<texels>` (the shadow map's side), `COWS_RENDER_SCALE=<share>`,
`COWS_DETAIL=<share>` (mesh detail, see `detailed` in `Render/Mesh`) or
`COWS_SKIP=<parts>` (any of sky, ground, objects, grass, glow, hud,
shadows, left out of the frame) and compares.
`COWS_QUALITY=low|medium|high` forces a quality tier at launch (`Scene/Quality`)
until the player picks one in Settings (the start menu's third item, an
`Editor` with one row: Auto, Low, Medium, High; it applies at once). On Auto
with nothing measured the run starts at high, `QualityGovernor`
(`Render/QualityGovernor`) drops tiers while the GPU's frame time is over
budget, and the tier it settles on is saved in `settings.json` and used from
then on.

## Build Commands

eacp is fetched by CPM (`CMake/Findeacp.cmake`, `eyalamirmusic/eacp#develop`).
To build against the local checkout instead (usually ahead of `develop`):

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug -DCPM_eacp_SOURCE=$HOME/Code/eacp
cmake --build build
open "build/Apps/CowsInLove/Cows In Love.app"
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
