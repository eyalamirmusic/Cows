#include "Scene/FrameRate.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tAutoCapsTheMenus = test("FrameRate/autoIsSixtyInTheMenusOnly") = []
{
    check(getFrameRateCap(FrameRate::Auto, true) == 60);
    check(getFrameRateCap(FrameRate::Auto, false) == 0);
};

auto tFixedRates = test("FrameRate/sixtyAndMaxHoldEverywhere") = []
{
    for (auto menus: {true, false})
    {
        check(getFrameRateCap(FrameRate::Sixty, menus) == 60);
        check(getFrameRateCap(FrameRate::Max, menus) == 0);
    }
};

auto tNamed = test("FrameRate/namedAsCowsFpsSpellsThem") = []
{
    check(parseFrameRateName("auto") == FrameRate::Auto);
    check(parseFrameRateName("60") == FrameRate::Sixty);
    check(parseFrameRateName("MAX") == FrameRate::Max);
    check(!parseFrameRateName("").has_value());
    check(!parseFrameRateName("120").has_value());
};

auto tLabels = test("FrameRate/labelsForTheSettingsRow") = []
{
    check(getFrameRateLabel(FrameRate::Auto) == "Auto");
    check(getFrameRateLabel(FrameRate::Sixty) == "60");
    check(getFrameRateLabel(FrameRate::Max) == "Max");
};

auto tUnknown = test("FrameRate/unknownValuesAreAuto") = []
{
    check(replaceUnknownValuesWithDefaults((FrameRate) 7) == FrameRate::Auto);
    check(replaceUnknownValuesWithDefaults((FrameRate) -1) == FrameRate::Auto);
    check(replaceUnknownValuesWithDefaults(FrameRate::Max) == FrameRate::Max);
};
