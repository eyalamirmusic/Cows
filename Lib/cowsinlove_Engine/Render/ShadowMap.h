#pragma once

#include "Render/Common.h"

namespace Cows
{
// Orthographic around the player: the only place a shadow falls that anyone sees.
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
