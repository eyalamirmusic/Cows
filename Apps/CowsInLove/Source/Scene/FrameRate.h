#pragma once

#include <array>
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

struct FrameRateAndLabel final
{
    FrameRate rate;
    std::string_view label;
};

// Every frame rate in the Settings row's order, with the label it shows. A new
// rate is one line here and one in the enum.
constexpr auto frameRatesAndLabels = std::array {
    FrameRateAndLabel {FrameRate::Auto, "Auto"},
    FrameRateAndLabel {FrameRate::Sixty, "60"},
    FrameRateAndLabel {FrameRate::Max, "Max"},
};

constexpr auto menuFramesPerSecond = 60;
constexpr auto noFrameRateCap = 0;

// `rate` moved `steps` along the list, wrapping round: the Settings row's < >.
FrameRate stepFrameRate(FrameRate rate, int steps);

// Reads a label, in any case, back as its rate. Used for COWS_FPS=auto|60|max;
// settings.json goes through Miro's enumerator names instead.
std::optional<FrameRate> parseFrameRateName(std::string_view name);

// COWS_FPS: that rate whatever Settings says.
std::optional<FrameRate> readFrameRateFromEnvironment();

// The cap to hand GPUView::setMaxFps, noFrameRateCap for none.
int getFrameRateCap(FrameRate rate, bool menusHaveInputAndCameraIsStill);

// `rate`, or Auto when it is outside the enum (a hand-edited settings.json).
FrameRate replaceUnknownValuesWithDefaults(FrameRate rate);
} // namespace Cows
