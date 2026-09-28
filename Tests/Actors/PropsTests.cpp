#include "Props/Bale.h"
#include "Props/Barn.h"
#include "Props/Crate.h"
#include "Props/Hedge.h"
#include "Props/Rock.h"
#include "Props/Tree.h"
#include "Snapshot.h"

#include <NanoTest/NanoTest.h>

#include <string>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
int instanceCount(const SurfaceBatch& batch)
{
    auto count = 0;

    for (const auto& list: batch.lists)
        count += (int) list.size();

    return count;
}

void checkSnapshot(const Scenery& scenery,
                   const std::string& name,
                   Vec3 target,
                   float distance,
                   float yaw = 0.6f)
{
    if (!hasDevice())
        return;

    auto view = SnapshotView {};
    view.batch = scenery.batch;
    view.camera.target = target;
    view.camera.yaw = yaw;
    view.camera.pitch = 0.3f;
    view.camera.distance = distance;

    auto image = snapshot(view, 200.f, 150.f, "actors-" + name);
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!isClearColor(image.at(image.width() / 2, image.height() / 2)));
}
} // namespace

auto tBarn = test("Props/barn") = []
{
    auto scenery = Scenery {};
    addBarn(scenery, {}, 0);

    check(scenery.blocks.size() == 5);
    check(scenery.colliders.empty());
    check(instanceCount(scenery.batch) == 6);
    check(scenery.blocks[0].top == barnRoofHeight);

    checkSnapshot(scenery, "barn", {-2.f, 3.f, 2.f}, 22.f);
};

auto tTree = test("Props/tree") = []
{
    auto scenery = Scenery {};
    auto random = std::mt19937 {7u};
    addTree(scenery, random, {});

    check(scenery.colliders.size() == 1);
    check(scenery.blocks.empty());
    check(instanceCount(scenery.batch) >= 2);

    checkSnapshot(scenery, "tree", {0.f, 2.5f, 0.f}, 11.f);
};

auto tHedge = test("Props/hedge") = []
{
    auto scenery = Scenery {};
    auto random = std::mt19937 {7u};
    addHedgePiece(scenery, random, {}, 0.3f, hedgeColors[0]);

    check(scenery.colliders.size() == 1);
    check(scenery.blocks.empty());
    check(instanceCount(scenery.batch) == 2);

    checkSnapshot(scenery, "hedge", {0.f, 1.5f, 0.f}, 8.f);
};

auto tBale = test("Props/bale") = []
{
    auto scenery = Scenery {};
    auto random = std::mt19937 {7u};
    addBale(scenery, random, {});

    check(scenery.colliders.size() == 1);
    check(scenery.colliders[0].top == baleRadius * 2.f);
    check(scenery.blocks.empty());
    check(instanceCount(scenery.batch) == 1);

    checkSnapshot(scenery, "bale", {0.f, 0.8f, 0.f}, 5.f, 2.2f);
};

auto tCrate = test("Props/crate") = []
{
    auto scenery = Scenery {};
    auto random = std::mt19937 {7u};
    auto top = addCrate(scenery, random, {});

    check(scenery.colliders.empty());
    check(scenery.blocks.size() == 1 || scenery.blocks.size() == 2);
    check(instanceCount(scenery.batch) == (int) scenery.blocks.size());
    check(top.y == scenery.blocks.back().top);

    checkSnapshot(scenery, "crate", {0.9f, 1.f, 0.f}, 7.f);
};

auto tRock = test("Props/rock") = []
{
    auto scenery = Scenery {};
    auto random = std::mt19937 {7u};
    addRock(scenery, random, {});

    check(scenery.colliders.size() == 1);
    check(scenery.blocks.empty());
    check(instanceCount(scenery.batch) == 1);

    checkSnapshot(scenery, "rock", {0.f, 0.3f, 0.f}, 4.f);
};

auto tSameDraws = test("Props/sameSeedSameProp") = []
{
    auto first = Scenery {};
    auto second = Scenery {};
    auto firstRandom = std::mt19937 {11u};
    auto secondRandom = std::mt19937 {11u};
    addTree(first, firstRandom, {});
    addTree(second, secondRandom, {});

    check(instanceCount(first.batch) == instanceCount(second.batch));
    check(firstRandom() == secondRandom());
};
