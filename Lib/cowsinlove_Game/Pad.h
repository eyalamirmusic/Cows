#pragma once

#include "Game.h"
#include "Render/Common.h"

namespace Cows
{
// Left, up and zoom-in are positive; the largest stick of several pads wins.
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
    bool backPressed = false;
    bool active = false;
    Graphics::GamepadFamily family = Graphics::GamepadFamily::Generic;
};

PadControls readPad(const Graphics::GameInputFrame& frame);

Hints padHints(Graphics::GamepadFamily family);
} // namespace Cows
