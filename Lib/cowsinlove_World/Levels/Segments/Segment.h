#pragma once

#include "Levels/Biome.h"

#include <memory>
#include <optional>

namespace Cows
{
struct Segment
{
    virtual ~Segment() = default;

    virtual void build(Level&, Layout&, Region) const {}

    float length = 0.f;
    std::optional<Biome> biome;
};

using SegmentList = Vector<std::shared_ptr<const Segment>>;
} // namespace Cows
