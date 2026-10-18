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
// The least the camera sees across in the ending, so the title fits on a
// phone held upright.
constexpr auto leastWidth = Maths::radians(52.f);
// The loft's title stands just out in front of the gable, over its apex, and
// rises only a little so it never passes over the hay door.
constexpr Maths::Vec3 loftTitleCenter {0.f, 7.f, 2.5f};
constexpr auto loftTitleDrop = 1.5f;
constexpr auto loftTitleScale = 0.55f;
constexpr auto loftCameraHeight = 3.f;
constexpr auto loftCameraDistance = 17.f;
constexpr auto loftCameraPitch = -0.08f;
// Where foundStart stands the player when her spot sets no heading: east of
// her, facing west.
constexpr auto foundStartHeading = Maths::pi;

// How the ending is staged where she hid: where the title stands in the
// stage's frame, its size and how far below that it rises from, and where the
// camera settles: its target's height over the stage, its distance and pitch.
struct Staging final
{
    Maths::Vec3 titleCenter;
    float titleScale = 1.f;
    float titleDrop = 0.f;
    float cameraHeight = 0.f;
    float cameraDistance = 0.f;
    float cameraPitch = 0.f;
};

// Out in the open the title stands well behind the pair; in the loft the
// camera settles out in front of the barn, looking up a little at the pair
// framed in the open hay door, the title just over the gable.
Staging stagingFor(Level::Hiding hiding);

// How far the title has risen into place, sinceFound seconds after she is found.
float titleRise(float sinceFound);

Maths::Mat4 titlePlacement(const Game& game);

// Where the player stands to be found at once: foundStartGap short of her
// along the heading the stage will face.
Maths::Vec3 foundStart(const Game& game);

// The ending has played out: start a new search.
bool loops(const Game& game);
} // namespace Cows::Ending
