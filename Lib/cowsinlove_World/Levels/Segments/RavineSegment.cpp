#include "Levels/Segments/RavineSegment.h"
#include "Props/Bale.h"
#include "Props/Bridge.h"

using namespace Maths;

namespace Cows
{
RavineSegment::RavineSegment(float lengthToUse)
{
    length = lengthToUse;
}

Vector<MoverSpawner> RavineSegment::baleSpawners(Vec2 bridge, Layout& layout)
{
    auto spawners = Vector<MoverSpawner> {};
    auto lane = 0;

    for (auto offset: {-baleLaneSpacing, 0.f, baleLaneSpacing})
    {
        auto z = bridge.y + offset;
        spawners.add({{bridge.x - baleSweep, z},
                      {bridge.x + baleSweep, z},
                      baleLanePeriods[lane++],
                      layout.unit()});
    }

    return spawners;
}

void RavineSegment::build(Level& level, Layout& layout, Region region) const
{
    auto middle = region.center();
    auto center =
        Vec2 {middle.x, middle.y + layout.between(-ravineJitter, ravineJitter)};
    auto bridge = Vec2 {layout.path.center().x, center.y};

    level.gaps.add({center, {region.size().x * 0.5f, ravineHalfDepth}, ravineDepth});
    level.killDepth = ravineKillDepth;

    addBridge(level, bridge, {bridgeHalfWidth, ravineHalfDepth + bridgeOverlap});

    for (const auto& spawner: baleSpawners(bridge, layout))
    {
        addBridgeBeam(level,
                      {bridge.x, spawner.from.y},
                      (baleSweep + rollingBaleRadius) * 2.f);
        spawner.spawn(level);
    }
}
} // namespace Cows
