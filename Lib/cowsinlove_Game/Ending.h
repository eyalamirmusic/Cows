#pragma once

#include "Game.h"

namespace Cows::Ending
{
constexpr auto titleText = "cowsinlove.com";
constexpr Maths::Vec3 titleCenter {0.f, 9.6f, -16.f};
constexpr auto titleScale = 1.12f;
constexpr Maths::Vec3 kissPoint {0.f, 1.75f, 0.f};
constexpr auto titleDrop = 5.f;
constexpr auto titleDelay = 0.6f;
constexpr auto titleRiseTime = 2.4f;
constexpr auto titleHold = 1.5f;
constexpr auto loopTime = titleDelay + titleRiseTime + titleHold;
constexpr auto endingHeight = 3.1f;
constexpr auto endingPitch = 0.08f;
constexpr auto endingDistance = 11.f;
constexpr auto settleTime = 3.f;
constexpr auto settleRate = 2.f;
constexpr auto foundStartGap = 3.f;
// So the title fits on a phone held upright.
constexpr auto leastWidth = Maths::radians(52.f);
constexpr Maths::Vec3 loftTitleCenter {0.f, 7.f, 2.5f};
// Small so the rising title never passes over the hay door.
constexpr auto loftTitleDrop = 1.5f;
constexpr auto loftTitleScale = 0.55f;
constexpr auto loftCameraHeight = 3.f;
constexpr auto loftCameraDistance = 17.f;
constexpr auto loftCameraPitch = -0.08f;
constexpr auto foundStartHeading = Maths::pi;

struct Staging final
{
    Maths::Vec3 titleCenter;
    float titleScale = 1.f;
    float titleDrop = 0.f;
    float cameraHeight = 0.f;
    float cameraDistance = 0.f;
    float cameraPitch = 0.f;
};

Staging stagingFor(Level::Hiding hiding);

float titleRise(float secondsSinceFound);

Maths::Mat4 titlePlacement(const Game& game);

Maths::Vec3 foundStart(const Game& game);

bool hasEndingPlayedOut(const Game& game);
} // namespace Cows::Ending
