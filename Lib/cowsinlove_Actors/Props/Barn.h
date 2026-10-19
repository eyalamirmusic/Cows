#pragma once

#include "Props/Scenery.h"

namespace Cows
{
struct BarnSpot final
{
    Maths::Vec3 at;
    float heading = 0.f;
};

struct BarnSpots final
{
    BarnSpot roof;
    BarnSpot loft;
    BarnSpot doorway;
    BarnSpot rampTop;
};

BarnSpots barnSpots(Maths::Vec2 center, int quarterTurns);

// At any turn, its climb and open doors included.
float barnReach();

void addBarn(Scenery& scenery, Maths::Vec2 center, int quarterTurns);
} // namespace Cows
