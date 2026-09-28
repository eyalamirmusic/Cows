#include "Render/Instances.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Maths;

auto tMakeInstancePacksColumns = test("Instances/makeInstancePacksColumns") = []
{
    auto model = Mat4::translation({1.f, 2.f, 3.f});
    auto material = Material {{0.2f, 0.4f, 0.6f}, 0.5f, 0.7f, 0.1f, 0.3f, 0.9f};
    auto instance = makeInstance(model, material);

    check(instance.model3.x == 1.f);
    check(instance.model3.y == 2.f);
    check(instance.model3.z == 3.f);
    check(instance.model3.w == 1.f);
    check(instance.normal0.x == 1.f);
    check(instance.color.x == 0.2f);
    check(instance.color.w == 0.5f);
    check(instance.material.x == 0.7f);
    check(instance.material.y == 0.1f);
    check(instance.material.z == 0.3f);
    check(instance.material.w == 0.9f);
    check(instance.pattern0.x == 1.f);
};

auto tBatchAccumulatesPerShape = test("Instances/batchAccumulatesPerShape") = []
{
    auto batch = SurfaceBatch {};
    auto instance = makeInstance({}, {});

    batch.add(Shape::Sphere, instance);
    batch.add(Shape::Sphere, instance);
    batch.add(Shape::Wedge, instance);

    check(batch.lists[(int) Shape::Sphere].size() == 2);
    check(batch.lists[(int) Shape::Wedge].size() == 1);
    check(batch.lists[(int) Shape::Box].empty());
};

auto tBatchClears = test("Instances/batchClears") = []
{
    auto batch = SurfaceBatch {};
    auto instance = makeInstance({}, {});

    for (auto shape = 0; shape < shapeCount; ++shape)
        batch.add((Shape) shape, instance);

    batch.clear();

    for (const auto& list: batch.lists)
        check(list.empty());
};

auto tMakeGlow = test("Instances/makeGlow") = []
{
    auto glow = makeGlow({1.f, 2.f, 3.f}, 4.f, {0.5f, 0.6f, 0.7f});

    check(glow.centerAndSize.x == 1.f);
    check(glow.centerAndSize.w == 4.f);
    check(glow.color.y == 0.6f);
    check(glow.color.w == 1.f);
};
