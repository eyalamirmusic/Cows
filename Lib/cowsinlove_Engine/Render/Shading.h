#pragma once

#include "Render/Lighting.h"

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
} // namespace Cows::Shading
