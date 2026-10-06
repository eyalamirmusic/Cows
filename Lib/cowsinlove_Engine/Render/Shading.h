#pragma once

#include "Render/Lighting.h"

#include <array>
#include <cstdint>

// The shading every surface in the scene shares, as shader-graph functions: they
// run inside a ShaderProgram's define().
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

// 1 where the key light reaches `world`, 0 where something stands in its way,
// softened over a few texels of the shadow map. Outside the map it reads no
// texel at all and answers 1.
Float shadowAt(LitProgram& scene, const Float3& world, const Float3& normal);

// How thick the haze is between the eye and `world`: 0 for none, rising with
// distance and thinning with height.
Float hazeAt(const Float3& eye, const Float3& world);

// `color` seen through hazeAt's haze, toward the horizon.
Float3
    withHaze(const SceneUniforms& scene, const Float3& color, const Float3& world);

// Linear light to the display: a soft shoulder instead of a hard clip, then
// the gamma curve.
Float3 toDisplay(const Float3& linear);

Float hash(const Float3& cell);
Float valueNoise(const Float3& position);

// valueNoise's kind of noise, hashed by multiplies and fract instead of a sine
// per corner: the same look in a different pattern, for a cheaper tier.
Float quickValueNoise(const Float3& position);

// valueNoise at a fixed z, read from NoiseLattice in one filtered fetch where
// valueNoise takes eight sines. `plane` picks the lattice channel (see
// latticePlanes). The same noise, give or take the hash's last bits and the
// filter's weights, repeating every NoiseLattice::size cells.
Float latticeNoise(const Uniform<Texture2D>& lattice,
                   const Float2& position,
                   int plane);
} // namespace Cows::Shading

namespace Cows
{
// The z each channel of NoiseLattice holds valueNoise at.
constexpr std::array<float, 4> latticePlanes {0.5f, 2.5f, 3.5f, 7.5f};

// valueNoise's lattice for the four latticePlanes, already mixed across z, as
// RGBA8 texels: one per cell, size cells on a side, tiling.
Vector<std::uint8_t> makeNoiseLattice();

struct NoiseLattice final
{
    NoiseLattice();

    static constexpr int size = 256;

    Texture texture;
};

// How a shader samples NoiseLattice::texture.
constexpr TextureSampling latticeSampling {TextureFilter::Linear,
                                           TextureAddressMode::Repeat};
} // namespace Cows
