#include "Layout/FitToSize.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows::Flex;

namespace
{
constexpr auto characterWidthPerPointSize = 0.5f;
constexpr auto step = 0.5f;
constexpr auto tapTarget = 64.f;

float measureByCharacters(std::string_view text, float pointSize)
{
    return (float) text.size() * pointSize * characterWidthPerPointSize;
}

bool onTheGrid(float size)
{
    return std::fmod(size, step) == 0.f;
}

TreeAtSize labels(eacp::Vector<std::string> texts, bool targetFloor)
{
    return [texts, targetFloor](float pointSize)
    {
        auto column = Node {};
        column.direction = Direction::Down;
        auto tree = Tree {column};

        for (const auto& text: texts)
        {
            auto row = Node {};
            row.text = Text {text, pointSize};
            row.padding = {.top = pointSize * 0.1f, .bottom = pointSize * 0.1f};
            row.height = Size::fit(targetFloor ? tapTarget : 0.f);
            tree.add(Tree::root, row);
        }

        return tree;
    };
}

bool fits(const TreeAtSize& build, eacp::Graphics::Rect area, float size)
{
    auto tree = build(size);
    tree.resolve(area, measureByCharacters);
    return !tree.overflows();
}
} // namespace

// Ten characters, half the size each, in 200 points: 40 points wide at most.
auto tLinear = test("FitToSize/linearTreesTakeTheClosedForm") = []
{
    auto build = labels({"abcdefghij", "abc"}, false);
    auto area = eacp::Graphics::Rect {0.f, 0.f, 200.f, 1000.f};
    auto size =
        largestPointSize(build, area, measureByCharacters, {4.f, 100.f, step})
            .pointSize;

    check(size == 40.f);
    check(fits(build, area, size));
    check(!fits(build, area, size + step));
};

// The 64 point floor holds until the text outgrows it, so the need is not linear.
auto tFloor = test("FitToSize/aTapTargetFloorFallsBackToTheSearch") = []
{
    auto build = labels({"ab", "ab", "ab"}, true);
    auto area = eacp::Graphics::Rect {0.f, 0.f, 1000.f, 200.f};
    auto size =
        largestPointSize(build, area, measureByCharacters, {4.f, 120.f, step})
            .pointSize;

    check(onTheGrid(size));
    check(fits(build, area, size));
    check(!fits(build, area, size + step));
};

auto tRange = test("FitToSize/staysInsideItsRangeOnTheGrid") = []
{
    auto build = labels({"ab"}, false);
    auto roomy = eacp::Graphics::Rect {0.f, 0.f, 1000.f, 1000.f};
    auto cramped = eacp::Graphics::Rect {0.f, 0.f, 4.f, 4.f};

    auto roomyFit =
        largestPointSize(build, roomy, measureByCharacters, {8.f, 33.3f, step});
    auto crampedFit =
        largestPointSize(build, cramped, measureByCharacters, {8.2f, 40.f, step});

    check(roomyFit.pointSize == 33.f && roomyFit.fits);
    check(crampedFit.pointSize == 8.5f && !crampedFit.fits);
    check(roundDownToGrid(13.74f, step) == 13.5f);
};
