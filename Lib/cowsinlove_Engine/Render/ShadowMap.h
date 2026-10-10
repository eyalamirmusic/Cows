#pragma once

#include "Render/Common.h"

namespace Cows
{
// The key light's view of the cows, as a depth per texel: an orthographic box
// around the player, since that is the only place a shadow falls that anyone
// can see.
struct ShadowMap final
{
    static constexpr auto fullResolution = 2048;

    explicit ShadowMap(int resolution = fullResolution);

    Maths::Mat4 lightViewProjection(const Maths::Vec3& towardLight,
                                    const Maths::Vec3& around) const;

    RenderPipelineDescriptor pipeline() const;

    Texture texture;
};
} // namespace Cows
