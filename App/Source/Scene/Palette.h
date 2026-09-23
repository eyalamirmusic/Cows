#pragma once

#include "Common.h"

#include <cmath>
#include <cstdint>

namespace Cows::Palette
{
// The terminal original's colours, authored in display (sRGB) space. Lighting
// runs in linear space, so every one of them goes through linear() first.
constexpr std::uint32_t sky = 0x87d7ff;
constexpr std::uint32_t title = 0xff00d7;
constexpr std::uint32_t sun = 0xffff00;
constexpr std::uint32_t cloud = 0xffffff;
constexpr std::uint32_t ground = 0x005f00;
constexpr std::uint32_t grass = 0x87ff87;
constexpr std::uint32_t heart = 0xff005f;

constexpr std::uint32_t hide = 0xfbf7ef;
constexpr std::uint32_t spot = 0x1c1a20;
constexpr std::uint32_t muzzle = 0xf6aab4;
constexpr std::uint32_t nostril = 0x7a3440;
constexpr std::uint32_t hoof = 0x3a2f2c;
constexpr std::uint32_t horn = 0xf3e2bd;
constexpr std::uint32_t innerEar = 0xf2a0ac;

inline float toLinear(float channel)
{
    return std::pow(channel, 2.2f);
}

inline Maths::Vec3 linear(std::uint32_t hex)
{
    auto channel = [hex](int shift)
    { return toLinear((float) ((hex >> shift) & 0xffu) / 255.f); };

    return {channel(16), channel(8), channel(0)};
}
} // namespace Cows::Palette
