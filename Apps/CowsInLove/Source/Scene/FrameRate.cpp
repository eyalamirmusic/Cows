#include "FrameRate.h"

#include "UI/Editor.h"

#include <eacp/Core/Utils/Environment.h>

#include <cctype>

namespace Cows
{
namespace
{
constexpr bool frameRatesAreInEnumOrder()
{
    for (auto index = 0; index < numFrameRates; ++index)
        if (static_cast<int>(frameRatesAndLabels[index].rate) != index)
            return false;

    return true;
}

static_assert(frameRatesAreInEnumOrder());

std::string makeLowercase(std::string_view text)
{
    auto lowercase = std::string {};

    for (auto character: text)
    {
        auto asUnsigned = static_cast<unsigned char>(character);
        auto lowered = std::tolower(asUnsigned);
        lowercase += static_cast<char>(lowered);
    }

    return lowercase;
}

int getFrameRateIndex(FrameRate rate)
{
    return static_cast<int>(rate);
}
} // namespace

FrameRate stepFrameRate(FrameRate rate, int steps)
{
    auto steppedIndex =
        wrapChoiceIndex(getFrameRateIndex(rate) + steps, numFrameRates);
    return frameRatesAndLabels[steppedIndex].rate;
}

std::optional<FrameRate> parseFrameRateName(std::string_view name)
{
    auto lowercaseName = makeLowercase(name);

    for (const auto& entry: frameRatesAndLabels)
        if (makeLowercase(entry.label) == lowercaseName)
            return entry.rate;

    return std::nullopt;
}

std::optional<FrameRate> readFrameRateFromEnvironment()
{
    return parseFrameRateName(eacp::getEnvValue("COWS_FPS"));
}

int getFrameRateCap(FrameRate rate, bool menusHaveInputAndCameraIsStill)
{
    auto alwaysSixty = rate == FrameRate::Sixty;
    auto sixtyInTheMenus = rate == FrameRate::Auto && menusHaveInputAndCameraIsStill;

    if (alwaysSixty || sixtyInTheMenus)
        return menuFramesPerSecond;

    return noFrameRateCap;
}

FrameRate replaceUnknownValuesWithDefaults(FrameRate rate)
{
    auto index = getFrameRateIndex(rate);
    auto isKnown = index >= 0 && index < numFrameRates;
    return isKnown ? rate : FrameRate::Auto;
}
} // namespace Cows
