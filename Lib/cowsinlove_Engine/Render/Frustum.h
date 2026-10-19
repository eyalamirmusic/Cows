#pragma once

#include "Render/Instances.h"

namespace Cows
{
// Conservative: false only when all eight corners lie past one side of the view.
bool boxInView(const Maths::Mat4& viewProjection,
               const Maths::Vec3& low,
               const Maths::Vec3& high);

bool sphereInView(const Maths::Mat4& viewProjection,
                  const Maths::Vec3& center,
                  float radius);

bool instanceInView(const Maths::Mat4& viewProjection,
                    const SurfaceInstance& instance,
                    float meshRadius);
} // namespace Cows
