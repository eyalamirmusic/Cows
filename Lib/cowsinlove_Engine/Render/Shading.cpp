#include "Render/Shading.h"

namespace Cows::Shading
{
namespace
{
constexpr auto shadowTexel = 1.f / 2048.f;
constexpr auto shadowSpread = 3.f;
constexpr auto shadowBias = 0.0015f;
constexpr auto hazeStart = 12.f;
constexpr auto hazeDensity = 0.055f;
constexpr auto hazeFalloff = 0.09f;
constexpr auto shoulder = 0.8f;

constexpr float taps[][2] = {
    {-0.94f, -0.4f},
    {0.95f, -0.77f},
    {-0.09f, -0.93f},
    {0.34f, 0.29f},
    {-0.92f, 0.46f},
    {-0.82f, 0.15f},
    {-0.38f, 0.28f},
    {-0.2f, -0.49f},
    {0.97f, 0.22f},
    {0.44f, -0.77f},
    {0.54f, 0.64f},
    {-0.26f, 0.95f},
    {0.2f, -0.1f},
    {0.06f, 0.52f},
    {-0.62f, -0.75f},
    {0.74f, -0.26f},
};

constexpr auto tapCount = (int) (sizeof(taps) / sizeof(taps[0]));

Float hashOf(const Float& x, const Float& y, const Float& z)
{
    return fract(sin(x * 127.1f + y * 311.7f + z * 74.7f) * 43758.547f);
}
} // namespace

Float3 ambient(const LightingUniforms& light, const Float3& normal)
{
    return mix(light.groundAmbient, light.skyAmbient, normal.y() * 0.5f + 0.5f);
}

Float3 shade(const SceneUniforms& scene, const Surface& surface)
{
    auto toLight = normalize(scene.keyDirection);
    auto toEye = normalize(scene.eyePosition - surface.world);
    auto normal = surface.normal;

    auto wrap = surface.softness;
    auto facingLight = dot(normal, toLight);
    auto diffuse = max((facingLight + wrap) / (wrap + 1.f), 0.f) * surface.shadow;

    auto halfway = normalize(toLight + toEye);
    auto shininess = surface.gloss * 90.f + 12.f;
    auto specular = pow(max(dot(normal, halfway), 0.f), shininess) * surface.gloss
                    * surface.shadow * 0.55f;

    auto facingEye = max(dot(normal, toEye), 0.f);
    auto towardRim = max(dot(normal, normalize(scene.rimDirection)), 0.f);
    auto rim = pow(1.f - facingEye, 3.f) * (0.25f + 0.75f * towardRim)
               * (1.f - wrap * 0.6f);

    auto light = scene.keyColor * diffuse + ambient(scene, normal);

    return surface.albedo * light + scene.keyColor * specular
           + scene.rimColor * rim * 0.55f;
}

Float shadowAt(const SceneUniforms& scene, const Float3& world, const Float3& normal)
{
    auto lifted = world + normal * 0.03f;
    auto clip = scene.lightViewProjection * float4(lifted, 1.f);
    auto uv = float2(clip.x() * 0.5f + 0.5f, 0.5f - clip.y() * 0.5f);
    auto depth = clip.z() - shadowBias;

    auto litAt = [&](const float (&tap)[2])
    {
        auto offset = float2(uv.x() + tap[0] * shadowTexel * shadowSpread,
                             uv.y() + tap[1] * shadowTexel * shadowSpread);
        return step(depth, sample(scene.shadowMap, offset).x());
    };

    auto lit = litAt(taps[0]);

    for (auto index = 1; index < tapCount; ++index)
        lit = lit + litAt(taps[index]);

    lit = lit / (float) tapCount;

    auto inside = step(0.002f, uv.x()) * step(uv.x(), 0.998f) * step(0.002f, uv.y())
                  * step(uv.y(), 0.998f) * step(clip.z(), 0.995f);

    return 1.f - inside * (1.f - lit);
}

Float3 withHaze(const SceneUniforms& scene, const Float3& color, const Float3& world)
{
    auto distance = length(world - scene.eyePosition);
    auto thickness = (1.f - exp(-max(distance - hazeStart, 0.f) * hazeDensity))
                     * exp(-max(world.y(), 0.f) * hazeFalloff);

    return mix(color, scene.horizonColor, thickness);
}

Float3 toDisplay(const Float3& linear)
{
    auto over = max(linear - shoulder, 0.f);
    auto soft = shoulder + (1.f - shoulder) * (1.f - exp(-over / (1.f - shoulder)));
    return pow(min(linear, soft), 1.f / 2.2f);
}

Float hash(const Float3& cell)
{
    return hashOf(cell.x(), cell.y(), cell.z());
}

Float valueNoise(const Float3& position)
{
    auto cell = floor(position);
    auto local = fract(position);
    auto eased = local * local * (3.f - local * 2.f);

    auto x = cell.x();
    auto y = cell.y();
    auto z = cell.z();

    auto corner = [&](float dx, float dy, float dz)
    { return hashOf(x + dx, y + dy, z + dz); };

    auto bottomFront = mix(corner(0.f, 0.f, 0.f), corner(1.f, 0.f, 0.f), eased.x());
    auto bottomBack = mix(corner(0.f, 0.f, 1.f), corner(1.f, 0.f, 1.f), eased.x());
    auto topFront = mix(corner(0.f, 1.f, 0.f), corner(1.f, 1.f, 0.f), eased.x());
    auto topBack = mix(corner(0.f, 1.f, 1.f), corner(1.f, 1.f, 1.f), eased.x());

    auto bottom = mix(bottomFront, bottomBack, eased.z());
    auto top = mix(topFront, topBack, eased.z());

    return mix(bottom, top, eased.y());
}
} // namespace Cows::Shading
