#pragma once

#include "Obstacles.h"

#include <cstdint>

namespace Cows
{
// Find the other cow: the player walks, jumps and climbs the meadow until the
// two meet up on her roof, and the postcard's kiss plays as the ending.
struct Game final
{
    enum class State
    {
        Searching,
        Found
    };

    Game();

    void reset(std::uint32_t newSeed);

    // `ahead` walks forward (1) or back (-1) along the cow's heading; `turn`
    // turns it left (1) or right (-1); `jump` leaps if it is on its feet.
    void update(float delta, float ahead, float turn, bool jump);

    float distance() const;

    // 0 far away from her, 1 right beside her.
    float warmth() const;

    // The ending's clock, on the postcard's timeline.
    float endingSeconds() const;

    // Where the ending plays: the pair's midpoint, turned so the player's cow
    // faces +x.
    Maths::Mat4 stage() const;

    State state = State::Searching;
    std::uint32_t seed = 0;
    Obstacles obstacles;

    Maths::Vec3 player;
    float playerHeading = 0.f;
    float verticalSpeed = 0.f;
    bool grounded = true;
    float bounce = 0.f;
    float hopClock = 0.f;
    float beatClock = 0.f;

    Maths::Vec3 partner;
    float partnerHeading = 0.f;

    Maths::Vec3 stageCenter;
    float stageHeading = 0.f;
    float sinceFound = 0.f;
};

std::uint32_t clockSeed();
} // namespace Cows
