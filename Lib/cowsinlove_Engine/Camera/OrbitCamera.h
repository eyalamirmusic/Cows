#pragma once

#include "Render/Common.h"

#include <optional>

namespace Cows
{
// Where an OrbitCamera stands: everything that places its eye.
struct CameraPose final
{
    Maths::Vec3 target;
    float yaw = 0.f;
    float pitch = 0.f;
    float distance = 0.f;
    float swayYaw = 0.f;
    float swayPitch = 0.f;
};

// `amount` of the way from `from` to `to`, yaw the short way round.
CameraPose blend(const CameraPose& from, const CameraPose& to, float amount);

// Fast out of the start, settling gently: 0 at 0, 1 at 1.
float easeOut(float amount);

// eacp clamps FrameTime::delta to 0.1 s, so at a slow phone's few frames a
// second the game runs in slow motion; the camera's swings between the menu,
// the editor and play take their steps from here instead, the clock's own time
// between frames, so Start lands on time however few frames there are.
struct SwingClock final
{
    float step(Threads::FrameTime time);

    std::optional<double> last;
};

// The idle sway drift() gives at `seconds`, as yaw and pitch.
Maths::Vec2 driftAt(float seconds);

struct OrbitCamera final
{
    void orbit(float horizontal, float vertical);
    void zoom(float amount);

    // A slow idle sway, so the postcard breathes while nobody drags it.
    void drift(float seconds);

    void follow(Maths::Vec3 goal, float delta);

    // Turns `amount` of the way round to `wantedYaw`, the short way.
    void turnToward(float wantedYaw, float amount);

    CameraPose pose() const;
    void setPose(const CameraPose& pose);

    Maths::Vec3 eye() const;
    Maths::Vec3 forward() const;
    Maths::Vec3 right() const;
    Maths::Vec3 up() const;

    Maths::Mat4 view() const;
    Maths::Mat4 projection(float aspect) const;

    // `fieldOfView`, widened on a narrow view so it still sees `leastWidth`
    // across.
    float verticalFieldOfView(float aspect) const;

    Maths::Vec3 target {0.f, 3.1f, 0.f};
    float yaw = 0.f;
    float pitch = 0.22f;
    float distance = 11.f;
    float swayYaw = 0.f;
    float swayPitch = 0.f;
    float fieldOfView = Maths::radians(42.f);
    float leastWidth = Maths::radians(40.f);
    float nearPlane = 0.1f;
    float farPlane = 400.f;
};
} // namespace Cows
