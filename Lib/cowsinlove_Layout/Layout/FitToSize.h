#pragma once

#include "Layout/Tree.h"

#include <functional>

namespace Cows::Flex
{
// The point sizes a fit may choose: from `least` to `most`, on whole steps of
// `step` so the answer is always one of a few sizes a font cache can keep.
struct PointSizeRange final
{
    float least = 0.f;
    float most = 0.f;
    float step = 0.5f;
};

// `size` rounded down to the range's grid.
float onGrid(float size, float step);

// Makes the tree for one point size.
using TreeAtSize = std::function<Tree(float pointSize)>;

// A size fit-to-size chose, and whether the tree fits at it: false when even
// the range's least overflows, so the caller can try another shape.
struct FittedSize final
{
    float pointSize = 0.f;
    bool fits = false;
};

// The largest size in `range` at which the tree `build` makes resolves into
// `available` with nothing overflowing; `range.least`, not fitting, when none
// does.
FittedSize largestPointSize(const TreeAtSize& build,
                            eacp::Graphics::Rect available,
                            const Measure& measure,
                            const PointSizeRange& range);
} // namespace Cows::Flex
