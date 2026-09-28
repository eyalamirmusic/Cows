#include "Levels/Layout.h"
#include "Props/Tree.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Maths;

auto tRegion = test("Layout/regionGeometry") = []
{
    auto region = Region {{-10.f, 0.f}, {10.f, 4.f}};
    check(region.area() == 80.f);
    check(region.center().x == 0.f && region.center().y == 2.f);
    check(region.contains({10.f, 4.f}) && !region.holds({10.f, 4.f}));
    check(region.inset(3.f).isEmpty() && !region.inset(1.f).isEmpty());
    check(Region {}.isEmpty());
};

auto tRoomOnOpenGround = test("Layout/hasRoomOnOpenGround") = []
{
    auto level = Level {};
    check(hasRoom(level, {40.f, 0.f, 0.f}, 3.f));
    check(!hasRoom(level, {10.f, 0.f, 0.f}, 3.f));
    check(!hasRoom(level, {90.f, 0.f, 0.f}, 3.f));
};

auto tRoomBesideATree = test("Layout/hasRoomBesideATree") = []
{
    auto level = Level {};
    auto random = std::mt19937 {7u};
    addTree(level, random, {40.f, 0.f});

    check(!hasRoom(level, {41.f, 0.f, 0.f}, 1.2f));
    check(hasRoom(level, {45.f, 0.f, 0.f}, 1.2f));
};

auto tRoomOverAGap = test("Layout/noRoomOverAGap") = []
{
    auto level = Level {};
    level.gaps.add({{40.f, 0.f}, {5.f, 5.f}, -30.f});
    check(!hasRoom(level, {40.f, 0.f, 0.f}, 1.f));
};

auto tSpots = test("Layout/spotsKeepOutOfTheClearing") = []
{
    auto layout = Layout {3u};
    layout.home = {20.f, 20.f};
    auto region = Region::arena();

    for (auto count = 0; count < 200; ++count)
    {
        auto at = layout.spot(region);
        check(distance(at, layout.home) > clearing);
        check(region.inset(arenaInset).contains(at));
    }
};
