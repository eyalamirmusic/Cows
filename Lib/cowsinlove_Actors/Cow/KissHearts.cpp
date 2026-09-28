#include "Cow/KissHearts.h"
#include "Animation/Choreography.h"
#include "Render/Palette.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto fewestHearts = 10;
constexpr auto mostHearts = 17;
constexpr auto spawnSpread = 0.9f;

// The lips meet a moment before the sway's closest point; the hearts go with
// the lips.
constexpr auto burstLead = 0.35f;

constexpr std::uint32_t heartColors[] = {0xff005f, 0xff3d80, 0xe8004f, 0xff6f9f};

float random(int kiss, int heart, int salt)
{
    auto value = (std::uint32_t) kiss * 747796405u
                 + (std::uint32_t) heart * 2891336453u
                 + (std::uint32_t) salt * 1181783497u + 12345u;
    value ^= value >> 16;
    value *= 2246822519u;
    value ^= value >> 13;
    value *= 3266489917u;
    value ^= value >> 16;
    return (float) (value & 0xffffffu) / (float) 0x1000000;
}

float smoothstep(float from, float to, float value)
{
    auto t = std::clamp((value - from) / (to - from), 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

float popIn(float t)
{
    t = std::clamp(t, 0.f, 1.f);
    auto overshoot = 1.9f;
    auto u = t - 1.f;
    return 1.f + u * u * ((overshoot + 1.f) * u + overshoot);
}

void addFlash(Vector<GlowInstance>& glows, float age, Vec3 at)
{
    auto strength = std::exp(-age * 2.5f);

    if (strength < 0.02f)
        return;

    auto color = Palette::linear(Palette::heart) * (1.4f * strength);
    glows.add(
        makeGlow(at + Vec3 {0.f, 0.f, 0.3f}, 1.1f + 0.8f * (1.f - strength), color));
}

void addHeart(SurfaceBatch& batch,
              Vector<GlowInstance>& glows,
              int kiss,
              int heart,
              float age,
              Vec3 origin)
{
    auto r = [&](int salt) { return random(kiss, heart, salt); };

    auto life = 1.3f + 0.35f * r(1);
    auto progress = age / life;

    if (age < 0.f || progress >= 1.f)
        return;

    auto rise = 2.f + 0.5f * r(2);
    auto wobbleRate = 2.5f + 2.f * r(3);
    auto wobblePhase = twoPi * r(4);
    auto settle = std::min(1.f, age * 2.f);

    auto x = (r(5) - 0.5f) * 0.6f + (r(6) - 0.5f) * 1.6f * age
             + std::sin(age * wobbleRate + wobblePhase) * 0.28f * settle;
    auto y = 0.2f + rise * age;
    auto z = 0.3f + (r(7) - 0.5f) * 0.7f + 0.25f * age
             + std::cos(age * wobbleRate * 0.7f + wobblePhase) * 0.12f;

    auto size = (0.12f + 0.08f * r(8)) * popIn(age / 0.3f)
                * (1.f - 0.35f * smoothstep(0.7f, 1.f, progress));
    auto alpha = 1.f - smoothstep(0.62f, 1.f, progress);

    auto spin = std::sin(age * 2.2f + wobblePhase) * 0.7f;
    auto tilt = std::sin(age * wobbleRate * 0.8f + wobblePhase) * 0.3f;
    auto position = origin + Vec3 {x, y, z};

    auto model = Mat4::translation(position) * Mat4::rotationY(spin)
                 * Mat4::rotationZ(tilt) * Mat4::scale(size);

    auto hex = heartColors[(int) (r(9) * 4.f) % 4];
    auto material = Material {Palette::linear(hex), alpha};
    material.emission = 0.35f;
    material.gloss = 1.f;

    batch.add(Shape::Heart, makeInstance(model, material));

    auto glow = Palette::linear(Palette::heart) * (0.3f * alpha);
    glows.add(makeGlow(position, size * 3.2f, glow));
}
} // namespace

void addKissHearts(SurfaceBatch& batch,
                   Vector<GlowInstance>& glows,
                   float seconds,
                   Vec3 kissPoint)
{
    auto kiss = Choreography::latestKiss(seconds + burstLead);

    if (kiss < 0)
        return;

    auto sinceKiss = seconds + burstLead - Choreography::kissTime(kiss);

    if (sinceKiss > Choreography::burstDuration + 1.f)
        return;

    addFlash(glows, sinceKiss, kissPoint);

    auto count =
        fewestHearts
        + (int) (random(kiss, 0, 0) * (float) (mostHearts - fewestHearts + 1));

    for (auto heart = 0; heart < count; ++heart)
    {
        auto spawn =
            spawnSpread * ((float) heart + random(kiss, heart, 10)) / (float) count;
        addHeart(batch, glows, kiss, heart, sinceKiss - spawn, kissPoint);
    }
}
} // namespace Cows
