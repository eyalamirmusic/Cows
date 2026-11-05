#include "Game.h"
#include "Props/Barn.h"

#include <NanoTest/NanoTest.h>

#include <algorithm>
#include <cmath>
#include <initializer_list>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto steadyFrame = 1.f / 60.f;
constexpr auto slowFrame = 0.1f;
constexpr auto longestJump = 200;
constexpr auto landed = 0.01f;
constexpr auto walkFrames = 120;
constexpr auto settleFrames = 30;
constexpr auto fallFrom = 20.f;
// Far above anything: floorAt from here is the highest surface over a point.
constexpr auto sky = 1000.f;
// How far along the loft from its middle she stands to be near its wall.
constexpr auto alongTheLoft = 5.5f;

Vec2 ground(Vec3 point)
{
    return {point.x, point.z};
}

Vec2 ahead(float heading)
{
    return {std::cos(heading), -std::sin(heading)};
}

Game inBarn(Vec2 at)
{
    auto game = Game {};
    game.reset(3);
    game.start();
    game.level = Level {};
    addBarn(game.level, {}, 0);
    game.partner = {60.f, 0.f, 60.f};
    game.player = {at.x, 0.f, at.y};
    return game;
}

void standOn(Game& game, float feet)
{
    game.player.y = feet;

    for (auto step = 0; step < settleFrames; ++step)
        game.update(steadyFrame, 0.f, 0.f, false);
}

// Jumps on the spot and returns the highest the feet rose.
float jumpOnce(Game& game, float frame)
{
    auto highest = game.player.y;
    game.update(frame, 0.f, 0.f, true);

    for (auto step = 0; step < longestJump; ++step)
    {
        highest = std::max(highest, game.player.y);
        game.update(frame, 0.f, 0.f, false);
    }

    return highest;
}

struct Spot final
{
    Vec2 at;
    float feet = 0.f;
};

const auto spots = barnSpots({}, 0);
const auto nearTheLoftsWall =
    ground(spots.loft.at) + ahead(spots.loft.heading) * alongTheLoft;
} // namespace

auto tUnderTheRoof = test("BarnJump/aJumpInsideNeverSurfacesOnTheRoof") = []
{
    for (auto frame: {steadyFrame, slowFrame})
    {
        for (auto spot: {Spot {ground(spots.doorway.at)},
                         Spot {{}},
                         Spot {ground(spots.loft.at)},
                         Spot {ground(spots.loft.at), spots.loft.at.y},
                         Spot {nearTheLoftsWall, spots.loft.at.y},
                         Spot {ground(spots.rampTop.at), spots.rampTop.at.y}})
        {
            auto game = inBarn(spot.at);
            standOn(game, spot.feet);
            auto floor = game.player.y;
            auto roof = game.level.floorAt(spot.at, sky);

            check(jumpOnce(game, frame) < roof);
            check(std::abs(game.player.y - floor) < landed);
            check(game.level.floorAt(ground(game.player), game.player.y) < roof);
        }
    }
};

auto tOnTheRoof = test("BarnJump/aJumpOnTheRoofLandsOnTheRoof") = []
{
    for (auto frame: {steadyFrame, slowFrame})
    {
        for (auto at: {ground(spots.roof.at), nearTheLoftsWall})
        {
            auto game = inBarn(at);
            auto roof = game.level.floorAt(at, sky);
            standOn(game, roof);

            jumpOnce(game, frame);
            check(std::abs(game.player.y - roof) < landed);
        }

        auto game = inBarn(nearTheLoftsWall);
        game.player.y = fallFrom;
        jumpOnce(game, frame);
        check(std::abs(game.player.y - game.level.floorAt(nearTheLoftsWall, sky))
              < landed);
    }
};

auto tClimb = test("BarnJump/theClimbStepsOntoTheRoof") = []
{
    auto climbTop = spots.climbTop;
    auto game = inBarn(ground(climbTop.at));
    game.playerHeading = climbTop.heading;
    standOn(game, climbTop.at.y);
    check(std::abs(game.player.y - climbTop.at.y) < landed);

    for (auto step = 0; step < walkFrames; ++step)
        game.update(steadyFrame, 1.f, 0.f, false);

    check(game.player.y > climbTop.at.y);
    check(std::abs(game.player.y - game.level.floorAt(ground(game.player), sky))
          < landed);
};
