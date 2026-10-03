#pragma once

#include "Level.h"
#include "Props/Props.h"

#include <array>
#include <cstdint>
#include <functional>
#include <random>

namespace Cows
{
// The whole arena is arenaSize on a side, centred on the origin; things are
// placed arenaInset in from its edges, so they spill no further than it.
constexpr auto arenaInset = 14.f;
constexpr auto arenaReach = arenaSize * 0.5f - arenaInset;
constexpr auto clearing = 9.f;
constexpr auto hidingKinds = 5;
constexpr auto nearestHiding = 25.f;

// The seeded draws a level is laid out from, and the hiding places found
// while laying it out. `home` is where the player starts: nothing is put in
// the clearing round it.
struct Layout final
{
    explicit Layout(std::uint32_t seed);

    float unit();
    float between(float from, float to);

    // A random spot in `region`'s placing area (arenaInset in from its
    // edges), outside the clearing.
    Maths::Vec2 spot(const Region& region);

    template <std::size_t Count>
    std::uint32_t pick(const std::array<std::uint32_t, Count>& colors)
    {
        return randomPick(random, colors);
    }

    int index(int count);

    // Scales a whole-arena count to `region`'s share of the arena.
    int share(int count, const Region& region) const;

    void addHiding(Level::Hiding kind, Maths::Vec3 at);

    bool inClearing(Maths::Vec2 point) const;

    // Whether `point` is within `margin` of the critical path.
    bool onPath(Maths::Vec2 point, float margin) const;

    std::mt19937 random;
    std::array<Vector<Maths::Vec3>, hidingKinds> hidings;
    Vector<Maths::Vec2> trees;
    Maths::Vec2 home;
    Region path;
};

bool hasRoom(const Level& level, Maths::Vec3 spot, float margin);

bool amongTrees(const Layout& layout, Maths::Vec3 spot);

// Picks where she hides among the hiding places found in `region`.
void chooseHideout(Level& level, Layout& layout, const Region& region);
} // namespace Cows
