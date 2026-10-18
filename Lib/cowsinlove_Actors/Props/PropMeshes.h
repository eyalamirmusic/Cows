#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

namespace Cows
{
// Whether `shape` is drawn from one of the props' mesh blobs: the barn's
// (BarnWall to BarnTwine) or the bale's (BaleStraw, BaleTwine).
bool isPropShape(Shape shape);

// A prop's mesh for one of its Shapes, in the prop's own space: one group of
// the prop's Resources/<prop>.mesh (built by tools/blender/<prop>.py), the file
// read once.
MeshData makePropMesh(Shape shape);
} // namespace Cows
