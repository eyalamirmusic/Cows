#pragma once

#include "Levels/Biome.h"

#include <memory>
#include <optional>

namespace Cows
{
// A prefab slice of a level, `length` deep along the path and as wide as the
// arena. The generator lays segments one after another, builds each, then
// populates those with a biome.
struct Segment
{
    virtual ~Segment() = default;

    // Lays the segment's own pieces into `region`; `layout.path` runs through
    // it.
    virtual void build(Level&, Layout&, Region) const {}

    float length = 0.f;
    std::optional<Biome> biome;
};

using SegmentList = Vector<std::shared_ptr<const Segment>>;
} // namespace Cows
