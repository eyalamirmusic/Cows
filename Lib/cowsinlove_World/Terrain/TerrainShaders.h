#pragma once

#include "Terrain/Grass.h"

#include "Render/Lighting.h"
#include "Render/Mesh.h"

namespace Cows
{
// Each contact is (x, z, strength): the soft dark patch right under a cow.
//
// With `cheapNoise` the meadow's two octaves come from a NoiseLattice
// (bound as `noise`) in two filtered fetches instead of sixteen sines.
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
    explicit GrassShader(int shadowTapsToUse = fullShadowTaps);

    void define() override;

    void reflectMembers(ShaderVisitor& visitor) override
    {
        visitScene(visitor);
        EACP_GPU_FIELDS(visitor, patchOffset)
    }

    Uniform<Float2> patchOffset;
};
} // namespace Cows
