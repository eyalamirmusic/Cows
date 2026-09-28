#include "Stages.h"

#include "Game.h"

#include <NanoTest/NanoTest.h>

#include <cstdlib>

using namespace nano;
using namespace Cows;

namespace
{
Level levelOf(const Stages& stages, std::uint32_t seed = 3)
{
    return stages.level()(seed);
}
} // namespace

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
    unsetenv("COWS_STAGE");
    auto stages = Stages {};
    auto game = Game {};
    game.makeLevel = stages.level();
    game.reset(3);
    check(!game.level.colliders.empty());
    check(!game.level.blocks.empty());
    check(game.level.gaps.empty());
    check(game.partner == game.level.hideout);
};

auto tAdvance = test("Stages/advanceGoesMeadowRavineMeadow") = []
{
    unsetenv("COWS_STAGE");
    auto stages = Stages {};
    check(stages.current == 0 && levelOf(stages).gaps.empty());

    stages.advance();
    check(stages.current == 1 && levelOf(stages).gaps.size() == 1);
    check(levelOf(stages).movers.size() == 3);

    stages.advance();
    check(stages.current == 0 && levelOf(stages).gaps.empty());
};

auto tStageFromEnv = test("Stages/COWS_STAGEPicksTheStart") = []
{
    setenv("COWS_STAGE", "1", 1);
    auto stages = Stages {};
    check(stages.current == 1);
    check(!levelOf(stages).gaps.empty());

    setenv("COWS_STAGE", "5", 1);
    check(Stages {}.current == 1);
    unsetenv("COWS_STAGE");
};
