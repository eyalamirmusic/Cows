#pragma once

#include "Render/Common.h"

namespace Cows
{
// All in linear space. The key light is a warm late-afternoon sun from over
// the viewer's right shoulder; the rim is the painted sun behind the cows,
// catching their edges.
struct Lighting final
{
    Maths::Vec3 keyDirection = Maths::normalize(Maths::Vec3 {0.8f, 0.95f, 0.3f});
    Maths::Vec3 keyColor {1.12f, 1.f, 0.84f};
    Maths::Vec3 skyAmbient {0.3f, 0.45f, 0.62f};
    Maths::Vec3 groundAmbient {0.1f, 0.16f, 0.05f};
    Maths::Vec3 rimDirection = Maths::normalize(Maths::Vec3 {0.45f, 0.35f, -0.8f});
    Maths::Vec3 rimColor {1.f, 0.82f, 0.5f};
    Maths::Vec3 zenithColor {0.12f, 0.4f, 0.86f};
    Maths::Vec3 skyColor {0.25f, 0.68f, 1.f};
    Maths::Vec3 horizonColor {0.5f, 0.78f, 1.f};
};

struct LightingUniforms
{
    void setLighting(const Lighting& lighting)
    {
        keyDirection = lighting.keyDirection;
        keyColor = lighting.keyColor;
        skyAmbient = lighting.skyAmbient;
        groundAmbient = lighting.groundAmbient;
        rimDirection = lighting.rimDirection;
        rimColor = lighting.rimColor;
        zenithColor = lighting.zenithColor;
        skyColor = lighting.skyColor;
        horizonColor = lighting.horizonColor;
    }

    void visitLighting(ShaderVisitor& visitor)
    {
        EACP_GPU_FIELDS(visitor,
                        keyDirection,
                        keyColor,
                        skyAmbient,
                        groundAmbient,
                        rimDirection,
                        rimColor,
                        zenithColor,
                        skyColor,
                        horizonColor);
    }

    Uniform<Float3> keyDirection;
    Uniform<Float3> keyColor;
    Uniform<Float3> skyAmbient;
    Uniform<Float3> groundAmbient;
    Uniform<Float3> rimDirection;
    Uniform<Float3> rimColor;
    Uniform<Float3> zenithColor;
    Uniform<Float3> skyColor;
    Uniform<Float3> horizonColor;
};

// The uniforms every shader that shades a surface in the scene carries: the
// light, the camera, and the shadow map with the transform into it.
struct SceneUniforms : LightingUniforms
{
    void visitScene(ShaderVisitor& visitor)
    {
        visitLighting(visitor);
        EACP_GPU_FIELDS(visitor,
                        viewProjection,
                        lightViewProjection,
                        eyePosition,
                        time,
                        shadowMap);
    }

    Uniform<Float4x4> viewProjection;
    Uniform<Float4x4> lightViewProjection;
    Uniform<Float3> eyePosition;
    Uniform<Float> time;
    Uniform<Texture2D> shadowMap;
};
} // namespace Cows
