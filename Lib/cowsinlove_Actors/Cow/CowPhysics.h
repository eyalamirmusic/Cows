#pragma once

namespace Cows
{
struct JumpPhysics final
{
    float launchSpeedMetresPerSecond = 0.f;
    float gravityMetresPerSecondSquared = 0.f;
};

// Stays where she stands: no launch and no gravity.
inline constexpr auto neitherJumpingNorFalling = JumpPhysics {};
} // namespace Cows
