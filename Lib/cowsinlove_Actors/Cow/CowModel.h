#pragma once

#include "Cow/Cow.h"
#include "Cow/CowPhysics.h"

namespace Cows
{
// The cow's parts, jump and animation, made by makeCowModel, the factory in the
// app's ReloadedCow library (Apps/CowsInLove/ReloadedCow).
struct CowModel
{
    virtual ~CowModel() = default;

    // Standing on y = 0, facing +x.
    virtual Vector<CowPart> makeParts() const = 0;

    // Standing on y = 0 in metres: how high she jumps and how long she is in
    // the air.
    virtual JumpPhysics getJumpPhysics() const = 0;

    virtual CowPose makeEndingPose(const Cow& cow,
                                   float seconds,
                                   const Maths::Mat4& stage) const = 0;

    // `heading` 0 faces +x.
    virtual CowPose makeRoamingPose(const Cow& cow,
                                    Maths::Vec3 position,
                                    float heading,
                                    float hopClock,
                                    float bounce,
                                    float seconds,
                                    float beatClock,
                                    float glow) const = 0;

    virtual void addPosedCowTo(SurfaceBatch& batch,
                               Vector<GlowInstance>& glows,
                               const Cow& cow,
                               const Vector<CowPart>& parts,
                               const CowPose& pose) const = 0;

    // (x, z, strength) of the contact shadow under the cow.
    virtual Maths::Vec3 getContactShadow(const CowPose& pose) const = 0;
};
} // namespace Cows
