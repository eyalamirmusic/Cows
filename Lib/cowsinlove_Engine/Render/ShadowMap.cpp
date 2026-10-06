#include "Render/ShadowMap.h"

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto halfExtent = 14.f;
constexpr auto lightDistance = 30.f;
constexpr Vec3 focusOffset {0.f, 1.2f, 0.5f};

TextureDescriptor describeTarget(int resolution)
{
    auto descriptor = TextureDescriptor {};
    descriptor.width = resolution;
    descriptor.height = resolution;
    descriptor.format = TextureFormat::R32Float;
    descriptor.renderTarget = true;
    descriptor.depth = true;
    return descriptor;
}
} // namespace

ShadowMap::ShadowMap(int resolution)
    : texture(Device::shared().makeTexture(describeTarget(resolution)))
{
}

Mat4 ShadowMap::lightViewProjection(const Vec3& towardLight,
                                    const Vec3& around) const
{
    auto focus = Vec3 {around.x, 0.f, around.z} + focusOffset;
    auto eye = focus + normalize(towardLight) * lightDistance;
    auto view = Mat4::lookAt(eye, focus, {0.f, 1.f, 0.f});
    auto projection = Mat4::orthographic(
        -halfExtent, halfExtent, -halfExtent, halfExtent, 1.f, lightDistance * 2.f);
    return projection * view;
}

RenderPipelineDescriptor ShadowMap::pipeline() const
{
    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = 1;
    descriptor.depth = true;
    descriptor.colorFormat = pixelFormatFor(TextureFormat::R32Float);
    descriptor.cullMode = CullMode::None;
    return descriptor;
}
} // namespace Cows
