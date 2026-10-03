#pragma once

#include "Levels/Segments/MoverSpawner.h"
#include "Levels/Segments/Segment.h"

#include <array>

namespace Cows
{
constexpr auto ravineLength = 18.f;
constexpr auto ravineDepth = -30.f;
constexpr auto ravineKillDepth = -12.f;
constexpr auto ravineHalfDepth = 6.5f;
constexpr auto ravineJitter = 1.f;
constexpr auto bridgeHalfWidth = 1.5f;
constexpr auto bridgeOverlap = 1.f;
constexpr auto baleSweep = 7.f;
constexpr auto baleLaneSpacing = 4.6f;
constexpr auto baleLanePeriods = std::array {4.6f, 5.8f, 5.2f};

// A ravine across the arena from side to side, ravineDepth deep, with one
// plank bridge over it on the critical path, and bales rolling across the
// bridge on beams: cross between them, or be knocked off.
struct RavineSegment final : Segment
{
    explicit RavineSegment(float lengthToUse = ravineLength);

    void build(Level& level, Layout& layout, Region region) const override;

    // The bales' spawners for a bridge at `bridge`, their phases drawn from
    // the layout.
    static Vector<MoverSpawner> baleSpawners(Maths::Vec2 bridge, Layout& layout);
};
} // namespace Cows
