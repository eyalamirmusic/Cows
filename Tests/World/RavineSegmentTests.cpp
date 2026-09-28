#include "Levels/LevelGenerator.h"
#include "Levels/Segments/RavineSegment.h"
#include "Fixtures.h"
#include "Props/Bale.h"
#include "Snapshot.h"
#include "Terrain/Ground.h"
#include "Terrain/TerrainShaders.h"

#include <NanoTest/NanoTest.h>

#include <algorithm>
#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
constexpr auto jumpHeight = 2.4f;
constexpr auto walkSpeed = 7.f;
constexpr auto playerRadius = 1.f;
constexpr auto clearWindow = 1.5f;

Level makeRavine(std::uint32_t seed)
{
    return generate(meadowRavineFixture(), seed);
}

const Gap& ravine(const Level& level)
{
    return level.gaps.front();
}

const Cows::Block& bridge(const Level& level)
{
    const auto& gap = ravine(level);

    return *std::find_if(level.blocks.begin(),
                         level.blocks.end(),
                         [&](const Cows::Block& block)
                         { return block.top == 0.f && gap.contains(block.center); });
}

bool same(const Level& a, const Level& b)
{
    if (a.colliders.size() != b.colliders.size()
        || a.blocks.size() != b.blocks.size() || a.movers.size() != b.movers.size())
        return false;

    for (auto index = 0; index < a.colliders.size(); ++index)
        if (a.colliders[index].center.x != b.colliders[index].center.x
            || a.colliders[index].center.y != b.colliders[index].center.y)
            return false;

    for (auto index = 0; index < a.blocks.size(); ++index)
        if (a.blocks[index].center.x != b.blocks[index].center.x
            || a.blocks[index].center.y != b.blocks[index].center.y)
            return false;

    return a.hideout.x == b.hideout.x && a.hideout.z == b.hideout.z
           && ravine(a).center.y == ravine(b).center.y;
}

void addAll(SurfaceBatch& into, const SurfaceBatch& from)
{
    for (auto index = 0; index < shapeCount; ++index)
        for (const auto& instance: from.lists[index])
            into.lists[index].add(instance);
}
} // namespace

auto tRavineSameSeed = test("RavineSegment/sameSeedSameLevel") = []
{
    check(same(makeRavine(7), makeRavine(7)));
    check(!same(makeRavine(7), makeRavine(8)));
};

auto tRavineFloor = test("RavineSegment/ravineFloorAndBridgeDeck") = []
{
    for (auto seed = 1u; seed <= 10u; ++seed)
    {
        auto level = makeRavine(seed);
        check(level.gaps.size() == 1);
        check(level.killDepth == ravineKillDepth);

        const auto& gap = ravine(level);
        const auto& deck = bridge(level);

        for (auto x = -80.f; x <= 80.f; x += 4.f)
        {
            auto at = Vec2 {x, gap.center.y};
            auto onDeck = deck.contains(at);

            check(level.hasGround(at) == onDeck);
            check(level.floorAt(at, 0.f) == (onDeck ? 0.f : ravineDepth));
        }

        for (auto z = deck.center.y - deck.half.y; z <= deck.center.y + deck.half.y;
             z += 0.25f)
            for (auto x: {-1.f, 0.f, 1.f})
            {
                auto at = Vec2 {deck.center.x + x * (bridgeHalfWidth - 0.05f), z};
                check(level.floorAt(at, 0.f) == 0.f);
                check(level.hasGround(at));
            }

        check(!gap.contains({deck.center.x, deck.center.y - deck.half.y}));
        check(!gap.contains({deck.center.x, deck.center.y + deck.half.y}));
    }
};

auto tBanks = test("RavineSegment/startSouthHideoutNorth") = []
{
    for (auto seed = 1u; seed <= 20u; ++seed)
    {
        auto level = makeRavine(seed);
        const auto& gap = ravine(level);

        check(level.start.z > gap.center.y + gap.half.y);
        check(level.hideout.z < gap.center.y - gap.half.y);
        check(std::abs(level.startHeading - halfPi) < 1e-6f);
        check(!level.overGap({level.hideout.x, level.hideout.z}));
    }
};

auto tJumpLines = test("RavineSegment/jumpLinesAreJumpableNotSteppable") = []
{
    auto level = makeRavine(3);
    auto lines = 0;

    for (const auto& block: level.blocks)
    {
        if (block.top == 0.f || block.half.x < 2.f || block.half.y > 1.f)
            continue;

        ++lines;
        check(block.top > stepUp && block.top < jumpHeight);
    }

    check(lines > 4 * 20);

    for (auto x = -90.f; x <= 90.f; x += 1.f)
    {
        auto crossings = 0;
        auto wasBlocked = false;

        for (auto z = level.start.z; z > ravine(level).center.y; z -= 0.1f)
        {
            auto blocked =
                distance(level.pushedOut({x, z}, playerRadius, 0.f), Vec2 {x, z})
                > 1e-4f;
            crossings += blocked && !wasBlocked ? 1 : 0;
            wasBlocked = blocked;
        }

        check(crossings >= 2);
    }
};

auto tMoversPeriodic = test("RavineSegment/moversArePeriodicAndStayOnTheBridge") = []
{
    auto level = makeRavine(5);
    const auto& deck = bridge(level);
    check(level.movers.size() == 3);

    for (const auto& mover: level.movers)
    {
        check(mover.collider.radius == rollingBaleRadius);
        check(mover.collider.top > stepUp);

        for (auto t = 0.f; t < 20.f; t += 0.37f)
        {
            auto at = mover.positionAt(t);
            auto later = mover.positionAt(t + mover.period);

            check(distance(at, later) < 1e-3f);
            check(std::abs(at.x - deck.center.x) <= baleSweep + 1e-3f);
            check(std::abs(at.y - deck.center.y) < deck.half.y);
        }
    }
};

auto tUpdateMoves = test("RavineSegment/updateMovesTheColliders") = []
{
    auto level = makeRavine(5);
    const auto& mover = level.movers.front();

    level.update(0.f);
    auto before = mover.collider.center;
    level.update(mover.period * 0.25f);
    auto after = mover.collider.center;

    check(distance(before, after) > 1.f);
    check(distance(after, mover.positionAt(mover.period * 0.25f)) < 1e-5f);
    check(level.moving.lists[(int) Shape::Barrel].size() == 3);
};

auto tClearWindow = test("RavineSegment/eachLaneLeavesAClearWindow") = []
{
    auto level = makeRavine(5);
    const auto& deck = bridge(level);
    auto reach = bridgeHalfWidth + rollingBaleRadius + playerRadius;
    auto step = 0.005f;

    for (const auto& mover: level.movers)
    {
        auto longest = 0.f;
        auto run = 0.f;

        for (auto t = 0.f; t < mover.period * 2.f; t += step)
        {
            auto clear = std::abs(mover.positionAt(t).x - deck.center.x) > reach;
            run = clear ? run + step : 0.f;
            longest = std::max(longest, run);
        }

        auto crossing = (rollingBaleRadius + playerRadius) * 2.f / walkSpeed;
        check(longest >= clearWindow);
        check(longest > crossing * 2.f);
    }

    check(baleLaneSpacing * 0.5f > rollingBaleRadius + playerRadius);
};

auto tRavineSnapshot = test("RavineSegment/snapshot") = []
{
    if (!hasDevice())
        return;

    auto level = makeRavine(3);
    const auto& deck = bridge(level);

    auto view = SnapshotView {};
    view.batch = level.batch;
    addAll(view.batch, level.moving);
    addAll(view.batch, makeChasms(level));
    view.camera.target = {deck.center.x, 0.f, deck.center.y + 4.f};
    view.camera.yaw = -halfPi + 0.5f;
    view.camera.pitch = 0.6f;
    view.camera.distance = 20.f;

    auto ground = Mesh {makeGround(level, 400.f)};
    auto groundShader = GroundShader {};
    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = view.sampleCount();
    descriptor.depth = true;
    groundShader.prepare(descriptor);

    view.drawOpaque = [&](RenderPass& pass, const Mat4& viewProjection)
    {
        view.setSceneUniforms(groundShader, viewProjection);
        groundShader.firstContact = Vec3 {};
        groundShader.secondContact = Vec3 {};
        pass.bind(groundShader, ground.vertices);
        pass.drawIndexed(ground.indices, ground.indexCount);
    };

    auto image = snapshot(view, 640.f, 400.f, "world-ravine-seed3");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!isClearColor(image.at(image.width() / 2, image.height() / 2)));
};
