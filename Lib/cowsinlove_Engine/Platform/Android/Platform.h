#pragma once

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

    void attach(Graphics::View& root, TouchControls& touchControls, Footer& footer);

    std::unique_ptr<TouchSurface> surface;
};
} // namespace Cows
