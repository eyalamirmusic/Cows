#include "Render/Shaders.h"
#include "Render/Shading.h"

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto spotThreshold = 0.55f;

Float3 transformed(const Float4x4& matrix, const Float3& point)
{
    return (matrix * float4(point, 1.f)).xyz();
}
} // namespace

SkyShader::SkyShader()
{
    compile();
}

void SkyShader::define()
{
    auto position = vertexInput(&CornerVertex::position);
    setPosition(float4(position, 1.f, 1.f));

    auto screen = varying(position);
    auto across = cameraRight * (screen.x() * lensScale.x());
    auto along = cameraUp * (screen.y() * lensScale.y());
    auto direction = normalize(cameraForward + across + along);

    auto height = max(direction.y(), 0.f);
    auto low = mix(horizonColor, skyColor, smoothstep(0.f, 0.2f, height));
    auto sky = mix(low, zenithColor, smoothstep(0.15f, 1.f, height));

    auto sunward = max(dot(direction, normalize(towardSun)), 0.f);
    auto glow = pow(sunward, 10.f) * 0.08f + pow(sunward, 80.f) * 0.25f;
    auto warm = float3(constant(1.f), 0.86f, 0.55f);

    setFragment(float4(Shading::toDisplay(sky + warm * glow), 1.f));
}

SurfaceShader::SurfaceShader(int shadowTapsToUse)
{
    shadowTaps = shadowTapsToUse;
    compile();
}

void SurfaceShader::define()
{
    auto position = vertexInput(&Vertex::position);
    auto normal = vertexInput(&Vertex::normal);

    auto model = float4x4(instanceInput(&SurfaceInstance::model0, 1),
                          instanceInput(&SurfaceInstance::model1, 1),
                          instanceInput(&SurfaceInstance::model2, 1),
                          instanceInput(&SurfaceInstance::model3, 1));
    auto pattern = float4x4(instanceInput(&SurfaceInstance::pattern0, 1),
                            instanceInput(&SurfaceInstance::pattern1, 1),
                            instanceInput(&SurfaceInstance::pattern2, 1),
                            instanceInput(&SurfaceInstance::pattern3, 1));

    auto normal0 = instanceInput(&SurfaceInstance::normal0, 1).xyz();
    auto normal1 = instanceInput(&SurfaceInstance::normal1, 1).xyz();
    auto normal2 = instanceInput(&SurfaceInstance::normal2, 1).xyz();

    auto world = transformed(model, position);
    auto worldNormal =
        normal0 * normal.x() + normal1 * normal.y() + normal2 * normal.z();

    setPosition(viewProjection * float4(world, 1.f));

    auto surface = varying(world);
    auto surfaceNormal = normalize(varying(worldNormal));
    auto patternPosition = varying(transformed(pattern, position));
    auto color = varying(instanceInput(&SurfaceInstance::color, 1));
    auto material = varying(instanceInput(&SurfaceInstance::material, 1));
    auto mist = varying(instanceInput(&SurfaceInstance::mist, 1));

    auto blotch = Shading::valueNoise(patternPosition * 1.8f) * 0.72f
                  + Shading::valueNoise(patternPosition * 4.6f) * 0.28f;
    auto edge = fwidth(blotch) + 0.004f;
    auto spots = smoothstep(spotThreshold - edge, spotThreshold + edge, blotch)
                 * material.x();

    auto spotColor = float3(constant(0.012f), 0.011f, 0.014f);
    auto albedo = mix(color.xyz(), spotColor, spots);

    auto lit = Shading::shade(*this,
                              {albedo,
                               surfaceNormal,
                               surface,
                               Shading::shadowAt(*this, surface, surfaceNormal),
                               material.z(),
                               material.w() * (1.f - spots * 0.4f)});

    auto emission = material.y();
    auto glowing = lit * max(1.f - emission, 0.f) + color.xyz() * emission;
    auto haze = Shading::hazeAt(eyePosition, surface) * mist;
    auto shaded = mix(glowing, horizonColor, haze);

    setFragment(float4(Shading::toDisplay(shaded), color.w()));
}

ShadowCasterShader::ShadowCasterShader()
{
    compile();
}

void ShadowCasterShader::define()
{
    auto position = vertexInput(&Vertex::position);
    auto model = float4x4(instanceInput(&SurfaceInstance::model0, 1),
                          instanceInput(&SurfaceInstance::model1, 1),
                          instanceInput(&SurfaceInstance::model2, 1),
                          instanceInput(&SurfaceInstance::model3, 1));

    auto clip = lightViewProjection * (model * float4(position, 1.f));
    setPosition(clip);

    auto depth = varying(clip.z());
    setFragment(float4(depth, 0.f, 0.f, 1.f));
}

GlowShader::GlowShader()
{
    compile();
}

void GlowShader::define()
{
    auto corner = vertexInput(&CornerVertex::position);
    auto centerAndSize = instanceInput(&GlowInstance::centerAndSize, 1);
    auto color = instanceInput(&GlowInstance::color, 1);

    auto offset =
        (cameraRight * corner.x() + cameraUp * corner.y()) * centerAndSize.w();
    auto world = centerAndSize.xyz() + offset;

    setPosition(viewProjection * float4(world, 1.f));

    auto uv = varying(corner);
    auto tint = varying(color.xyz());
    auto mist = varying(color.w());
    auto surface = varying(world);

    auto distance = dot(uv, uv);
    auto falloff = exp(-distance * 4.5f) * (1.f - smoothstep(0.55f, 1.f, distance));
    auto clear = 1.f - Shading::hazeAt(eyePosition, surface) * mist;

    setFragment(float4(tint * (falloff * clear), falloff * clear));
}
} // namespace Cows
