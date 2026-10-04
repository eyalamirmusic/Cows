#include "Pad.h"

#include <algorithm>
#include <cmath>

namespace Cows
{
namespace
{
using Graphics::GamepadAxis;
using Graphics::GamepadButton;
using Graphics::GamepadState;
using Graphics::Point;

constexpr auto deadZone = 0.15f;
constexpr auto fullPush = 0.95f;
constexpr auto curveExponent = 1.6f;

float magnitude(Point stick)
{
    return std::hypot(stick.x, stick.y);
}

float rescaled(float amount)
{
    return std::clamp((amount - deadZone) / (fullPush - deadZone), 0.f, 1.f);
}

Point outsideDeadZone(Point stick)
{
    auto size = magnitude(stick);

    if (size < deadZone)
        return {0.f, 0.f};

    auto scale = rescaled(size) / size;
    return {stick.x * scale, stick.y * scale};
}

float curved(float value)
{
    return std::copysign(std::pow(std::abs(value), curveExponent), value);
}

float digital(const GamepadState& pad,
              GamepadButton positive,
              GamepadButton negative)
{
    return (pad.isDown(positive) ? 1.f : 0.f) - (pad.isDown(negative) ? 1.f : 0.f);
}

bool anyButton(const GamepadState& pad)
{
    for (auto index = 0; index < GamepadState::buttonCount; ++index)
    {
        auto button = (GamepadButton) index;

        if (pad.isDown(button) || pad.wasPressed(button))
            return true;
    }

    return false;
}

float trigger(const GamepadState& pad, GamepadAxis axis)
{
    return pad.axis(axis) < deadZone ? 0.f : rescaled(pad.axis(axis));
}
} // namespace

PadControls readPad(const Graphics::GameInputFrame& frame)
{
    auto controls = PadControls {};
    auto walkStick = Point {0.f, 0.f};
    auto lookStick = Point {0.f, 0.f};
    auto dpadAhead = 0.f;
    auto dpadTurn = 0.f;

    for (const auto& pad: frame.gamepads())
    {
        auto left = outsideDeadZone(pad.leftStick());
        auto right = outsideDeadZone(pad.rightStick());

        if (magnitude(left) > magnitude(walkStick))
            walkStick = left;

        if (magnitude(right) > magnitude(lookStick))
            lookStick = right;

        dpadAhead += digital(pad, GamepadButton::DpadUp, GamepadButton::DpadDown);
        dpadTurn += digital(pad, GamepadButton::DpadLeft, GamepadButton::DpadRight);

        auto zoomIn = trigger(pad, GamepadAxis::RightTrigger);
        auto zoomOut = trigger(pad, GamepadAxis::LeftTrigger);
        controls.zoom += zoomIn - zoomOut;

        controls.jumping |= pad.isDown(GamepadButton::South);
        controls.jumpPressed |= pad.wasPressed(GamepadButton::South);
        controls.mooPressed |= pad.wasPressed(GamepadButton::West)
                               || pad.wasPressed(GamepadButton::East);
        controls.againPressed |= pad.wasPressed(GamepadButton::North);
        controls.recenterPressed |= pad.wasPressed(GamepadButton::RightStick);

        auto used = magnitude(left) > 0.f || magnitude(right) > 0.f || zoomIn > 0.f
                    || zoomOut > 0.f || anyButton(pad);

        if (used)
        {
            controls.active = true;
            controls.family = pad.family();
        }
    }

    controls.ahead = std::clamp(walkStick.y + dpadAhead, -1.f, 1.f);
    controls.turn = std::clamp(curved(-walkStick.x) + dpadTurn, -1.f, 1.f);
    controls.lookX = curved(lookStick.x);
    controls.lookY = curved(lookStick.y);
    controls.zoom = std::clamp(controls.zoom, -1.f, 1.f);
    return controls;
}

Hints padHints(Graphics::GamepadFamily family)
{
    switch (family)
    {
        case Graphics::GamepadFamily::Xbox:
            return Hints::Xbox;
        case Graphics::GamepadFamily::PlayStation:
            return Hints::PlayStation;
        case Graphics::GamepadFamily::Nintendo:
            return Hints::Nintendo;
        case Graphics::GamepadFamily::Generic:
            break;
    }

    return Hints::Gamepad;
}
} // namespace Cows
