#include "Snapshot.h"
#include "Terrain/Grass.h"
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
bool finite(Vec4 vector)
{
    return std::isfinite(vector.x) && std::isfinite(vector.y)
           && std::isfinite(vector.z) && std::isfinite(vector.w);
}

bool middleHasGrass(const Graphics::Image& image)
{
    auto centerX = image.width() / 2;
    auto centerY = image.height() / 2;

    for (auto y = centerY - 10; y < centerY + 10; ++y)
        for (auto x = centerX - 20; x < centerX + 20; ++x)
            if (!isClearColor(image.at(x, y)))
                return true;

    return false;
}
} // namespace

auto tCount = test("Grass/bladesPerTile") = []
{ check(makeGrassTile().size() == bladeCount); };

auto tWithinTile = test("Grass/bladesLieWithinTheTile") = []
{
    auto blades = makeGrassTile();

    check(std::all_of(blades.begin(),
                      blades.end(),
                      [](const BladeInstance& blade)
                      {
                          auto x = blade.placement.x;
                          auto z = blade.placement.y;
                          return x >= 0.f && x <= meadowTile && z >= 0.f
                                 && z <= meadowTile;
                      }));
};

auto tFinite = test("Grass/finiteVertexData") = []
{
    auto blade = makeBlade();

    check(!blade.vertices.empty());
    check(blade.indices.size() % 3 == 0);

    for (const auto& vertex: blade.vertices)
        check(std::isfinite(vertex.shape.x) && std::isfinite(vertex.shape.y));

    for (auto index: blade.indices)
        check(index < blade.vertices.size());

    for (const auto& instance: makeGrassTile())
        check(finite(instance.placement) && finite(instance.look)
              && finite(instance.form));
};

auto tGrassSnapshot = test("Grass/snapshot") = []
{
    if (!hasDevice())
        return;

    auto blades = makeGrassTile();
    auto blade = makeBlade();

    auto view = SnapshotView {};
    view.camera.target = {meadowTile * 0.5f, 0.2f, meadowTile * 0.5f};
    view.camera.yaw = 0.6f;
    view.camera.pitch = 0.25f;
    view.camera.distance = 6.f;
    view.time = 1.f;

    auto grassShader = GrassShader {};
    grassShader.setVertices(blade.vertices.data(), blade.vertices.size());
    grassShader.setIndices(blade.indices.data(), blade.indices.size());
    grassShader.setInstances(1, blades.data(), blades.size());

    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = view.sampleCount();
    descriptor.depth = true;
    descriptor.cullMode = CullMode::None;
    grassShader.prepare(descriptor);

    view.drawOpaque = [&](RenderPass& pass, const Mat4& viewProjection)
    {
        view.setSceneUniforms(grassShader, viewProjection);
        grassShader.patchOffset = Vec2 {};
        pass.drawInstanced(grassShader, blades.size());
    };

    auto image = snapshot(view, 300.f, 200.f, "world-grass");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(middleHasGrass(image));
};
