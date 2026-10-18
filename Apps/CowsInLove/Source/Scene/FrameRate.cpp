#include "FrameRate.h"

#include <eacp/Core/Utils/Environment.h>

#include <cctype>

namespace Cows
{
std::string frameRateLabel(FrameRate rate)
{
    switch (rate)
    {
        case FrameRate::Auto:
            return "Auto";
        case FrameRate::Sixty:
            return "60";
        case FrameRate::Max:
            return "Max";
    }

    return "Auto";
}

std::optional<FrameRate> frameRateNamed(std::string_view name)
{
    auto lower = std::string {};

    for (auto character: name)
        lower += (char) std::tolower((unsigned char) character);

    for (auto rate: {FrameRate::Auto, FrameRate::Sixty, FrameRate::Max})
    {
        auto label = frameRateLabel(rate);

        for (auto& character: label)
            character = (char) std::tolower((unsigned char) character);

        if (lower == label)
            return rate;
    }

    return std::nullopt;
}

std::optional<FrameRate> frameRateOverride()
{
    return frameRateNamed(eacp::getEnvValue("COWS_FPS"));
}

int maxFramesPerSecond(FrameRate rate, bool menusHaveInput)
{
    switch (rate)
    {
        case FrameRate::Auto:
            return menusHaveInput ? menuFramesPerSecond : 0;
        case FrameRate::Sixty:
            return menuFramesPerSecond;
        case FrameRate::Max:
            return 0;
    }

    return 0;
}

FrameRate withKnownValue(FrameRate rate)
{
    auto index = (int) rate;
    return index < 0 || index >= frameRates ? FrameRate::Auto : rate;
}
} // namespace Cows
