#pragma once

#include "HudLayer.h"
#include "TouchSurface.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <memory>

namespace Cows
{
struct Platform final
{
    static constexpr auto touch = true;

    // An app started by `am start` has no environment of its own, so the
    // COWS_* settings come from the `debug.cows.env` system property instead:
    // `adb shell setprop debug.cows.env "COWS_PROFILE=1 COWS_SEED=3"`.
    static void importSettings();

    void attach(Graphics::View& root,
                TouchControls& touchControls,
                FooterView& footer);

    // Android has one surface, so the footer and the controls are painted over
    // the scene inside its own pass.
    void drawOverlay(GPUView& scene, RenderPass& pass);

    std::unique_ptr<TouchSurface> surface;
    std::unique_ptr<HudLayer> hud;
};
} // namespace Cows
