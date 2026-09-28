#include "Animation/Choreography.h"
#include "Render/Common.h"

#include <cmath>

using namespace Maths;

namespace Cows::Choreography
{
namespace
{
float phaseOf(float seconds, float period, float offset = 0.f)
{
    auto cycles = seconds / period + offset;
    return cycles - std::floor(cycles);
}

float hopPhase(float seconds, bool second)
{
    return phaseOf(seconds, hopPeriod, second ? 0.5f : 0.f);
}
} // namespace

float closeness(float seconds)
{
    return 0.5f - 0.5f * std::cos(twoPi * seconds / swayPeriod);
}

float kissTime(int kiss)
{
    return ((float) kiss + 0.5f) * swayPeriod;
}

int latestKiss(float seconds)
{
    return (int) std::floor(seconds / swayPeriod - 0.5f);
}

float hopHeight(float seconds, bool second)
{
    auto phase = hopPhase(seconds, second);
    return 4.f * phase * (1.f - phase);
}

float landingSquash(float seconds, bool second)
{
    auto phase = hopPhase(seconds, second);
    auto fromGround = std::min(phase, 1.f - phase);
    return std::exp(-fromGround * fromGround / 0.006f);
}

float heartbeat(float seconds)
{
    auto wave = std::sin(twoPi * (phaseOf(seconds, heartbeatPeriod) - 0.05f));
    return std::copysign(std::pow(std::abs(wave), 0.6f), wave);
}

float sunPulse(float seconds)
{
    return 0.5f + 0.5f * std::cos(twoPi * phaseOf(seconds, sunPulsePeriod));
}
} // namespace Cows::Choreography
