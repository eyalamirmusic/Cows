# Find the Other Cow — game plan

Turn the postcard into a tiny single-player game: you are the left cow, the
other cow is somewhere out in the meadow. Walk around, find her, and the
original kiss animation plays as the ending. Speed over quality; nothing shit.

## Loop

1. **Start**: player cow at the origin facing +x. Partner placed at a random
   spot 45–80 units away (seeded from the clock; `COWS_SEED=<n>` overrides).
   Title hidden, footer says `wasd / arrows to walk  -  drag to look  -  q to quit`.
2. **Searching**: WASD/arrows move the player relative to camera yaw; the cow
   turns toward its motion. Hops (existing hop choreography) only while moving,
   idle stands still (tail still swishes). Heart eyes beat faster and glow
   harder the nearer the partner is (period 0.8s → 0.25s at 5 units), the
   warmer/colder hint. Play area clamped to ±90.
3. **Found**: when the two are within `kissGap`-ish (~3.5 units), lock both cows
   facing each other 2×kissReach apart, restart the clock at the sway cycle's
   kiss, and run the original kiss/heart-burst choreography at that spot.
   Title rises in over the pair, footer: `you found her  -  r to play again  -  q to quit`.
   `r` resets with a new seed.

## Hiding

A flat meadow hides nothing, so:

- Haze/fog pulled in so the partner fades at ~35 units (tune the existing haze
  in `Shading`, one constant).
- ~60 scattered bushes/rocks (spheres, palette greens/greys, instanced via the
  existing backdrop batch) so there's something to look behind. Partner is
  guaranteed to spawn behind a bush from the origin's point of view isn't
  needed — the fog does the work.

## Camera

`OrbitCamera` stays: its `target` follows the player each frame (smoothed),
drag still orbits, scroll zooms, drift only after the win. Movement is in the
camera's yaw frame.

## Things that assume the origin (must follow the player)

- `ShadowMap` focus → player's xz.
- Grass: meadow is a 60-unit disc round the origin. Make the disc a tile and
  draw it 4× with a per-draw offset snapped to a grid round the player (uniform
  `patchOffset` on `GrassShader`), or just spread `bladeCount` uniformly over
  ±100 and accept sparser grass. Either is fine; pick the quicker.
- Sun, clouds, hills: backdrop, translate by the player's xz so they stay at the
  horizon.
- `groundShader` contact shadows: from the two cows' real positions.
- `kissPoint` for hearts: midpoint of the two heads at the win.

## Code shape

- New `Scene/Game.h/.cpp`: `struct Game` with state (`Searching`/`Found`),
  player position/heading/speed, partner position, seed, `update(dt, input)`,
  `distance()`, `reset()`. Pure logic, no GPU.
- `Cow` gets a free placement: `Mat4 placement(Vec3 position, float heading, float hopPhase)`
  path alongside the existing choreographed one (used again for the ending).
- `CowsView` owns the `Game`, reads keys via `keyDown/keyUp` (eacp `View` has
  both), passes input in `update`, and drives instances from game state.
- `FooterView` reads the game state for its text (pointer or callback).
- `RootView` forwards keys it doesn't handle to the scene.

## Obstacles (added)

Fog and bushes alone aren't a search. The meadow gets real obstacles that
block movement and sight, built from the existing primitives so they match
the look: trees (capsule trunk + sphere canopies), hedgerows (capsules/barrels
lying along the ground in lines), hay bales (barrels on their side), rocks
(squashed spheres). 120–200 of them laid out with a seeded RNG in clusters and
hedgerow lines, leaving corridors and pockets; the partner spawns in a pocket
not visible from the origin. Each has a collision circle (x, z, r) and the
player slides around them. Lives in `Scene/Obstacles.h/.cpp`.
