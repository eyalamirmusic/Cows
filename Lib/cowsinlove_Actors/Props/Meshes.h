#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

#include <functional>

namespace Cows
{
// The meshes ShapeMeshes leaves to the game: the cow's (Cow/Wardrobe) and the
// props' Blender meshes (Props/PropMeshes).
MeshData makeContentMesh(Shape shape);

// makeContentMesh with the cow's steps coarsened by `detail` (see `detailed`);
// the props' meshes have one detail.
std::function<MeshData(Shape)> contentMeshes(float detail);
} // namespace Cows
