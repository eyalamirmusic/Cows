#pragma once

#include "Terrain/Grass.h"

#include "Render/Lighting.h"
#include "Render/Mesh.h"

namespace Cows
{
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

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitScene(visitor);
        EACP_GPU_FIELDS(visitor, patchOffset)
    }

    Uniform<Float2> patchOffset;
};
} // namespace Cows
