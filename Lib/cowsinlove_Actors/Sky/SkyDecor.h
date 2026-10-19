#pragma once

#include "Render/Instances.h"

namespace Cows
{
// Placed round `origin` so they stay on the horizon wherever the player walks.
namespace SkyDecor
{
constexpr Maths::Vec3 sunCenter {23.f, 9.f, -42.f};

void addSun(SurfaceBatch& batch,
            Vector<GlowInstance>& glows,
            float seconds,
            Maths::Vec3 origin);
void addClouds(SurfaceBatch& batch, float seconds, Maths::Vec3 origin);
void addHills(SurfaceBatch& batch, Maths::Vec3 origin);
} // namespace SkyDecor
} // namespace Cows
