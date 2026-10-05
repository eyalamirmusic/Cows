#include "Game.h"
#include "Levels/LevelGenerator.h"
#include "Fixtures.h"
#include "Props/Bale.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto frame = 1.f / 60.f;
constexpr auto edge = -5.f;

Level levelWithAGap()
{
    auto level = Level {};
    level.gaps.add({{0.f, edge - 10.f}, {100.f, 10.f}, -30.f});
    level.killDepth = -12.f;
    level.startHeading = halfPi;
    return level;
}

Game gameOn(const Level& level)
{
    auto game = Game {};
    game.makeLevel = [level](std::uint32_t) { return level; };
    game.reset(3);
    game.start();
    game.partner = {60.f, 0.f, 60.f};
    return game;
}
} // namespace

auto tFallAndRespawn = test("Respawn/walkingOffTheEdgeRespawnsAtTheCheckpoint") = []
{
    auto game = gameOn(levelWithAGap());
    check(game.player.z == 0.f && game.playerHeading == halfPi);

    auto fell = false;
    auto lowest = 0.f;

    for (auto count = 0; count < 600 && !fell; ++count)
    {
        game.update(frame, 1.f, 0.f, false);
        lowest = std::min(lowest, game.player.y);
        fell = game.sinceFell == 0.f;
    }

    check(fell);
    check(lowest < 0.f);
    check(game.player.y == 0.f && game.grounded);
    check(game.player.z > edge && game.player.z < edge + 0.5f);
    check(game.playerHeading == halfPi);
    check(game.justFell());
    check(footerText(game, "", Hints::Keys)
          == "back on your feet  -  mind the edge");

    game.update(2.5f, 0.f, 0.f, false);
    check(!game.justFell());
};

auto tNoKillDepth = test("Respawn/noKillDepthNeverRespawns") = []
{
    auto level = levelWithAGap();
    level.killDepth = 0.f;
    auto game = gameOn(level);

    for (auto count = 0; count < 300; ++count)
        game.update(frame, 1.f, 0.f, false);

    check(game.player.y < -12.f);
    check(!game.justFell());
};

auto tBridgeIsNoCheckpoint = test("Respawn/checkpointStaysOnFirmGround") = []
{
    auto level = levelWithAGap();
    level.blocks.add({{0.f, edge - 10.f}, {1.5f, 11.f}, 0.f, {}});
    auto game = gameOn(level);

    for (auto count = 0; count < 60; ++count)
        game.update(frame, 1.f, 0.f, false);

    check(game.player.z < edge - 1.f && game.player.y == 0.f);
    check(game.checkpoint.z >= edge);
};

auto tMoverPushes = test("Respawn/aMoverPushesThePlayerOut") = []
{
    auto level = Level {};
    level.movers.add(makeRollingBale({-4.f, 0.f}, {4.f, 0.f}, 8.f, 0.f));
    auto game = gameOn(level);
    game.player = {-2.5f, 0.f, 0.f};

    game.update(frame, 0.f, 0.f, false);
    check(distance(Vec2 {game.player.x, game.player.z},
                   game.level.movers[0].collider.center)
          >= rollingBaleRadius + 1.f - 1e-3f);

    for (auto count = 0; count < 120; ++count)
        game.update(frame, 0.f, 0.f, false);

    check(game.player.x > game.level.movers[0].collider.center.x);
};

auto tLevelTime = test("Respawn/gameTimeDrivesTheMovers") = []
{
    auto level = Level {};
    level.movers.add(makeRollingBale({-4.f, 20.f}, {4.f, 20.f}, 8.f, 0.f));
    auto game = gameOn(level);

    game.update(2.f, 0.f, 0.f, false);
    check(std::abs(game.seconds - 2.f) < 1e-6f);
    check(distance(game.level.movers[0].collider.center,
                   game.level.movers[0].positionAt(2.f))
          < 1e-5f);
};

auto tCrossing = test("Respawn/aWellTimedRunCrossesTheRavine") = []
{
    auto level = generate(meadowRavineFixture(), 3);
    const auto& gap = level.gaps.front();
    auto bridgeX = level.criticalPath.center().x;
    auto southRim = gap.center.y + gap.half.y;
    auto northRim = gap.center.y - gap.half.y;

    auto crossed = 0;
    auto knocked = 0;

    for (auto start = 0.f; start < 42.f; start += 0.25f)
    {
        auto game = gameOn(level);
        game.seconds = start;
        game.player = {bridgeX, 0.f, southRim + 1.5f};
        game.playerHeading = halfPi;

        for (auto count = 0; count < 300; ++count)
        {
            game.update(frame, 1.f, 0.f, false);

            if (game.sinceFell == 0.f || game.player.z < northRim - 1.5f)
                break;
        }

        if (game.player.z < northRim - 1.5f && game.player.y == 0.f)
            ++crossed;
        else
            ++knocked;
    }

    check(crossed > 0);
    check(knocked > 0);
};
