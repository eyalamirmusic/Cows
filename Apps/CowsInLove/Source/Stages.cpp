#include "Stages.h"
#include "Levels/LevelGenerator.h"
#include "Templates.h"

#include <eacp/Core/Utils/Environment.h>

#include <chrono>

namespace Cows
{
namespace
{
std::uint32_t clockSeed()
{
    return (std::uint32_t) std::chrono::steady_clock::now()
        .time_since_epoch()
        .count();
}

int firstStage(int count)
{
    auto fixed = getEnvValue("COWS_STAGE");

    if (fixed.empty())
        return 0;

    auto stage = std::stoi(fixed);
    return ((stage % count) + count) % count;
}
} // namespace

Stages::Stages()
    : templates {meadowTemplate(), meadowRavineTemplate()}
    , current(firstStage(templates.size()))
{
}

std::uint32_t Stages::firstSeed() const
{
    auto fixed = getEnvValue("COWS_SEED");
    return fixed.empty() ? clockSeed() : (std::uint32_t) std::stoul(fixed);
}

std::uint32_t Stages::nextSeed() const
{
    return clockSeed();
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
} // namespace Cows
