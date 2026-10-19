#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
// How often the scene is drawn, as the player picked in Settings. Max is every
// refresh of the display; Sixty is 60 a second everywhere; Auto is every
// refresh in play and while the camera swings, and 60 while the menu or the
// editor has the input, where nothing moves fast enough to need more.
enum class FrameRate
{
    Auto,
    Sixty,
    Max
};

constexpr auto numFrameRates = 3;
constexpr auto menuFramesPerSecond = 60;

std::string getFrameRateLabel(FrameRate rate);

// COWS_FPS=auto|60|max: that rate whatever Settings says.
std::optional<FrameRate> parseFrameRateName(std::string_view name);
std::optional<FrameRate> readFrameRateFromEnvironment();

// The cap to hand GPUView::setMaxFps, 0 for none.
int getFrameRateCap(FrameRate rate, bool menusHaveInput);

// `rate`, or Auto when it names no rate.
FrameRate replaceUnknownValuesWithDefaults(FrameRate rate);
} // namespace Cows
