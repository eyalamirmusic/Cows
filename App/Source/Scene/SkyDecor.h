#pragma once

#include "Instances.h"

namespace Cows
{
// The backdrop: the pulsing sun with its rays, the two clouds drifting behind
// it, and the hills on the horizon.
namespace SkyDecor
{
constexpr Maths::Vec3 sunCenter {23.f, 9.f, -42.f};

void addSun(SurfaceBatch& batch, Vector<GlowInstance>& glows, float seconds);
void addClouds(SurfaceBatch& batch, float seconds);
void addHills(SurfaceBatch& batch);
} // namespace SkyDecor
} // namespace Cows
