#include "Cow/Wardrobe.h"
#include "Cow/HeartMesh.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr Vec3 crownOfHead {1.38f, 1.99f, 0.f};
constexpr auto crownPoints = 5;
constexpr auto frogEye = 0.11f;
constexpr auto clothOut = 1.035f;
constexpr auto bandOut = 1.05f;
constexpr auto sleeveOut = 1.08f;
constexpr auto cuffOut = 1.15f;
constexpr auto cuffHeight = 0.075f;
constexpr auto clothRim = 0.04f;
constexpr auto bandWidth = 0.14f;
constexpr auto bandOverlap = 0.03f;
constexpr Vec2 bellyLatitude {-halfPi, halfPi};
constexpr Vec2 bellyLongitude {-halfPi, halfPi};
constexpr Vec2 seatLatitude {-halfPi, -0.22f};
constexpr Vec2 seatLongitude {-0.75f * pi, 0.75f * pi};

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

    hat.add(Shape::Sphere, {0.f, 0.02f, 0.f}, {0.36f, 0.03f, 0.26f}, leather);

    for (auto side: {-1.f, 1.f})
        hat.add(Shape::Sphere,
                {0.f, 0.07f, 0.24f * side},
                {0.3f, 0.025f, 0.12f},
                leather,
                Mat4::rotationX(-0.7f * side));

    hat.add(Shape::Barrel, {0.f, -0.02f, 0.f}, {0.28f, 0.38f, 0.24f}, leather);
    hat.add(Shape::Sphere,
            {0.f, 0.345f, 0.f},
            {0.1f, 0.03f, 0.045f},
            cloth(Palette::hatBand, 0.3f));
    hat.add(Shape::Cylinder,
            {0.f, 0.02f, 0.f},
            {0.285f, 0.06f, 0.245f},
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

    hat.add(Shape::Cylinder, {0.f, 0.f, 0.f}, {0.36f, 0.15f, 0.36f}, gold);

    for (auto point = 0; point < crownPoints; ++point)
    {
        auto angle = twoPi * (float) point / (float) crownPoints;
        auto at = Vec3 {0.16f * std::cos(angle), 0.14f, 0.16f * std::sin(angle)};
        hat.add(Shape::Cone, at, {0.075f, 0.13f, 0.075f}, gold);
        hat.add(Shape::Sphere,
                at + Vec3 {0.f, 0.14f, 0.f},
                {0.022f, 0.022f, 0.022f},
                gold);
    }

    hat.add(Shape::Sphere,
            {0.18f, 0.075f, 0.f},
            {0.025f, 0.035f, 0.035f},
            shiny(Palette::heart));
}
void addFrogHat(Vector<CowPart>& parts)
{
    auto hat = HatParts {parts, seatAt(0.f, 0.f)};
    auto plush = cloth(Palette::frog, 0.1f);
    auto lining = cloth(Palette::frogLining, 0.1f);

    hat.add(Shape::Sphere, {-0.04f, -0.1f, 0.f}, {0.36f, 0.24f, 0.34f}, plush);

    for (auto side: {-1.f, 1.f})
    {
        auto eye = Vec3 {0.06f, 0.15f, 0.14f * side};
        hat.add(Shape::Sphere, eye, {frogEye, frogEye, frogEye}, plush);
        hat.add(Shape::Sphere,
                eye + Vec3 {0.085f, 0.03f, 0.f},
                {0.035f, 0.08f, 0.08f},
                cloth(Palette::cloud, 0.6f));
        hat.add(Shape::Sphere,
                eye + Vec3 {0.11f, 0.035f, 0.f},
                {0.02f, 0.045f, 0.04f},
                cloth(Palette::spot, 0.9f));

        hat.add(Shape::Capsule,
                {0.f, -0.52f, 0.33f * side},
                {0.85f, 0.44f, 0.25f},
                plush);
        hat.add(Shape::Capsule,
                {0.f, -0.48f, 0.355f * side},
                {0.4f, 0.34f, 0.1f},
                lining);
    }
}

Mat4 aroundTorso(float scale)
{
    auto middle = Vec3 {0.f, 0.5f, 0.f};
    return torsoPlacement() * Mat4::translation(middle) * Mat4::scale(scale)
           * Mat4::translation(-middle);
}

MeshData makeWaistband(Vec2 latitude, Vec2 longitude)
{
    auto band = makeBarrelPatch(latitude,
                                {longitude.y - bandWidth, longitude.y + bandOverlap},
                                clothRim,
                                48,
                                3);
    append(band,
           makeBarrelPatch(latitude,
                           {longitude.x - bandOverlap, longitude.x + bandWidth},
                           clothRim,
                           48,
                           3));
    return band;
}

void addSleeves(Vector<CowPart>& parts, LegPair pair)
{
    auto denim = cloth(Palette::denim, 0.15f);
    auto seam = cloth(Palette::denimSeam, 0.15f);

    for (const auto& leg: legPlacements(pair))
    {
        auto foot = leg.column(3);
        auto cuffLift = Mat4::translation({0.f, hoofTop - 0.02f - foot.y, 0.f});

        parts.add({Shape::Capsule,
                   Bone::Body,
                   leg * Mat4::scale({sleeveOut, 1.f, sleeveOut}),
                   denim});
        parts.add({Shape::Capsule,
                   Bone::Body,
                   cuffLift * leg * Mat4::scale({cuffOut, cuffHeight, cuffOut}),
                   seam});
    }
}

void addGarment(Vector<CowPart>& parts, Shape garment, Shape waistband)
{
    parts.add(
        {garment, Bone::Body, aroundTorso(clothOut), cloth(Palette::denim, 0.15f)});
    parts.add({waistband,
               Bone::Body,
               aroundTorso(bandOut),
               cloth(Palette::denimSeam, 0.15f)});
}
} // namespace

void addPants(Vector<CowPart>& parts, Pants pants)
{
    switch (pants)
    {
        case Pants::BothLegs:
            addSleeves(parts, LegPair::Back);
            addSleeves(parts, LegPair::Front);
            addGarment(parts, Shape::Belly, Shape::BellyBand);
            break;
        case Pants::BackLegs:
            addSleeves(parts, LegPair::Back);
            addGarment(parts, Shape::Seat, Shape::SeatBand);
            break;
        case Pants::None:
            break;
    }
}

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
        case Hat::FrogHat:
            addFrogHat(parts);
            break;
        case Hat::None:
            break;
    }
}

MeshData makePantsMesh(Shape shape)
{
    switch (shape)
    {
        case Shape::Belly:
            return makeBarrelPatch(bellyLatitude, bellyLongitude, clothRim, 48, 24);
        case Shape::BellyBand:
            return makeWaistband(bellyLatitude, bellyLongitude);
        case Shape::Seat:
            return makeBarrelPatch(seatLatitude, seatLongitude, clothRim, 24, 36);
        case Shape::SeatBand:
            return makeWaistband(seatLatitude, seatLongitude);
        default:
            return {};
    }
}

MeshData makeCowMesh(Shape shape)
{
    return shape == Shape::Heart ? makeHeart() : makePantsMesh(shape);
}

Vector<CowPart> makeCowParts(const CowSkin& skin)
{
    auto parts = makeCowParts();
    addHat(parts, skin.hat);
    addPants(parts, skin.pants);
    return parts;
}
} // namespace Cows
