#pragma once

#include "UI/Overlay.h"
#include "UI/TouchControls.h"

namespace Cows
{
struct Platform final
{
    static constexpr auto touch = false;

    static void importSettings() {}

    void attach(Graphics::View&, TouchControls&, Footer&) {}
};
} // namespace Cows
