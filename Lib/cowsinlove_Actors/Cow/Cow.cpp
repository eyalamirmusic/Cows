#include "Cow/Cow.h"
#include "Animation/Choreography.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto kissReach = 1.98f;
constexpr auto hopLift = 0.15f;
constexpr auto eyeSize = 0.13f;
constexpr Vec3 neckPivot {1.1f, 1.5f, 0.f};
constexpr Vec3 tailPivot {-1.07f, 1.42f, 0.f};
constexpr auto tailSwing = -0.3f;
constexpr auto tailUnderHide = 0.08f;

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

Mat4 about(Vec3 pivot, const Mat4& rotation)
{
    return Mat4::translation(pivot) * rotation * Mat4::translation(-pivot);
}

float hopTime(float seconds, float side)
{
    return seconds + (side > 0.f ? 0.5f * Choreography::hopPeriod : 0.f);
}

float kissLean(float seconds)
{
    return std::pow(Choreography::closeness(seconds), 3.f);
}

float legsAt(LegPair pair)
{
    return pair == LegPair::Back ? -0.6f : 0.58f;
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

Mat4 headPose(float hop, float lean, float moo)
{
    auto nod = 0.06f * (hop - 0.5f) - 0.12f * lean + 0.4f * moo;
    auto tilt = 0.2f * lean;

    return about(neckPivot, Mat4::rotationX(tilt) * Mat4::rotationZ(nod));
}

Mat4 tailPose(float seconds, float side)
{
    auto swish = std::sin(twoPi * seconds / 1.3f + side);
    return about(tailPivot,
                 Mat4::rotationX(0.45f * swish) * Mat4::rotationZ(0.1f * swish));
}

Mat4 eyePose(float beat)
{
    return Mat4::scale(1.f + 0.24f * beat);
}

Mat4 squashed(float hop, float squash)
{
    auto stretch = 1.f + 0.05f * hop - 0.1f * squash;
    auto widen = 1.f / std::sqrt(stretch);
    return Mat4::scale({widen, stretch, widen});
}
} // namespace

Mat4 torsoPlacement()
{
    return place({0.f, 1.13f, 0.f}, {1.12f, 2.24f, 1.04f}, Mat4::rotationZ(-halfPi))
           * Mat4::translation({0.f, -0.5f, 0.f});
}

std::array<Mat4, 2> legPlacements(LegPair pair)
{
    auto x = legsAt(pair);
    return {place({x, 0.1f, -0.27f}, {1.25f, 0.95f, 1.25f}),
            place({x, 0.1f, 0.27f}, {1.25f, 0.95f, 1.25f})};
}

Vec3 tailRoot()
{
    auto hanging = transformDirection(Mat4::rotationZ(tailSwing), {0.f, -1.f, 0.f});
    return tailPivot + hanging * tailUnderHide;
}

Vector<CowPart> makeCowParts()
{
    auto parts = Vector<CowPart> {};

    addLegs(parts);
    addTorso(parts);
    addTail(parts);
    addFace(parts);
    addCrown(parts);

    return parts;
}

float cowReach(float seconds)
{
    auto gap = Choreography::kissGap
               + (Choreography::apartGap - Choreography::kissGap)
                     * (1.f - Choreography::closeness(seconds));

    return kissReach * gap / Choreography::kissGap;
}

Mat4 Cow::placement(float seconds, float reach) const
{
    auto time = hopTime(seconds, side);
    auto hop = Choreography::hopHeight(time, false);
    auto squash = Choreography::landingSquash(time, false);

    auto stretch = 1.f + 0.05f * hop - 0.1f * squash;
    auto widen = 1.f / std::sqrt(stretch);
    auto heading = side < 0.f ? 0.f : pi;
    auto lean = 0.05f * kissLean(seconds);

    return Mat4::translation({side * reach, hopLift * hop, 0.f})
           * Mat4::rotationY(heading) * Mat4::rotationZ(-lean)
           * Mat4::scale({widen, stretch, widen});
}

Mat4 Cow::placement(Vec3 position, float heading, float hopClock, float bounce) const
{
    auto time = hopTime(hopClock, side);
    auto hop = bounce * Choreography::hopHeight(time, false);
    auto squash = bounce * Choreography::landingSquash(time, false);

    return Mat4::translation(position + Vec3 {0.f, hopLift * hop, 0.f})
           * Mat4::rotationY(heading) * squashed(hop, squash);
}

CowPose Cow::pose(float seconds, const Mat4& stage) const
{
    auto pose = CowPose {};
    pose.world = stage * placement(seconds, cowReach(seconds));
    pose.hop = Choreography::hopHeight(hopTime(seconds, side), false);
    pose.lean = kissLean(seconds);
    pose.beat = Choreography::heartbeat(seconds);
    pose.seconds = seconds;
    return pose;
}

CowPose Cow::freePose(Vec3 position,
                      float heading,
                      float hopClock,
                      float bounce,
                      float seconds,
                      float beatClock,
                      float glow) const
{
    auto pose = CowPose {};
    pose.world = placement(position, heading, hopClock, bounce);
    pose.hop = bounce * Choreography::hopHeight(hopTime(hopClock, side), false);
    pose.beat = Choreography::heartbeat(beatClock);
    pose.glow = glow;
    pose.seconds = seconds;
    return pose;
}

void Cow::addTo(SurfaceBatch& batch,
                Vector<GlowInstance>& glows,
                const Vector<CowPart>& parts,
                const CowPose& cowPose) const
{
    const auto& world = cowPose.world;
    auto head = headPose(cowPose.hop, cowPose.lean, cowPose.moo);
    auto tail = tailPose(cowPose.seconds, side);
    auto beat = cowPose.beat;
    auto eye = eyePose(beat);
    auto seed = Mat4::translation(spotSeed);

    for (const auto& part: parts)
    {
        auto pose = Mat4 {};

        if (part.bone == Bone::Head)
            pose = head * part.transform;
        else if (part.bone == Bone::Tail)
            pose = tail * part.transform;
        else if (part.bone == Bone::Eye)
            pose = head * part.transform * eye;
        else
            pose = part.transform;

        auto material = part.material;

        if (part.bone == Bone::Eye)
        {
            material.emission = 1.2f + 0.8f * std::max(beat, 0.f);

            auto center = transformPoint(world * pose, {0.f, 0.f, 0.3f});
            auto glow = Palette::linear(Palette::heart) * (0.22f + 0.16f * beat)
                        * cowPose.glow;
            glows.add(makeGlow(center, 0.24f + 0.05f * beat, glow));
        }

        batch.add(part.shape,
                  makeInstance(world * pose,
                               material,
                               seed * Mat4::scale(part.spotScale) * part.transform));
    }
}

Vec3 Cow::contact(const CowPose& pose) const
{
    auto at = pose.world.column(3);
    return {at.x, at.z, 1.f - 0.5f * pose.hop};
}
} // namespace Cows
