#pragma once

#include "Render/Common.h"

#include <limits>
#include <optional>

namespace Cows
{
struct CameraPose final
{
    Maths::Vec3 target;
    float yaw = 0.f;
    float pitch = 0.f;
    float distance = 0.f;
    float swayYaw = 0.f;
    float swayPitch = 0.f;
};

// Yaw goes the short way round.
CameraPose blend(const CameraPose& from, const CameraPose& to, float amount);

float easeOut(float amount);

// eacp clamps FrameTime::delta to 0.1 s; swings step on the clock's own time.
struct SwingClock final
{
    float step(Threads::FrameTime time);

    std::optional<double> last;
};

float distanceToFit(float size, float share, float verticalFov);

CameraPose placeTargetOnScreen(CameraPose pose,
                               Graphics::Point onScreenNdc,
                               float verticalFov,
                               float aspect);

Maths::Vec2 driftAt(float seconds);

constexpr auto unlimitedReach = std::numeric_limits<float>::infinity();

struct OrbitCamera final
{
    void orbit(float horizontal, float vertical);
    void zoom(float amount);

    void drift(float seconds);

    void follow(Maths::Vec3 goal, float delta);

    void turnToward(float wantedYaw, float amount);

    CameraPose pose() const;
    void setPose(const CameraPose& to);

    Maths::Vec3 eye() const;
    Maths::Vec3 forward() const;
    Maths::Vec3 right() const;
    Maths::Vec3 up() const;

    Maths::Mat4 view() const;
    Maths::Mat4 projection(float aspect) const;

    float verticalFieldOfView(float aspect) const;

    Maths::Vec3 target {0.f, 3.1f, 0.f};
    float yaw = 0.f;
    float pitch = 0.22f;
    float distance = 11.f;
    // Caps the eye's distance when something is in the way; pose() keeps distance.
    float reach = unlimitedReach;
    float swayYaw = 0.f;
    float swayPitch = 0.f;
    float fieldOfView = Maths::radians(42.f);
    float leastWidth = Maths::radians(40.f);
    float nearPlane = 0.1f;
    float farPlane = 400.f;
};
} // namespace Cows
