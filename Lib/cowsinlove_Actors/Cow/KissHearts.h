#pragma once

#include "Render/Instances.h"

namespace Cows
{
// The burst of hearts that rises from where the two heads meet, every kiss.
// Worked out from the clock alone, so any moment can be drawn without having
// played the ones before it.
void addKissHearts(SurfaceBatch& batch,
                   Vector<GlowInstance>& glows,
                   float seconds,
                   Maths::Vec3 kissPoint);
} // namespace Cows
