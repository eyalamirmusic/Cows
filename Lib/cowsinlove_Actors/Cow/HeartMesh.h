#pragma once

#include "Render/Mesh.h"

namespace Cows
{
// A puffy heart, about two units across, facing +z with its point down; its
// steps coarsened by `detail` (see `detailed`).
MeshData makeHeart(float detail = 1.f);
} // namespace Cows
