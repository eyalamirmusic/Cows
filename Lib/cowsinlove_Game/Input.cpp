#include "Input.h"

#include <algorithm>

namespace Cows
{
void Input::setHeld(std::uint16_t keyCode, bool down)
{
    using namespace Graphics::KeyCode;

    switch (keyCode)
    {
        case W:
        case K:
        case UpArrow:
            walkingForward = down;
            break;
        case S:
        case J:
        case DownArrow:
            walkingBack = down;
            break;
        case A:
        case H:
        case LeftArrow:
            walkingLeft = down;
            break;
        case D:
        case L:
        case RightArrow:
            walkingRight = down;
            break;
        case Space:
            jumping = down;
            break;
        default:
            break;
    }
}

float Input::walkAhead() const
{
    auto keys = (walkingForward ? 1.f : 0.f) - (walkingBack ? 1.f : 0.f);
    return std::clamp(keys + stickAhead + padAhead, -1.f, 1.f);
}

float Input::walkTurn() const
{
    auto keys = (walkingLeft ? 1.f : 0.f) - (walkingRight ? 1.f : 0.f);
    return std::clamp(keys + stickTurn + padTurn, -1.f, 1.f);
}

void Input::setStick(float ahead, float turn)
{
    stickAhead = ahead;
    stickTurn = turn;
}

void Input::setPad(float ahead, float turn, bool jumping)
{
    padAhead = ahead;
    padTurn = turn;
    padJumping = jumping;
}

void Input::jump()
{
    jumpPending = true;
}
} // namespace Cows
