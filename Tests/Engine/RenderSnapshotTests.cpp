#include "Snapshot.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;

// One white sphere at the origin, lit by the scene's light with nothing in the
// way of it, seen by the orbit camera from four units off.
auto tLitSphere = test("RenderSnapshot/litSphere") = []
{
    if (!hasDevice())
        return;

    auto view = SnapshotView {};
    view.batch.add(Shape::Sphere, makeInstance({}, Material {{0.8f, 0.8f, 0.8f}}));
    view.camera.target = {};
    view.camera.distance = 4.f;
    view.camera.pitch = 0.f;

    auto image = snapshot(view, 160.f, 120.f, "engine-sphere");
    check(image.isValid());

    if (!image.isValid())
        return;

    auto width = image.width();
    auto height = image.height();
    auto centre = image.at(width / 2, height / 2);

    check(!isClearColor(centre));
    check(centre.r > 0.3f && centre.g > 0.3f && centre.b > 0.3f);
    check(isClearColor(image.at(1, 1)));
    check(isClearColor(image.at(width - 2, 1)));
    check(isClearColor(image.at(1, height - 2)));
    check(isClearColor(image.at(width - 2, height - 2)));
};
