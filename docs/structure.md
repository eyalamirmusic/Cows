# Project structure

Each layer is its own static library under `Lib/`, linking only the layers
below it; the app links the top. One NanoTest executable per library under
`Tests/`; render tests draw off-screen through `View::renderToImage` and save
PNGs to `docs/shots/tests/`.

```
Lib/cowsinlove_Engine       <- eacp-gpu            Render, Camera, UI, Platform
Lib/cowsinlove_AudioEngine  <- eacp, MakeASound    sample playback
Lib/cowsinlove_Actors       <- Engine, AudioEngine Animation, Cow, Props, Sky
Lib/cowsinlove_World        <- Actors              Level, Terrain, Levels
Lib/cowsinlove_Game         <- World              Game, Ending, Input, Title
Apps/CowsInLove             <- Game                CowsApp, Scene
```

The pattern every engine converges on: split by **dependency layer**, then by
**thing** inside each layer. Unreal does it with modules and Actors, Godot with
scenes, Unity projects with `Scripts/{Player,Enemies,Levels,UI}`. The rule
underneath is the same: lower layers never include upper ones.

```
Lib/
  cowsinlove_Engine/              knows nothing about cows
    Render/    Common, Mesh, Instances, Shaders, Shading, Lighting, ShadowMap,
               Palette
    Camera/    OrbitCamera
    UI/        Overlay (footer), TouchControls
    Platform/  macOS/ (also Windows), iOS/, Android/: Platform per platform
               (iOS, Android: TouchSurface; Android: HudLayer); the current
               one is on the include path

  cowsinlove_AudioEngine/         SamplePlayer: the sample player half of Moo;
                                  output through MakeASound on every platform

  cowsinlove_Actors/              a thing in the world: model + behaviour
    Animation/ Choreography: the original's timings (sway, kiss, heartbeat,
               hops, sun pulse, title wave)
    Cow/       Cow, Moo (the cow's voice, with Resources/moo.f32),
               KissHearts, HeartMesh
    Props/     Collision (Collider, Block), Scenery, Props (shared helpers),
               Barn, Tree, Hedge, Bale, Crate, Rock   (split out of Obstacles.cpp),
               Log, Fence, Bridge, Mover (a rolling barrel on a closed-form
               timeline)
    Sky/       Sun, Clouds, Hills (SkyDecor)

  cowsinlove_World/               how actors are laid out and collide
    Level          the interface: colliders, blocks, gaps, movers, start,
                   killDepth, floorAt, hasGround, update(seconds), hideout
    Terrain/       Ground (the plane minus gaps, chasm rock), Grass
                   (makeBlade, makeGrassTile, GrassField), TerrainShaders
    Levels/        level generation, in the usual segment/prefab vocabulary:
      Region         an xz rectangle
      Layout         the seeded draws, the start's clearing, the critical
                     path, hiding places, hasRoom, chooseHideout (the goal)
      Biome          what dresses a region: prop counts for the whole arena,
                     scaled by area. meadowBiome(); populate() is the
                     populate pass
      Segments/      Segment: a prefab slice `length` deep with build() and
                     an optional biome. MeadowSegment (just a biome),
                     JumpLineSegment (a full-width line of logs or fences),
                     RavineSegment (gap, bridge on the path, bales from
                     MoverSpawners)
      LevelTemplate  the content: segments in order, width, how far the path
                     may stray, start heading
      LevelGenerator generate(template, seed): lays segments along -z from
                     the start, reserves the critical path (start to the last
                     segment; populate never blocks it), builds each segment,
                     populates the biomes, picks the goal in the last segment
                     (no templates ship in World: the segment lists a game
                     plays, and their seeds, are the app's content)

  cowsinlove_Game/                rules, nothing that owns a GPU pass
    Game           state machine, player movement, found test, moo hint,
                   footer text; builds its level through the `makeLevel`
                   hook the app sets, never naming a generator
    Ending         title rise and placement, kiss point, camera settle
                   numbers, loop after the title
    Input          held keys and touch stick -> walk ahead / turn, jump
    Title/         TitleFont, TitleShader

Apps/CowsInLove/Source/
  Main.cpp, CowsApp.{h,cpp}       app wiring
  Stages                          the content: the level templates in order
                                  (meadow; meadow -> ravine -> meadow), advance
                                  after each ending (wrapping, nothing saved),
                                  COWS_STAGE to start elsewhere, and where the
                                  seeds come from (COWS_SEED, else the clock)
  Scene/CowsView                  the GPUView: gathers instances, shadow pass,
                                  main pass, camera steering, mouse; forwards
                                  keys to Input

Tests/
  Support/                        TestMain, SnapshotView
  Engine/, AudioEngine/, Actors/, World/, Game/   one executable per library
  CowsInLove/                     the app's own content (Stages)
```

## Why this shape

- **Engine/** is the part you could lift into the next game untouched.
  Everything in it already avoids game types, so this is mostly a move.
- **Actors/** is one directory per kind of thing. `Obstacles.cpp` was 619
  lines because it was six props plus a level generator plus the hideout
  picker in one file. Each prop is now `Props/Barn.{h,cpp}` with its mesh and
  collider, added to a `Scenery`; a random prop takes the level's
  `std::mt19937` and draws from it in the original order, so seeded layouts
  are unchanged. The generator (placement loops, hideouts) is now
  `World/Levels/Meadow.cpp`, and what was `Obstacles` is `World/Level`.
- **World/Levels/** generates levels from templates: a template is a list of
  segments (prefab slices), the generator lays them along the path, builds
  them, populates their biomes and picks the goal. A meadow can appear in
  several stages of one game and in other games. `Level` is the interface
  `Game` talks to, and the app hands `Game` a `LevelMaker` (`generate` bound
  to a template) and a seed, so template and seed are the content and the
  libraries never name a level.
- **Game/** holds rules, nothing that owns a GPU pass (the title shader is
  a program the app draws). `Choreography` was planned here
  as the design of *this* game, but Cow, KissHearts and SkyDecor all move to
  its timings, so it lives in `Actors/Animation` below them.
- **Scene/CowsView** was 722 lines; with input and the ending's rules moved
  to Game it is orchestration only. Camera steering stays in it: it is a
  series of `OrbitCamera` calls, and only its numbers moved to `Ending`.

## Conventions

- Each library's root is its PUBLIC include directory; include with the
  subdirectory in the path (`#include "Render/Mesh.h"`, `#include
  "Cow/Cow.h"`, `#include "Props/Barn.h"`). A library can only see the
  libraries it links, so an upward include fails to compile.
- One CMake source list per directory, so adding a prop or level is one line in
  the right place.

## Order of moves

Each step is a mechanical `git mv` plus include fixes, building between steps:

1. Engine/Render (lowest risk) — done
2. Actors: Animation, Cow, Sky, and Props out of Obstacles — done
3. World: Level, Levels/Meadow, Terrain (Grass and its shaders) — done
4. Game: Game, Ending, Input, Title out of the app and CowsView — done

