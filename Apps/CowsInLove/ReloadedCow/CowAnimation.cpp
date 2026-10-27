#include "ReloadedCowModel.h"
#include "Animation/Choreography.h"
#include "Render/Palette.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto kissReach = 1.98f;
constexpr auto hopLift = 0.15f;
constexpr Vec3 neckPivot {1.1f, 1.5f, 0.f};

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

float cowReach(float seconds)
{
    auto gap = Choreography::kissGap
               + (Choreography::apartGap - Choreography::kissGap)
                     * (1.f - Choreography::closeness(seconds));

    return kissReach * gap / Choreography::kissGap;
}

Mat4 placeInTheEnding(const Cow& cow, float seconds, float reach)
{
    auto time = hopTime(seconds, cow.side);
    auto hop = Choreography::hopHeight(time, false);
    auto squash = Choreography::landingSquash(time, false);

    auto stretch = 1.f + 0.05f * hop - 0.1f * squash;
    auto widen = 1.f / std::sqrt(stretch);
    auto heading = cow.side < 0.f ? 0.f : pi;
    auto lean = 0.05f * kissLean(seconds);

    return Mat4::translation({cow.side * reach, hopLift * hop, 0.f})
           * Mat4::rotationY(heading) * Mat4::rotationZ(-lean)
           * Mat4::scale({widen, stretch, widen});
}

Mat4 placeRoaming(
    const Cow& cow, Vec3 position, float heading, float hopClock, float bounce)
{
    auto time = hopTime(hopClock, cow.side);
    auto hop = bounce * Choreography::hopHeight(time, false);
    auto squash = bounce * Choreography::landingSquash(time, false);

    return Mat4::translation(position + Vec3 {0.f, hopLift * hop, 0.f})
           * Mat4::rotationY(heading) * squashed(hop, squash);
}
} // namespace

CowPose ReloadedCowModel::makeEndingPose(const Cow& cow,
                                         float seconds,
                                         const Mat4& stage) const
{
    auto pose = CowPose {};
    pose.world = stage * placeInTheEnding(cow, seconds, cowReach(seconds));
    pose.hop = Choreography::hopHeight(hopTime(seconds, cow.side), false);
    pose.lean = kissLean(seconds);
    pose.beat = Choreography::heartbeat(seconds);
    pose.seconds = seconds;
    return pose;
}

CowPose ReloadedCowModel::makeRoamingPose(const Cow& cow,
                                          Vec3 position,
                                          float heading,
                                          float hopClock,
                                          float bounce,
                                          float seconds,
                                          float beatClock,
                                          float glow) const
{
    auto pose = CowPose {};
    pose.world = placeRoaming(cow, position, heading, hopClock, bounce);
    pose.hop = bounce * Choreography::hopHeight(hopTime(hopClock, cow.side), false);
    pose.beat = Choreography::heartbeat(beatClock);
    pose.glow = glow;
    pose.seconds = seconds;
    return pose;
}

void ReloadedCowModel::addPosedCowTo(SurfaceBatch& batch,
                                     Vector<GlowInstance>& glows,
                                     const Cow& cow,
                                     const Vector<CowPart>& parts,
                                     const CowPose& cowPose) const
{
    const auto& world = cowPose.world;
    auto head = headPose(cowPose.hop, cowPose.lean, cowPose.moo);
    auto tail = tailPose(cowPose.seconds, cow.side);
    auto beat = cowPose.beat;
    auto eye = eyePose(beat);
    auto seed = Mat4::translation(cow.spotSeed);

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

Vec3 ReloadedCowModel::getContactShadow(const CowPose& pose) const
{
    auto at = pose.world.column(3);
    return {at.x, at.z, 1.f - 0.5f * pose.hop};
}
} // namespace Cows
