#include "Input.h"

#include <algorithm>
#include <iterator>

namespace Cows
{
namespace
{
enum class HeldControl
{
    Forward,
    Back,
    Left,
    Right,
    Jump,
    None
};

struct KeyAndControl final
{
    std::uint16_t keyCode = 0;
    HeldControl control = HeldControl::None;
};

struct WalkAxis final
{
    bool positiveKeyHeld = false;
    bool negativeKeyHeld = false;
    float stick = 0.f;
    float pad = 0.f;
};

float combineWalkAxis(const WalkAxis& axis)
{
    auto keys =
        (axis.positiveKeyHeld ? 1.f : 0.f) - (axis.negativeKeyHeld ? 1.f : 0.f);
    return std::clamp(keys + axis.stick + axis.pad, -1.f, 1.f);
}
} // namespace

void Input::setHeld(std::uint16_t keyCode, bool down)
{
    using namespace Graphics::KeyCode;

    static constexpr KeyAndControl keysAndControls[] = {
        {W, HeldControl::Forward},
        {K, HeldControl::Forward},
        {UpArrow, HeldControl::Forward},
        {S, HeldControl::Back},
        {J, HeldControl::Back},
        {DownArrow, HeldControl::Back},
        {A, HeldControl::Left},
        {H, HeldControl::Left},
        {LeftArrow, HeldControl::Left},
        {D, HeldControl::Right},
        {L, HeldControl::Right},
        {RightArrow, HeldControl::Right},
        {Space, HeldControl::Jump},
    };

    auto found =
        std::ranges::find(keysAndControls, keyCode, &KeyAndControl::keyCode);

    if (found == std::end(keysAndControls))
        return;

    switch (found->control)
    {
        case HeldControl::Forward:
            walkingForward = down;
            break;
        case HeldControl::Back:
            walkingBack = down;
            break;
        case HeldControl::Left:
            walkingLeft = down;
            break;
        case HeldControl::Right:
            walkingRight = down;
            break;
        case HeldControl::Jump:
            jumping = down;
            break;
        case HeldControl::None:
            break;
    }
}

float Input::walkAhead() const
{
    return combineWalkAxis({walkingForward, walkingBack, stickAhead, padAhead});
}

float Input::walkTurn() const
{
    return combineWalkAxis({walkingLeft, walkingRight, stickTurn, padTurn});
}

void Input::jump()
{
    jumpPending = true;
}
} // namespace Cows
