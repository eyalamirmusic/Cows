#include "Input.h"

#include <NanoTest/NanoTest.h>

#include <cstdint>
#include <initializer_list>

using namespace nano;
using namespace Cows;
using namespace Graphics::KeyCode;

namespace
{
using Keys = std::initializer_list<std::uint16_t>;
}

auto tAhead = test("Input/aheadKeys") = []
{
    for (auto key: Keys {W, K, UpArrow})
    {
        auto input = Input {};
        input.setHeld(key, true);
        check(input.walkAhead() > 0.f);
        check(input.walkTurn() == 0.f);
        input.setHeld(key, false);
        check(input.walkAhead() == 0.f);
    }

    for (auto key: Keys {S, J, DownArrow})
    {
        auto input = Input {};
        input.setHeld(key, true);
        check(input.walkAhead() < 0.f);
        input.setHeld(key, false);
        check(input.walkAhead() == 0.f);
    }
};

auto tTurnKeys = test("Input/turnKeys") = []
{
    for (auto key: Keys {A, H, LeftArrow})
    {
        auto input = Input {};
        input.setHeld(key, true);
        check(input.walkTurn() > 0.f);
        check(input.walkAhead() == 0.f);
        input.setHeld(key, false);
        check(input.walkTurn() == 0.f);
    }

    for (auto key: Keys {D, L, RightArrow})
    {
        auto input = Input {};
        input.setHeld(key, true);
        check(input.walkTurn() < 0.f);
        input.setHeld(key, false);
        check(input.walkTurn() == 0.f);
    }
};

auto tStick = test("Input/stickPassesThrough") = []
{
    auto input = Input {};
    input.setStick(0.4f, -0.7f);
    check(input.walkAhead() == 0.4f);
    check(input.walkTurn() == -0.7f);

    input.setStick(0.f, 0.f);
    check(input.walkAhead() == 0.f);
    check(input.walkTurn() == 0.f);
};

auto tKeyPlusStick = test("Input/keysAndStickAddThenClamp") = []
{
    auto input = Input {};
    input.setHeld(W, true);
    input.setStick(-1.f, 0.f);
    check(input.walkAhead() == 0.f);

    input.setStick(-0.25f, 0.f);
    check(input.walkAhead() == 0.75f);

    input.setStick(1.f, 0.f);
    check(input.walkAhead() == 1.f);

    input.setHeld(D, true);
    input.setStick(0.f, 0.5f);
    check(input.walkTurn() == -0.5f);
};

auto tJumpKeys = test("Input/jump") = []
{
    auto input = Input {};
    input.setHeld(Space, true);
    check(input.jumping);
    input.setHeld(Space, false);
    check(!input.jumping);

    input.jump();
    check(input.jumpPending);
};

auto tPadSums = test("Input/padAddsToKeysAndStickThenClamps") = []
{
    auto input = Input {};
    input.setPad(0.5f, -0.25f, false);
    check(input.walkAhead() == 0.5f);
    check(input.walkTurn() == -0.25f);

    input.setStick(0.25f, 0.f);
    check(input.walkAhead() == 0.75f);

    input.setHeld(W, true);
    check(input.walkAhead() == 1.f);

    input.setHeld(D, true);
    input.setPad(0.f, -1.f, false);
    check(input.walkTurn() == -1.f);
};

auto tPadKeepsStick = test("Input/padDoesNotClearTouchStick") = []
{
    auto input = Input {};
    input.setStick(0.6f, 0.3f);
    input.setPad(0.f, 0.f, false);
    check(input.walkAhead() == 0.6f);
    check(input.walkTurn() == 0.3f);

    input.setPad(-0.6f, 0.f, false);
    check(input.walkAhead() == 0.f);
    check(input.stickAhead == 0.6f);
};

auto tPadJump = test("Input/padJumpingHolds") = []
{
    auto input = Input {};
    input.setPad(0.f, 0.f, true);
    check(input.padJumping);
    check(!input.jumping);
    check(!input.jumpPending);

    input.setPad(0.f, 0.f, false);
    check(!input.padJumping);
};
