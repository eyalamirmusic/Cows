#include "Level.h"
#include "Props/Barn.h"
#include "Props/Tree.h"

#include <NanoTest/NanoTest.h>

#include <cmath>
#include <random>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
bool near(float a, float b)
{
    return std::abs(a - b) < 1e-4f;
}

Level levelWithBlock()
{
    auto level = Level {};
    level.blocks.add(Cows::Block {{0.f, 0.f}, {2.f, 2.f}, 3.f, {}});
    return level;
}
} // namespace

auto tRoof = test("Level/floorOnBarnRoof") = []
{
    auto level = Level {};
    addBarn(level, {}, 0);

    check(near(level.floorAt({}, barnRoofHeight), barnRoofHeight));
    check(level.floorAt({}, 0.f) < barnRoofHeight);
};

auto tRamp = test("Level/floorHalfwayUpARamp") = []
{
    auto level = Level {};
    level.blocks.add(Cows::Block {{0.f, 0.f}, {3.5f, 1.5f}, 2.f, {1.f, 0.f}});

    check(near(level.floorAt({0.f, 0.f}, 0.5f), 1.f));
    check(near(level.floorAt({0.f, 0.f}, 0.f), 0.f));
};

auto tOpenGround = test("Level/floorOnOpenGround") = []
{
    auto level = levelWithBlock();

    check(near(level.floorAt({10.f, 10.f}, 0.f), 0.f));
    check(near(Level {}.floorAt({}, 5.f), 0.f));
};

auto tOutOfBlock = test("Level/pushedOutOfABlock") = []
{
    auto level = levelWithBlock();
    auto point = level.pushedOut({2.5f, 0.5f}, 1.f, 0.f);

    check(near(point.x, 3.f));
    check(near(point.y, 0.5f));

    auto inside = level.pushedOut({1.5f, 0.2f}, 1.f, 0.f);
    check(near(inside.x, 3.f));
    check(near(inside.y, 0.2f));

    check(near(level.pushedOut({2.5f, 0.5f}, 1.f, 3.f).x, 2.5f));
};

auto tOutOfTree = test("Level/pushedOutOfATree") = []
{
    auto level = Level {};
    auto random = std::mt19937 {7u};
    addTree(level, random, {});
    const auto& trunk = level.colliders[0];

    auto point = level.pushedOut({0.3f, 0.f}, 1.f, 0.f);

    check(near(distance(point, trunk.center), trunk.radius + 1.f));
    check(point.x > 0.f);
};

auto tFree = test("Level/isFree") = []
{
    auto level = levelWithBlock();

    check(!level.isFree({0.5f, 0.5f}, 0.f));
    check(!level.isFree({2.5f, 0.f}, 1.f));
    check(level.isFree({2.5f, 0.f}, 0.f));
    check(level.isFree({20.f, 20.f}, 2.5f));
};
