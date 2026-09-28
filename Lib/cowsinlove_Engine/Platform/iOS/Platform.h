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

    void attach(Graphics::View& root,
                TouchControls& touchControls,
                FooterView& footer);

    std::unique_ptr<TouchSurface> surface;
};
} // namespace Cows
