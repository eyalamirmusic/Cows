#include "Scene/FrameRate.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tAutoCapsTheMenus = test("FrameRate/autoIsSixtyInTheMenusOnly") = []
{
    check(getFrameRateCap(FrameRate::Auto, true) == menuFramesPerSecond);
    check(getFrameRateCap(FrameRate::Auto, false) == noFrameRateCap);
};

auto tFixedRates = test("FrameRate/thirtySixtyAndMaxHoldEverywhere") = []
{
    for (auto menus: {true, false})
    {
        check(getFrameRateCap(FrameRate::Thirty, menus) == thirtyFramesPerSecond);
        check(getFrameRateCap(FrameRate::Sixty, menus) == menuFramesPerSecond);
        check(getFrameRateCap(FrameRate::Max, menus) == noFrameRateCap);
    }
};

auto tNamed = test("FrameRate/namedAsCowsFpsSpellsThem") = []
{
    check(parseFrameRateName("auto") == FrameRate::Auto);
    check(parseFrameRateName("30") == FrameRate::Thirty);
    check(parseFrameRateName("60") == FrameRate::Sixty);
    check(parseFrameRateName("MAX") == FrameRate::Max);
    check(!parseFrameRateName("").has_value());
    check(!parseFrameRateName("120").has_value());
};

auto tUnknown = test("FrameRate/unknownValuesAreAuto") = []
{
    constexpr auto pastTheEnd = 7;
    constexpr auto beforeTheStart = -1;
    auto pastTheEndRate = static_cast<FrameRate>(pastTheEnd);
    auto beforeTheStartRate = static_cast<FrameRate>(beforeTheStart);

    check(replaceUnknownValuesWithDefaults(pastTheEndRate) == FrameRate::Auto);
    check(replaceUnknownValuesWithDefaults(beforeTheStartRate) == FrameRate::Auto);
    check(replaceUnknownValuesWithDefaults(FrameRate::Max) == FrameRate::Max);
};

auto tStepping = test("FrameRate/steppingWrapsRoundBothEnds") = []
{
    check(stepFrameRate(FrameRate::Auto, 1) == FrameRate::Thirty);
    check(stepFrameRate(FrameRate::Thirty, 1) == FrameRate::Sixty);
    check(stepFrameRate(FrameRate::Max, 1) == FrameRate::Auto);
    check(stepFrameRate(FrameRate::Auto, -1) == FrameRate::Max);
    check(stepFrameRate(FrameRate::Sixty, 5) == FrameRate::Max);
};
