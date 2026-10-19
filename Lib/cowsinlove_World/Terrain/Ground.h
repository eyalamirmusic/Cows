#pragma once

#include "Level.h"
#include "Render/Mesh.h"

namespace Cows
{
constexpr auto groundSize = 600.f;

MeshData makeGround(const Level& level, float size);

SurfaceBatch makeChasms(const Level& level);
} // namespace Cows
