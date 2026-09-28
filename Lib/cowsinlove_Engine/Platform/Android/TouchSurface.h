#pragma once

#include "UI/TouchControls.h"

#include <functional>

namespace Cows
{
// Android only. Feeds every finger to `controls` as its own pointer, numbered
// from 1 as on iOS (the mouse is 0), and reports the window's safe area.
struct TouchSurface final
{
    explicit TouchSurface(TouchControls& controls);
    ~TouchSurface();

    TouchSurface(const TouchSurface&) = delete;
    TouchSurface& operator=(const TouchSurface&) = delete;

    std::function<void(Graphics::Insets)> onSafeAreaChanged =
        [](Graphics::Insets) {};
};
} // namespace Cows
