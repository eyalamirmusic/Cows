#include "Terrain/Grass.h"
#include "Terrain/Ground.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
Level levelWithGap()
{
    auto level = Level {};
    level.gaps.add({{0.f, 10.f}, {300.f, 6.f}, -30.f});
    return level;
}
} // namespace

auto tPlainPlane = test("Ground/noGapsIsThePlane") = []
{
    auto ground = makeGround(Level {}, groundSize);
    auto plane = makePlane(groundSize);

    check(ground.indices.size() == plane.indices.size());
    check(ground.vertices.size() == plane.vertices.size());

    for (auto index = 0; index < plane.vertices.size(); ++index)
    {
        check(ground.vertices[index].position.x == plane.vertices[index].position.x);
        check(ground.vertices[index].position.z == plane.vertices[index].position.z);
    }

    for (auto index = 0; index < plane.indices.size(); ++index)
        check(ground.indices[index] == plane.indices[index]);

    check(makeChasms(Level {}).lists[(int) Shape::Box].empty());
};

auto tCutOut = test("Ground/gapsAreCutOut") = []
{
    auto level = levelWithGap();
    auto ground = makeGround(level, groundSize);
    auto area = 0.f;

    for (auto first = 0; first + 2 < ground.indices.size(); first += 3)
    {
        auto a = ground.vertices[(int) ground.indices[first]].position;
        auto b = ground.vertices[(int) ground.indices[first + 1]].position;
        auto c = ground.vertices[(int) ground.indices[first + 2]].position;
        auto middle = Vec2 {(a.x + b.x + c.x) / 3.f, (a.z + b.z + c.z) / 3.f};

        check(!level.overGap(middle));
        area +=
            std::abs((b.x - a.x) * (c.z - a.z) - (c.x - a.x) * (b.z - a.z)) * 0.5f;
    }

    check(std::abs(area - (groundSize * groundSize - 600.f * 12.f)) < 1.f);
    check(!makeChasms(level).lists[(int) Shape::Box].empty());
};

auto tGrassTiles = test("Ground/grassKeepsOffGaps") = []
{
    auto level = levelWithGap();
    auto field = GrassField {};
    field.layOver(level);

    check(&field.tileAt({0.f, 48.f}) == &field.tile);
    check(&field.tileAt({0.f, -48.f}) == &field.tile);

    auto corner = Vec2 {0.f, 0.f};
    const auto& cut = field.tileAt(corner);
    check(&cut != &field.tile);
    check(cut.size() < field.tile.size());
    check(&field.tileAt(corner) == &cut);

    for (const auto& blade: cut)
        check(!level.overGap(corner + Vec2 {blade.placement.x, blade.placement.y}));

    auto kept = 0;

    for (const auto& blade: field.tile)
        if (!level.overGap(corner + Vec2 {blade.placement.x, blade.placement.y}))
            ++kept;

    check(cut.size() <= kept && cut.size() > kept - field.tile.size() / 20);
};
