#include "UI/FontCache.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
constexpr auto family = "Menlo";
constexpr auto firstSize = 10.f;
} // namespace

auto tFontHit = test("FontCache/aSecondAskIsTheSameFont") = []
{
    auto cache = FontCache {};
    const auto& first = cache.fontFor(family, firstSize);
    const auto& again = cache.fontFor(family, firstSize);

    check(&first == &again);
    check(cache.size() == 1);
};

auto tFontMiss = test("FontCache/familyStyleAndSizeEachMakeANewFont") = []
{
    auto cache = FontCache {};
    cache.fontFor(family, firstSize);
    cache.fontFor("Comic Neue", firstSize);
    cache.fontFor(family, firstSize, Text::FontStyle::Bold);
    cache.fontFor(family, firstSize + fontSizeStep);

    check(cache.size() == 4);
};

auto tFontGrid = test("FontCache/sizesLandOnTheGrid") = []
{
    constexpr auto offGrid = 13.37f;
    constexpr auto onGrid = 13.f;

    auto cache = FontCache {};
    const auto& font = cache.fontFor(family, offGrid);

    check(font.pointSize == onGrid);
    check(&cache.fontFor(family, onGrid + fontSizeStep * 0.9f) == &font);
    check(cache.size() == 1);
    check(onFontGrid(0.1f) == fontSizeStep);
};
