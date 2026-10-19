#pragma once

#include "Render/Common.h"

namespace Cows
{
// Steer: x ahead, y turn. Look: x and y the drag. Zoom: x the amount.
struct ControlEvent final
{
    enum class Kind
    {
        Steer,
        Jump,
        Moo,
        Restart,
        Look,
        Zoom
    };

    Kind kind;
    float x = 0.f;
    float y = 0.f;
};

inline float wheelZoom(const Graphics::MouseEvent& event)
{
    constexpr auto lineZoom = 0.1f;
    constexpr auto preciseZoom = 0.01f;

    auto scale = event.preciseScrolling ? preciseZoom : lineZoom;
    return event.delta.y * scale;
}
} // namespace Cows
