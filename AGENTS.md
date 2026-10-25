# AGENTS.md

This file guides coding agents (and people) working in this repository.

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
under `Apps/CowsInLove/`.

- `Lib/cowsinlove_Layout` — a flexbox-lite, GPU-free and game-free (for eacp
  to take): `Layout/Tree` (`Flex::Tree` of `Node`s — Across/Down, `Size::fit`,
  `grow`, `fixed`, `share`, padding, gap, alignment, text leaves measured
  through a caller's `Measure`, shared `column`s — resolved to whole-point
  rects with `overflow` per node) and `Layout/FitToSize` (`largestPointSize`
  on a `fontSizeStep` grid)
- `Lib/cowsinlove_MeshLoader/MeshLoader/MeshBlob` — `readMeshBlob` reads
  the binary mesh blobs the Blender scripts write (format in
  `tools/blender/README.md`) into `MeshGroup`s, one per material;
  `findGroup`. Links only eacp-core; the game turns groups into `MeshData`
  where it uses them
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
    (the swing's step on the clock, past eacp's 0.1 s delta clamp);
    `distanceToFit` (how far back something fills a share of the view) and
    `placeTargetOnScreen` (a pose moved so what was at its target shows at a
    point on screen), which frame the cow in the menu's `cowArea`
  - `UI/Hud` — draws the HUD at the end of the scene's own pass: discs and
    rings through eacp's `UI::ShapeBatch`, text through `Text::TextRenderer`
    (Menlo), on every platform; `CowsView` owns one and calls `drawHud`;
    `strokeRoundedRect` for the menu's selection ring
  - `UI/Menu` — the start menu: `MenuItem`s in Comic Neue Bold (embedded
    from `Resources/ComicNeue-Bold.ttf`, OFL, registered as a memory font
    through `Text::registerMemoryFont`; see `Resources/CREDITS.md`), one
    column at the largest size that fits `itemArea` (through Layout). Wide
    (`w >= h`) the title gets 0.34 of the content's height and the items' column
    sits beside `cowArea` (0.55 of the width); tall the title 0.18, `cowArea`
    0.4, the items below. Every label, About and its note included, is held to
    `smallestReadableSize`; when neither shape can, the cow's and the title's
    room give way a step at a time (`Menu::Shape`), and in the smallest windows
    the cow stands behind the items. `titleArea`, `cowArea` and `itemArea` are
    worked out from the window on demand; `menuPose` frames the cow in
    `cowArea`. A `MenuItem` is `{label, note, scale, enabled}`, `scale` sizing
    one item against the rest (Start biggest, About smallest; `makeMainMenuItems`
    and `makeAboutItems` in the app's `MainMenu.h`, which names every string it
    shows); `setItems` replaces them. Hover and click, tap (targets at least
    64 pt), and a selection ring for keys and controllers; a disabled item is
    grey and never chosen; `opacity` fades it
  - `UI/Editor` — the cow editor's rows ("Hat  <  Top Hat  >", then Done) in
    the menu's look over the live scene: a heading, then a table through
    Layout (names in one shared column, values in another as wide as the
    widest choice of any row), then Done, right of `cowArea` (0.55 of the
    width) when wide and below it (0.55 of the height) when tall, in
    `panelArea`. Text is held to `smallestReadableSize`: the cow's room gives
    way first, then each name goes over its value (`stacked`). `setRows` and
    `setTitle` lay it out again; a row's `choice` alone doesn't. `editorPose`
    frames the cow in `cowArea` (`framedCow`, `CowFraming`); `makeDressRows` and
    `makeQualityRow` in `MainMenu.h` build the rows. Hover and click, tap, a
    selection ring for keys and controllers; a drag or wheel off the rows
    goes out through `onControl` as Look / Zoom. `topClearance` keeps it
    under the macOS title bar, which the safe area does not count
  - `UI/MenuStyle` — the pink Comic Neue look the menu and the editor share,
    `ScreenFrame` (`contentArea`: the safe area, the top at least
    `topClearance` — `macTitleBarClearance` on macOS — and the footer's strip,
    `Footer::reserveFor`, kept clear), `tapTarget`, `smallestReadableSize`
    (18 pt on a phone to 22 pt on a desktop), `largestShape` (the first of a
    list of shapes to fit, unless another fits text a quarter larger) and
    `menuFont`. `UI/FontCache` — every UI font, on the 0.5 pt `fontSizeStep`
    grid (`Layout/FitToSize`'s, which `PointSizeRange::step` defaults to), in
    an `unordered_map` behind `Singleton::get<FontCache>()` (`uiFont`); the
    grid bounds it to a few hundred fonts, so nothing is evicted and a
    reference it hands out stays good
  - `UI/Overlay` — `Footer` (the footer text, drawn through `Hud`), and
    `RootView` (q quits; Esc goes to `onEscape` first, quitting when it
    declines)
  - `UI/TouchControls` — on-screen stick, Moo / Jump / Again, drag to look,
    pinch to zoom, and a back arrow in the top-left corner (under
    `topClearance`, in Searching and Found alike) that pauses into the menu;
    reports everything through one `onControl` callback as a
    `ControlEvent` (`UI/ControlEvent.h`: steer, jump, moo, restart, look,
    zoom or pause), and shows Again when `showAgain`;
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
    `Cow/CowPhysics` — `getJumpPhysics`, the launch speed and gravity worked
    out from the jump's height and airtime, which `Game` starts from;
    `Cow/CowModel` — `CowModel`, a `Plugins::Reloadable` whose `makeParts`
    and `getJumpPhysics` (`EACP_RELOADABLE`, defined in `Cow.cpp` and
    `CowPhysics.cpp` over the free functions) run the hot-reload module's
    latest code; `CowsView` owns one;
    `Cow/HeartMesh` — puffy heart; `Cow/KissHearts` — the heart burst at each
    kiss (stateless); `Cow/Moo` — the recorded moo and her answer, embedded
    from `Resources/moo.f32` (see `Resources/CREDITS.md`)
  - `Cow/CowSkin` — what the player's cow wears: one enum per item class
    (`Hat`, `Pants`), saved through Miro as enumerator names in the app's
    `Settings` (`replaceUnknownValuesWithDefaults` puts anything unnamed back to the
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
  - `Props/Barn` — a gambrel-roofed barn, 16 by 22: open front doors (facing
    -z in barn space, `quarterTurns` turns it), a hay loft across the front
    over them, open to the meadow through the hay door in the gable (its
    leaves stand open), up a railed ramp from the back of the floor along
    the +x wall, open stalls and a trough along the -x wall, and outside a climb of bale stacks a jump
    apart up the -x wall onto the roof, whose two pitches (eaves to
    `barnRoofBreakX` / `barnRoofBreakHeight`, then the ridge) are floating
    ramp `Block`s she walks on; a cupola at the back of the ridge, leaving
    its middle clear. The walls stand as high as the roof over them, so
    nothing inside can be jumped over. `barnSpots` names its places (the
    ridge and the loft, where she hides; the doorway, the ramp's top, the
    climb's top) and `barnReach` its size; every dimension lives in
    `Barn.cpp`, the numbers the mesh shares with the collision repeated at the
    top of `tools/blender/barn.py` (keep them in step), which models it and writes it through `tools/prop-mesh.sh barn` to
    `Resources/barn.mesh` (committed; builds never need Blender), embedded
    and read once by `Props/PropMeshes` into one mesh per material: the
    `Shape`s `BarnWall`, `BarnTrim`, `BarnRoof`, `BarnWood`, `BarnHay`,
    `BarnMetal`, `BarnTwine`, each placed once per barn with its colour. The
    barn's floor is a `Clearing` (`Props/Scenery`), where `GrassField` grows
    no grass. `Props/Meshes` — `contentMeshes(detail)` / `makeContentMesh`:
    the meshes `ShapeMeshes` leaves to the game, the cow's (`cowMeshes`) and
    the props'. `Props/PropMeshes` — the props modelled in Blender: one
    table of `Shape` → embedded mesh blob and group
    (`tools/blender/<prop>.py`, written by `tools/prop-mesh.sh <prop>` to
    `Resources/<prop>.mesh`, each file read once), so another Blender prop is
    its rows there, its `Shape`s and its `res_embed_add` file
  - `Props/Bale` — the round bale (`tools/blender/bale.py`, under 1,500
    triangles): a faceted drum with rounded shoulders, domed ends coiled in a
    spiral, three twine bands and a few loose straws, as `Shape::BaleStraw` and
    `BaleTwine` in `makeBarrel`'s frame (along y from 0 to 1, half a unit in
    radius); `baleParts` gives them with their colours, the barn's hay and
    twine
  - `Props/Mover` — a drum rolling to and fro on a closed-form timeline
    (`positionAt`, `modelAt`) with a collider that follows it, drawn as its
    `parts`; `lyingDrum` lays `makeBarrel`'s frame on its side, for bales
    and movers alike; `makeRollingBale` in `Props/Bale`
- `Lib/cowsinlove_World` — how actors are laid out and collided with. Links
  Actors.
  - `Level` — a `Scenery` plus the hideout, `start` / `startHeading`, `gaps`
    (holes in the ground with a floor depth), `movers` (their `moving` batch
    rebuilt by `update(seconds)`), `killDepth` and `criticalPath`;
    `pushedOut` / `floorAt` / `overGap` / `isFree`; `maxStepUpHeight`.
    A floating block (`Block::bottom` set: a loft, a lintel, a roof's pitch)
    is passed under by feet below its bottom and more than a step under its
    top there; `ceilingAt` is the lowest bottom over the feet, which `Game`
    keeps her jump `playerHeight` short of; `clearReach` clips the camera's
    line (a floating ramp counts as a slab a step deep)
  - `Levels/` — level generation: `Region` (an xz
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
    `start()` leaves the menu, `openMenu()` pauses into it, `getStateBehindMenu()` is the
    state behind it; a `reset` from play skips the menu), player movement (one world unit is a metre; the jump is set by its height and airtime in `Cow/CowPhysics`), found test, moo cooldown, the moo
    hint, the level's clock (`seconds`, driving its movers), the checkpoint and
    respawn below `killDepth` (`sinceFell`), `footerText` (for a `Hints`), and
    `getInputRecipient` (an `InputRecipient`, worked out from the state, an
    open editor and the swing: menu, editor or play takes touches; play from
    the moment Start swings the camera down); `reset(seed)` builds the level through its
    `makeLevel` hook, which the app sets (the library never names a level)
  - `Ending` — the ending's numbers (title rise, kiss point, camera settle,
    `leastWidth` so the title fits a phone held upright), `stagingFor(hiding)`
    (where the title stands and the camera settles: out in the open behind
    the pair; for the loft from the meadow in front, the pair framed in the
    hay door and the title over the ridge), `titlePlacement`, `foundStart`,
    `hasEndingPlayedOut` (start again after the title)
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
- `Apps/CowsInLove/Source/Stages` — the content. A stage is a position in the
  ordered list of level templates (meadow, then meadow → ravine → meadow); a
  level is one generated from a template and a seed. `Stages {stage}` starts
  there (wrapped into range), `advance()` goes to the next after the ending
  (wrapping; nothing is saved), `level()` is the current one as a
  `LevelMaker`; `clockSeed()` is a seed from the clock. `CowsApp` reads
  `COWS_STAGE` (else 0) and `COWS_SEED` (else `clockSeed()`) and hands both to
  `CowsView`; `CowsArt` passes its own. `r` retries the stage with a fresh seed
- `Apps/CowsInLove/Source/Settings` — everything kept between runs, one Miro
  document, `settings.json` in `FilePath::appSupportDirectory()`: the
  `CowSkin`, the `QualityPreference` and the `FrameRate`; anything unreadable
  or unnamed loads as the default, and a new setting is one field. No other file is written
- `Apps/CowsInLove/Source/Scene/Quality` — `Quality` (Low, Medium, High) and
  `getSettingsForQuality`, the one place every tier's knobs are set (MSAA, render scale,
  mesh detail, grass tiles, density and blade segments, grass and ground noise,
  shadow map side and taps); `QualityChoice` (Auto or a tier) and
  `chooseQualityToUse` (COWS_QUALITY, else the choice, else the measured tier)
- `Apps/CowsInLove/Source/Scene/FrameRate` — `FrameRate` (Auto, 30, 60, Max) and
  `getFrameRateCap`, the cap `CowsView::applyFrameRateCap` hands
  `GPUView::setMaxFps` each update: Max every display refresh, 30 or 60 everywhere,
  Auto every refresh in play and during a swing and 60 while the menu or the
  editor has the input
- `Apps/CowsInLove/Source/CowsApp` — the window: scene, footer, touch controls,
  root view, and the `GameInput` the scene polls for controllers; wires them the
  same on every platform and ends by attaching the platform (the app has no
  platform directories or branches). The footer names the last-used input's
  controls; the touch controls hide while a controller was used last
- `Apps/CowsInLove/Source/Scene/CowsView` — the `GPUView`, made from a
  `QualityPreference`, the `Stages` and the first round's seed (later rounds
  take `clockSeed()`): gathers instances,
  shadow pass, main pass (ending with the `Hud`), camera steering, mouse;
  forwards keys to `Input`; polls `GameInput` each `update`, sends the
  controller's buttons through `control()` as touch does, orbits on the right
  stick (a look, mouse drag included, holds off the chase for `lookHold`), and
  tracks the last-used input as `hints`. In the menu it holds the camera in
  front of the cow (`menuPose`, drifting), draws the menu title, and takes
  Enter / Space / arrows / A D and the controller's South / Start / stick /
  D-pad; Start swings the camera behind the cow over 1.1 s with an ease-out
  while the menu fades and the title slides up, then play starts (`playPose`);
  Esc, Android's back, the controller's Start and the touch back arrow swing back,
  even while Start's swing is still under way. Dress Your Cow and Settings swing
  round to the cow (`editorPose`) and hand keys and the controller to that
  `Editor` (`openEditor(which)`); the camera orbits freely there (drag, right
  stick, touch drag), and Done, Esc and the controller's East / Start swing
  back to the menu. `CowsApp` saves `Settings` on every change
- `tools/blender` — Blender scripts for meshes (`barn.py`); see its README
- `tools/perf.sh` (`just perf`) and `tools/perf-stats.py` (`just perf-compare`)
  time the five scenarios and compare two builds; `tools/perf-sample.sh` takes
  `sample` call graphs that `tools/perf-sample-top.py` reduces to on-CPU
  frames; `tools/powermetrics.sh` captures power that `tools/perf-power.py`
  reads
- `tools/Art` — `CowsArt`, a macOS tool that renders the icon, key art, logo
  and store screenshots from the game's own views (`CowsArt screens <dir>
  [menu|dress|settings|about]` renders those screens at sixteen phone,
  tablet and desktop shapes, logging each one's text size and shape;
  `CowsArt barn <dir>` stands her at spots inside a barn with the camera
  swung each way its eye could leave it, logging where the eye ends up;
  `CowsArt endings <dir>` renders the ending for a seed of each hiding kind
  at three moments, and the hold upright);
  `tools/store-art.sh` and
  `tools/screenshots.sh` drive it. `tools/release-*.sh`, `release-msix.ps1`
  and `steam-upload.sh` build and package each store; `Deploy/README.md` is
  the runbook
- `Tests/Layout`, `Tests/Engine`, `Tests/AudioEngine`, `Tests/Actors`, `Tests/World`,
  `Tests/Game` — one NanoTest executable per library, and `Tests/CowsInLove`
  for the app's content; run with `ctest --test-dir build` or `just test`.
  `Tests/Support` holds the shared `TestMain` and `SnapshotView`, which draws a
  `SurfaceBatch` (plus anything in `drawOpaque`) off-screen and saves
  `docs/shots/tests/<name>.png`; snapshot tests skip without a GPU

`COWS_TIME=<seconds>` starts the clock there and `COWS_FREEZE=1` stops it (the
camera's swing between screens still runs), for
screenshots; `COWS_SEED=<n>` fixes the level, `COWS_STAGE=<n>` starts on
stage n (1 is the ravine) and `COWS_FOUND=1` starts beside her, skipping the menu; `COWS_WALK=1` holds walk ahead from the start, for profiling; `COWS_MENU=0` skips
the menu, `COWS_DRESS=1` opens the cow editor, `COWS_SETTINGS=1` opens
Settings, `COWS_START=<seconds>` presses Start after that long and
`COWS_WINDOW=<w>x<h>` opens the window at that size in points, as far as the screen allows (run the binary in `build/Apps/CowsInLove/Cows In Love.app/Contents/MacOS/` directly).
`COWS_PROFILE=1` logs, once a second, the frame interval, the CPU time of each
part of a frame, the GPU time of the frame and of each pass (`shadows`,
`scene`) and the instances and triangles each pass drew, and `input to
<menu|editor|play>` whenever that changes (`tools/perf.sh` reruns a run that
logs one in its window); a profiling run
varies one of `COWS_BLADES=<n>` (blades drawn per grass tile), `COWS_MSAA=<n>`,
`COWS_SHADOW=<texels>` (the shadow map's side), `COWS_RENDER_SCALE=<share>`,
`COWS_DETAIL=<share>` (mesh detail, see `detailed` in `Render/Mesh`) or
`COWS_SKIP=<parts>` (any of sky, ground, objects, grass, glow, hud,
shadows, left out of the frame) and compares.
`COWS_QUALITY=low|medium|high` forces a quality tier at launch (`Scene/Quality`)
until the player picks one in Settings (the start menu's third item, an
`Editor` with two rows, Quality: Auto, Low, Medium, High and Frame rate: Auto,
30, 60, Max; each applies at once). `COWS_FPS=auto|30|60|max` forces the frame rate
for the run whatever Settings says (`Scene/FrameRate`); `tools/perf.sh` pins it. On Auto
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

Every target builds with floating-point contraction off (`-ffp-contract=off` in
the top `CMakeLists.txt`, for Clang and GCC; MSVC does not fuse): a seed must
generate the same level on every platform, and FMA changes the low bits the level
generator's placement hinges on. `LevelGeneratorTests` checks the hashes.

iOS: `tools/ios.sh sim [shot.png]` builds `build-ios/` and runs it on the "Cows iPhone"
simulator; `tools/ios.sh device` signs with `COWS_TEAM` (default: Jamie's Personal Team) and runs it on the phone.

`just hotreload` (`-DCOWS_HOT_RELOAD_COW=ON`, `build-hot`) rebuilds `Cow/Cow.cpp`
and `Cow/CowPhysics.cpp` into a module while the app runs; each reload moves
`CowsView`'s `CowModel` onto the fresh code, and `CowsApp` pulls its parts into
the scene and its jump into `Game::setJumpPhysics`. Edit function bodies only:
a changed header is refused (undo it or restart), and a struct that cannot be
swapped stops the app with its name. Call `CowModel`'s methods through a
reference: clang binds a call on a named object or member to the app's copy.

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

## Naming

- Long, verbose, English, JUCE-style names. A name a stranger can't read aloud
  and understand is wrong: `pullCameraInFrontOfBlockingProps`, not
  `keepEyeClear` (whose eye?); `onRowValueStepped`, not `onStep`.
- Functions are verbs that say what they do: `makeFrameRateRow`,
  `getQualityChoiceLabel`, `parseFrameRateName`, `readFrameRateFromEnvironment`,
  `replaceUnknownValuesWithDefaults`; not `frameRateRow`, `frameRateNamed`,
  `withKnownValue`.
- Casts go in named temporaries (`auto numRows = static_cast<int>(rows.size());`),
  never nested inline. An enum is never cast to an index: look it up in its
  table (`frameRatesAndLabels`), whose order is the row's. Stepping an enum is
  a helper next to it (`stepFrameRate(rate, steps)`, through
  `stepThroughEntries`), not arithmetic at the call site.
- No magic indices: dispatch on a row's or item's name, not `row == 1`.

## Dead code

`just dead-code` (`tools/dead-code.sh`, macOS, `brew install llvm cppcheck`)
lists code nothing uses and deletes nothing. It builds `build-dead-code` once
with extra unused warnings and dead-stripped links that write link maps, then
runs clang-tidy's include-cleaner, cppcheck's unusedFunction, a libclang pass
matching every declaration to its references (`tools/dead-code-refs.py`), the
link maps against our objects' symbols (`tools/dead-code-link.py`), and checks
for files nothing builds, includes, calls or shows. "Test-only" lists what only
`Tests/` uses: delete it with its tests or keep it as a test seam, on purpose.

CI gates on it: the `dead-code` job in `.github/workflows/build.yml` runs
`tools/dead-code.sh --check` on macOS, which fails (with inline PR
annotations, through `tools/dead-code-check.py`) on any compiler or tidy
warning, cppcheck finding, unused or test-only symbol, symbol no executable
keeps, or file nothing builds, includes, embeds or calls; images no doc shows
are only listed. Test-only code kept on purpose gets a line in
`tools/dead-code-allow.txt`, the symbol and why; a line nothing matches fails
too. Run `tools/dead-code.sh --check` locally before pushing.
