#pragma once

#include "Render/Instances.h"

namespace Cows
{
// Whether any of the box from `low` to `high` can land on screen through
// `viewProjection`: false only when all eight corners are past one side of the
// view, or all behind the eye. Conservative, as culling has to be: a box it
// keeps may still miss the screen, one it drops never reaches it.
bool boxInView(const Maths::Mat4& viewProjection,
               const Maths::Vec3& low,
               const Maths::Vec3& high);

bool sphereInView(const Maths::Mat4& viewProjection,
                  const Maths::Vec3& center,
                  float radius);

// Whether an instance of a mesh `meshRadius` across from its origin can land on
// screen: its sphere, carried by the instance's model matrix and grown by the
// matrix's largest scale.
bool instanceInView(const Maths::Mat4& viewProjection,
                    const SurfaceInstance& instance,
                    float meshRadius);
} // namespace Cows
