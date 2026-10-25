#pragma once

namespace Cows
{
struct JumpPhysics final
{
    float launchSpeedMetresPerSecond = 0.f;
    float gravityMetresPerSecondSquared = 0.f;
};

// Standing on y = 0 in metres: how high she jumps and how long she is in the air.
JumpPhysics getJumpPhysics();
} // namespace Cows
