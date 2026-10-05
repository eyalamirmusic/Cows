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
    // How much smaller its spots are than the hide's, for a print.
    float spotScale = 1.f;
};

// The cow, standing on y = 0 and facing +x, in its rest pose.
Vector<CowPart> makeCowParts();

enum class LegPair
{
    Front,
    Back
};

// Where the rest pose's torso (a Barrel) and the capsules of each pair of legs
// sit, for clothes cut to fit them.
Maths::Mat4 torsoPlacement();
std::array<Maths::Mat4, 2> legPlacements(LegPair pair);

// Where the tail comes out of the rump in the rest pose.
Maths::Vec3 tailRoot();

// The top of each hoof, which the legs run down into.
constexpr auto hoofTop = 0.22f;

// Everything that moves a cow this frame.
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

struct Cow final
{
    // Where this cow stands, `reach` from the middle.
    Maths::Mat4 placement(float seconds, float reach) const;

    // Standing anywhere, facing `heading` (0 is +x); `bounce` fades the hops in
    // and out.
    Maths::Mat4 placement(Maths::Vec3 position,
                          float heading,
                          float hopClock,
                          float bounce) const;

    // The postcard's choreography, played on `stage`.
    CowPose pose(float seconds, const Maths::Mat4& stage = {}) const;

    CowPose freePose(Maths::Vec3 position,
                     float heading,
                     float hopClock,
                     float bounce,
                     float seconds,
                     float beatClock,
                     float glow) const;

    void addTo(SurfaceBatch& batch,
               Vector<GlowInstance>& glows,
               const Vector<CowPart>& parts,
               const CowPose& pose) const;

    // (x, z, strength) of the soft contact shadow right under the cow, which
    // fades as it hops.
    Maths::Vec3 contact(const CowPose& pose) const;

    // -1 for the cow on the left, facing right; 1 for the one on the right.
    float side = -1.f;
    Maths::Vec3 spotSeed;
};

// How far from the middle each cow stands right now.
float cowReach(float seconds);
} // namespace Cows
