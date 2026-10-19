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

// Without spotted it skips the patchwork noise, whatever the material says.
struct SurfaceShader final : LitProgram
{
    explicit SurfaceShader(int shadowTapsToUse = fullShadowTaps,
                           bool spottedToUse = true);

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override { visitScene(visitor); }

    bool spotted = true;
};

struct ShadowCasterShader final : ShaderProgram
{
    ShadowCasterShader();

    void define() override;

    Uniform<Float4x4> lightViewProjection;

    EACP_SHADER(lightViewProjection)
};

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
