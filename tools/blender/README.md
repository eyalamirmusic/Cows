# Blender meshes

`barn.py` models the barn in Blender's Python API (no hand-made `.blend`
file) and exports it as an OBJ, one group per material (`wall`, `trim`,
`roof`, `wood`, `hay`, `metal`, `twine`), each a `Shape` in the game.

To regenerate `Lib/cowsinlove_Actors/Resources/barn.obj` after changing the
script or the numbers in `Props/Barn.cpp`:

```bash
brew install --cask blender   # once
tools/barn-mesh.sh            # BLENDER=<path> to use another Blender
```

then rebuild and look at the barn snapshots (`ctest --test-dir build -R
Actors`, `docs/shots/tests/actors-barn*.png`).

The numbers at the top of `barn.py` up to the model's own details are
`Props/Barn.cpp`'s: the collision is built from those, so change both
together. The OBJ is committed, so a build never needs Blender.
