#pragma once

#include "Cow/Cow.h"

#include <array>
#include <string>

namespace Cows::Pitch
{
// Where the camera looks from, as OrbitCamera holds it.
struct Shot final
{
    Maths::Vec3 target;
    float yaw = 0.f;
    float pitch = 0.f;
    float distance = 0.f;
};

// A cow design for the redesign pitch: its rest-pose parts on the game's bones,
// and the points the animation and the wardrobe hang off. A mockup, never game
// code.
struct Design final
{
    std::string name;
    Vector<CowPart> parts;
    Maths::Vec3 neckPivot;
    Maths::Vec3 tailPivot;
    // Where hats sit, and how much bigger the head is than today's.
    Maths::Vec3 crown;
    float headScale = 1.f;
    // The x of the muzzle's tip, which the kiss brings to the middle.
    float muzzleTip = 0.f;
    // The torso's Barrel and the legs' capsules (back -z, back +z, front -z,
    // front +z), which the pants are cut to fit.
    Maths::Mat4 torso;
    std::array<Maths::Mat4, 4> legs;
    bool barrelBody = true;
    // Where in the patchwork its spots are drawn from.
    Maths::Vec3 spotSeed;
};

Design currentCow();
Design plushCow();
Design storybookCow();
Design terminalCow();
} // namespace Cows::Pitch
