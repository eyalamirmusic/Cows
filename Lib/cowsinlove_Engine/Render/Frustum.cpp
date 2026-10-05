#include "Render/Frustum.h"

#include <array>

using namespace Maths;

namespace Cows
{
bool boxInView(const Mat4& viewProjection, const Vec3& low, const Vec3& high)
{
    auto corners = std::array<Vec4, 8> {};

    for (auto index = 0; index < 8; ++index)
    {
        auto corner = Vec3 {(index & 1) ? high.x : low.x,
                            (index & 2) ? high.y : low.y,
                            (index & 4) ? high.z : low.z};
        corners[(size_t) index] =
            viewProjection * Vec4 {corner.x, corner.y, corner.z, 1.f};
    }

    auto allOutside = [&](auto outside)
    {
        for (const auto& corner: corners)
            if (!outside(corner))
                return false;

        return true;
    };

    return !(allOutside([](const Vec4& c) { return c.w <= 0.f; })
             || allOutside([](const Vec4& c) { return c.x < -c.w; })
             || allOutside([](const Vec4& c) { return c.x > c.w; })
             || allOutside([](const Vec4& c) { return c.y < -c.w; })
             || allOutside([](const Vec4& c) { return c.y > c.w; }));
}
} // namespace Cows
