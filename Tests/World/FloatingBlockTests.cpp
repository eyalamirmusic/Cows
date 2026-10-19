#include "Level.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto loftBottom = 3.3f;
constexpr auto loftTop = 3.6f;
constexpr auto eaves = 5.5f;
constexpr auto pitchBreak = 7.5f;
constexpr auto ridge = 9.5f;
constexpr auto margin = 0.3f;

bool near(float a, float b)
{
    return std::abs(a - b) < 1e-4f;
}

bool near(Vec3 a, Vec3 b)
{
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

Cows::Block loft()
{
    return {{0.f, 5.f}, {5.7f, 3.f}, loftTop, {}, loftBottom};
}

Level underTheGambrelRoofsPositiveXHalf()
{
    auto level = Level {};
    level.blocks.add(loft());
    level.blocks.add({{4.75f, 0.f}, {1.25f, 8.f}, pitchBreak, {-1.f, 0.f}, eaves});
    level.blocks.add({{1.75f, 0.f}, {1.75f, 8.f}, ridge, {-1.f, 0.f}, pitchBreak});
    return level;
}
} // namespace

auto tFloatingUnderLoft = test("FloatingBlock/walkUnderALoft") = []
{
    auto level = Level {};
    level.blocks.add(loft());

    check(level.pushedOut({0.f, 5.f}, 0.8f, 0.f) == Vec2 {0.f, 5.f});
    check(near(level.floorAt({0.f, 5.f}, 0.f), 0.f));
    check(near(level.floorAt({0.f, 5.f}, 3.f), 0.f));
};

auto tFloatingOntoLoft = test("FloatingBlock/stepOntoALoftFromItsBottom") = []
{
    auto level = Level {};
    level.blocks.add(loft());

    check(near(level.floorAt({0.f, 5.f}, loftBottom), loftTop));
    check(near(level.floorAt({0.f, 5.f}, loftTop), loftTop));
};

auto tFloatingWallAboveBottom =
    test("FloatingBlock/pushesOutFeetAboveItsBottom") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 0.f}, {1.f, 1.f}, 5.f, {}, 1.f});

    check(level.pushedOut({-1.2f, 0.f}, 0.5f, 0.f) == Vec2 {-1.2f, 0.f});
    check(near(level.pushedOut({-1.2f, 0.f}, 0.5f, 2.f).x, -1.5f));
};

auto tFloatingRoof = test("FloatingBlock/loftUnderTheRoofAndTheRoofsSlope") = []
{
    auto level = underTheGambrelRoofsPositiveXHalf();

    check(near(level.floorAt({0.f, 5.f}, loftTop), loftTop));
    check(level.pushedOut({3.f, 0.f}, 0.8f, loftTop) == Vec2 {3.f, 0.f});
    check(near(level.floorAt({4.75f, 0.f}, 6.4f), 6.5f));
    check(near(level.floorAt({1.75f, 0.f}, 8.4f), 8.5f));
    check(near(level.floorAt({0.f, 0.f}, ridge), ridge));
};

auto tFloatingOccupied = test("FloatingBlock/stillOccupiesItsFootprint") = []
{
    auto level = Level {};
    level.blocks.add(loft());

    check(!level.isFree({0.f, 5.f}, 0.f));
};

auto tClearReachWall = test("ClearReach/stopsShortOfAWall") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 5.f}, {3.f, 0.5f}, 4.f, {}});

    auto clear = level.clearReach({0.f, 2.f, 0.f}, {0.f, 2.f, 10.f}, margin);
    check(near(clear, {0.f, 2.f, 4.5f - margin}));
};

auto tClearReachOpen = test("ClearReach/reachesTheEyeWhenNothingIsInTheWay") = []
{
    auto level = Level {};
    level.blocks.add({{10.f, 0.f}, {1.f, 1.f}, 4.f, {}});

    auto eye = Vec3 {0.f, 2.f, 10.f};
    check(near(level.clearReach({0.f, 2.f, 0.f}, eye, margin), eye));
};

auto tClearReachUnder = test("ClearReach/passesUnderAFloatingBlock") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 5.f}, {3.f, 0.5f}, 6.f, {}, 4.f});

    auto eye = Vec3 {0.f, 3.f, 10.f};
    check(near(level.clearReach({0.f, 2.f, 0.f}, eye, margin), eye));
};

auto tClearReachBelowCeiling =
    test("ClearReach/belowACeilingInsideItsFootprint") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 0.f}, {10.f, 10.f}, 4.f, {}, loftBottom});

    auto eye = Vec3 {0.f, 3.f, 6.f};
    check(near(level.clearReach({0.f, 2.f, 0.f}, eye, margin), eye));
};

auto tClearReachCeiling = test("ClearReach/stopsUnderACeiling") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 0.f}, {10.f, 10.f}, 4.f, {}, loftBottom});

    auto clear = level.clearReach({0.f, 2.f, 0.f}, {0.f, 4.6f, 0.f}, margin);
    check(near(clear, {0.f, loftBottom - margin, 0.f}));
};

auto tClearReachInside = test("ClearReach/passesThroughABlockItStartsIn") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 0.f}, {1.f, 1.f}, 4.f, {}});

    auto eye = Vec3 {0.f, 2.f, 10.f};
    check(near(level.clearReach({0.f, 2.f, 0.f}, eye, margin), eye));
};

auto tClearReachSlope = test("ClearReach/followsARampsSlope") = []
{
    auto level = Level {};
    level.blocks.add({{0.f, 0.f}, {4.f, 4.f}, 4.f, {1.f, 0.f}});

    auto overSlope = Vec3 {3.f, 3.9f, 0.f};
    check(near(level.clearReach({-3.f, 1.5f, 0.f}, overSlope, 0.f), overSlope));

    auto clear = level.clearReach({-3.f, 3.f, 0.f}, {3.f, 3.f, 0.f}, 0.f);
    check(near(clear, {2.f, 3.f, 0.f}));
};
