#include "Scene/CowsView.h"

#include "Ending.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto frameStep = 1.f / 60.f;
constexpr auto meadowStage = 0;
constexpr auto ravineStage = 1;
constexpr auto anySeed = 3u;
constexpr auto zoomedFullyOut = -100.f;
constexpr auto zoomedFullyIn = 100.f;
constexpr auto pitchedFullyDown = -10.f;
constexpr auto playSeconds = 4.f;
constexpr auto fallSeconds = 6.f;
constexpr auto roundingMargin = 1e-3f;

bool isEyeAboveGround(const CowsView& view)
{
    auto eye = view.camera.eye();
    auto floor = view.game.level.groundAt({eye.x, eye.z});
    return eye.y >= floor + cameraGroundClearance - roundingMargin;
}

void pitchFullyDown(CowsView& view, float zoom)
{
    view.camera.zoom(zoom);
    view.camera.orbit(0.f, pitchedFullyDown);
}

// Runs the view's frames for `seconds`, walking, turning and jumping, and
// whether the eye was above the ground every frame.
bool eyeStaysAboveGroundWhilePlaying(CowsView& view, float seconds)
{
    view.input.walkingForward = true;
    view.input.walkingLeft = true;
    view.input.jumping = true;

    auto allAbove = true;
    auto clock = Threads::FrameTime {0.0, frameStep};

    for (auto time = 0.f; time < seconds; time += frameStep)
    {
        clock.time += frameStep;
        view.update(clock);
        allAbove = allAbove && isEyeAboveGround(view);
    }

    return allAbove;
}
} // namespace

auto tMeadowZoomedOut = test("CameraGround/eyeStaysAboveTheMeadowZoomedOut") = []
{
    if (!Device::shared().isValid())
        return;

    auto view = CowsView {{QualityChoice::High, {}}, Stages {meadowStage}, anySeed};
    view.game.start();
    pitchFullyDown(view, zoomedFullyOut);
    auto wantedPitch = view.camera.pitch;

    check(eyeStaysAboveGroundWhilePlaying(view, playSeconds));
    check(view.camera.pitch == wantedPitch);
};

auto tRavineFall = test("CameraGround/eyeStaysAboveTheRavineFallingIn") = []
{
    if (!Device::shared().isValid())
        return;

    for (auto zoom: {zoomedFullyOut, zoomedFullyIn})
    {
        auto view =
            CowsView {{QualityChoice::High, {}}, Stages {ravineStage}, anySeed};
        auto& game = view.game;
        game.start();
        pitchFullyDown(view, zoom);

        auto gap = game.level.gaps.front();
        game.player = {gap.center.x, 0.f, gap.center.y};
        view.camera.target = game.player;

        check(eyeStaysAboveGroundWhilePlaying(view, fallSeconds));
        check(game.sinceFell < fallSeconds);
    }
};

auto tEnding = test("CameraGround/eyeStaysAboveGroundInTheEnding") = []
{
    if (!Device::shared().isValid())
        return;

    auto view = CowsView {{QualityChoice::High, {}}, Stages {meadowStage}, anySeed};
    auto& game = view.game;
    game.start();
    pitchFullyDown(view, zoomedFullyOut);
    game.player = Ending::foundStart(game);

    auto allAbove = true;
    auto clock = Threads::FrameTime {0.0, frameStep};

    for (auto time = 0.f; time < Ending::loopTime; time += frameStep)
    {
        clock.time += frameStep;
        view.update(clock);
        allAbove = allAbove && isEyeAboveGround(view);
    }

    check(game.state == Game::State::Found);
    check(allAbove);
};
