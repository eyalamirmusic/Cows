#include "Levels/Region.h"

using namespace Maths;

namespace Cows
{
Region Region::arena()
{
    auto half = arenaSize * 0.5f;
    return {{-half, -half}, {half, half}};
}

bool Region::contains(Vec2 point, float margin) const
{
    return point.x >= min.x - margin && point.x <= max.x + margin
           && point.y >= min.y - margin && point.y <= max.y + margin;
}

bool Region::holds(Vec2 point) const
{
    return point.x > min.x && point.x < max.x && point.y > min.y && point.y < max.y;
}

Region Region::inset(float by) const
{
    return {min + Vec2 {by, by}, max - Vec2 {by, by}};
}

bool Region::isEmpty() const
{
    return max.x <= min.x || max.y <= min.y;
}

Vec2 Region::center() const
{
    return (min + max) * 0.5f;
}

Vec2 Region::size() const
{
    return max - min;
}

float Region::area() const
{
    return size().x * size().y;
}
} // namespace Cows
