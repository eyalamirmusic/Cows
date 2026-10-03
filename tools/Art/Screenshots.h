#pragma once

#include <cstdint>
#include <string>

namespace Cows
{
// A screen the store wants screenshots of: its size in points and its scale.
struct Screen final
{
    float width = 960.f;
    float height = 540.f;
    float scale = 2.f;
    bool phone = false;
};

// Steam: 1920x1080.
constexpr Screen desktopScreen {960.f, 540.f, 2.f, false};
// Mac App Store: 2880x1800.
constexpr Screen macScreen {1440.f, 900.f, 2.f, false};
// App Store 6.9" iPhone: 1320x2868.
constexpr Screen iPhone69 {440.f, 956.f, 3.f, true};
// App Store 6.5" iPhone: 1284x2778.
constexpr Screen iPhone65 {428.f, 926.f, 3.f, true};
// Google Play phone, 9:16: 1080x1920.
constexpr Screen playPhone {360.f, 640.f, 3.f, true};
// Google Play 7" tablet, 9:16: 1440x2560.
constexpr Screen playTablet7 {720.f, 1280.f, 2.f, true};
// Google Play 10" tablet, 9:16: 2160x3840.
constexpr Screen playTablet10 {900.f, 1600.f, 2.4f, true};

// Renders the store screenshots through the game's own views (scene, footer
// and, on a phone, the touch controls) into `directory`.
void renderScreenshots(const std::string& directory, const Screen& screen);

// The first meadow seed where she hides in the open, so the pair stand on the
// grass rather than on a crate or a roof.
std::uint32_t openMeadowSeed();
} // namespace Cows
