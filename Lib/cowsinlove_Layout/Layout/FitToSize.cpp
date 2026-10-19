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

// Unbounded when the need does not grow with the size.
float sizeWhereNeedReaches(
    float lowSize, float lowNeed, float highSize, float highNeed, float room)
{
    auto slope = (highNeed - lowNeed) / (highSize - lowSize);

    if (slope <= 0.f)
        return unbounded;

    return lowSize + (room - lowNeed) / slope;
}

// Bisection holds because fitting only ever stops as the size grows.
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

float roundDownToGrid(float size, float step)
{
    return std::floor(size / step) * step;
}

// Extrapolates linearly from two trees; falls back to a search when that is wrong.
FittedSize largestPointSize(const TreeAtSize& build,
                            Rect available,
                            const Measure& measure,
                            const PointSizeRange& range)
{
    auto step = range.step;
    auto low = std::ceil(range.least / step) * step;
    auto high = std::max(low, roundDownToGrid(range.most, step));

    if (!fitsAt(build, available, measure, low))
        return {low, false};

    if (high <= low || fitsAt(build, available, measure, high))
        return {high, true};

    auto lowNeed = fitSizeAt(build, available, measure, low);
    auto highNeed = fitSizeAt(build, available, measure, high);
    auto guess = std::min(
        sizeWhereNeedReaches(low, lowNeed.x, high, highNeed.x, available.w),
        sizeWhereNeedReaches(low, lowNeed.y, high, highNeed.y, available.h));
    auto onStep = std::clamp(roundDownToGrid(guess, step), low, high - step);

    if (fitsAt(build, available, measure, onStep)
        && !fitsAt(build, available, measure, onStep + step))
        return {onStep, true};

    auto steps = (int) std::lround((high - low) / step);
    return {searchGrid(build, available, measure, low, step, steps - 1), true};
}
} // namespace Cows::Flex
