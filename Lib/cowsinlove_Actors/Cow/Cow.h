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

enum class LegPair
{
    Front,
    Back
};

// The body the parts are placed on and the clothes are cut from, standing on
// y = 0 and facing +x.
Maths::Mat4 torsoPlacement();
std::array<Maths::Mat4, 2> legPlacements(LegPair pair);

Maths::Vec3 tailRoot();

constexpr auto hoofTop = 0.22f;
constexpr Maths::Vec3 tailPivot {-1.07f, 1.42f, 0.f};
constexpr auto tailSwing = -0.3f;

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
    // -1 for the cow on the left, 1 for the one on the right.
    float side = -1.f;
    Maths::Vec3 spotSeed;
};
} // namespace Cows
