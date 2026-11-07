#include "Render/QualityGovernor.h"

#include <algorithm>

namespace Cows
{
QualityGovernor::QualityGovernor(const Settings& settingsToUse)
    : settings(settingsToUse)
    , level(settingsToUse.start)
    , toSettle(settingsToUse.settle)
{
}

bool QualityGovernor::consumeSettleFrame()
{
    if (toSettle <= 0)
        return false;

    --toSettle;
    return true;
}

bool QualityGovernor::hasReachedMinimumWindowTime() const
{
    return frames >= settings.minimumWindow && total >= settings.windowMilliseconds;
}

bool QualityGovernor::isWindowFull() const
{
    return frames >= settings.window || hasReachedMinimumWindowTime();
}

double QualityGovernor::takeWindowAverage()
{
    auto average = total / frames;
    total = 0.0;
    frames = 0;
    ++windows;
    return average;
}

bool QualityGovernor::isStillClimbing() const
{
    return !cameDown && windows <= settings.climbWindows;
}

bool QualityGovernor::shouldClimb(double average) const
{
    return isStillClimbing() && average < settings.headroom
           && level + 1 < settings.levels;
}

int QualityGovernor::chooseNextLevel(double average) const
{
    if (average > settings.budget * settings.collapse)
        return 0;

    if (average > settings.budget)
        return std::max(level - 1, 0);

    if (shouldClimb(average))
        return level + 1;

    return level;
}

bool QualityGovernor::addFrame(double gpuMilliseconds)
{
    if (gpuMilliseconds <= 0.0 || consumeSettleFrame())
        return false;

    total += gpuMilliseconds;
    ++frames;

    if (!isWindowFull())
        return false;

    auto next = chooseNextLevel(takeWindowAverage());

    if (next == level)
    {
        ++steadyWindows;
        return false;
    }

    cameDown = cameDown || next < level;
    level = next;
    steadyWindows = 0;
    toSettle = settings.settle;
    return true;
}

bool QualityGovernor::decided() const
{
    return steadyWindows >= settings.decideWindows;
}
} // namespace Cows
