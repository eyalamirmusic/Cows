#pragma once

#include "Render/Instances.h"
#include "Render/Lighting.h"
#include "Render/Mesh.h"

namespace Cows
{
struct CornerVertex final
{
    Maths::Vec2 position;
};

// A full-screen triangle that turns each pixel back into the view ray through
// it, so the gradient and the sun's glow sit still in the world as the camera
// turns.
struct SkyShader final
    : ShaderProgram
    , LightingUniforms
{
    SkyShader();

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitLighting(visitor);
        EACP_GPU_FIELDS(
            visitor, cameraForward, cameraRight, cameraUp, lensScale, towardSun)
    }

    Uniform<Float3> cameraForward;
    Uniform<Float3> cameraRight;
    Uniform<Float3> cameraUp;
    Uniform<Float2> lensScale;
    Uniform<Float3> towardSun;
};

// Every mesh in the scene but the grass and the title, drawn instanced: each
// instance brings its own transform and material.
struct SurfaceShader final
    : ShaderProgram
    , SceneUniforms
{
    SurfaceShader();

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override { visitScene(visitor); }
};

// Writes the key light's depth for everything that casts a shadow.
struct ShadowCasterShader final : ShaderProgram
{
    ShadowCasterShader();

    void define() override;

    Uniform<Float4x4> lightViewProjection;

    EACP_SHADER(lightViewProjection)
};

// Soft additive light around bright things: the sun, the eyes, the hearts.
// It fades into the mist as the surfaces do, by each glow's `mist`.
struct GlowShader final : ShaderProgram
{
    GlowShader();

    void define() override;

    Uniform<Float4x4> viewProjection;
    Uniform<Float3> cameraRight;
    Uniform<Float3> cameraUp;
    Uniform<Float3> eyePosition;

    EACP_SHADER(viewProjection, cameraRight, cameraUp, eyePosition)
};
} // namespace Cows
