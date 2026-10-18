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

constexpr auto frameRates = 3;
constexpr auto menuFramesPerSecond = 60;

std::string frameRateLabel(FrameRate rate);

// COWS_FPS=auto|60|max: that rate whatever Settings says.
std::optional<FrameRate> frameRateNamed(std::string_view name);
std::optional<FrameRate> frameRateOverride();

// The cap to hand GPUView::setMaxFps, 0 for none.
int maxFramesPerSecond(FrameRate rate, bool menusHaveInput);

// `rate`, or Auto when it names no rate.
FrameRate withKnownValue(FrameRate rate);
} // namespace Cows
