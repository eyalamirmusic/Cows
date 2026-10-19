#pragma once

#include "Level.h"
#include "Props/Props.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>

namespace Cows
{
// Props sit this far in from the arena's edges so they never spill past it.
constexpr auto arenaInset = 14.f;
constexpr auto arenaReach = arenaSize * 0.5f - arenaInset;
constexpr auto clearing = 9.f;
constexpr auto hidingKinds = 6;
constexpr auto nearestHiding = 25.f;

struct HidingSpot final
{
    Maths::Vec3 at;
    std::optional<float> heading;
};

struct Layout final
{
    explicit Layout(std::uint32_t seed);

    float unit();
    float between(float from, float to);

    Maths::Vec2 spot(const Region& region);

    template <std::size_t Count>
    std::uint32_t pick(const std::array<std::uint32_t, Count>& colors)
    {
        return randomPick(random, colors);
    }

    int index(int count);

    int share(int count, const Region& region) const;

    void addHiding(Level::Hiding kind,
                   Maths::Vec3 at,
                   std::optional<float> heading = std::nullopt);

    bool inClearing(Maths::Vec2 point) const;

    bool onPath(Maths::Vec2 point, float margin) const;

    std::mt19937 random;
    std::array<Vector<HidingSpot>, hidingKinds> hidings;
    Vector<Maths::Vec2> trees;
    Maths::Vec2 home;
    Region path;
};

bool hasRoom(const Level& level, Maths::Vec3 spot, float margin);

bool amongTrees(const Layout& layout, Maths::Vec3 spot);

void chooseHideout(Level& level, Layout& layout, const Region& region);
} // namespace Cows
