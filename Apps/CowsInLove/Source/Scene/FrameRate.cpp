#include "FrameRate.h"

#include "UI/Editor.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <cctype>

namespace Cows
{
namespace
{
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
} // namespace

FrameRate stepFrameRate(FrameRate rate, int steps)
{
    auto isCurrent = [rate](const FrameRateAndLabel& entry)
    { return entry.rate == rate; };
    return stepThroughEntries(frameRatesAndLabels, isCurrent, steps).rate;
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
    if (rate == FrameRate::Thirty)
        return thirtyFramesPerSecond;

    auto alwaysSixty = rate == FrameRate::Sixty;
    auto sixtyInTheMenus = rate == FrameRate::Auto && menusHaveInputAndCameraIsStill;

    if (alwaysSixty || sixtyInTheMenus)
        return menuFramesPerSecond;

    return noFrameRateCap;
}

FrameRate replaceUnknownValuesWithDefaults(FrameRate rate)
{
    auto entry =
        std::ranges::find(frameRatesAndLabels, rate, &FrameRateAndLabel::rate);
    auto isKnown = entry != frameRatesAndLabels.end();
    return isKnown ? rate : FrameRate::Auto;
}
} // namespace Cows
