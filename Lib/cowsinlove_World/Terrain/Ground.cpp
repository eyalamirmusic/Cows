#include "Terrain/Ground.h"
#include "Props/Props.h"

#include <algorithm>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr std::uint32_t rockColor = 0x4a3e33;
constexpr std::uint32_t floorColor = 0x2a231d;
constexpr auto wallThickness = 1.f;
constexpr auto ledge = 0.02f;

Vector<float> cutsAlong(const Level& level, float half, bool alongX)
{
    auto cuts = Vector<float> {-half, half};

    for (const auto& gap: level.gaps)
    {
        auto center = alongX ? gap.center.x : gap.center.y;
        auto extent = alongX ? gap.half.x : gap.half.y;

        for (auto edge: {center - extent, center + extent})
            cuts.add(std::clamp(edge, -half, half));
    }

    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    return cuts;
}

void addQuad(MeshData& mesh, float left, float right, float near, float far)
{
    auto up = Vec3 {0.f, 1.f, 0.f};
    auto base = (std::uint32_t) mesh.vertices.size();

    mesh.vertices.add({{left, 0.f, far}, up});
    mesh.vertices.add({{right, 0.f, far}, up});
    mesh.vertices.add({{right, 0.f, near}, up});
    mesh.vertices.add({{left, 0.f, near}, up});

    mesh.indices.add({base, base + 1u, base + 2u, base, base + 2u, base + 3u});
}

void addSlab(SurfaceBatch& batch, Vec3 bottom, Vec3 size, const Material& material)
{
    batch.add(Shape::Box,
              makeInstance(Mat4::translation(bottom) * Mat4::scale(size), material));
}
} // namespace

MeshData makeGround(const Level& level, float size)
{
    if (level.gaps.empty())
        return makePlane(size);

    auto half = size * 0.5f;
    auto xs = cutsAlong(level, half, true);
    auto zs = cutsAlong(level, half, false);
    auto mesh = MeshData {};

    for (auto column = 0; column + 1 < xs.size(); ++column)
        for (auto row = 0; row + 1 < zs.size(); ++row)
        {
            auto middle = Vec2 {(xs[column] + xs[column + 1]) * 0.5f,
                                (zs[row] + zs[row + 1]) * 0.5f};

            if (!level.overGap(middle))
                addQuad(mesh, xs[column], xs[column + 1], zs[row], zs[row + 1]);
        }

    return mesh;
}

SurfaceBatch makeChasms(const Level& level)
{
    auto batch = SurfaceBatch {};
    auto rock = matte(rockColor, 0.1f, 0.05f);
    auto floor = matte(floorColor, 0.1f, 0.05f);

    for (const auto& gap: level.gaps)
    {
        auto depth = -gap.depth - ledge;
        auto bottom = gap.depth;
        auto width = gap.half.x * 2.f + wallThickness * 2.f;
        auto length = gap.half.y * 2.f;

        for (auto side: {-1.f, 1.f})
        {
            auto z = gap.center.y + side * (gap.half.y + wallThickness * 0.5f);
            addSlab(batch,
                    {gap.center.x, bottom, z},
                    {width, depth, wallThickness},
                    rock);

            auto x = gap.center.x + side * (gap.half.x + wallThickness * 0.5f);
            addSlab(batch,
                    {x, bottom, gap.center.y},
                    {wallThickness, depth, length},
                    rock);
        }

        addSlab(batch,
                {gap.center.x, bottom - wallThickness, gap.center.y},
                {width, wallThickness, length + wallThickness * 2.f},
                floor);
    }

    return batch;
}
} // namespace Cows
