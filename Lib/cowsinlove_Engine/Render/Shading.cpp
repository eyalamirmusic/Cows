#include "Render/Shading.h"

#include <array>
#include <cmath>

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

constexpr auto taps = std::to_array<std::array<float, 2>>({
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
});

constexpr auto fewTaps = std::to_array<std::array<float, 2>>({
    {-0.55f, -0.2f},
    {0.2f, -0.55f},
    {0.55f, 0.2f},
    {-0.2f, 0.55f},
});

// 43758.5, not the textbook 43758.5453: the look was tuned while eacp printed
// shader literals to six digits, so this is the constant the GPU always saw.
Float hashOf(const Float& x, const Float& y, const Float& z)
{
    return fract(sin(x * 127.1f + y * 311.7f + z * 74.7f) * 43758.5f);
}
// A hash with no sine (Dave Hoskins' hash13), cheaper where sines are dear.
Float quickHashOf(const Float& x, const Float& y, const Float& z)
{
    auto p = fract(float3(x, y, z) * 0.1031f);
    p = p + dot(p, float3(p.y(), p.z(), p.x()) + 33.33f);
    return fract((p.x() + p.y()) * p.z());
}

template <typename Hash>
Float valueNoiseHashed(const Float3& position, Hash hashOfCorner)
{
    auto cell = floor(position);
    auto local = fract(position);
    auto eased = local * local * (3.f - local * 2.f);

    auto x = cell.x();
    auto y = cell.y();
    auto z = cell.z();

    auto corner = [&](float dx, float dy, float dz)
    { return hashOfCorner(x + dx, y + dy, z + dz); };

    auto bottomFront = mix(corner(0.f, 0.f, 0.f), corner(1.f, 0.f, 0.f), eased.x());
    auto bottomBack = mix(corner(0.f, 0.f, 1.f), corner(1.f, 0.f, 1.f), eased.x());
    auto topFront = mix(corner(0.f, 1.f, 0.f), corner(1.f, 1.f, 0.f), eased.x());
    auto topBack = mix(corner(0.f, 1.f, 1.f), corner(1.f, 1.f, 1.f), eased.x());

    auto bottom = mix(bottomFront, bottomBack, eased.z());
    auto top = mix(topFront, topBack, eased.z());

    return mix(bottom, top, eased.y());
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

Float shadowAt(LitProgram& scene, const Float3& world, const Float3& normal)
{
    auto lifted = world + normal * 0.03f;
    auto clip = scene.lightViewProjection * float4(lifted, 1.f);
    auto uv = float2(clip.x() * 0.5f + 0.5f, 0.5f - clip.y() * 0.5f);
    auto depth = clip.z() - shadowBias;

    auto inside = step(0.002f, uv.x()) * step(uv.x(), 0.998f) * step(0.002f, uv.y())
                  * step(uv.y(), 0.998f) * step(clip.z(), 0.995f);

    auto shadow = scene.var(1.f);

    scene.ifThen(
        inside > 0.5f,
        [&]
        {
            auto litAt = [&](const std::array<float, 2>& tap)
            {
                auto offset = float2(uv.x() + tap[0] * shadowTexel * shadowSpread,
                                     uv.y() + tap[1] * shadowTexel * shadowSpread);
                return step(depth, sample(scene.shadowMap, offset, 0.f).x());
            };

            auto averageOf = [&](const auto& pattern)
            {
                auto lit = litAt(pattern[0]);

                for (auto index = 1; index < (int) pattern.size(); ++index)
                    lit = lit + litAt(pattern[(size_t) index]);

                return lit / (float) pattern.size();
            };

            shadow = scene.shadowTaps < LitProgram::fullShadowTaps
                         ? averageOf(fewTaps)
                         : averageOf(taps);
        });

    return shadow.get();
}

Float hazeAt(const Float3& eye, const Float3& world)
{
    auto distance = length(world - eye);
    return (1.f - exp(-max(distance - hazeStart, 0.f) * hazeDensity))
           * exp(-max(world.y(), 0.f) * hazeFalloff);
}

Float3 withHaze(const SceneUniforms& scene, const Float3& color, const Float3& world)
{
    return mix(color, scene.horizonColor, hazeAt(scene.eyePosition, world));
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
    return valueNoiseHashed(position, hashOf);
}

Float quickValueNoise(const Float3& position)
{
    return valueNoiseHashed(position, quickHashOf);
}

Float latticeNoise(const Uniform<Texture2D>& lattice,
                   const Float2& position,
                   int plane)
{
    auto cell = floor(position);
    auto local = fract(position);
    auto eased = local * local * (3.f - local * 2.f);
    auto at = (cell + eased + 0.5f) * (1.f / (float) NoiseLattice::size);
    auto texel = sample(lattice, at, 0.f);

    return plane == 0   ? texel.x()
           : plane == 1 ? texel.y()
           : plane == 2 ? texel.z()
                        : texel.w();
}
} // namespace Cows::Shading

namespace Cows
{
namespace
{
float hashOnCpu(float x, float y, float z)
{
    auto value = std::sin(x * 127.1f + y * 311.7f + z * 74.7f) * 43758.5f;
    return value - std::floor(value);
}

float latticeValue(int x, int y, float z)
{
    auto cell = std::floor(z);
    auto local = z - cell;
    auto eased = local * local * (3.f - local * 2.f);
    auto below = hashOnCpu((float) x, (float) y, cell);
    auto above = hashOnCpu((float) x, (float) y, cell + 1.f);
    return below + (above - below) * eased;
}
} // namespace

Vector<std::uint8_t> makeNoiseLattice()
{
    constexpr auto size = NoiseLattice::size;
    auto texels = Vector<std::uint8_t> {};
    texels.resize(size * size * 4);

    for (auto y = 0; y < size; ++y)
        for (auto x = 0; x < size; ++x)
            for (auto plane = 0; plane < 4; ++plane)
            {
                auto value = latticeValue(x, y, latticePlanes[(size_t) plane]);
                texels[(size_t) ((y * size + x) * 4 + plane)] =
                    (std::uint8_t) std::lround(value * 255.f);
            }

    return texels;
}

namespace
{
Texture makeLatticeTexture()
{
    auto descriptor = TextureDescriptor {};
    descriptor.width = NoiseLattice::size;
    descriptor.height = NoiseLattice::size;
    descriptor.format = TextureFormat::RGBA8Unorm;

    auto texels = makeNoiseLattice();
    return Device::shared().makeTexture(descriptor, texels.data());
}
} // namespace

NoiseLattice::NoiseLattice()
    : texture(makeLatticeTexture())
{
}
} // namespace Cows
