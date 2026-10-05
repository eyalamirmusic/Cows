#include "Camera/OrbitCamera.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
bool near(float a, float b, float margin = 1e-4f)
{
    return std::abs(a - b) <= margin;
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

auto tBlendEnds = test("OrbitCamera/blendRunsFromOnePoseToTheOther") = []
{
    auto from = CameraPose {{0.f, 2.f, 0.f}, 0.3f, 0.1f, 9.f, 0.05f, 0.01f};
    auto to = CameraPose {{4.f, 2.f, -2.f}, 2.f, 0.22f, 11.f, 0.f, 0.f};

    auto start = blend(from, to, 0.f);
    check(near(start.yaw, from.yaw) && near(start.distance, from.distance));
    check(near(start.swayYaw, from.swayYaw));

    auto end = blend(from, to, 1.f);
    check(near(end.yaw, to.yaw) && near(end.pitch, to.pitch));
    check(near(end.distance, to.distance) && near(end.target.x, to.target.x));
    check(near(end.swayYaw, 0.f) && near(end.swayPitch, 0.f));
};

auto tBlendShortWay = test("OrbitCamera/blendTurnsTheShortWayRound") = []
{
    auto from = CameraPose {};
    from.yaw = 0.2f;
    auto to = from;
    to.yaw = twoPi - 0.2f;

    check(near(blend(from, to, 0.5f).yaw, 0.f));
};

auto tEaseOut = test("OrbitCamera/easeOutLeavesFastAndSettles") = []
{
    check(near(easeOut(0.f), 0.f) && near(easeOut(1.f), 1.f));
    check(easeOut(0.25f) > 0.5f);
    check(easeOut(-1.f) == 0.f && easeOut(2.f) == 1.f);

    auto last = 0.f;
    auto lastStep = 1.f;

    for (auto step = 1; step <= 10; ++step)
    {
        auto now = easeOut((float) step / 10.f);
        check(now > last);
        check(now - last < lastStep + 1e-5f);
        lastStep = now - last;
        last = now;
    }
};
