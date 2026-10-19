#pragma once

#include "Render/Instances.h"

namespace Cows
{
// Stateless, so any moment can be drawn without playing the ones before it.
void addKissHearts(SurfaceBatch& batch,
                   Vector<GlowInstance>& glows,
                   float seconds,
                   Maths::Vec3 kissPoint);
} // namespace Cows
