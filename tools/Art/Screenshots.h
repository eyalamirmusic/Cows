#pragma once

#include <cstdint>
#include <string>

namespace Cows
{
// Width and height in points.
struct Screen final
{
    float width = 960.f;
    float height = 540.f;
    float scale = 2.f;
    bool phone = false;
};

constexpr Screen desktopScreen {960.f, 540.f, 2.f, false};
constexpr Screen macScreen {1440.f, 900.f, 2.f, false};
constexpr Screen iPhone69 {440.f, 956.f, 3.f, true};
constexpr Screen iPhone65 {428.f, 926.f, 3.f, true};
constexpr Screen playPhone {360.f, 640.f, 3.f, true};
constexpr Screen playTablet7 {720.f, 1280.f, 2.f, true};
constexpr Screen playTablet10 {900.f, 1600.f, 2.4f, true};

void renderScreenshots(const std::string& directory, const Screen& screen);

void renderScreens(const std::string& directory, const std::string& only);

std::uint32_t openMeadowSeed();
} // namespace Cows
