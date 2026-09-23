#pragma once

#include "Instances.h"

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
};

// The cow, standing on y = 0 and facing +x, in its rest pose.
Vector<CowPart> makeCowParts();

struct Cow final
{
    // Where this cow stands, `reach` from the middle.
    Maths::Mat4 placement(float seconds, float reach) const;

    void addTo(SurfaceBatch& batch,
               Vector<GlowInstance>& glows,
               const Vector<CowPart>& parts,
               float seconds) const;

    // (x, z, strength) of the soft contact shadow right under the cow, which
    // fades as it hops.
    Maths::Vec3 contact(float seconds) const;

    // -1 for the cow on the left, facing right; 1 for the one on the right.
    float side = -1.f;
    Maths::Vec3 spotSeed;
};

// How far from the middle each cow stands right now.
float cowReach(float seconds);
} // namespace Cows
