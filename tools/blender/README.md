# Blender meshes

Each prop modelled in Blender is a script here, written in Blender's Python
API (no hand-made `.blend` file), that writes one binary mesh blob with one
group per material, each group a `Shape` in the game (`Props/PropMeshes` lists them):

- `barn.py` — the barn: `wall`, `trim`, `roof`, `wood`, `hay`, `metal`,
  `twine`
- `bale.py` — the round bale, in `makeBarrel`'s frame (along y from 0 to 1,
  half a unit in radius): `straw`, `twine`

`meshkit.py` holds what they share: solids written in the game's space and
the blob writer.

To regenerate `Lib/cowsinlove_Actors/Resources/<prop>.mesh` after changing a
script (or, for the barn, the numbers in `Props/Barn.cpp`):

```bash
brew install --cask blender   # once
tools/prop-mesh.sh barn       # or bale; BLENDER=<path> to use another Blender
```

then rebuild and look at the snapshots (`ctest --test-dir build -R Props/`,
`docs/shots/tests/actors-barn*.png`, `actors-bale*.png`).

The numbers at the top of `barn.py` up to the model's own details are
`Props/Barn.cpp`'s: the collision is built from those, so change both
together. The bale's collision is `Props/Bale`'s own and only needs the
frame kept. The blobs are committed, so a build never needs Blender.

A new prop is a `<prop>.py` here, its `Shape`s at the end of the enum in
`Render/Instances.h`, its rows in `Props/PropMeshes.cpp` and its blob in the
`res_embed_add` of `Lib/cowsinlove_Actors/CMakeLists.txt`.

## The mesh blob

Little-endian, read by `Lib/cowsinlove_MeshLoader` (`readMeshBlob`):

| Field | Type | |
|---|---|---|
| magic | 4 bytes | `CMSH` |
| version | u16 | 1 |
| groups | u16 | how many follow |
| low, high | f32 × 3 each | the bounds every position is quantised against |
| *per group:* name length | u8 | |
| name | ASCII | the material, e.g. `trim` |
| vertices, indices | u32 each | counts |
| *per vertex:* position | i16 × 3 | -32768 at `low` to 32767 at `high` |
| normal | i8 × 3 | the face's normal × 127 (flat shading) |
| indices | u16 each, u32 when vertices ≥ 65536 | triangles, counter-clockwise from outside |

Positions land within half a step of 65535 across the bounds (about 0.2 mm
on the barn), well under its thinnest trim.
