#include "Camera/OrbitCamera.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
bool near(float a, float b, float tolerance = 1e-4f)
{
    return std::abs(a - b) <= tolerance;
}
} // namespace

auto tOrbitClampsPitch = test("OrbitCamera/orbitClampsPitch") = []
{
    auto camera = OrbitCamera {};

    camera.orbit(0.f, 10.f);
    check(near(camera.pitch, 1.3f));

    camera.orbit(0.f, -10.f);
    check(near(camera.pitch, -0.08f));
};

auto tOrbitTurnsYaw = test("OrbitCamera/orbitTurnsYaw") = []
{
    auto camera = OrbitCamera {};
    camera.orbit(0.5f, 0.f);

    check(near(camera.yaw, -0.5f));
};

auto tZoomClampsDistance = test("OrbitCamera/zoomClampsDistance") = []
{
    auto camera = OrbitCamera {};

    camera.zoom(100.f);
    check(near(camera.distance, 5.f));

    camera.zoom(-100.f);
    check(near(camera.distance, 40.f));
};

auto tEyeSitsAtDistance = test("OrbitCamera/eyeSitsAtDistance") = []
{
    auto camera = OrbitCamera {};

    check(near(length(camera.eye() - camera.target), camera.distance));
    check(near(length(camera.forward()), 1.f));
    check(near(dot(camera.forward(), camera.right()), 0.f));
    check(near(dot(camera.forward(), camera.up()), 0.f));
};

auto tProjectionAspect = test("OrbitCamera/projectionAspect") = []
{
    auto camera = OrbitCamera {};

    for (auto aspect: {0.5f, 1.f, 16.f / 9.f})
    {
        auto projection = camera.projection(aspect);
        check(near(projection.at(1, 1) / projection.at(0, 0), aspect));
    }
};

auto tFieldOfViewWidensWhenNarrow =
    test("OrbitCamera/fieldOfViewWidensWhenNarrow") = []
{
    auto camera = OrbitCamera {};
    auto previous = camera.verticalFieldOfView(0.25f);

    for (auto aspect = 0.3f; aspect < 4.f; aspect += 0.05f)
    {
        auto fieldOfView = camera.verticalFieldOfView(aspect);
        check(fieldOfView <= previous);
        previous = fieldOfView;
    }

    check(camera.verticalFieldOfView(0.25f) > camera.fieldOfView);
    check(near(camera.verticalFieldOfView(2.f), camera.fieldOfView));
};
