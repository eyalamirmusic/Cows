#pragma once

#include "UI/Overlay.h"
#include "UI/TouchControls.h"

namespace Cows
{
struct Platform final
{
    static constexpr auto touch = false;

    void attach(Graphics::View&, TouchControls&, FooterView&) {}
    void drawOverlay(GPUView&, RenderPass&) {}
};
} // namespace Cows
