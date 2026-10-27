#pragma once

#include "Cow/CowModel.h"

namespace Cows
{
// Its parts in CowParts.cpp, its jump in CowJump.cpp and its animation in
// CowAnimation.cpp; only makeCowModel (CowModelFactory.cpp) hands one out.
struct ReloadedCowModel final : CowModel
{
    Vector<CowPart> makeParts() const override;

    JumpPhysics getJumpPhysics() const override;

    CowPose makeEndingPose(const Cow& cow,
                           float seconds,
                           const Maths::Mat4& stage) const override;

    CowPose makeRoamingPose(const Cow& cow,
                            Maths::Vec3 position,
                            float heading,
                            float hopClock,
                            float bounce,
                            float seconds,
                            float beatClock,
                            float glow) const override;

    void addPosedCowTo(SurfaceBatch& batch,
                       Vector<GlowInstance>& glows,
                       const Cow& cow,
                       const Vector<CowPart>& parts,
                       const CowPose& pose) const override;

    Maths::Vec3 getContactShadow(const CowPose& pose) const override;
};
} // namespace Cows
