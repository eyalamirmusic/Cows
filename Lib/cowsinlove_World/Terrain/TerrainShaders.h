#pragma once

#include "Terrain/Grass.h"

#include "Render/Lighting.h"
#include "Render/Mesh.h"

namespace Cows
{
// Contacts are (x, z, strength); cheapNoise: two `noise` fetches, not 16 sines.
struct GroundShader final : LitProgram
{
    explicit GroundShader(int shadowTapsToUse = fullShadowTaps,
                          bool cheapNoiseToUse = false);

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitScene(visitor);
        EACP_GPU_FIELDS(visitor, firstContact, secondContact, noise)
    }

    Uniform<Float3> firstContact;
    Uniform<Float3> secondContact;
    Uniform<Texture2D> noise;
    bool cheapNoise = false;
};

struct GrassShader final : LitProgram
{
    explicit GrassShader(int shadowTapsToUse = fullShadowTaps,
                         bool quickNoiseToUse = false);

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitScene(visitor);
        EACP_GPU_FIELDS(visitor, patchOffset)
    }

    Uniform<Float2> patchOffset;
    bool quickNoise = false;
};
} // namespace Cows
