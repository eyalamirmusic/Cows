#include "Pad.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using Graphics::GameInputQueue;
using Graphics::GamepadAxis;
using Graphics::GamepadButton;
using Graphics::GamepadFamily;

namespace
{
bool near(float value, float wanted)
{
    return std::abs(value - wanted) < 1e-4f;
}

struct Pads final
{
    Pads() { queue.gamepadConnected(0, GamepadFamily::Xbox, 0, 0.0); }

    PadControls read() { return readPad(queue.snapshot(time += 0.016)); }

    void press(GamepadButton button, bool down = true, int id = 0)
    {
        queue.gamepadButtonChanged(id, button, down, time);
    }

    GameInputQueue queue;
    double time = 0.0;
};
} // namespace

auto tDeadZone = test("Pad/deadZoneReadsZero") = []
{
    auto pads = Pads {};
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftX, 0.1f);
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, -0.1f);
    pads.queue.gamepadAxisChanged(0, GamepadAxis::RightY, 0.14f);

    auto controls = pads.read();
    check(controls.ahead == 0.f);
    check(controls.turn == 0.f);
    check(controls.lookY == 0.f);
    check(!controls.active);
};

auto tFullStick = test("Pad/fullStickWalksFull") = []
{
    auto pads = Pads {};
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, 0.97f);
    auto controls = pads.read();
    check(controls.ahead == 1.f);
    check(controls.turn == 0.f);
    check(controls.active);

    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, -1.f);
    check(pads.read().ahead == -1.f);

    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, 0.f);
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftX, -1.f);
    check(pads.read().turn == 1.f);
};

auto tTurnCurve = test("Pad/turnCurveIsGentleNearCenter") = []
{
    auto pads = Pads {};
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftX, 0.55f);

    auto linear = (0.55f - 0.15f) / (0.95f - 0.15f);
    auto turn = pads.read().turn;
    check(turn < 0.f);
    check(-turn < linear);
    check(near(turn, -std::pow(linear, 1.6f)));
};

auto tDpad = test("Pad/dpadIsDigital") = []
{
    auto pads = Pads {};
    pads.press(GamepadButton::DpadUp);
    pads.press(GamepadButton::DpadLeft);
    auto controls = pads.read();
    check(controls.ahead == 1.f);
    check(controls.turn == 1.f);
    check(controls.active);

    pads.press(GamepadButton::DpadUp, false);
    pads.press(GamepadButton::DpadLeft, false);
    pads.press(GamepadButton::DpadDown);
    pads.press(GamepadButton::DpadRight);
    controls = pads.read();
    check(controls.ahead == -1.f);
    check(controls.turn == -1.f);

    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, -1.f);
    check(pads.read().ahead == -1.f);
};

auto tSouth = test("Pad/southIsHeldJumpAndAnEdge") = []
{
    auto pads = Pads {};
    pads.press(GamepadButton::South);
    auto controls = pads.read();
    check(controls.jumping);
    check(controls.jumpPressed);
    check(controls.active);

    controls = pads.read();
    check(controls.jumping);
    check(!controls.jumpPressed);

    pads.press(GamepadButton::South, false);
    controls = pads.read();
    check(!controls.jumping);
    check(!controls.jumpPressed);

    pads.press(GamepadButton::South);
    pads.press(GamepadButton::South, false);
    controls = pads.read();
    check(!controls.jumping);
    check(controls.jumpPressed);
};

auto tMoo = test("Pad/westAndEastMoo") = []
{
    for (auto button: {GamepadButton::West, GamepadButton::East})
    {
        auto pads = Pads {};
        pads.press(button);
        auto controls = pads.read();
        check(controls.mooPressed);
        check(!controls.jumpPressed);
        check(!controls.againPressed);
        check(!pads.read().mooPressed);
    }
};

auto tNorth = test("Pad/northRestarts") = []
{
    auto pads = Pads {};
    pads.press(GamepadButton::North);
    auto controls = pads.read();
    check(controls.againPressed);
    check(!controls.mooPressed);
    check(!controls.jumping);
    check(!pads.read().againPressed);
};

auto tTwoPads = test("Pad/largestStickOfTwoPadsWins") = []
{
    auto pads = Pads {};
    pads.queue.gamepadConnected(1, GamepadFamily::PlayStation, 1, 0.0);
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, 0.5f);
    pads.queue.gamepadAxisChanged(1, GamepadAxis::LeftY, -0.9f);
    pads.press(GamepadButton::West, true, 0);

    auto controls = pads.read();
    check(controls.ahead < -0.9f);
    check(controls.mooPressed);
    check(controls.family == GamepadFamily::PlayStation);

    pads.queue.gamepadAxisChanged(1, GamepadAxis::LeftY, 0.f);
    controls = pads.read();
    check(near(controls.ahead, (0.5f - 0.15f) / (0.95f - 0.15f)));
    check(controls.family == GamepadFamily::Xbox);
};

auto tDisconnect = test("Pad/disconnectStopsWalking") = []
{
    auto pads = Pads {};
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftY, 1.f);
    pads.press(GamepadButton::South);
    check(pads.read().ahead == 1.f);

    pads.queue.gamepadDisconnected(0, pads.time);
    auto controls = pads.read();
    check(controls.ahead == 0.f);
    check(!controls.jumping);
    check(!controls.active);
};

auto tReleaseAll = test("Pad/releaseAllStopsWalking") = []
{
    auto pads = Pads {};
    pads.queue.gamepadAxisChanged(0, GamepadAxis::LeftX, 1.f);
    pads.queue.gamepadAxisChanged(0, GamepadAxis::RightX, 1.f);
    pads.press(GamepadButton::South);
    check(pads.read().turn == -1.f);

    pads.queue.releaseAll(pads.time);
    auto controls = pads.read();
    check(controls.turn == 0.f);
    check(controls.lookX == 0.f);
    check(!controls.jumping);
    check(!controls.active);
};
