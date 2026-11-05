#include "Refinements.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows::Pitch
{
namespace
{
constexpr auto near = 0.02f;
constexpr Vec3 eyeAt {1.62f, 1.82f, 0.2f};
constexpr Vec3 earAt {1.28f, 1.84f, 0.37f};
constexpr Vec3 innerEarAt {1.32f, 1.84f, 0.38f};
constexpr Vec3 nostrilAt {1.98f, 1.57f, 0.1f};
constexpr Vec3 headAt {1.42f, 1.72f, 0.f};
constexpr auto legTop = 1.05f;

constexpr auto headShot = Shot {{1.5f, 1.8f, 0.f}, 0.9f, 0.1f, 2.2f};
constexpr auto faceShot = Shot {{1.7f, 1.7f, 0.f}, 1.25f, 0.08f, 1.6f};
constexpr auto tailShot = Shot {{-1.1f, 1.f, 0.f}, -1.1f, 0.1f, 2.4f};
constexpr auto legShot = Shot {{0.f, 0.45f, 0.f}, 0.5f, 0.1f, 3.4f};
constexpr auto bodyShot = Shot {{0.3f, 1.2f, 0.f}, 0.8f, 0.15f, 5.f};
constexpr auto underShot = Shot {{-0.35f, 0.6f, 0.f}, 0.35f, -0.05f, 2.2f};

Material plain(std::uint32_t hex, float gloss = 0.3f)
{
    auto material = Material {Palette::linear(hex)};
    material.gloss = gloss;
    return material;
}

Mat4 place(Vec3 position, Vec3 size, const Mat4& rotation = {})
{
    return Mat4::translation(position) * rotation * Mat4::scale(size);
}

Vec3 positionOf(const CowPart& part)
{
    auto at = part.transform.column(3);
    return {at.x, at.y, at.z};
}

bool at(const CowPart& part, Vec3 point)
{
    return distance(positionOf(part), point) < near;
}

Vec3 mirrored(Vec3 point, float side)
{
    return {point.x, point.y, point.z * side};
}

// A rotation taking +y to `direction`.
Mat4 pointing(Vec3 direction)
{
    auto up = normalize(direction);
    auto reference =
        std::abs(up.x) < 0.9f ? Vec3 {1.f, 0.f, 0.f} : Vec3 {0.f, 0.f, 1.f};
    auto across = normalize(cross(up, reference));
    auto third = cross(across, up);

    auto rotation = Mat4 {};
    rotation.setColumn(0, {across.x, across.y, across.z, 0.f});
    rotation.setColumn(1, {up.x, up.y, up.z, 0.f});
    rotation.setColumn(2, {third.x, third.y, third.z, 0.f});
    return rotation;
}

void removeWhere(Vector<CowPart>& parts,
                 const std::function<bool(const CowPart&)>& doomed)
{
    auto kept = Vector<CowPart> {};

    for (const auto& part: parts)
        if (!doomed(part))
            kept.add(part);

    parts = kept;
}

// Bigger ears held out flatter and turned forward, so the pink inside shows.
void ears(Design& design)
{
    removeWhere(design.parts,
                [](const CowPart& part)
                {
                    for (auto side: {-1.f, 1.f})
                        if (at(part, mirrored(earAt, side))
                            || at(part, mirrored(innerEarAt, side)))
                            return true;
                    return false;
                });

    auto hide = Material {Palette::linear(Palette::hide)};
    hide.spots = 1.f;
    hide.gloss = 0.25f;

    for (auto side: {-1.f, 1.f})
    {
        auto out = Mat4::rotationX(0.22f * side) * Mat4::rotationY(-0.55f * side);
        design.parts.add(
            {Shape::Sphere,
             Bone::Head,
             place({1.27f, 1.86f, 0.4f * side}, {0.07f, 0.12f, 0.24f}, out),
             hide});
        design.parts.add(
            {Shape::Sphere,
             Bone::Head,
             place({1.3f, 1.86f, 0.41f * side}, {0.04f, 0.09f, 0.19f}, out),
             plain(Palette::innerEar)});
    }
}

// Three lashes off the outer top of each heart eye.
void lashes(Design& design)
{
    auto lash = plain(Palette::spot, 0.2f);

    for (auto side: {-1.f, 1.f})
    {
        auto facing = std::atan2(0.78f, 0.63f * side);
        auto normal = Vec3 {std::sin(facing), 0.f, std::cos(facing)};
        auto outward = Vec3 {-0.63f, 0.f, 0.78f * side};
        auto up = Vec3 {0.f, 1.f, 0.f};
        auto eye = mirrored(eyeAt, side);

        for (auto step = 0; step < 3; ++step)
        {
            auto turn = 0.35f + 0.4f * (float) step;
            auto direction = up * std::cos(turn) + outward * std::sin(turn);
            auto base = eye + up * 0.085f + outward * (0.05f + 0.03f * (float) step)
                        - up * (0.025f * (float) step) + normal * 0.01f;
            design.parts.add({Shape::Capsule,
                              Bone::Head,
                              Mat4::translation(base) * pointing(direction)
                                  * Mat4::scale({0.11f, 0.075f, 0.11f}),
                              lash});
        }
    }
}

// Nostrils as slanted commas, a little larger, and a smile under the muzzle.
void muzzle(Design& design)
{
    removeWhere(design.parts,
                [](const CowPart& part)
                {
                    return at(part, mirrored(nostrilAt, 1.f))
                           || at(part, mirrored(nostrilAt, -1.f));
                });

    auto dark = plain(Palette::nostril, 0.4f);

    for (auto side: {-1.f, 1.f})
        design.parts.add({Shape::Sphere,
                          Bone::Head,
                          place({1.975f, 1.575f, 0.105f * side},
                                {0.032f, 0.07f, 0.045f},
                                Mat4::rotationX(-0.45f * side)),
                          dark});
}

// A fuller tuft: three dark puffs where one ball was.
void tailTuft(Design& design)
{
    auto tuft = plain(Palette::spot, 0.15f);
    tuft.softness = 0.4f;

    auto tip = Mat4 {};

    for (const auto& part: design.parts)
        if (part.bone == Bone::Tail && part.shape == Shape::Sphere)
            tip = part.transform
                  * Mat4::scale({1.f / 0.09f, 1.f / 0.15f, 1.f / 0.09f});

    removeWhere(design.parts,
                [](const CowPart& part)
                { return part.bone == Bone::Tail && part.shape == Shape::Sphere; });

    design.parts.add(
        {Shape::Sphere, Bone::Tail, tip * place({}, {0.12f, 0.19f, 0.12f}), tuft});

    for (auto side: {-1.f, 1.f})
        design.parts.add({Shape::Sphere,
                          Bone::Tail,
                          tip
                              * place({0.05f * side, 0.06f, 0.03f * side},
                                      {0.08f, 0.13f, 0.08f},
                                      Mat4::rotationZ(0.4f * side)),
                          tuft});
}

// Cloven hooves: two toes side by side instead of one round pad.
void hooves(Design& design)
{
    removeWhere(design.parts,
                [](const CowPart& part)
                {
                    return part.shape == Shape::Capsule && part.bone == Bone::Body
                           && std::abs(positionOf(part).y) < 0.001f;
                });

    auto hoof = plain(Palette::hoof, 0.5f);

    for (const auto& leg: design.legs)
    {
        auto foot = leg.column(3);

        for (auto side: {-1.f, 1.f})
            design.parts.add({Shape::Capsule,
                              Bone::Body,
                              place({foot.x + 0.01f, 0.f, foot.z + 0.1f * side},
                                    {1.45f, 0.22f, 0.72f}),
                              hoof});
    }
}

// Feet set a little wider than the hips and the pairs a touch further apart,
// for a planted, sturdier stance; the pants follow legPlacements.
void stance(Design& design)
{
    for (auto& leg: design.legs)
    {
        auto foot = leg.column(3);
        auto top = Vec3 {foot.x, legTop, foot.z};
        auto splay = Mat4::rotationX(0.07f * (foot.z > 0.f ? 1.f : -1.f))
                     * Mat4::rotationZ(0.05f * (foot.x > 0.f ? -1.f : 1.f));
        leg = Mat4::translation(top) * splay * Mat4::translation(-top) * leg;
    }

    auto index = 0;
    auto hoofIndex = 0;
    for (auto& part: design.parts)
    {
        if (part.shape != Shape::Capsule || part.bone != Bone::Body)
            continue;

        auto y = positionOf(part).y;

        if (std::abs(y - 0.1f) < 0.001f && index < 4)
            part.transform = design.legs[(size_t) index++];
        else if (std::abs(y) < 0.001f && hoofIndex < 4)
        {
            auto foot = design.legs[(size_t) hoofIndex++].column(3);
            auto size = Vec3 {1.5f, hoofTop, 1.5f};
            part.transform = place({foot.x, 0.f, foot.z}, size);
        }
    }
}

// Softer light on the hide: wrapped round the terminator like the clouds, a
// little more sheen.
void softness(Design& design)
{
    for (auto& part: design.parts)
        if (part.material.spots > 0.f)
        {
            part.material.softness = 0.3f;
            part.material.gloss = 0.35f;
        }
}

// Fewer, bigger patches on the body; the face keeps today's small ones.
void spots(Design& design)
{
    for (auto& part: design.parts)
        if (part.material.spots > 0.f && part.bone == Bone::Body)
            part.spotScale = 0.75f;
}

// A rounder, fuller udder with the teats splayed.
void udder(Design& design)
{
    auto pink = plain(Palette::muzzle, 0.5f);

    removeWhere(design.parts,
                [](const CowPart& part)
                {
                    auto at = positionOf(part);
                    return part.bone == Bone::Body && at.y < 0.7f && at.y > 0.4f
                           && at.x < -0.2f && at.x > -0.55f;
                });

    design.parts.add({Shape::Sphere,
                      Bone::Body,
                      place({-0.38f, 0.66f, 0.f}, {0.25f, 0.17f, 0.22f}),
                      pink});

    for (auto x: {-0.47f, -0.29f})
        for (auto z: {-0.09f, 0.09f})
            design.parts.add(
                {Shape::Capsule,
                 Bone::Body,
                 place({x, 0.37f, z},
                       {0.28f, 0.13f, 0.28f},
                       Mat4::rotationX(z > 0.f ? -0.25f : 0.25f)
                           * Mat4::rotationZ(x < -0.38f ? -0.2f : 0.2f)),
                 pink});
}

// Pink blush discs on the cheeks, under the eyes.
void blush(Design& design)
{
    auto pink = plain(0xff8fae, 0.2f);
    pink.emission = 0.1f;

    for (auto side: {-1.f, 1.f})
        design.parts.add({Shape::Sphere,
                          Bone::Head,
                          place({headAt.x + 0.17f, 1.65f, 0.27f * side},
                                {0.02f, 0.06f, 0.08f},
                                Mat4::rotationY(-0.95f * side)),
                          pink});
}
} // namespace

const Vector<Refinement>& refinements()
{
    static const auto all = Vector<Refinement> {
        {"ears", headShot, ears},
        {"lashes", faceShot, lashes},
        {"muzzle", faceShot, muzzle},
        {"blush", faceShot, blush},
        {"tail-tuft", tailShot, tailTuft},
        {"hooves", legShot, hooves},
        {"stance", legShot, stance},
        {"udder", underShot, udder},
        {"spots", bodyShot, spots},
        {"softness", bodyShot, softness},
    };
    return all;
}

Design refined(const Vector<std::string>& names)
{
    auto design = currentCow();
    design.name = "refined";

    for (const auto& name: names)
        for (const auto& refinement: refinements())
            if (refinement.name == name)
                refinement.apply(design);

    return design;
}
} // namespace Cows::Pitch
