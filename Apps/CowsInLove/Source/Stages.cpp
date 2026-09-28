#include "Stages.h"
#include "Levels/LevelGenerator.h"
#include "Templates.h"

#include <chrono>
#include <cstdlib>

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
    auto* fixed = std::getenv("COWS_STAGE");

    if (fixed == nullptr)
        return 0;

    auto stage = (int) std::strtol(fixed, nullptr, 10);
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
    auto* fixed = std::getenv("COWS_SEED");
    return fixed != nullptr ? (std::uint32_t) std::strtoul(fixed, nullptr, 10)
                            : clockSeed();
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
