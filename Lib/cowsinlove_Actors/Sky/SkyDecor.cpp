#include "Sky/SkyDecor.h"
#include "Animation/Choreography.h"
#include "Render/Palette.h"

#include <array>
#include <cmath>

using namespace Maths;

namespace Cows::SkyDecor
{
namespace
{
constexpr auto sunRadius = 2.3f;
constexpr auto rayCount = 8;
constexpr auto raySpin = 0.07f;

constexpr auto cloudDepth = -50.f;
constexpr auto cloudDrift = 0.8f;
constexpr auto cloudSpan = 96.f;

struct Puff final
{
    Vec3 offset;
    float radius = 1.f;
};

constexpr auto puffs = std::to_array<Puff>({
    {{0.f, 0.f, 0.f}, 1.05f},
    {{1.15f, -0.25f, 0.1f}, 0.82f},
    {{-1.1f, -0.3f, 0.f}, 0.78f},
    {{0.5f, 0.6f, -0.1f}, 0.82f},
    {{-0.55f, 0.45f, 0.15f}, 0.72f},
    {{1.95f, -0.5f, 0.f}, 0.52f},
    {{-1.9f, -0.55f, 0.f}, 0.5f},
    {{0.1f, -0.45f, 0.35f}, 0.75f},
    {{1.2f, 0.3f, -0.3f}, 0.6f},
});

struct Cloud final
{
    float start = 0.f;
    float height = 0.f;
    float scale = 1.f;
};

constexpr auto clouds = std::to_array<Cloud>({
    {-30.f, 10.5f, 2.6f},
    {14.f, 14.5f, 2.2f},
});

float wrapped(float x)
{
    auto half = cloudSpan * 0.5f;
    auto shifted = std::fmod(x + half, cloudSpan);
    return (shifted < 0.f ? shifted + cloudSpan : shifted) - half;
}
} // namespace

void addSun(SurfaceBatch& batch,
            Vector<GlowInstance>& glows,
            float seconds,
            Vec3 origin)
{
    auto center = origin + sunCenter;

    auto pulse = Choreography::sunPulse(seconds);
    auto yellow = Palette::linear(Palette::sun);

    auto core = Material {yellow};
    core.emission = 1.25f + 0.35f * pulse;
    core.gloss = 0.f;
    core.mist = 0.f;

    batch.add(
        Shape::Sphere,
        makeInstance(Mat4::translation(center) * Mat4::scale(sunRadius), core));

    auto ray = core;
    ray.emission = 1.1f + 0.4f * pulse;

    for (auto index = 0; index < rayCount; ++index)
    {
        auto angle = twoPi * (float) index / (float) rayCount + raySpin * seconds;
        auto cardinal = index % 2 == 0;
        auto length = (cardinal ? 2.3f : 1.6f) * (0.9f + 0.18f * pulse);
        auto model = Mat4::translation(center) * Mat4::rotationZ(angle)
                     * Mat4::translation({0.f, sunRadius + 0.75f, 0.f})
                     * Mat4::scale({2.2f, length, 2.2f});

        batch.add(Shape::Capsule, makeInstance(model, ray));
    }

    auto halo = Vec3 {1.f, 0.85f, 0.35f} * (0.28f + 0.14f * pulse);
    auto inner = Vec3 {1.f, 0.9f, 0.2f} * (0.25f + 0.15f * pulse);

    glows.add(
        makeGlow(center + Vec3 {0.f, 0.f, 2.f}, 15.f + 1.5f * pulse, halo, 0.f));
    glows.add(makeGlow(center + Vec3 {0.f, 0.f, 2.5f}, 6.5f, inner, 0.f));
}

void addClouds(SurfaceBatch& batch, float seconds, Vec3 origin)
{
    auto material = Material {Palette::linear(Palette::cloud)};
    material.softness = 1.f;
    material.gloss = 0.f;

    for (const auto& cloud: clouds)
    {
        auto x = wrapped(cloud.start + cloudDrift * seconds);
        auto at = origin + Vec3 {x, cloud.height, cloudDepth};

        for (const auto& puff: puffs)
        {
            auto radius = puff.radius * cloud.scale;
            auto model = Mat4::translation(at + puff.offset * cloud.scale)
                         * Mat4::scale({radius, radius * 0.86f, radius * 0.8f});

            batch.add(Shape::Sphere, makeInstance(model, material));
        }
    }
}

void addHills(SurfaceBatch& batch, Vec3 origin)
{
    struct Hill final
    {
        Vec3 center;
        Vec3 size;
        std::uint32_t color;
    };

    constexpr auto hills = std::to_array<Hill>({
        {{-46.f, -2.f, -78.f}, {34.f, 9.f, 16.f}, 0x2f8f3a},
        {{-8.f, -3.5f, -96.f}, {40.f, 10.f, 18.f}, 0x3a9a40},
        {{34.f, -2.5f, -84.f}, {36.f, 9.5f, 16.f}, 0x2c8a36},
        {{70.f, -2.f, -70.f}, {30.f, 8.f, 14.f}, 0x35933c},
    });

    for (const auto& hill: hills)
    {
        auto material = Material {Palette::linear(hill.color)};
        material.softness = 0.4f;
        material.gloss = 0.f;

        batch.add(Shape::Sphere,
                  makeInstance(Mat4::translation(origin + hill.center)
                                   * Mat4::scale(hill.size),
                               material));
    }
}
} // namespace Cows::SkyDecor
