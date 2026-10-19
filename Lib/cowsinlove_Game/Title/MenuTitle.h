#pragma once

#include "Camera/OrbitCamera.h"
#include "Title/TitleFont.h"

namespace Cows::MenuTitle
{
constexpr auto text = "cows in love";
constexpr auto upperLine = "cows";
constexpr auto lowerLine = "in love";
constexpr auto lineDrop = 2.4f;
constexpr auto depth = 8.f;

TitleMesh makeWide();

TitleMesh makeTall();

// `area` and `viewSize` are in points, y down.
Maths::Mat4 placement(const OrbitCamera& camera,
                      Graphics::Point viewSize,
                      const Graphics::Rect& area,
                      const TitleMesh& title);
} // namespace Cows::MenuTitle
