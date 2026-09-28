#include "Levels/Meadow.h"
#include "Render/Mesh.h"
#include "Snapshot.h"
#include "Terrain/TerrainShaders.h"

#include <NanoTest/NanoTest.h>

#include <array>
#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
constexpr auto arenaReach = 86.f;
constexpr auto nearestHiding = 25.f;

bool same(const Collider& a, const Collider& b)
{
    return a.center.x == b.center.x && a.center.y == b.center.y
           && a.radius == b.radius && a.top == b.top;
}

bool same(const Cows::Block& a, const Cows::Block& b)
{
    return a.center.x == b.center.x && a.center.y == b.center.y
           && a.half.x == b.half.x && a.half.y == b.half.y && a.top == b.top
           && a.rise.x == b.rise.x && a.rise.y == b.rise.y;
}

bool same(const Level& a, const Level& b)
{
    if (a.colliders.size() != b.colliders.size()
        || a.blocks.size() != b.blocks.size())
        return false;

    for (auto index = 0; index < (int) a.colliders.size(); ++index)
        if (!same(a.colliders[index], b.colliders[index]))
            return false;

    for (auto index = 0; index < (int) a.blocks.size(); ++index)
        if (!same(a.blocks[index], b.blocks[index]))
            return false;

    return a.hideout.x == b.hideout.x && a.hideout.y == b.hideout.y
           && a.hideout.z == b.hideout.z && a.hiding == b.hiding;
}
} // namespace

auto tSameSeed = test("Meadow/sameSeedSameLevel") = []
{
    check(same(makeMeadow(7), makeMeadow(7)));
    check(!same(makeMeadow(7), makeMeadow(8)));
};

auto tHideouts = test("Meadow/hideoutRules") = []
{
    auto kinds = std::array<int, 5> {};

    for (auto seed = 1u; seed <= 40u; ++seed)
    {
        auto level = makeMeadow(seed);
        auto at = Vec2 {level.hideout.x, level.hideout.z};

        check(length(at) >= nearestHiding);
        check(std::abs(at.x) <= arenaReach && std::abs(at.y) <= arenaReach);
        check(std::abs(level.floorAt(at, level.hideout.y) - level.hideout.y)
              <= 0.01f);

        ++kinds[(std::size_t) level.hiding];
    }

    for (auto count: kinds)
        check(count > 0);
};

auto tMeadowSnapshot = test("Meadow/snapshot") = []
{
    if (!hasDevice())
        return;

    auto level = makeMeadow(3);
    auto view = SnapshotView {};
    view.batch = level.batch;
    view.camera.target = {};
    view.camera.yaw = 0.6f;
    view.camera.pitch = 1.4f;
    view.camera.distance = 90.f;
    view.camera.fieldOfView = radians(100.f);
    view.camera.leastWidth = radians(100.f);

    auto ground = Mesh {makePlane(260.f)};
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

    auto image = snapshot(view, 400.f, 300.f, "world-meadow-seed3");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!isClearColor(image.at(image.width() / 2, image.height() / 2)));
};
