#include "Title/TitleShader.h"
#include "Animation/Choreography.h"

#include "Render/Shading.h"

using namespace Maths;

namespace Cows
{
namespace
{
Float3 transformed(const Float4x4& matrix, const Float3& point)
{
    return (matrix * float4(point, 1.f)).xyz();
}
} // namespace

TitleShader::TitleShader()
{
    compile();
}

void TitleShader::define()
{
    auto position = vertexInput(&TitleVertex::position);
    auto normal = vertexInput(&TitleVertex::normal);
    auto letter = vertexInput(&TitleVertex::letter);

    auto angle = time * (twoPi / Choreography::titleWavePeriod) - letter * 0.55f;
    auto bob = float3(cos(angle) * 0.1f, sin(angle) * 0.18f, 0.f);

    auto world = transformed(placement, position + bob);
    auto worldNormal = (placement * float4(normal, 0.f)).xyz();

    setPosition(viewProjection * float4(world, 1.f));

    auto surface = varying(world);
    auto surfaceNormal = normalize(varying(worldNormal));

    auto albedo = titleColor * (0.85f + 0.25f * surfaceNormal.y());
    auto lit = Shading::shade(*this,
                              {albedo,
                               surfaceNormal,
                               surface,
                               constant(1.f),
                               constant(0.15f),
                               constant(1.f)});

    auto candy = lit + titleColor * 0.22f;

    setFragment(
        float4(Shading::toDisplay(Shading::withHaze(*this, candy, surface)), 1.f));
}
} // namespace Cows
