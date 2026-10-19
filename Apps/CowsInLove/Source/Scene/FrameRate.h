#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
enum class FrameRate
{
    Auto,
    Thirty,
    Sixty,
    Max
};

struct FrameRateAndLabel final
{
    FrameRate rate;
    std::string_view label;
};

constexpr auto frameRatesAndLabels = std::array {
    FrameRateAndLabel {FrameRate::Auto, "Auto"},
    FrameRateAndLabel {FrameRate::Thirty, "30"},
    FrameRateAndLabel {FrameRate::Sixty, "60"},
    FrameRateAndLabel {FrameRate::Max, "Max"},
};

constexpr auto thirtyFramesPerSecond = 30;
constexpr auto menuFramesPerSecond = 60;
constexpr auto noFrameRateCap = 0;

FrameRate stepFrameRate(FrameRate rate, int steps);

std::optional<FrameRate> parseFrameRateName(std::string_view name);

std::optional<FrameRate> readFrameRateFromEnvironment();

int getFrameRateCap(FrameRate rate, bool menusHaveInputAndCameraIsStill);

FrameRate replaceUnknownValuesWithDefaults(FrameRate rate);
} // namespace Cows
