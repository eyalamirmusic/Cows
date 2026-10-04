#include "Snapshot.h"
#include "UI/Hud.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;

namespace
{
bool anyDrawn(const Graphics::Image& image, Graphics::Point center, float radius)
{
    auto x0 = (int) (center.x - radius);
    auto y0 = (int) (center.y - radius);

    for (auto y = y0; y < (int) (center.y + radius); ++y)
        for (auto x = x0; x < (int) (center.x + radius); ++x)
            if (x >= 0 && y >= 0 && x < image.width() && y < image.height()
                && !isClearColor(image.at(x, y)))
                return true;

    return false;
}
} // namespace

// The touch controls and the footer, drawn through the HUD pass over an empty
// scene: the rings and the labels land where TouchControls laid them out, and
// the corners stay clear.
auto tHudDrawsControlsAndFooter = test("HudSnapshot/controlsAndFooter") = []
{
    if (!hasDevice())
        return;

    auto width = 390.f;
    auto height = 640.f;

    auto controls = TouchControls {};
    controls.setBounds({0.f, 0.f, width, height});
    controls.resized();
    controls.pointerDown(1, controls.jumpCenter);

    auto footer = Footer {};
    footer.text = [] { return std::string {"stick to walk  -  Moo  -  Jump"}; };

    auto hud = Hud {};
    auto view = SnapshotView {};
    view.drawOverlay = [&](Frame& frame, RenderPass& pass)
    {
        hud.begin(frame, pass, view.sampleCount());
        footer.draw(hud);
        controls.draw(hud);
        hud.end();
    };

    auto image = snapshot(view, width, height, "engine-hud");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(anyDrawn(image, controls.stickHome, 70.f));
    check(anyDrawn(image, controls.mooCenter, 32.f));
    check(!isClearColor(
        image.at((int) controls.jumpCenter.x, (int) controls.jumpCenter.y + 30)));
    check(anyDrawn(image, {width * 0.5f, height - 20.f}, 12.f));

    auto ringTop =
        image.at((int) controls.stickHome.x, (int) (controls.stickHome.y - 64.f));
    check(ringTop.r > 0.8f && ringTop.g > 0.8f && ringTop.b > 0.8f);

    check(isClearColor(image.at(1, 1)));
    check(isClearColor(image.at(image.width() - 2, 1)));
    check(isClearColor(image.at(1, image.height() - 2)));
    check(isClearColor(image.at(image.width() - 2, image.height() - 2)));
};

// The footer with a controller's words, at the window's size and at its
// smallest, where it must still fit between the margins.
auto tHudPadHints = test("HudSnapshot/footerWithPadHints") = []
{
    if (!hasDevice())
        return;

    auto footer = Footer {};
    footer.text = []
    {
        return std::string {"left stick to walk  -  A to jump  -  X to moo  -  "
                            "right stick to look"};
    };

    auto hud = Hud {};
    auto view = SnapshotView {};
    view.drawOverlay = [&](Frame& frame, RenderPass& pass)
    {
        hud.begin(frame, pass, view.sampleCount());
        footer.draw(hud);
        hud.end();
    };

    auto image = snapshot(view, 1280.f, 800.f, "engine-hud-pad");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(anyDrawn(image, {640.f, 780.f}, 12.f));
    check(isClearColor(image.at(1, 1)));

    view.setBounds({0.f, 0.f, 640.f, 400.f});
    auto small = view.renderToImage(1.f);
    check(small.isValid());

    if (!small.isValid())
        return;

    check(anyDrawn(small, {320.f, 380.f}, 12.f));

    for (auto y = 0; y < small.height(); ++y)
        for (auto x: {0, 1, 2, 3, small.width() - 4, small.width() - 1})
            check(isClearColor(small.at(x, y)));
};
