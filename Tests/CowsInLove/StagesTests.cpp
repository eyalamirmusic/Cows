#include "Stages.h"

#include "Game.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
constexpr auto anySeed = 3u;

Level levelOf(const Stages& stages)
{
    return stages.level()(anySeed);
}
} // namespace

auto tMeadow = test("Stages/levelIsAMeadow") = []
{
    auto stages = Stages {};
    auto game = Game {neitherJumpingNorFalling};
    game.makeLevel = stages.level();
    game.reset(anySeed);
    check(!game.level.colliders.empty());
    check(!game.level.blocks.empty());
    check(game.level.gaps.empty());
    check(game.partner == game.level.hideout);
};

auto tAdvance = test("Stages/advanceGoesMeadowRavineMeadow") = []
{
    auto stages = Stages {};
    check(stages.current == 0 && levelOf(stages).gaps.empty());

    stages.advance();
    check(stages.current == 1 && levelOf(stages).gaps.size() == 1);
    check(levelOf(stages).movers.size() == 3);

    stages.advance();
    check(stages.current == 0 && levelOf(stages).gaps.empty());
};

auto tStageWraps = test("Stages/stageWrapsIntoRange") = []
{
    check(Stages {5}.current == 1);
    check(Stages {-1}.current == 1);
};
