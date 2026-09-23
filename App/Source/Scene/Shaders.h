#pragma once

#include "Grass.h"
#include "Instances.h"
#include "Lighting.h"
#include "Mesh.h"
#include "TitleFont.h"

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

// Each contact is (x, z, strength): the soft dark patch right under a cow.
struct GroundShader final
    : ShaderProgram
    , SceneUniforms
{
    GroundShader();

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitScene(visitor);
        EACP_GPU_FIELDS(visitor, firstContact, secondContact)
    }

    Uniform<Float3> firstContact;
    Uniform<Float3> secondContact;
};

struct GrassShader final
    : ShaderProgram
    , SceneUniforms
{
    GrassShader();

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override { visitScene(visitor); }
};

// The title, each letter riding its own little loop, a wave travelling
// through the word.
struct TitleShader final
    : ShaderProgram
    , SceneUniforms
{
    TitleShader();

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitScene(visitor);
        EACP_GPU_FIELDS(visitor, placement, titleColor)
    }

    Uniform<Float4x4> placement;
    Uniform<Float3> titleColor;
};

// Soft additive light around bright things: the sun, the eyes, the hearts.
struct GlowShader final : ShaderProgram
{
    GlowShader();

    void define() override;

    Uniform<Float4x4> viewProjection;
    Uniform<Float3> cameraRight;
    Uniform<Float3> cameraUp;

    EACP_SHADER(viewProjection, cameraRight, cameraUp)
};
} // namespace Cows
