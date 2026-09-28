#include "Camera/OrbitCamera.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto minPitch = -0.08f;
constexpr auto maxPitch = 1.3f;
constexpr auto minDistance = 5.f;
constexpr auto maxDistance = 40.f;
constexpr Vec3 worldUp {0.f, 1.f, 0.f};
constexpr auto followRate = 6.f;
} // namespace

void OrbitCamera::orbit(float horizontal, float vertical)
{
    yaw -= horizontal;
    pitch = std::clamp(pitch + vertical, minPitch, maxPitch);
}

void OrbitCamera::zoom(float amount)
{
    distance = std::clamp(distance * std::exp(-amount), minDistance, maxDistance);
}

void OrbitCamera::drift(float seconds)
{
    swayYaw = 0.07f * std::sin(seconds * 0.11f);
    swayPitch = 0.015f * std::sin(seconds * 0.17f + 1.f);
}

void OrbitCamera::follow(Vec3 goal, float delta)
{
    target += (goal - target) * std::min(1.f, delta * followRate);
}

void OrbitCamera::turnToward(float wantedYaw, float amount)
{
    yaw += std::remainder(wantedYaw - yaw, twoPi) * std::clamp(amount, 0.f, 1.f);
}

Vec3 OrbitCamera::eye() const
{
    auto turn = yaw + swayYaw;
    auto tilt = pitch + swayPitch;
    auto offset = Vec3 {std::cos(tilt) * std::sin(turn),
                        std::sin(tilt),
                        std::cos(tilt) * std::cos(turn)};

    return target + offset * distance;
}

Vec3 OrbitCamera::forward() const
{
    return normalize(target - eye());
}

Vec3 OrbitCamera::right() const
{
    return normalize(cross(forward(), worldUp));
}

Vec3 OrbitCamera::up() const
{
    return cross(right(), forward());
}

Mat4 OrbitCamera::view() const
{
    return Mat4::lookAt(eye(), target, worldUp);
}

Mat4 OrbitCamera::projection(float aspect) const
{
    return Mat4::perspective(
        aspect, verticalFieldOfView(aspect), nearPlane, farPlane);
}

float OrbitCamera::verticalFieldOfView(float aspect) const
{
    auto fitsWidth = 2.f * std::atan(std::tan(leastWidth * 0.5f) / aspect);
    return std::max(fieldOfView, fitsWidth);
}
} // namespace Cows
