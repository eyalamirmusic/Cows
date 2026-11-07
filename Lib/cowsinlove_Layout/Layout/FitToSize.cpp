#include "Layout/FitToSize.h"

#include <algorithm>
#include <cmath>

namespace Cows::Flex
{
namespace
{
using eacp::Graphics::Point;
using eacp::Graphics::Rect;

struct FitProblem final
{
    const TreeAtSize& build;
    Rect available;
    const Measure& measure;
};

struct SizeAndNeed final
{
    float size = 0.f;
    float need = 0.f;
};

bool fitsAt(const FitProblem& problem, float pointSize)
{
    auto tree = problem.build(pointSize);
    tree.resolve(problem.available, problem.measure);
    return !tree.overflows();
}

Point fitSizeAt(const FitProblem& problem, float pointSize)
{
    auto tree = problem.build(pointSize);
    tree.resolve(problem.available, problem.measure);
    return tree.fitSize();
}

// Unbounded when the need does not grow with the size.
float sizeWhereNeedReaches(SizeAndNeed low, SizeAndNeed high, float room)
{
    auto slope = (high.need - low.need) / (high.size - low.size);

    if (slope <= 0.f)
        return unbounded;

    return low.size + (room - low.need) / slope;
}

// Bisection holds because fitting only ever stops as the size grows.
float searchGrid(const FitProblem& problem, float low, float step, int steps)
{
    auto fitting = 0;
    auto failing = steps + 1;

    while (failing - fitting > 1)
    {
        auto middle = (fitting + failing) / 2;

        if (fitsAt(problem, low + step * (float) middle))
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
    auto problem = FitProblem {build, available, measure};
    auto step = range.step;
    auto low = std::ceil(range.least / step) * step;
    auto high = std::max(low, roundDownToGrid(range.most, step));

    if (!fitsAt(problem, low))
        return {low, false};

    if (high <= low || fitsAt(problem, high))
        return {high, true};

    auto lowNeed = fitSizeAt(problem, low);
    auto highNeed = fitSizeAt(problem, high);
    auto guess = std::min(
        sizeWhereNeedReaches({low, lowNeed.x}, {high, highNeed.x}, available.w),
        sizeWhereNeedReaches({low, lowNeed.y}, {high, highNeed.y}, available.h));
    auto onStep = std::clamp(roundDownToGrid(guess, step), low, high - step);

    if (fitsAt(problem, onStep) && !fitsAt(problem, onStep + step))
        return {onStep, true};

    auto steps = (int) std::lround((high - low) / step);
    return {searchGrid(problem, low, step, steps - 1), true};
}
} // namespace Cows::Flex
