#include "Stages.h"

#include "Game.h"

#include <NanoTest/NanoTest.h>

#include <cstdlib>

using namespace nano;
using namespace Cows;

namespace
{
auto tFixedSeed = test("Stages/firstSeedHonoursCOWS_SEED") = []
{
    setenv("COWS_SEED", "42", 1);
    auto stages = Stages {};
    check(stages.firstSeed() == 42u);
    unsetenv("COWS_SEED");
};

auto tClockSeeds = test("Stages/seedsChangeBetweenRounds") = []
{
    unsetenv("COWS_SEED");
    auto stages = Stages {};
    auto first = stages.firstSeed();
    auto next = stages.nextSeed();
    check(first != 0u);
    check(next != first || stages.nextSeed() != next);
};

auto tMeadow = test("Stages/levelIsAMeadow") = []
{
    auto stages = Stages {};
    auto game = Game {};
    game.makeLevel = stages.level;
    game.reset(3);
    check(!game.level.colliders.empty());
    check(!game.level.blocks.empty());
    check(game.partner == game.level.hideout);
};
} // namespace
