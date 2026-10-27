#include "ReloadedCowModel.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto eyeSize = 0.13f;

Material hideMaterial()
{
    auto material = Material {Palette::linear(Palette::hide)};
    material.spots = 1.f;
    material.gloss = 0.25f;
    return material;
}

Material plain(std::uint32_t hex, float gloss = 0.3f)
{
    auto material = Material {Palette::linear(hex)};
    material.gloss = gloss;
    return material;
}

Material eyeMaterial()
{
    auto material = Material {Palette::linear(Palette::heart)};
    material.emission = 1.f;
    material.gloss = 1.f;
    return material;
}

Mat4 place(Vec3 position, Vec3 size, const Mat4& rotation = {})
{
    return Mat4::translation(position) * rotation * Mat4::scale(size);
}

CowPart ball(Bone bone,
             Vec3 position,
             Vec3 size,
             const Material& material,
             const Mat4& rotation = {})
{
    return {Shape::Sphere, bone, place(position, size, rotation), material};
}

void addLegs(Vector<CowPart>& parts)
{
    for (auto pair: {LegPair::Back, LegPair::Front})
        for (const auto& leg: legPlacements(pair))
        {
            auto foot = leg.column(3);
            parts.add({Shape::Capsule, Bone::Body, leg, hideMaterial()});
            parts.add({Shape::Capsule,
                       Bone::Body,
                       place({foot.x, 0.f, foot.z}, {1.5f, hoofTop, 1.5f}),
                       plain(Palette::hoof, 0.5f)});
        }
}

void addTorso(Vector<CowPart>& parts)
{
    auto hide = hideMaterial();

    parts.add({Shape::Barrel, Bone::Body, torsoPlacement(), hide});
    parts.add(ball(Bone::Body,
                   {0.93f, 1.38f, 0.f},
                   {0.42f, 0.36f, 0.32f},
                   hide,
                   Mat4::rotationZ(0.6f)));

    auto udder = plain(Palette::muzzle, 0.5f);
    parts.add(ball(Bone::Body, {-0.38f, 0.64f, 0.f}, {0.22f, 0.15f, 0.2f}, udder));

    for (auto x: {-0.46f, -0.3f})
        for (auto z: {-0.08f, 0.08f})
            parts.add(ball(Bone::Body, {x, 0.5f, z}, {0.04f, 0.07f, 0.04f}, udder));
}

void addTail(Vector<CowPart>& parts)
{
    auto swing = Mat4::rotationZ(tailSwing);
    auto tail = Mat4::translation(tailPivot) * swing;

    parts.add({Shape::Capsule,
               Bone::Tail,
               tail * Mat4::rotationZ(pi) * Mat4::scale({0.36f, 0.78f, 0.36f}),
               hideMaterial()});
    parts.add({Shape::Sphere,
               Bone::Tail,
               tail * place({0.f, -0.8f, 0.f}, {0.09f, 0.15f, 0.09f}),
               plain(Palette::spot, 0.2f)});
}

void addFace(Vector<CowPart>& parts)
{
    auto muzzle = plain(Palette::muzzle, 0.6f);
    auto dark = plain(Palette::nostril, 0.4f);

    parts.add(ball(
        Bone::Head, {1.42f, 1.72f, 0.f}, {0.37f, 0.34f, 0.31f}, hideMaterial()));
    parts.add(ball(Bone::Head, {1.74f, 1.53f, 0.f}, {0.27f, 0.21f, 0.27f}, muzzle));
    parts.add(ball(Bone::Head, {1.84f, 1.4f, 0.f}, {0.12f, 0.02f, 0.14f}, dark));

    for (auto z: {-0.1f, 0.1f})
        parts.add(
            ball(Bone::Head, {1.98f, 1.57f, z}, {0.035f, 0.055f, 0.045f}, dark));

    for (auto side: {-1.f, 1.f})
    {
        auto facing = std::atan2(0.78f, 0.63f * side);
        parts.add({Shape::Heart,
                   Bone::Eye,
                   place({1.62f, 1.82f, 0.2f * side},
                         {eyeSize, eyeSize, eyeSize},
                         Mat4::rotationY(facing)),
                   eyeMaterial()});
    }
}

void addCrown(Vector<CowPart>& parts)
{
    auto horn = plain(Palette::horn, 0.6f);
    auto ear = hideMaterial();
    auto innerEar = plain(Palette::innerEar, 0.3f);
    auto tuft = plain(Palette::spot, 0.2f);

    for (auto side: {-1.f, 1.f})
    {
        auto outward = Mat4::rotationX(-0.55f * side) * Mat4::rotationZ(0.15f);
        auto hornAt = Vec3 {1.36f, 1.96f, 0.16f * side};
        parts.add({Shape::Horn,
                   Bone::Head,
                   place(hornAt, {0.15f, 0.27f, 0.15f}, outward),
                   horn});

        auto droop = Mat4::rotationX(0.45f * side) * Mat4::rotationY(-0.25f * side);
        auto earAt = Vec3 {1.28f, 1.84f, 0.37f * side};
        auto innerAt = Vec3 {1.32f, 1.84f, 0.38f * side};
        parts.add(ball(Bone::Head, earAt, {0.08f, 0.1f, 0.21f}, ear, droop));
        parts.add(ball(Bone::Head, innerAt, {0.05f, 0.07f, 0.16f}, innerEar, droop));
    }

    parts.add(ball(Bone::Head, {1.44f, 2.03f, 0.f}, {0.1f, 0.08f, 0.1f}, tuft));
    parts.add(ball(Bone::Head, {1.52f, 2.f, 0.05f}, {0.07f, 0.06f, 0.07f}, tuft));
    parts.add(ball(Bone::Head, {1.5f, 2.f, -0.06f}, {0.07f, 0.06f, 0.07f}, tuft));
}
} // namespace

Vector<CowPart> ReloadedCowModel::makeParts() const
{
    auto parts = Vector<CowPart> {};

    addLegs(parts);
    addTorso(parts);
    addTail(parts);
    addFace(parts);
    addCrown(parts);

    return parts;
}
} // namespace Cows
