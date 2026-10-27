#include "ReloadedCowModel.h"

namespace Cows
{
namespace
{
constexpr auto jumpHeightMetres = 2.4f;
constexpr auto jumpAirTimeSeconds = 0.8f;

// Constant acceleration, symmetric arc: v = g T / 2 and h = v T / 4, so v = 4h / T.
float launchSpeedFor(float heightMetres, float airTimeSeconds)
{
    return 4.f * heightMetres / airTimeSeconds;
}

// Constant acceleration, symmetric arc: h = g T² / 8, so g = 8h / T².
float gravityFor(float heightMetres, float airTimeSeconds)
{
    return 8.f * heightMetres / (airTimeSeconds * airTimeSeconds);
}
} // namespace

JumpPhysics ReloadedCowModel::getJumpPhysics() const
{
    return {launchSpeedFor(jumpHeightMetres, jumpAirTimeSeconds),
            gravityFor(jumpHeightMetres, jumpAirTimeSeconds)};
}
} // namespace Cows
