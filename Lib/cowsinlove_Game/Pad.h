#pragma once

#include "Game.h"
#include "Render/Common.h"

namespace Cows
{
// What the controllers ask for this frame: the left stick and D-pad as walk
// ahead / turn (left positive), the right stick as look (y up), the triggers
// as zoom (in positive), and the buttons' edges. Several controllers act as
// one: the largest stick wins, buttons are ORed.
struct PadControls final
{
    float ahead = 0.f;
    float turn = 0.f;
    float lookX = 0.f;
    float lookY = 0.f;
    float zoom = 0.f;
    bool jumping = false;
    bool jumpPressed = false;
    bool mooPressed = false;
    bool againPressed = false;
    bool recenterPressed = false;
    bool startPressed = false;
    bool active = false;
    Graphics::GamepadFamily family = Graphics::GamepadFamily::Generic;
};

PadControls readPad(const Graphics::GameInputFrame& frame);

Hints padHints(Graphics::GamepadFamily family);
} // namespace Cows
