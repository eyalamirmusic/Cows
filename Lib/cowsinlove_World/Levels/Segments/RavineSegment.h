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

struct RavineSegment final : Segment
{
    explicit RavineSegment(float lengthToUse = ravineLength);

    void build(Level& level, Layout& layout, Region region) const override;

    static Vector<MoverSpawner> baleSpawners(Maths::Vec2 bridge, Layout& layout);
};
} // namespace Cows
