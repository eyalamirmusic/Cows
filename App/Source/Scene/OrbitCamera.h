#pragma once

#include "Common.h"

namespace Cows
{
struct OrbitCamera final
{
    void orbit(float horizontal, float vertical);
    void zoom(float amount);

    // A slow idle sway, so the postcard breathes while nobody drags it.
    void drift(float seconds);

    Maths::Vec3 eye() const;
    Maths::Vec3 forward() const;
    Maths::Vec3 right() const;
    Maths::Vec3 up() const;

    Maths::Mat4 view() const;
    Maths::Mat4 projection(float aspect) const;

    Maths::Vec3 target {0.f, 3.1f, 0.f};
    float yaw = 0.f;
    float pitch = 0.05f;
    float distance = 11.f;
    float swayYaw = 0.f;
    float swayPitch = 0.f;
    float fieldOfView = Maths::radians(42.f);
    float nearPlane = 0.1f;
    float farPlane = 400.f;
};
} // namespace Cows
