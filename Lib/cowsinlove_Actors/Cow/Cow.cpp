#include "Cow/Cow.h"

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto tailUnderHide = 0.08f;

float legsAt(LegPair pair)
{
    return pair == LegPair::Back ? -0.6f : 0.58f;
}

Mat4 placeLeg(float x, float z)
{
    return Mat4::translation({x, 0.1f, z}) * Mat4::scale({1.25f, 0.95f, 1.25f});
}
} // namespace

Mat4 torsoPlacement()
{
    return Mat4::translation({0.f, 1.13f, 0.f}) * Mat4::rotationZ(-halfPi)
           * Mat4::scale({1.12f, 2.24f, 1.04f})
           * Mat4::translation({0.f, -0.5f, 0.f});
}

std::array<Mat4, 2> legPlacements(LegPair pair)
{
    auto x = legsAt(pair);
    return {placeLeg(x, -0.27f), placeLeg(x, 0.27f)};
}

Vec3 tailRoot()
{
    auto hanging = transformDirection(Mat4::rotationZ(tailSwing), {0.f, -1.f, 0.f});
    return tailPivot + hanging * tailUnderHide;
}
} // namespace Cows
