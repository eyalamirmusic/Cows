#include "Props/Collision.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
Vec2 absolute(Vec2 vector)
{
    return {std::abs(vector.x), std::abs(vector.y)};
}

float Block::heightAt(Vec2 point) const
{
    if (lengthSquared(rise) == 0.f)
        return top;

    auto along = dot(absolute(rise), half);
    auto t = (dot(point - center, rise) / along + 1.f) * 0.5f;
    return top * std::clamp(t, 0.f, 1.f);
}

Vec2 Block::closestTo(Vec2 point) const
{
    return {std::clamp(point.x, center.x - half.x, center.x + half.x),
            std::clamp(point.y, center.y - half.y, center.y + half.y)};
}

bool Block::contains(Vec2 point, float margin) const
{
    return std::abs(point.x - center.x) <= half.x + margin
           && std::abs(point.y - center.y) <= half.y + margin;
}
} // namespace Cows
