# Project structure

Each layer is its own static library under `Lib/`, linking only the layers
below it; the app links the top. One NanoTest executable per library under
`Tests/`; render tests draw off-screen through `View::renderToImage` and save
PNGs to `docs/shots/tests/`.

```
Lib/cowsinlove_Engine       <- eacp-gpu            Render, Camera, UI, Platform
Lib/cowsinlove_AudioEngine  <- eacp                sample playback
Lib/cowsinlove_Actors       <- Engine, AudioEngine Animation, Cow, Props, Sky
Lib/cowsinlove_World        <- Actors              Level, Terrain, Levels/Meadow
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
    Platform/  macOS/, iOS/: Platform per platform (iOS: TouchSurface); the
               current one is on the include path

  cowsinlove_AudioEngine/         SamplePlayer: the sample player half of Moo

  cowsinlove_Actors/              a thing in the world: model + behaviour
    Animation/ Choreography: the original's timings (sway, kiss, heartbeat,
               hops, sun pulse, title wave)
    Cow/       Cow, Moo (the cow's voice, with Resources/moo.f32),
               KissHearts, HeartMesh
    Props/     Collision (Collider, Block), Scenery, Props (shared helpers),
               Barn, Tree, Hedge, Bale, Crate, Rock   (split out of Obstacles.cpp)
    Sky/       Sun, Clouds, Hills (SkyDecor)

  cowsinlove_World/               how actors are laid out and collide
    Level          the interface: colliders, blocks, floorAt, hideouts
    Terrain/       Grass (makeBlade, makeGrassTile), TerrainShaders
    Levels/        reusable generators, each `Level makeX(seed)`: Meadow now,
                   Farmyard, ... later. Which one a game plays, and with what
                   seed, is the app's content

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
  Stages                          the content: which levels are played and
                                  where their seeds come from (COWS_SEED, else
                                  the clock; a fresh meadow each round)
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
- **World/Levels/** is a catalogue of reusable generators; a meadow can appear
  in several stages of one game and in other games. `Level` is the interface
  `Game` talks to, and the app hands `Game` a `LevelMaker` and a seed, so the
  seed is the content and the libraries never name a level.
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

