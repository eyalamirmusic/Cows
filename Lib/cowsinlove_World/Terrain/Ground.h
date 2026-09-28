#pragma once

#include "Level.h"
#include "Render/Mesh.h"

namespace Cows
{
constexpr auto groundSize = 600.f;

// The ground plane, `size` on a side about the origin, with the level's gaps
// cut out of it. With no gaps it is makePlane(size).
MeshData makeGround(const Level& level, float size);

// Rock walls and floors lining the level's gaps.
SurfaceBatch makeChasms(const Level& level);
} // namespace Cows
