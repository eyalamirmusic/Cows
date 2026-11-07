#pragma once

#include "Render/Instances.h"

#include <array>

namespace Cows
{
enum class Bone
{
    Body,
    Head,
    Tail,
    Eye
};

struct CowPart final
{
    Shape shape = Shape::Sphere;
    Bone bone = Bone::Body;
    Maths::Mat4 transform;
    Material material;
    float spotScale = 1.f;
};

// Standing on y = 0, facing +x.
Vector<CowPart> makeCowParts();

enum class LegPair
{
    Front,
    Back
};

Maths::Mat4 torsoPlacement();
std::array<Maths::Mat4, 2> legPlacements(LegPair pair);

Maths::Vec3 tailRoot();

constexpr auto hoofTop = 0.22f;

struct CowPose final
{
    Maths::Mat4 world;
    float hop = 0.f;
    float lean = 0.f;
    float beat = 0.f;
    float glow = 1.f;
    float moo = 0.f;
    float seconds = 0.f;
};

// `heading` 0 faces +x.
struct FreeCowMotion final
{
    Maths::Vec3 position;
    float heading = 0.f;
    float hopClock = 0.f;
    float bounce = 0.f;
    float seconds = 0.f;
    float beatClock = 0.f;
    float glow = 1.f;
};

struct Cow final
{
    Maths::Mat4 placement(float seconds, float reach) const;

    Maths::Mat4 placement(const FreeCowMotion& motion) const;

    CowPose pose(float seconds, const Maths::Mat4& stage = {}) const;

    CowPose freePose(const FreeCowMotion& motion) const;

    void addTo(SurfaceBatch& batch,
               Vector<GlowInstance>& glows,
               const Vector<CowPart>& parts,
               const CowPose& pose) const;

    // (x, z, strength) of the contact shadow under the cow.
    Maths::Vec3 contact(const CowPose& pose) const;

    // -1 for the cow on the left, 1 for the one on the right.
    float side = -1.f;
    Maths::Vec3 spotSeed;
};

float cowReach(float seconds);
} // namespace Cows
