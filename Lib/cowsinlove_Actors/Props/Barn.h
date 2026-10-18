#pragma once

#include "Props/Scenery.h"

namespace Cows
{
// Somewhere to stand at a barn: where the feet go, and which way to face.
struct BarnSpot final
{
    Maths::Vec3 at;
    float heading = 0.f;
};

// The barn's places, turned and placed as addBarn puts it: the middle of the
// ridge, facing along it; the middle of the loft, facing along it; the middle
// of the doorway in the front wall, facing in; and the top of the ramp up to
// the loft, facing up it.
struct BarnSpots final
{
    BarnSpot roof;
    BarnSpot loft;
    BarnSpot doorway;
    BarnSpot rampTop;
};

BarnSpots barnSpots(Maths::Vec2 center, int quarterTurns);

// How far from its center the barn reaches, turned any way: its climb and
// its open doors included.
float barnReach();

// A gambrel-roofed barn you can walk into through its open front doors, with
// a hay loft over the back half and a ramp up to it inside, and a climb of
// bales up its side onto the roof.
void addBarn(Scenery& scenery, Maths::Vec2 center, int quarterTurns);
} // namespace Cows
