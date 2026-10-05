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

// The title on one line, for a wide screen.
TitleMesh makeWide();

// "cows" over "in love", for a tall one.
TitleMesh makeTall();

// Places `title` in front of `camera`, filling `area` (in points, y down, of a
// view `viewSize` points across) as fully as its shape allows, facing the lens.
Maths::Mat4 placement(const OrbitCamera& camera,
                      Graphics::Point viewSize,
                      const Graphics::Rect& area,
                      const TitleMesh& title);
} // namespace Cows::MenuTitle
