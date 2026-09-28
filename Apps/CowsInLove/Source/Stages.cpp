#include "Stages.h"

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
} // namespace

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
} // namespace Cows
