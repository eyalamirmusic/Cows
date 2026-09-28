#include "Game.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto walkSpeed = 7.f;
constexpr auto turnSpeed = 2.4f;
constexpr auto frame = 0.01f;

bool near(float a, float b, float margin = 1e-3f)
{
    return std::abs(a - b) < margin;
}

Game openField()
{
    auto game = Game {};
    game.reset(3);
    game.level = Level {};
    game.partner = {60.f, 0.f, 60.f};
    return game;
}
} // namespace

auto tWalk = test("Game/walkingMovesAlongTheHeading") = []
{
    auto game = openField();
    game.update(frame, 1.f, 0.f, false);

    check(near(game.player.x, walkSpeed * frame));
    check(near(game.player.z, 0.f));

    game.playerHeading = halfPi;
    auto before = game.player;
    game.update(frame, 1.f, 0.f, false);

    check(near(game.player.x, before.x));
    check(near(game.player.z, before.z - walkSpeed * frame));
};

auto tTurn = test("Game/turningChangesTheHeading") = []
{
    auto game = openField();
    game.update(frame, 0.f, 1.f, false);
    check(near(game.playerHeading, turnSpeed * frame, 1e-5f));

    game.update(frame, 0.f, -1.f, false);
    check(near(game.playerHeading, 0.f, 1e-5f));
};

auto tJump = test("Game/jumpRisesThenLands") = []
{
    auto game = openField();
    game.update(frame, 0.f, 0.f, true);

    check(!game.grounded);
    check(game.player.y > 0.f);

    auto peak = game.player.y;

    for (auto count = 0; count < 300 && !game.grounded; ++count)
    {
        game.update(frame, 0.f, 0.f, false);
        peak = std::max(peak, game.player.y);
    }

    check(game.grounded);
    check(peak > 1.f);
    check(near(game.player.y, 0.f));
};

auto tFound = test("Game/foundBesideHer") = []
{
    auto game = openField();
    game.partner = {0.f, 0.f, -4.f};
    game.update(frame, 0.f, 0.f, false);

    check(game.state == Game::State::Found);
    check(near(game.stageHeading, halfPi));
    check(near(game.stageCenter.z, -4.f));
    check(game.sinceFound == 0.f);
};

auto tNotFoundFar = test("Game/searchingWhenTooFar") = []
{
    auto game = openField();
    game.partner = {4.6f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    check(game.state == Game::State::Searching);
};

auto tNotFoundAbove = test("Game/searchingWhenSheIsTooHigh") = []
{
    auto game = openField();
    game.partner = {3.f, 1.6f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    check(game.state == Game::State::Searching);
};

auto tWarmth = test("Game/warmthRisesAsSheNears") = []
{
    auto game = openField();
    game.partner = {85.f, 0.f, 0.f};
    check(game.warmth() == 0.f);

    game.partner = {1.f, 0.f, 0.f};
    check(game.warmth() == 1.f);

    auto last = 2.f;

    for (auto x = 0.f; x < 100.f; x += 0.5f)
    {
        game.partner = {x, 0.f, 0.f};
        check(game.warmth() <= last);
        last = game.warmth();
    }
};

auto tMooCooldown = test("Game/mooWaitsForTheCooldown") = []
{
    auto game = openField();
    game.moo();
    check(game.sinceMoo == 0.f);

    game.update(1.f, 0.f, 0.f, false);
    game.moo();
    check(near(game.sinceMoo, 1.f));

    game.update(2.3f, 0.f, 0.f, false);
    game.moo();
    check(game.sinceMoo == 0.f);
};

auto tReset = test("Game/resetStartsANewSearch") = []
{
    auto game = openField();
    game.partner = {3.f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    game.update(1.f, 0.f, 0.f, false);
    check(game.state == Game::State::Found);
    check(game.sinceFound > 0.f);

    game.reset(5);
    check(game.state == Game::State::Searching);
    check(game.sinceFound == 0.f);
    check(game.seed == 5u);
};

auto tEndingClock = test("Game/endingSecondsFollowSinceFound") = []
{
    auto game = openField();
    game.partner = {3.f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);

    auto start = game.endingSeconds();
    game.update(1.25f, 0.f, 0.f, false);

    check(near(game.sinceFound, 1.25f));
    check(near(game.endingSeconds() - start, 1.25f));
};
