#include "Cow/Wardrobe.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr Vec3 crownOfHead {1.38f, 1.99f, 0.f};
constexpr auto crownPoints = 5;

Material cloth(std::uint32_t hex, float gloss = 0.2f)
{
    auto material = Material {Palette::linear(hex)};
    material.gloss = gloss;
    return material;
}

Material shiny(std::uint32_t hex)
{
    auto material = cloth(hex, 1.f);
    material.emission = 0.15f;
    return material;
}

struct HatParts final
{
    void add(Shape shape, Vec3 position, Vec3 size, const Material& material)
    {
        add(shape, position, size, material, {});
    }

    void add(Shape shape,
             Vec3 position,
             Vec3 size,
             const Material& material,
             const Mat4& rotation)
    {
        auto local = Mat4::translation(position) * rotation * Mat4::scale(size);
        parts.add({shape, Bone::Head, seat * local, material});
    }

    Vector<CowPart>& parts;
    Mat4 seat;
};

Mat4 seatAt(float tiltBack, float tiltSide)
{
    return Mat4::translation(crownOfHead) * Mat4::rotationZ(tiltBack)
           * Mat4::rotationX(tiltSide);
}

void addTopHat(Vector<CowPart>& parts)
{
    auto hat = HatParts {parts, seatAt(0.22f, 0.08f)};
    auto felt = cloth(Palette::topHat, 0.35f);

    hat.add(Shape::Cylinder, {0.f, 0.f, 0.f}, {0.56f, 0.035f, 0.56f}, felt);
    hat.add(Shape::Cylinder, {0.f, 0.02f, 0.f}, {0.34f, 0.38f, 0.34f}, felt);
    hat.add(Shape::Cylinder,
            {0.f, 0.035f, 0.f},
            {0.355f, 0.07f, 0.355f},
            cloth(Palette::heart, 0.4f));
}

void addCowboyHat(Vector<CowPart>& parts)
{
    auto hat = HatParts {parts, seatAt(0.2f, -0.06f)};
    auto leather = cloth(Palette::cowboyHat, 0.25f);

    hat.add(Shape::Sphere, {0.f, 0.02f, 0.f}, {0.4f, 0.035f, 0.36f}, leather);
    hat.add(Shape::Barrel, {0.f, -0.02f, 0.f}, {0.36f, 0.3f, 0.32f}, leather);
    hat.add(Shape::Cylinder,
            {0.f, 0.02f, 0.f},
            {0.345f, 0.06f, 0.305f},
            cloth(Palette::hatBand, 0.3f));
}

void addPartyHat(Vector<CowPart>& parts)
{
    auto hat = HatParts {parts, seatAt(0.12f, 0.3f)};

    hat.add(Shape::Cone,
            {0.f, 0.f, 0.f},
            {0.32f, 0.46f, 0.32f},
            cloth(Palette::partyHat, 0.4f));
    hat.add(Shape::Cylinder,
            {0.f, 0.f, 0.f},
            {0.33f, 0.035f, 0.33f},
            cloth(Palette::heart, 0.4f));
    hat.add(Shape::Sphere,
            {0.f, 0.47f, 0.f},
            {0.06f, 0.06f, 0.06f},
            cloth(Palette::pompom, 0.1f));
}

void addBeanie(Vector<CowPart>& parts)
{
    auto hat = HatParts {parts, seatAt(0.05f, 0.f)};
    auto knit = cloth(Palette::beanie, 0.1f);

    hat.add(Shape::Sphere, {0.f, -0.02f, 0.f}, {0.31f, 0.21f, 0.29f}, knit);
    hat.add(Shape::Cylinder,
            {0.f, -0.07f, 0.f},
            {0.635f, 0.09f, 0.595f},
            cloth(Palette::beanieCuff, 0.1f));
    hat.add(Shape::Sphere,
            {0.f, 0.22f, 0.f},
            {0.075f, 0.075f, 0.075f},
            cloth(Palette::cloud, 0.05f));
}

void addCrown(Vector<CowPart>& parts)
{
    auto hat = HatParts {parts, seatAt(0.1f, -0.1f)};
    auto gold = shiny(Palette::gold);

    hat.add(Shape::Cylinder, {0.f, 0.f, 0.f}, {0.34f, 0.1f, 0.34f}, gold);

    for (auto point = 0; point < crownPoints; ++point)
    {
        auto angle = twoPi * (float) point / (float) crownPoints;
        auto at = Vec3 {0.15f * std::cos(angle), 0.09f, 0.15f * std::sin(angle)};
        hat.add(Shape::Cone, at, {0.075f, 0.13f, 0.075f}, gold);
        hat.add(Shape::Sphere,
                at + Vec3 {0.f, 0.14f, 0.f},
                {0.022f, 0.022f, 0.022f},
                gold);
    }

    hat.add(Shape::Sphere,
            {0.17f, 0.05f, 0.f},
            {0.025f, 0.035f, 0.035f},
            shiny(Palette::heart));
}
} // namespace

void addHat(Vector<CowPart>& parts, Hat hat)
{
    switch (hat)
    {
        case Hat::TopHat:
            addTopHat(parts);
            break;
        case Hat::CowboyHat:
            addCowboyHat(parts);
            break;
        case Hat::PartyHat:
            addPartyHat(parts);
            break;
        case Hat::Beanie:
            addBeanie(parts);
            break;
        case Hat::Crown:
            addCrown(parts);
            break;
        case Hat::None:
            break;
    }
}

Vector<CowPart> makeCowParts(const CowSkin& skin)
{
    auto parts = makeCowParts();
    addHat(parts, skin.hat);
    return parts;
}
} // namespace Cows
