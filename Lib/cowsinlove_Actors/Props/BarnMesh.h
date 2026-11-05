#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

namespace Cows
{
// Whether `shape` is one of the barn's: BarnWall to BarnTwine.
bool isBarnShape(Shape shape);

// The barn's mesh for one of its Shapes, in barn space: its group of
// Resources/barn.obj (built by tools/blender/barn.py), read once.
MeshData makeBarnMesh(Shape shape);
} // namespace Cows
