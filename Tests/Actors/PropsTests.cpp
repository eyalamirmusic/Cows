#include "Props/Bale.h"
#include "Props/Barn.h"
#include "Props/Crate.h"
#include "Props/Hedge.h"
#include "Props/Meshes.h"
#include "Props/PropMeshes.h"
#include "Props/Props.h"
#include "Props/Rock.h"
#include "Props/Tree.h"
#include "Snapshot.h"

#include <NanoTest/NanoTest.h>

#include <algorithm>
#include <cmath>
#include <limits>
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

constexpr auto snapshotPitch = 0.3f;
constexpr auto snapshotWidth = 200.f;
constexpr auto snapshotHeight = 150.f;

struct Shot final
{
    Vec3 target;
    float distance = 10.f;
    float yaw = 0.6f;
    float pitch = snapshotPitch;
    float width = snapshotWidth;
    float height = snapshotHeight;
};

Shot looking(Vec3 eye, Vec3 target)
{
    auto away = eye - target;
    auto reach = length(away);
    return {target, reach, std::atan2(away.x, away.z), std::asin(away.y / reach)};
}

void checkSnapshot(const Scenery& scenery, const std::string& name, const Shot& shot)
{
    if (!hasDevice())
        return;

    auto view = SnapshotView {makeContentMesh};
    view.batch = scenery.batch;
    view.camera.target = shot.target;
    view.camera.yaw = shot.yaw;
    view.camera.pitch = shot.pitch;
    view.camera.distance = shot.distance;

    auto image = snapshot(view, shot.width, shot.height, "actors-" + name);
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!isClearColor(image.at(image.width() / 2, image.height() / 2)));
}

void checkSnapshot(const Scenery& scenery,
                   const std::string& name,
                   Vec3 target,
                   float distance,
                   float yaw = 0.6f)
{
    checkSnapshot(scenery, name, Shot {target, distance, yaw});
}

constexpr auto playerHeight = 1.5f;
constexpr auto stepUp = 0.55f;
constexpr auto jumpHeight = 2.4f;
constexpr auto climbJump = 1.8f;
constexpr auto distanceFromDoorwayMiddleThatIsSurelyWall = 4.f;
constexpr auto vaneHeight = 1.2f;

constexpr auto barnBlocks = 34;
constexpr auto barnColliders = 2;
constexpr auto barnShapes = 7;
constexpr auto barnShotWidth = 480.f;
constexpr auto barnShotHeight = 360.f;
constexpr std::uint32_t meadowColor = 0x86b04f;
constexpr auto meadowSide = 80.f;
constexpr auto meadowThickness = 0.02f;

void addMeadow(Scenery& scenery)
{
    scenery.batch.add(
        Shape::Box,
        makeInstance(Mat4::translation({0.f, -meadowThickness, 0.f})
                         * Mat4::scale({meadowSide, meadowThickness, meadowSide}),
                     matte(meadowColor)));
}

Vec2 ground(Vec3 point)
{
    return {point.x, point.z};
}

Vec2 ahead(float heading)
{
    return {std::cos(heading), -std::sin(heading)};
}

bool floats(const Cows::Block& block)
{
    return block.bottom > 0.f;
}

bool ramps(const Cows::Block& block)
{
    return lengthSquared(block.rise) > 0.f;
}

Vector<Cows::Block> blocksAt(const Scenery& scenery, Vec2 point, bool floating)
{
    auto found = Vector<Cows::Block> {};

    for (const auto& block: scenery.blocks)
        if (block.contains(point) && floats(block) == floating)
            found.add(block);

    return found;
}

float highestTop(const Scenery& scenery)
{
    auto top = 0.f;

    for (const auto& block: scenery.blocks)
        top = std::max(top, block.top);

    return top;
}

struct ClimbTop final
{
    Cows::Block block;
    Vec2 toward;
};

ClimbTop climbTop(const Scenery& scenery)
{
    auto top = ClimbTop {};

    for (const auto& block: scenery.blocks)
    {
        auto underRoof = false;

        for (const auto& roof: blocksAt(scenery, block.center, true))
            underRoof = underRoof || ramps(roof);

        if (!floats(block) && !underRoof && block.top > top.block.top)
            top.block = block;
    }

    auto toBarn = scenery.clearings[0].center - top.block.center;
    top.toward = std::abs(toBarn.x) > std::abs(toBarn.y)
                     ? Vec2 {std::copysign(1.f, toBarn.x), 0.f}
                     : Vec2 {0.f, std::copysign(1.f, toBarn.y)};
    return top;
}

float roofEdgeAhead(const Scenery& scenery, Vec2 from, Vec2 toward)
{
    constexpr auto stride = 0.02f;

    for (auto reach = 0.f; reach < barnReach(); reach += stride)
    {
        auto at = from + toward * reach;

        for (const auto& block: blocksAt(scenery, at, true))
            if (ramps(block))
                return block.heightAt(at);
    }

    return std::numeric_limits<float>::max();
}

Scenery barnScenery()
{
    auto scenery = Scenery {};
    addBarn(scenery, {}, 0);
    return scenery;
}
} // namespace

auto tBarn = test("Props/barn") = []
{
    auto scenery = barnScenery();

    check(scenery.blocks.size() == barnBlocks);
    check(scenery.colliders.size() == barnColliders);
    check(instanceCount(scenery.batch) == barnShapes);
    check(scenery.clearings.size() == 1);
};

auto tBarnDoorway = test("Props/barnDoorwayIsOpenAndTheWallBesideItIsNot") = []
{
    auto scenery = barnScenery();
    auto doorway = barnSpots({}, 0).doorway;
    auto at = ground(doorway.at);

    check(blocksAt(scenery, at, false).empty());

    auto lowest = std::numeric_limits<float>::max();

    for (const auto& block: blocksAt(scenery, at, true))
        if (!ramps(block))
            lowest = std::min(lowest, block.bottom);

    check(lowest >= jumpHeight + playerHeight);
    check(lowest < std::numeric_limits<float>::max());

    auto forward = ahead(doorway.heading);
    auto beside =
        at
        + Vec2 {forward.y, -forward.x} * distanceFromDoorwayMiddleThatIsSurelyWall;
    check(!blocksAt(scenery, beside, false).empty());
};

auto tBarnLoft = test("Props/barnLoftIsWalkedUnderAndNotJumpedOnto") = []
{
    auto scenery = barnScenery();
    auto loft = barnSpots({}, 0).loft;
    auto floors = blocksAt(scenery, ground(loft.at), true);

    auto found = false;

    for (const auto& block: floors)
    {
        if (ramps(block) || block.top != loft.at.y)
            continue;

        found = true;
        check(block.bottom >= playerHeight);
        check(block.top > jumpHeight + stepUp);
    }

    check(found);
};

auto tBarnRoof = test("Props/barnRidgeIsTheRoofsTop") = []
{
    auto scenery = barnScenery();
    auto roof = barnSpots({}, 0).roof;
    auto highest = 0.f;

    for (const auto& block: blocksAt(scenery, ground(roof.at), true))
        highest = std::max(highest, block.heightAt(ground(roof.at)));

    check(std::abs(highest - roof.at.y) < 1e-4f);
};

auto tBarnMesh = test("Props/barnMeshHasEveryMaterial") = []
{
    constexpr auto mostTriangles = 15000;
    auto vaneTop = highestTop(barnScenery()) + vaneHeight;
    auto triangles = 0;

    for (auto shape: {Shape::BarnWall,
                      Shape::BarnTrim,
                      Shape::BarnRoof,
                      Shape::BarnWood,
                      Shape::BarnHay,
                      Shape::BarnMetal,
                      Shape::BarnTwine})
    {
        auto mesh = makePropMesh(shape);
        check(!mesh.indices.empty());
        triangles += (int) mesh.indices.size() / 3;

        for (const auto& vertex: mesh.vertices)
            check(vertex.position.y >= 0.f && vertex.position.y <= vaneTop);
    }

    check(triangles < mostTriangles);
    check(!isPropShape(Shape::Box));
};

auto tBarnClimb = test("Props/barnClimbIsAJumpAtATime") = []
{
    auto scenery = barnScenery();
    const auto& floor = scenery.clearings[0];
    auto tops = Vector<float> {};

    for (const auto& block: scenery.blocks)
    {
        auto outside = std::abs(block.center.x - floor.center.x) > floor.half.x
                       || std::abs(block.center.y - floor.center.y) > floor.half.y;
        auto stack = block.half.x >= 1.f && block.half.y >= 1.f;

        if (outside && !floats(block) && (ramps(block) || stack))
            tops.add(block.top);
    }

    std::sort(tops.begin(), tops.end());
    check(tops.size() > 2);

    for (auto index = 1; index < (int) tops.size(); ++index)
        check(tops[index] - tops[index - 1] <= climbJump);

    auto top = climbTop(scenery);
    check(std::abs(top.block.top - tops.back()) < 1e-4f);

    auto eaves = roofEdgeAhead(scenery, top.block.center, top.toward);
    check(eaves - top.block.top > 0.f && eaves - top.block.top <= stepUp);
};

auto tBarnShots = test("Props/barnShots") = []
{
    auto scenery = Scenery {};
    addBarn(scenery, {}, 0);
    addMeadow(scenery);

    auto shot = [&](const std::string& name, Shot view)
    {
        view.width = barnShotWidth;
        view.height = barnShotHeight;
        checkSnapshot(scenery, name, view);
    };

    shot("barn", {{-2.f, 4.f, 2.f}, 40.f, 0.6f});
    shot("barn-front", {{0.f, 5.f, 0.f}, 34.f, 3.7f, 0.18f});
    shot("barn-back", {{0.f, 5.f, 0.f}, 34.f, -0.55f, 0.2f});
    shot("barn-roof", {{-4.f, 6.f, 0.f}, 34.f, -0.9f, 0.55f});
    shot("barn-inside", looking({0.f, 1.5f, -10.3f}, {0.f, 3.f, 6.f}));
    shot("barn-ramp", looking({1.5f, 1.5f, 9.f}, {6.5f, 2.5f, 0.f}));
    shot("barn-under-loft", looking({2.f, 1.5f, -1.f}, {-7.f, 1.5f, -7.f}));
    shot("barn-from-loft", looking({0.5f, 6.5f, -9.f}, {0.f, 2.f, 10.f}));
    shot("barn-loft-bales", looking({1.f, 6.5f, -6.f}, {-5.5f, 5.6f, -9.5f}));
    shot("barn-climb", looking({-15.f, 3.f, -5.f}, {-9.f, 3.f, 1.f}));
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
    check(instanceCount(scenery.batch) == (int) baleParts().size());

    checkSnapshot(scenery, "bale", {0.f, 0.8f, 0.f}, 5.f, 2.2f);
};

auto tBaleMesh = test("Props/baleMeshFitsTheDrumFrame") = []
{
    constexpr auto mostTriangles = 1500;
    constexpr auto halfUnit = 0.5f;
    // Loose straws stand a little proud of the drum.
    constexpr auto strawReach = 0.04f;
    auto triangles = 0;

    for (const auto& part: baleParts())
    {
        auto mesh = makePropMesh(part.shape);
        check(!mesh.indices.empty());
        triangles += (int) mesh.indices.size() / 3;

        for (const auto& vertex: mesh.vertices)
        {
            auto radial = length(Vec2 {vertex.position.x, vertex.position.z});
            check(vertex.position.y >= 0.f && vertex.position.y <= 1.f);
            check(radial <= halfUnit + strawReach);
        }
    }

    check(triangles < mostTriangles);
};

constexpr auto baleShotWidth = 480.f;
constexpr auto baleShotHeight = 360.f;

namespace
{
void addBaleAt(Scenery& scenery, const Mat4& model)
{
    for (const auto& part: baleParts())
        scenery.batch.add(part.shape, makeInstance(model, part.material));
}
} // namespace

auto tBaleShots = test("Props/baleShots") = []
{
    constexpr auto closeHeading = 1.1f;
    constexpr auto stackLength = 1.7f;

    auto close = Scenery {};
    addMeadow(close);
    addBaleAt(close, lyingDrum({}, 0.f, closeHeading, 0.f, baleRadius, stackLength));
    const auto closeEye = Vec3 {2.9f, 2.1f, 3.4f};
    auto closeShot = looking(closeEye, {0.f, baleRadius, 0.f});
    closeShot.width = baleShotWidth;
    closeShot.height = baleShotHeight;
    checkSnapshot(close, "bale-close", closeShot);

    auto stack = Scenery {};
    addMeadow(stack);
    auto across = baleRadius * 2.f;
    auto restHeight = baleRadius * std::sqrt(3.f);

    for (auto side: {-0.5f, 0.5f})
        addBaleAt(
            stack,
            lyingDrum({side * across, 0.f}, 0.f, 0.f, 0.f, baleRadius, stackLength));

    addBaleAt(stack, lyingDrum({}, restHeight, 0.f, 0.f, baleRadius, stackLength));
    checkSnapshot(stack,
                  "bale-stack",
                  Shot {{0.f, 1.3f, 0.f},
                        7.f,
                        0.7f,
                        snapshotPitch,
                        baleShotWidth,
                        baleShotHeight});
};

auto tRollingBale = test("Props/rollingBale") = []
{
    constexpr auto track = 4.f;
    constexpr auto period = 8.f;
    constexpr auto midRoll = period * 0.3f;

    auto mover = makeRollingBale({-track, 0.f}, {track, 0.f}, period, 0.f);
    auto scenery = Scenery {};
    addMeadow(scenery);
    addBaleAt(scenery, mover.modelAt(midRoll));

    auto at = mover.positionAt(midRoll);
    checkSnapshot(scenery,
                  "bale-rolling",
                  Shot {{at.x, rollingBaleRadius, at.y},
                        8.f,
                        0.5f,
                        snapshotPitch,
                        baleShotWidth,
                        baleShotHeight});
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
