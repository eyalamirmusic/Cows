#include "Designs.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows::Pitch
{
namespace
{
// Wardrobe.cpp's crownOfHead and Cow.cpp's pivots, for today's cow.
constexpr Vec3 todaysCrown {1.45f, 2.f, 0.f};
constexpr Vec3 todaysNeck {1.1f, 1.5f, 0.f};
constexpr Vec3 todaysTail {-1.07f, 1.42f, 0.f};
constexpr auto todaysMuzzleTip = 2.01f;

constexpr auto plushSpots = 0.7f;
constexpr auto storybookSpots = 0.5f;

Material hide(float gloss = 0.25f)
{
    auto material = Material {Palette::linear(Palette::hide)};
    material.spots = 1.f;
    material.gloss = gloss;
    return material;
}

Material plain(std::uint32_t hex, float gloss = 0.3f)
{
    auto material = Material {Palette::linear(hex)};
    material.gloss = gloss;
    return material;
}

Material shiny(std::uint32_t hex)
{
    auto material = plain(hex, 1.f);
    material.emission = 0.15f;
    return material;
}

Material eyes()
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

struct Parts final
{
    void ball(Bone bone,
              Vec3 at,
              Vec3 size,
              const Material& material,
              const Mat4& rotation = {})
    {
        list.add({Shape::Sphere, bone, place(at, size, rotation), material, spots});
    }

    void add(Shape shape, Bone bone, const Mat4& transform, const Material& material)
    {
        list.add({shape, bone, transform, material, spots});
    }

    // A Box from `low` to `high`, corner to corner.
    void box(Bone bone, Vec3 low, Vec3 high, const Material& material)
    {
        auto size = high - low;
        auto at = Vec3 {0.5f * (low.x + high.x), low.y, 0.5f * (low.z + high.z)};
        add(Shape::Box, bone, place(at, size), material);
    }

    void heartEye(Vec3 at, float size, float facing)
    {
        add(Shape::Heart,
            Bone::Eye,
            place(at, {size, size, size}, Mat4::rotationY(facing)),
            eyes());
    }

    Vector<CowPart> list;
    float spots = 1.f;
};

Mat4 barrelBody(Vec3 middle, Vec3 size)
{
    return place(middle, size, Mat4::rotationZ(-halfPi))
           * Mat4::translation({0.f, -0.5f, 0.f});
}

std::array<Mat4, 4> legsAt(float back, float front, float apart, Vec3 size, float y)
{
    return {place({back, y, -apart}, size),
            place({back, y, apart}, size),
            place({front, y, -apart}, size),
            place({front, y, apart}, size)};
}

void addLegs(Parts& parts, const std::array<Mat4, 4>& legs, Vec3 hoof)
{
    for (const auto& leg: legs)
    {
        auto foot = leg.column(3);
        parts.add(Shape::Capsule, Bone::Body, leg, hide());
        parts.add(Shape::Capsule,
                  Bone::Body,
                  place({foot.x, 0.f, foot.z}, hoof),
                  plain(Palette::hoof, 0.5f));
    }
}

void addUdder(Parts& parts, Vec3 at, Vec3 size, float teat)
{
    auto udder = plain(Palette::muzzle, 0.5f);
    parts.ball(Bone::Body, at, size, udder);

    for (auto x: {-0.4f, 0.4f})
        for (auto z: {-0.4f, 0.4f})
            parts.ball(Bone::Body,
                       {at.x + x * size.x, at.y - 0.9f * size.y, at.z + z * size.z},
                       {teat, 1.8f * teat, teat},
                       udder);
}

Mat4 hanging(Vec3 pivot, float swing)
{
    return Mat4::translation(pivot) * Mat4::rotationZ(swing) * Mat4::rotationZ(pi);
}
} // namespace

Design currentCow()
{
    auto design = Design {};
    design.name = "current";
    design.parts = makeCowParts();
    design.neckPivot = todaysNeck;
    design.tailPivot = todaysTail;
    design.crown = todaysCrown;
    design.muzzleTip = todaysMuzzleTip;
    design.torso = torsoPlacement();

    auto back = legPlacements(LegPair::Back);
    auto front = legPlacements(LegPair::Front);
    design.legs = {back[0], back[1], front[0], front[1]};
    return design;
}

// A bean of a body on stubby legs, a head two thirds again as big with the
// eyes set wide and forward, blush, floppy ears, nub horns, a pom tail and
// fewer, bigger spots.
Design plushCow()
{
    auto design = Design {};
    design.name = "plush";
    design.spotSeed = {1.7f, 0.9f, 2.3f};
    design.neckPivot = {0.75f, 1.5f, 0.f};
    design.tailPivot = {-0.88f, 1.35f, 0.f};
    design.crown = {1.15f, 2.27f, 0.f};
    design.headScale = 1.45f;
    design.muzzleTip = 1.91f;
    design.torso = barrelBody({0.f, 1.02f, 0.f}, {1.34f, 1.8f, 1.24f});
    design.legs = legsAt(-0.5f, 0.52f, 0.34f, {1.75f, 0.7f, 1.75f}, 0.08f);

    auto parts = Parts {};
    parts.spots = plushSpots;
    auto muzzle = plain(Palette::muzzle, 0.6f);
    auto dark = plain(Palette::nostril, 0.4f);
    auto tuft = plain(Palette::spot, 0.2f);
    auto blush = plain(0xff8fae, 0.2f);
    blush.emission = 0.1f;

    addLegs(parts, design.legs, {2.f, 0.2f, 2.f});
    parts.add(Shape::Barrel, Bone::Body, design.torso, hide());
    parts.ball(Bone::Body, {0.6f, 1.12f, 0.f}, {0.5f, 0.55f, 0.56f}, hide());
    addUdder(parts, {-0.25f, 0.44f, 0.f}, {0.2f, 0.12f, 0.18f}, 0.035f);

    auto tail = hanging(design.tailPivot, -0.5f);
    parts.add(Shape::Capsule,
              Bone::Tail,
              tail * Mat4::scale({0.42f, 0.45f, 0.42f}),
              hide());
    parts.add(Shape::Sphere,
              Bone::Tail,
              tail * place({0.f, 0.48f, 0.f}, {0.14f, 0.14f, 0.14f}),
              tuft);

    parts.ball(Bone::Head, {1.12f, 1.78f, 0.f}, {0.62f, 0.56f, 0.58f}, hide());
    parts.ball(Bone::Head, {1.55f, 1.5f, 0.f}, {0.36f, 0.28f, 0.44f}, muzzle);
    parts.ball(Bone::Head, {1.85f, 1.37f, 0.f}, {0.07f, 0.015f, 0.13f}, dark);

    for (auto z: {-0.13f, 0.13f})
        parts.ball(Bone::Head, {1.88f, 1.55f, z}, {0.04f, 0.06f, 0.05f}, dark);

    for (auto side: {-1.f, 1.f})
    {
        parts.heartEye(
            {1.64f, 1.92f, 0.27f * side}, 0.17f, std::atan2(0.85f, 0.5f * side));
        parts.ball(Bone::Head,
                   {1.56f, 1.66f, 0.38f * side},
                   {0.02f, 0.08f, 0.11f},
                   blush,
                   Mat4::rotationY(-0.77f * side));

        auto flop = Mat4::rotationX(0.7f * side) * Mat4::rotationY(-0.2f * side);
        parts.ball(Bone::Head,
                   {1.f, 1.98f, 0.56f * side},
                   {0.14f, 0.1f, 0.26f},
                   hide(),
                   flop);
        parts.ball(Bone::Head,
                   {1.04f, 1.97f, 0.57f * side},
                   {0.09f, 0.07f, 0.19f},
                   plain(Palette::innerEar),
                   flop);

        parts.add(Shape::Horn,
                  Bone::Head,
                  place({1.08f, 2.22f, 0.2f * side},
                        {0.13f, 0.17f, 0.13f},
                        Mat4::rotationX(0.3f * side)),
                  plain(Palette::horn, 0.6f));
    }

    parts.ball(Bone::Head, {1.18f, 2.33f, 0.f}, {0.13f, 0.09f, 0.13f}, tuft);
    parts.ball(Bone::Head, {1.3f, 2.3f, 0.07f}, {0.09f, 0.07f, 0.09f}, tuft);
    parts.ball(Bone::Head, {1.28f, 2.3f, -0.08f}, {0.09f, 0.07f, 0.09f}, tuft);

    design.parts = parts.list;
    return design;
}

// A proper dairy cow out of a picture book: longer and higher on the leg, hip
// bones, a long muzzle with a nose pad, ears out sideways, horns that sweep out
// and up, a big udder, a collar and bell, a long tail with a tuft, and big
// Holstein patches.
Design storybookCow()
{
    auto design = Design {};
    design.name = "storybook";
    design.spotSeed = {11.9f, 6.3f, 16.1f};
    design.neckPivot = {1.15f, 1.6f, 0.f};
    design.tailPivot = {-1.28f, 1.62f, 0.f};
    design.crown = {1.68f, 2.24f, 0.f};
    design.headScale = 0.85f;
    design.muzzleTip = 2.33f;
    design.torso = barrelBody({0.f, 1.28f, 0.f}, {1.04f, 2.6f, 0.96f});
    design.legs = legsAt(-0.9f, 0.85f, 0.28f, {1.15f, 1.12f, 1.15f}, 0.12f);

    auto parts = Parts {};
    parts.spots = storybookSpots;
    auto pad = plain(Palette::muzzle, 0.6f);
    auto dark = plain(Palette::nostril, 0.4f);
    auto tuft = plain(Palette::spot, 0.2f);
    auto horn = plain(Palette::horn, 0.6f);

    addLegs(parts, design.legs, {1.35f, 0.2f, 1.35f});
    parts.add(Shape::Barrel, Bone::Body, design.torso, hide());
    parts.ball(Bone::Body, {1.05f, 1.45f, 0.f}, {0.44f, 0.42f, 0.36f}, hide());
    parts.ball(Bone::Body,
               {1.38f, 1.7f, 0.f},
               {0.36f, 0.3f, 0.28f},
               hide(),
               Mat4::rotationZ(0.8f));

    for (auto side: {-1.f, 1.f})
        parts.ball(
            Bone::Body, {-0.95f, 1.66f, 0.3f * side}, {0.13f, 0.1f, 0.12f}, hide());

    addUdder(parts, {-0.5f, 0.8f, 0.f}, {0.3f, 0.2f, 0.26f}, 0.045f);

    auto collar = plain(Palette::heart, 0.3f);
    parts.add(
        Shape::Cylinder,
        Bone::Body,
        place({1.42f, 1.72f, 0.f}, {0.66f, 0.08f, 0.62f}, Mat4::rotationZ(-0.77f))
            * Mat4::translation({0.f, -0.5f, 0.f}),
        collar);
    parts.add(Shape::Cone,
              Bone::Body,
              place({1.66f, 1.27f, 0.f}, {0.18f, 0.2f, 0.18f}),
              shiny(Palette::gold));
    parts.ball(Bone::Body,
               {1.66f, 1.27f, 0.f},
               {0.045f, 0.045f, 0.045f},
               plain(Palette::hoof));
    parts.ball(Bone::Body,
               {1.66f, 1.48f, 0.f},
               {0.04f, 0.04f, 0.04f},
               shiny(Palette::gold));

    auto tail = hanging(design.tailPivot, -0.18f);
    parts.add(Shape::Capsule,
              Bone::Tail,
              tail * Mat4::scale({0.32f, 0.95f, 0.32f}),
              hide());
    parts.add(Shape::Sphere,
              Bone::Tail,
              tail * place({0.f, 0.98f, 0.f}, {0.08f, 0.17f, 0.08f}),
              tuft);

    parts.ball(Bone::Head, {1.7f, 2.f, 0.f}, {0.3f, 0.28f, 0.27f}, hide());
    parts.ball(Bone::Head,
               {2.02f, 1.82f, 0.f},
               {0.3f, 0.21f, 0.23f},
               hide(),
               Mat4::rotationZ(-0.45f));
    parts.ball(Bone::Head,
               {2.22f, 1.7f, 0.f},
               {0.12f, 0.15f, 0.2f},
               pad,
               Mat4::rotationZ(-0.45f));

    for (auto z: {-0.08f, 0.08f})
        parts.ball(Bone::Head, {2.31f, 1.72f, z}, {0.03f, 0.05f, 0.04f}, dark);

    for (auto side: {-1.f, 1.f})
    {
        parts.heartEye(
            {1.92f, 2.06f, 0.17f * side}, 0.13f, std::atan2(0.7f, 0.71f * side));

        auto out = Mat4::rotationX(0.25f * side) * Mat4::rotationY(-0.2f * side);
        parts.ball(Bone::Head,
                   {1.6f, 2.08f, 0.38f * side},
                   {0.07f, 0.09f, 0.2f},
                   hide(),
                   out);
        parts.ball(Bone::Head,
                   {1.63f, 2.08f, 0.39f * side},
                   {0.045f, 0.065f, 0.15f},
                   plain(Palette::innerEar),
                   out);

        auto baseAt = Vec3 {1.62f, 2.2f, 0.15f * side};
        auto sweep = Mat4::rotationX(1.15f * side);
        auto along = transformDirection(sweep, {0.f, 1.f, 0.f});
        parts.add(Shape::Horn,
                  Bone::Head,
                  place(baseAt, {0.13f, 0.26f, 0.13f}, sweep),
                  horn);
        parts.add(Shape::Horn,
                  Bone::Head,
                  place(baseAt + along * 0.21f,
                        {0.09f, 0.24f, 0.09f},
                        Mat4::rotationX(0.15f * side)),
                  horn);
    }

    parts.ball(Bone::Head, {1.72f, 2.27f, 0.f}, {0.09f, 0.07f, 0.09f}, tuft);

    design.parts = parts.list;
    return design;
}

// The cowsay cow in blocks: a box body with square pixel patches, a box head,
// ^ ^ horns, a (__) muzzle, || legs, a "w" udder and a \/\ tail.
Design terminalCow()
{
    auto design = Design {};
    design.name = "terminal";
    design.neckPivot = {1.f, 1.5f, 0.f};
    design.tailPivot = {-1.05f, 1.45f, 0.f};
    design.crown = {1.32f, 1.92f, 0.f};
    design.headScale = 1.f;
    design.muzzleTip = 1.8f;
    design.barrelBody = false;

    auto parts = Parts {};
    auto white = plain(Palette::hide, 0.25f);
    auto patch = plain(Palette::spot, 0.2f);
    auto pink = plain(Palette::muzzle, 0.5f);
    auto dark = plain(Palette::nostril, 0.4f);
    auto hoof = plain(Palette::hoof, 0.5f);

    for (auto x: {-0.75f, 0.75f})
        for (auto z: {-0.28f, 0.28f})
        {
            parts.box(Bone::Body,
                      {x - 0.08f, 0.12f, z - 0.08f},
                      {x + 0.08f, 0.75f, z + 0.08f},
                      white);
            parts.box(Bone::Body,
                      {x - 0.1f, 0.f, z - 0.1f},
                      {x + 0.1f, 0.14f, z + 0.1f},
                      hoof);
        }

    parts.box(Bone::Body, {-1.05f, 0.7f, -0.48f}, {1.f, 1.55f, 0.48f}, white);
    parts.box(Bone::Body, {-0.7f, 0.95f, 0.47f}, {-0.25f, 1.4f, 0.49f}, patch);
    parts.box(Bone::Body, {0.15f, 0.85f, -0.49f}, {0.6f, 1.25f, -0.47f}, patch);
    parts.box(Bone::Body, {-1.06f, 1.2f, -0.2f}, {-0.65f, 1.56f, 0.49f}, patch);
    parts.box(Bone::Body, {0.3f, 1.3f, 0.2f}, {0.65f, 1.56f, 0.49f}, patch);

    parts.box(Bone::Body, {-0.45f, 0.55f, -0.15f}, {-0.1f, 0.71f, 0.15f}, pink);

    for (auto x: {-0.38f, -0.17f})
        for (auto z: {-0.08f, 0.08f})
            parts.box(Bone::Body,
                      {x - 0.025f, 0.47f, z - 0.025f},
                      {x + 0.025f, 0.56f, z + 0.025f},
                      pink);

    auto segment =
        [&](Vec3 from, float swing, float length, const Material& material)
    {
        auto turn =
            Mat4::translation(from) * Mat4::rotationZ(swing) * Mat4::rotationZ(pi);
        parts.add(Shape::Box,
                  Bone::Tail,
                  turn * Mat4::scale({0.07f, length, 0.07f}),
                  material);
        return transformPoint(turn, {0.f, length, 0.f});
    };
    auto end = segment(design.tailPivot, -0.47f, 0.3f, white);
    end = segment(end, 0.3f, 0.28f, white);
    end = segment(end, -0.47f, 0.22f, white);
    segment(end, 0.f, 0.1f, patch);

    parts.box(Bone::Head, {1.f, 1.3f, -0.33f}, {1.6f, 1.95f, 0.33f}, white);
    parts.box(Bone::Head, {1.15f, 1.6f, 0.05f}, {1.45f, 1.96f, 0.34f}, patch);
    parts.box(Bone::Head, {1.45f, 1.3f, -0.27f}, {1.78f, 1.62f, 0.27f}, pink);

    for (auto z: {-0.12f, 0.12f})
        parts.box(
            Bone::Head, {1.77f, 1.45f, z - 0.04f}, {1.8f, 1.53f, z + 0.04f}, dark);

    for (auto side: {-1.f, 1.f})
    {
        parts.heartEye({1.62f, 1.78f, 0.16f * side}, 0.12f, halfPi);
        parts.add(Shape::Cone,
                  Bone::Head,
                  place({1.25f, 1.94f, 0.22f * side}, {0.12f, 0.22f, 0.12f}),
                  plain(Palette::horn, 0.6f));
        parts.box(Bone::Head,
                  {1.12f, 1.72f, side < 0.f ? -0.5f : 0.33f},
                  {1.28f, 1.84f, side < 0.f ? -0.33f : 0.5f},
                  white);
    }

    design.parts = parts.list;
    return design;
}
} // namespace Cows::Pitch
