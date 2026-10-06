#pragma once

#include "Render/Common.h"

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

// The cow's wardrobe, picked to sit beside the original's colours.
constexpr std::uint32_t topHat = 0x26222c;
constexpr std::uint32_t cowboyHat = 0xa8743f;
constexpr std::uint32_t hatBand = 0x5a3a22;
constexpr std::uint32_t partyHat = 0x5fd7ff;
constexpr std::uint32_t pompom = 0xffff87;
constexpr std::uint32_t beanie = 0x5f87d7;
constexpr std::uint32_t beanieCuff = 0x4a6cb4;
constexpr std::uint32_t gold = 0xffd75f;
constexpr std::uint32_t frog = 0x87d700;
constexpr std::uint32_t frogLining = 0xd7ffaf;
constexpr std::uint32_t denim = 0x3a6ea5;
constexpr std::uint32_t denimSeam = 0x24476e;
constexpr std::uint32_t trafficCone = 0xff5f00;
constexpr std::uint32_t trafficConeBase = 0x2a2a2e;
constexpr std::uint32_t reflector = 0xf4f4f0;
constexpr std::uint32_t wizardHat = 0x3b2a8f;
constexpr std::uint32_t wizardStar = 0xfff3a8;

constexpr Graphics::Color display(std::uint32_t hex)
{
    return {(float) ((hex >> 16) & 0xffu) / 255.f,
            (float) ((hex >> 8) & 0xffu) / 255.f,
            (float) (hex & 0xffu) / 255.f};
}

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
