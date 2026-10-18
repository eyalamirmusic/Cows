#include "Scene/FrameRate.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tAutoCapsTheMenus = test("FrameRate/autoIsSixtyInTheMenusOnly") = []
{
    check(maxFramesPerSecond(FrameRate::Auto, true) == 60);
    check(maxFramesPerSecond(FrameRate::Auto, false) == 0);
};

auto tFixedRates = test("FrameRate/sixtyAndMaxHoldEverywhere") = []
{
    for (auto menus: {true, false})
    {
        check(maxFramesPerSecond(FrameRate::Sixty, menus) == 60);
        check(maxFramesPerSecond(FrameRate::Max, menus) == 0);
    }
};

auto tNamed = test("FrameRate/namedAsCowsFpsSpellsThem") = []
{
    check(frameRateNamed("auto") == FrameRate::Auto);
    check(frameRateNamed("60") == FrameRate::Sixty);
    check(frameRateNamed("MAX") == FrameRate::Max);
    check(!frameRateNamed("").has_value());
    check(!frameRateNamed("120").has_value());
};

auto tLabels = test("FrameRate/labelsForTheSettingsRow") = []
{
    check(frameRateLabel(FrameRate::Auto) == "Auto");
    check(frameRateLabel(FrameRate::Sixty) == "60");
    check(frameRateLabel(FrameRate::Max) == "Max");
};

auto tUnknown = test("FrameRate/unknownValuesAreAuto") = []
{
    check(withKnownValue((FrameRate) 7) == FrameRate::Auto);
    check(withKnownValue((FrameRate) -1) == FrameRate::Auto);
    check(withKnownValue(FrameRate::Max) == FrameRate::Max);
};
