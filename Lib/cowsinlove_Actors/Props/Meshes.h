#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

#include <functional>

namespace Cows
{
MeshData makeContentMesh(Shape shape);

// `detail` coarsens only the cow's meshes; the props' have one detail.
std::function<MeshData(Shape)> contentMeshes(float detail);
} // namespace Cows
