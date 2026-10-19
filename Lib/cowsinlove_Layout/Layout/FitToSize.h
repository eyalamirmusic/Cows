#pragma once

#include "Layout/Tree.h"

#include <functional>

namespace Cows::Flex
{
// So a window being resized asks for a few font sizes rather than hundreds.
constexpr auto fontSizeStep = 0.5f;

struct PointSizeRange final
{
    float least = 0.f;
    float most = 0.f;
    float step = fontSizeStep;
};

float roundDownToGrid(float size, float step);

using TreeAtSize = std::function<Tree(float pointSize)>;

struct FittedSize final
{
    float pointSize = 0.f;
    bool fits = false;
};

// range.least, with fits false, when nothing in range fits.
FittedSize largestPointSize(const TreeAtSize& build,
                            eacp::Graphics::Rect available,
                            const Measure& measure,
                            const PointSizeRange& range);
} // namespace Cows::Flex
