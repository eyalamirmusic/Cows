#pragma once

#include "Title/TitleFont.h"

#include "Render/Lighting.h"
#include "Render/Mesh.h"

namespace Cows
{
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
} // namespace Cows
