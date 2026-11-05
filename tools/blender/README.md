# Blender meshes

Each prop modelled in Blender is a script here, written in Blender's Python
API (no hand-made `.blend` file), that exports one OBJ with one group per
material, each group a `Shape` in the game (`Props/PropMeshes` lists them):

- `barn.py` — the barn: `wall`, `trim`, `roof`, `wood`, `hay`, `metal`,
  `twine`
- `bale.py` — the round bale, in `makeBarrel`'s frame (along y from 0 to 1,
  half a unit in radius): `straw`, `twine`

`meshkit.py` holds what they share: solids written in the game's space and
the export.

To regenerate `Lib/cowsinlove_Actors/Resources/<prop>.obj` after changing a
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
frame kept. The OBJs are committed, so a build never needs Blender.

A new prop is a `<prop>.py` here, its `Shape`s at the end of the enum in
`Render/Instances.h`, its rows in `Props/PropMeshes.cpp` and its OBJ in the
`res_embed_add` of `Lib/cowsinlove_Actors/CMakeLists.txt`.
