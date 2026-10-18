#include "Stages.h"
#include "Levels/LevelGenerator.h"
#include "Templates.h"

#include <chrono>

namespace Cows
{
namespace
{
int wrapped(int stage, int count)
{
    return ((stage % count) + count) % count;
}
} // namespace

Stages::Stages(int stage)
    : templates {meadowTemplate(), meadowRavineTemplate()}
    , current(wrapped(stage, templates.size()))
{
}

void Stages::advance()
{
    current = (current + 1) % templates.size();
}

LevelMaker Stages::level() const
{
    return [levelTemplate = templates[current]](std::uint32_t seed)
    { return generate(levelTemplate, seed); };
}

std::uint32_t clockSeed()
{
    return (std::uint32_t) std::chrono::steady_clock::now()
        .time_since_epoch()
        .count();
}
} // namespace Cows
