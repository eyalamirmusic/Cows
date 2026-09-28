#include "Levels/LevelGenerator.h"

using namespace Maths;

namespace Cows
{
namespace
{
Region criticalPath(const Vector<Region>& regions, float pathX, float startZ)
{
    auto goalEdge = regions.back().max.y;

    if (startZ <= goalEdge)
        return {};

    return {{pathX - pathHalfWidth, goalEdge}, {pathX + pathHalfWidth, startZ}};
}
} // namespace

Vector<Region> segmentRegions(const LevelTemplate& levelTemplate)
{
    auto total = 0.f;

    for (const auto& segment: levelTemplate.segments)
        total += segment->length;

    auto half = levelTemplate.width * 0.5f;
    auto south = total * 0.5f;
    auto regions = Vector<Region> {};

    for (const auto& segment: levelTemplate.segments)
    {
        regions.add(Region {{-half, south - segment->length}, {half, south}});
        south -= segment->length;
    }

    return regions;
}

Level generate(const LevelTemplate& levelTemplate, std::uint32_t seed)
{
    auto level = Level {};
    auto layout = Layout {seed * 2654435761u + 17u};
    auto regions = segmentRegions(levelTemplate);

    if (regions.empty())
        return level;

    auto reach = levelTemplate.pathReach;
    auto pathX = reach > 0.f ? layout.between(-reach, reach) : 0.f;
    auto startZ = regions.front().center().y;

    layout.home = {pathX, startZ};
    layout.path = criticalPath(regions, pathX, startZ);
    level.start = {pathX, 0.f, startZ};
    level.startHeading = levelTemplate.startHeading;
    level.criticalPath = layout.path;

    const auto& segments = levelTemplate.segments;

    for (auto index = 0; index < segments.size(); ++index)
        segments[index]->build(level, layout, regions[index]);

    for (auto index = 0; index < segments.size(); ++index)
        if (segments[index]->biome)
            populate(level, layout, regions[index], *segments[index]->biome);

    chooseHideout(level, layout, regions.back());
    level.update(0.f);
    return level;
}
} // namespace Cows
