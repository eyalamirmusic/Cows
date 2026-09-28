#include "Templates.h"
#include "Levels/Segments/JumpLineSegment.h"
#include "Levels/Segments/MeadowSegment.h"
#include "Levels/Segments/RavineSegment.h"

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto startMeadow = 36.f;
constexpr auto jumpLineGap = 10.f;
constexpr auto goalMeadow = 106.f;
constexpr auto meadowRavinePathReach = 40.f;

std::shared_ptr<const Segment> jumpLine(JumpLineSegment::Obstacle obstacle)
{
    return std::make_shared<JumpLineSegment>(obstacle, jumpLineGap);
}
} // namespace

LevelTemplate meadowTemplate()
{
    auto levelTemplate = LevelTemplate {};
    levelTemplate.segments.add(std::make_shared<MeadowSegment>(arenaSize));
    return levelTemplate;
}

LevelTemplate meadowRavineTemplate()
{
    using Obstacle = JumpLineSegment::Obstacle;

    auto levelTemplate = LevelTemplate {};
    levelTemplate.segments = {std::make_shared<MeadowSegment>(startMeadow),
                              jumpLine(Obstacle::Log),
                              jumpLine(Obstacle::Fence),
                              std::make_shared<RavineSegment>(),
                              jumpLine(Obstacle::Fence),
                              jumpLine(Obstacle::Log),
                              std::make_shared<MeadowSegment>(goalMeadow)};
    levelTemplate.pathReach = meadowRavinePathReach;
    levelTemplate.startHeading = halfPi;
    return levelTemplate;
}
} // namespace Cows
