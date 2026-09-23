#pragma once

#include "Common.h"

namespace Cows
{
// The key light's view of the cows, as a depth per texel: an orthographic box
// around the middle of the meadow, since that is the only place a shadow
// falls that anyone can see.
struct ShadowMap final
{
    ShadowMap();

    Maths::Mat4 lightViewProjection(const Maths::Vec3& towardLight) const;

    RenderPipelineDescriptor pipeline() const;

    Texture texture;
};
} // namespace Cows
