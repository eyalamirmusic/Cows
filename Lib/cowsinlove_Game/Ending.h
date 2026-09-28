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

// How far the title has risen into place, sinceFound seconds after she is found.
float titleRise(float sinceFound);

Maths::Mat4 titlePlacement(const Game& game);

// The ending has played out: start a new search.
bool loops(const Game& game);
} // namespace Cows::Ending
