#include "Stages.h"

#include "Game.h"

#include <NanoTest/NanoTest.h>

#include <cstdlib>

using namespace nano;
using namespace Cows;

namespace
{
void setEnv(const char* name, const char* value)
{
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

void unsetEnv(const char* name)
{
#if defined(_WIN32)
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

Level levelOf(const Stages& stages, std::uint32_t seed = 3)
{
    return stages.level()(seed);
}
} // namespace

auto tFixedSeed = test("Stages/firstSeedHonoursCOWS_SEED") = []
{
    setEnv("COWS_SEED", "42");
    auto stages = Stages {};
    check(stages.firstSeed() == 42u);
    unsetEnv("COWS_SEED");
};

auto tClockSeeds = test("Stages/seedsChangeBetweenRounds") = []
{
    unsetEnv("COWS_SEED");
    auto stages = Stages {};
    auto first = stages.firstSeed();
    auto next = stages.nextSeed();
    check(first != 0u);
    check(next != first || stages.nextSeed() != next);
};

auto tMeadow = test("Stages/levelIsAMeadow") = []
{
    unsetEnv("COWS_STAGE");
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
    unsetEnv("COWS_STAGE");
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
    setEnv("COWS_STAGE", "1");
    auto stages = Stages {};
    check(stages.current == 1);
    check(!levelOf(stages).gaps.empty());

    setEnv("COWS_STAGE", "5");
    check(Stages {}.current == 1);
    unsetEnv("COWS_STAGE");
};
