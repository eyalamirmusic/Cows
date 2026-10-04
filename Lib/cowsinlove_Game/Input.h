#pragma once

#include "Render/Common.h"

#include <cstdint>

namespace Cows
{
// The walking controls: held keys (wasd / hjkl / arrows, space), the
// on-screen stick and the controller, summed into how far to walk ahead and
// turn.
struct Input final
{
    void setHeld(std::uint16_t keyCode, bool down);
    float walkAhead() const;
    float walkTurn() const;
    void setStick(float ahead, float turn);
    void setPad(float ahead, float turn, bool jumping);
    void jump();

    bool walkingForward = false;
    bool walkingBack = false;
    bool walkingLeft = false;
    bool walkingRight = false;
    bool jumping = false;
    bool jumpPending = false;
    float stickAhead = 0.f;
    float stickTurn = 0.f;
    float padAhead = 0.f;
    float padTurn = 0.f;
    bool padJumping = false;
};
} // namespace Cows
