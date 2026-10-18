#include "Layout/FitToSize.h"

#include <algorithm>
#include <cmath>

namespace Cows::Flex
{
namespace
{
using eacp::Graphics::Point;
using eacp::Graphics::Rect;

bool fitsAt(const TreeAtSize& build,
            Rect available,
            const Measure& measure,
            float pointSize)
{
    auto tree = build(pointSize);
    tree.resolve(available, measure);
    return !tree.overflows();
}

Point fitSizeAt(const TreeAtSize& build,
                Rect available,
                const Measure& measure,
                float pointSize)
{
    auto tree = build(pointSize);
    tree.resolve(available, measure);
    return tree.fitSize();
}

// Where a straight line through (lowSize, lowNeed) and (highSize, highNeed)
// reaches `room`: the size a need that grows in proportion to the point size
// stops fitting at. Unbounded when the need does not grow.
float sizeWhereNeedReaches(
    float lowSize, float lowNeed, float highSize, float highNeed, float room)
{
    auto slope = (highNeed - lowNeed) / (highSize - lowSize);

    if (slope <= 0.f)
        return unbounded;

    return lowSize + (room - lowNeed) / slope;
}

// The largest grid step from `low` to `high` that fits, by halving, given that
// fitting only ever stops as the size grows.
float searchGrid(const TreeAtSize& build,
                 Rect available,
                 const Measure& measure,
                 float low,
                 float step,
                 int steps)
{
    auto fitting = 0;
    auto failing = steps + 1;

    while (failing - fitting > 1)
    {
        auto middle = (fitting + failing) / 2;

        if (fitsAt(build, available, measure, low + step * (float) middle))
            fitting = middle;
        else
            failing = middle;
    }

    return low + step * (float) fitting;
}
} // namespace

float onGrid(float size, float step)
{
    return std::floor(size / step) * step;
}

// Two trees, at the least and the most, give each axis's need as a line in the
// point size when the tree scales linearly; where the first line reaches the
// room is the answer, checked once, one step up and on it. Anything the lines
// get wrong (a least size, a tap target's floor) falls back to a search.
FittedSize largestPointSize(const TreeAtSize& build,
                            Rect available,
                            const Measure& measure,
                            const PointSizeRange& range)
{
    auto step = range.step;
    auto low = std::ceil(range.least / step) * step;
    auto high = std::max(low, onGrid(range.most, step));

    if (!fitsAt(build, available, measure, low))
        return {low, false};

    if (high <= low || fitsAt(build, available, measure, high))
        return {high, true};

    auto lowNeed = fitSizeAt(build, available, measure, low);
    auto highNeed = fitSizeAt(build, available, measure, high);
    auto guess = std::min(
        sizeWhereNeedReaches(low, lowNeed.x, high, highNeed.x, available.w),
        sizeWhereNeedReaches(low, lowNeed.y, high, highNeed.y, available.h));
    auto onStep = std::clamp(onGrid(guess, step), low, high - step);

    if (fitsAt(build, available, measure, onStep)
        && !fitsAt(build, available, measure, onStep + step))
        return {onStep, true};

    auto steps = (int) std::lround((high - low) / step);
    return {searchGrid(build, available, measure, low, step, steps - 1), true};
}
} // namespace Cows::Flex
