#pragma once

#include "Render/Lighting.h"

#include <array>
#include <cstdint>

namespace Cows::Shading
{
struct Surface final
{
    Float3 albedo;
    Float3 normal;
    Float3 world;
    Float shadow;
    Float softness;
    Float gloss;
};

Float3 ambient(const LightingUniforms& light, const Float3& normal);

Float3 shade(const SceneUniforms& scene, const Surface& surface);

// 1 lit, 0 shadowed, 1 outside the map.
Float shadowAt(LitProgram& scene, const Float3& world, const Float3& normal);

Float hazeAt(const Float3& eye, const Float3& world);

Float3
    withHaze(const SceneUniforms& scene, const Float3& color, const Float3& world);

// A soft shoulder instead of a hard clip, then gamma.
Float3 toDisplay(const Float3& linear);

Float valueNoise(const Float3& position);

// Hashed by multiplies and fract instead of sines: cheaper, a different pattern.
Float quickValueNoise(const Float3& position);

// valueNoise at latticePlanes[plane] in one fetch, tiling every NoiseLattice::size.
Float latticeNoise(const Uniform<Texture2D>& lattice,
                   const Float2& position,
                   int plane);
} // namespace Cows::Shading

namespace Cows
{
// The z each NoiseLattice channel holds valueNoise at.
constexpr std::array<float, 4> latticePlanes {0.5f, 2.5f, 3.5f, 7.5f};

// RGBA8, one texel per cell, already mixed across z.
Vector<std::uint8_t> makeNoiseLattice();

struct NoiseLattice final
{
    NoiseLattice();

    static constexpr int size = 256;

    Texture texture;
};

constexpr TextureSampling latticeSampling {TextureFilter::Linear,
                                           TextureAddressMode::Repeat};
} // namespace Cows
