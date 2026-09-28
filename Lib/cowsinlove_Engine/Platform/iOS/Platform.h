#pragma once

#include "UI/Overlay.h"
#include "UI/TouchControls.h"
#include "TouchSurface.h"

#include <memory>

namespace Cows
{
struct Platform final
{
    static constexpr auto touch = true;

    static void importSettings() {}

    void attach(Graphics::View& root,
                TouchControls& touchControls,
                FooterView& footer);

    void drawOverlay(GPUView&, RenderPass&) {}

    std::unique_ptr<TouchSurface> surface;
};
} // namespace Cows
