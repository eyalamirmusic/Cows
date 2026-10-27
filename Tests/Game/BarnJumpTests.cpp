#include "Game.h"
#include "Props/Barn.h"
#include "TestCow.h"

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
constexpr auto farAboveEverything = 1000.f;
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
    auto game = Game {getCowModelForTests().getJumpPhysics()};
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

float jumpOnTheSpotAndGetHighestFeet(Game& game, float frame)
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

struct ClimbTop final
{
    Vec2 at;
    float feet = 0.f;
    float heading = 0.f;
};

ClimbTop climbTop(const Level& level)
{
    auto top = Cows::Block {};

    for (const auto& block: level.blocks)
    {
        auto underRoof = false;

        for (const auto& roof: level.blocks)
            underRoof = underRoof
                        || (roof.bottom > 0.f && lengthSquared(roof.rise) > 0.f
                            && roof.contains(block.center));

        if (block.bottom == 0.f && !underRoof && block.top > top.top)
            top = block;
    }

    auto toBarn = level.clearings[0].center - top.center;
    auto toward = std::abs(toBarn.x) > std::abs(toBarn.y)
                      ? Vec2 {std::copysign(1.f, toBarn.x), 0.f}
                      : Vec2 {0.f, std::copysign(1.f, toBarn.y)};
    return {top.center, top.top, std::atan2(-toward.y, toward.x)};
}
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
            auto roof = game.level.floorAt(spot.at, farAboveEverything);

            check(jumpOnTheSpotAndGetHighestFeet(game, frame) < roof);
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
            auto roof = game.level.floorAt(at, farAboveEverything);
            standOn(game, roof);

            jumpOnTheSpotAndGetHighestFeet(game, frame);
            check(std::abs(game.player.y - roof) < landed);
        }

        auto game = inBarn(nearTheLoftsWall);
        game.player.y = fallFrom;
        jumpOnTheSpotAndGetHighestFeet(game, frame);
        check(std::abs(game.player.y
                       - game.level.floorAt(nearTheLoftsWall, farAboveEverything))
              < landed);
    }
};

auto tClimb = test("BarnJump/theClimbStepsOntoTheRoof") = []
{
    auto game = inBarn({});
    auto top = climbTop(game.level);
    game.player = {top.at.x, 0.f, top.at.y};
    game.playerHeading = top.heading;
    standOn(game, top.feet);
    check(std::abs(game.player.y - top.feet) < landed);

    for (auto step = 0; step < walkFrames; ++step)
        game.update(steadyFrame, 1.f, 0.f, false);

    check(game.player.y > top.feet);
    check(std::abs(game.player.y
                   - game.level.floorAt(ground(game.player), farAboveEverything))
          < landed);
};
