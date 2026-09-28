#include "Sky/SkyDecor.h"
#include "Snapshot.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

auto tSkyInstances = test("Sky/addsSunCloudsAndHills") = []
{
    auto batch = SurfaceBatch {};
    auto glows = Vector<GlowInstance> {};

    SkyDecor::addSun(batch, glows, 0.f, {});
    check(!glows.empty());

    SkyDecor::addClouds(batch, 0.f, {});
    SkyDecor::addHills(batch, {});
    check(!batch.lists[(int) Shape::Sphere].empty());
};

auto tSkySnapshot = test("Sky/snapshot") = []
{
    if (!hasDevice())
        return;

    auto view = SnapshotView {};
    SkyDecor::addSun(view.batch, view.glows, 0.f, {});
    SkyDecor::addClouds(view.batch, 0.f, {});
    SkyDecor::addHills(view.batch, {});
    view.camera.target = {0.f, 5.f, 0.f};
    view.camera.pitch = 0.f;
    view.camera.distance = 10.f;

    auto image = snapshot(view, 320.f, 180.f, "actors-sky");
    check(image.isValid());

    if (!image.isValid())
        return;

    auto aspect = (float) image.width() / (float) image.height();
    auto sun = transformPoint(view.camera.projection(aspect) * view.camera.view(),
                              SkyDecor::sunCenter);
    auto x = (int) ((sun.x * 0.5f + 0.5f) * (float) image.width());
    auto y = (int) ((0.5f - sun.y * 0.5f) * (float) image.height());

    check(x > 0 && x < image.width() && y > 0 && y < image.height());
    check(!isClearColor(image.at(x, y)));
    check(!isClearColor(image.at(image.width() / 2, image.height() * 3 / 5)));
};
