#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

namespace Cows
{
bool isPropShape(Shape shape);

MeshData makePropMesh(Shape shape);
} // namespace Cows
