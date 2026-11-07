#include "Render/Frustum.h"

#include <algorithm>
#include <array>

using namespace Maths;

namespace Cows
{
namespace
{
Vec3 boxCorner(const Vec3& low, const Vec3& high, int index)
{
    auto pickAxis = [index](int bit, float lowValue, float highValue)
    { return (index & bit) != 0 ? highValue : lowValue; };

    return {pickAxis(1, low.x, high.x),
            pickAxis(2, low.y, high.y),
            pickAxis(4, low.z, high.z)};
}

using ClipTest = bool (*)(const Vec4&);

const std::array<ClipTest, 5> outsideClipPlanes {
    [](const Vec4& c) { return c.w <= 0.f; },
    [](const Vec4& c) { return c.x < -c.w; },
    [](const Vec4& c) { return c.x > c.w; },
    [](const Vec4& c) { return c.y < -c.w; },
    [](const Vec4& c) { return c.y > c.w; },
};
} // namespace

bool boxInView(const Mat4& viewProjection, const Vec3& low, const Vec3& high)
{
    auto corners = std::array<Vec4, 8> {};

    for (auto index = 0; index < 8; ++index)
    {
        auto corner = boxCorner(low, high, index);
        corners[(size_t) index] =
            viewProjection * Vec4 {corner.x, corner.y, corner.z, 1.f};
    }

    auto areAllCornersOutside = [&corners](ClipTest isOutside)
    { return std::all_of(corners.begin(), corners.end(), isOutside); };

    return std::none_of(
        outsideClipPlanes.begin(), outsideClipPlanes.end(), areAllCornersOutside);
}

bool sphereInView(const Mat4& viewProjection, const Vec3& center, float radius)
{
    auto reach = Vec3 {radius, radius, radius};
    return boxInView(viewProjection, center - reach, center + reach);
}

bool instanceInView(const Mat4& viewProjection,
                    const SurfaceInstance& instance,
                    float meshRadius)
{
    auto scale = std::max({length(instance.model0.xyz()),
                           length(instance.model1.xyz()),
                           length(instance.model2.xyz())});

    return sphereInView(viewProjection, instance.model3.xyz(), meshRadius * scale);
}
} // namespace Cows
