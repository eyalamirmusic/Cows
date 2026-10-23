#pragma once

#include "Render/Instances.h"

#include <eacp/Core/Plugins/PluginExport.h>

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

// Standing on y = 0, facing +x. The module's entry point; eacp silences this
// warning on the app and the module only, not on the library the header is in.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
EACP_HOT_RELOAD_EXPORT Vector<CowPart> makeCowParts();
#pragma clang diagnostic pop

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

struct Cow final
{
    Maths::Mat4 placement(float seconds, float reach) const;

    // `heading` 0 faces +x.
    Maths::Mat4 placement(Maths::Vec3 position,
                          float heading,
                          float hopClock,
                          float bounce) const;

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

    // (x, z, strength) of the contact shadow under the cow.
    Maths::Vec3 contact(const CowPose& pose) const;

    // -1 for the cow on the left, 1 for the one on the right.
    float side = -1.f;
    Maths::Vec3 spotSeed;
};

float cowReach(float seconds);
} // namespace Cows
