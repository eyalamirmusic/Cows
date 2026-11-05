#pragma once

#include "Props/Scenery.h"

namespace Cows
{
// The barn's bones, which its mesh (tools/blender/barn.py, exported to
// Resources/barn.obj) and its collision share: keep the two in step. In barn
// space the front, with the big doors, faces -z; `quarterTurns` turns it.
constexpr auto barnHalfWidth = 6.f;
constexpr auto barnHalfDepth = 8.f;
constexpr auto barnWallThickness = 0.3f;
constexpr auto barnEaves = 5.5f;
constexpr auto barnRidge = 9.5f;
constexpr auto barnDoorHalfWidth = 2.f;
constexpr auto barnDoorHeight = 4.f;

// The hay loft: a floor across the back of the barn, reached by a ramp up
// the +x wall from the front corner.
constexpr auto barnLoftHeight = 3.6f;
constexpr auto barnLoftThickness = 0.3f;
constexpr auto barnLoftDepth = 6.f;
constexpr auto barnRampWidth = 2.2f;

// Where she hides on the loft, in barn space before turning.
constexpr auto barnLoftSpotX = 0.f;
constexpr auto barnLoftSpotZ = barnHalfDepth - 3.f;

Maths::Vec2 turned(Maths::Vec2 offset, int quarterTurns);

// A gambrel-roofed barn you can walk into through its open front doors, with
// a hay loft over the back half and a ramp up to it inside.
void addBarn(Scenery& scenery, Maths::Vec2 center, int quarterTurns);
} // namespace Cows
