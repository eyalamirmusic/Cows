#include "Terrain/TerrainShaders.h"

#include "Render/Shading.h"

using namespace Maths;

namespace Cows
{
GroundShader::GroundShader(int shadowTapsToUse, bool cheapNoiseToUse)
    : cheapNoise(cheapNoiseToUse)
{
    shadowTaps = shadowTapsToUse;
    noise.sampling = latticeSampling;
    compile();
}

void GroundShader::define()
{
    auto position = vertexInput(&Vertex::position);
    setPosition(viewProjection * float4(position, 1.f));

    auto world = varying(position);
    auto ground = world.xz();
    auto up = float3(constant(0.f), 1.f, 0.f);

    auto broad = cheapNoise ? Shading::latticeNoise(noise, ground * 0.22f, 0)
                            : Shading::valueNoise(float3(ground * 0.22f, 0.5f));
    auto fine = cheapNoise ? Shading::latticeNoise(noise, ground * 1.7f, 1)
                           : Shading::valueNoise(float3(ground * 1.7f, 2.5f));
    auto meadow = broad * 0.7f + fine * 0.3f;

    auto deep = float3(constant(0.004f), 0.1f, 0.006f);
    auto lush = float3(constant(0.07f), 0.3f, 0.04f);
    auto albedo = mix(deep, lush, meadow);

    auto contactOf = [&](const Float3& contact)
    {
        auto offset = (ground - contact.xy()) * float2(constant(0.62f), 1.1f);
        auto reach = length(offset);
        return 1.f - 0.55f * contact.z() * (1.f - smoothstep(0.1f, 1.25f, reach));
    };

    auto contact = contactOf(firstContact) * contactOf(secondContact);
    auto shadow = Shading::shadowAt(*this, world, up);
    auto lit =
        Shading::shade(*this,
                       {albedo, up, world, shadow, constant(0.f), constant(0.05f)})
        * contact;

    setFragment(
        float4(Shading::toDisplay(Shading::withHaze(*this, lit, world)), 1.f));
}

GrassShader::GrassShader(int shadowTapsToUse)
{
    shadowTaps = shadowTapsToUse;
    compile();
}

void GrassShader::define()
{
    auto shape = vertexInput(&BladeVertex::shape);
    auto placement = instanceInput(&BladeInstance::placement, 1);
    auto look = instanceInput(&BladeInstance::look, 1);
    auto form = instanceInput(&BladeInstance::form, 1);

    auto base = float3(
        placement.x() + patchOffset.x(), 0.f, placement.y() + patchOffset.y());
    auto along = shape.y();
    auto side = shape.x();
    auto twist = form.x();
    auto fold = form.y();
    auto seed = form.w();

    auto ground = float2(base.x(), base.z());
    auto patch = Shading::valueNoise(float3(ground * 0.07f, 3.5f)) * 0.65f
                 + Shading::valueNoise(float3(ground * 0.19f, 7.5f)) * 0.35f;
    auto dry = smoothstep(0.36f, 0.56f, patch);
    auto clump = Shading::valueNoise(float3(ground * 0.55f, 11.5f));
    auto thinned = step(seed, 1.f - dry * 0.45f - (1.f - clump) * 0.25f);

    auto height = (placement.w() + 0.16f * clump) * (1.1f - 0.35f * dry) * thinned;
    auto width = look.x() * thinned;

    auto facing = placement.z() + twist * along;
    auto across = float3(cos(facing), 0.f, sin(facing));
    auto face = float3(-across.z(), 0.f, across.x());

    auto gust =
        sin(time * 1.7f + base.x() * 0.45f + base.z() * 0.25f + look.y() * 0.4f)
            * 0.5f
        + sin(time * 0.6f + base.x() * 0.12f - base.z() * 0.08f) * 0.5f;
    auto bend = (look.w() * 0.7f + gust * 0.4f + 0.3f) * along * along * height;

    auto taper = (1.f - pow(along, 2.6f) * 0.8f)
                 * (0.75f + 0.25f * smoothstep(0.f, 0.2f, along));
    auto curl = fold * 0.7f;
    auto halfWidth = width * taper;

    auto world =
        base + across * (side * halfWidth)
        + face * ((1.f - side * side) * curl * halfWidth)
        + float3(bend * 0.94f, along * height * (1.f - 0.2f * bend), bend * 0.33f);

    setPosition(viewProjection * float4(world, 1.f));

    auto surface = varying(world);
    auto tip = varying(along);
    auto shade = varying(look.z());
    auto blade = varying(float4(side, curl * 2.f, form.z(), seed));
    auto faceDirection = varying(face);
    auto acrossDirection = varying(across);
    auto dryness = varying(dry);

    auto x = blade.x();
    auto toEye = normalize(eyePosition - surface);
    auto toLight = normalize(keyDirection);

    auto folded = faceDirection + acrossDirection * (x * blade.y());
    auto eyeSide = step(0.f, dot(folded, toEye)) * 2.f - 1.f;
    auto sided = folded * eyeSide;
    auto normal = normalize(sided + float3(constant(0.f), 1.6f, 0.f));

    auto distance = length(eyePosition - surface);
    auto detail = 1.f - smoothstep(4.f, 14.f, distance);

    auto streaks = Shading::valueNoise(
        float3(x * 3.5f + blade.w() * 97.f, tip * 2.5f, blade.w() * 31.f));
    auto mottle = Shading::valueNoise(surface * 7.f);
    auto rib = 1.f - smoothstep(0.f, 0.35f, abs(x));
    auto edge = smoothstep(0.6f, 1.f, abs(x));

    auto deep = float3(constant(0.003f), 0.085f, 0.004f);
    auto fresh = float3(constant(0.12f), 0.6f, 0.1f);
    auto golden = float3(constant(0.3f), 0.55f, 0.06f);
    auto straw = float3(constant(0.55f), 0.52f, 0.12f);

    auto hue = blade.z() - 0.5f;
    auto jitter = float3(1.f + hue * 0.25f, 1.f + hue * 0.05f, 1.f - hue * 0.4f);

    auto tipColor = mix(fresh, golden, shade * shade * 0.6f);
    tipColor = mix(tipColor, straw, dryness * (0.6f + 0.3f * shade)) * jitter;
    auto albedo = mix(deep, tipColor * (0.55f + 0.45f * shade), pow(tip, 1.3f))
                  * (0.5f + 0.5f * tip);

    auto grain = mix(constant(1.f),
                     (0.85f + 0.3f * streaks) * (0.92f + 0.16f * mottle)
                         * (1.f + 0.14f * rib) * (1.f - 0.08f * edge),
                     detail);
    auto rootShade = 0.7f + 0.3f * smoothstep(0.f, 0.4f, tip);
    albedo = albedo * grain * rootShade;

    auto shadow = Shading::shadowAt(*this, surface, normal);
    auto gloss = 0.1f + 0.2f * rib * detail;
    auto lit = Shading::shade(
        *this, {albedo, normal, surface, shadow, constant(0.5f), gloss});

    auto behind = max(-dot(sided, toLight), 0.f);
    auto against = pow(max(dot(-toLight, toEye), 0.f), 4.f);
    auto glow = (behind * 0.5f + against * 0.5f) * shadow * (0.3f + 0.7f * tip);
    auto pale =
        mix(tipColor, float3(constant(0.7f), 0.78f, 0.4f), 0.5f + 0.3f * dryness);
    auto sheen =
        pow(tip, 2.5f) * (0.2f + 0.8f * shade * shade) * (0.4f + 0.6f * shadow);
    lit = lit + pale * keyColor * (glow * 0.5f + sheen * 0.45f);

    setFragment(
        float4(Shading::toDisplay(Shading::withHaze(*this, lit, surface)), 1.f));
}
} // namespace Cows
