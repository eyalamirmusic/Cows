#pragma once

#include "Render/Common.h"

namespace Cows
{
// Whether any of the box from `low` to `high` can land on screen through
// `viewProjection`: false only when all eight corners are past one side of the
// view, or all behind the eye. Conservative, as culling has to be: a box it
// keeps may still miss the screen, one it drops never reaches it.
bool boxInView(const Maths::Mat4& viewProjection,
               const Maths::Vec3& low,
               const Maths::Vec3& high);
} // namespace Cows
