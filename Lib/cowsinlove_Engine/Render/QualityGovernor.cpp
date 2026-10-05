#include "Render/QualityGovernor.h"

namespace Cows
{
QualityGovernor::QualityGovernor(const Settings& settingsToUse)
    : settings(settingsToUse)
    , level(settingsToUse.start)
    , toSettle(settingsToUse.settle)
{
}

bool QualityGovernor::addFrame(double gpuMilliseconds)
{
    if (gpuMilliseconds <= 0.0)
        return false;

    if (toSettle > 0)
    {
        --toSettle;
        return false;
    }

    total += gpuMilliseconds;
    ++frames;

    auto full = frames >= settings.window
                || (frames >= settings.minimumWindow
                    && total >= settings.windowMilliseconds);

    if (!full)
        return false;

    auto average = total / frames;
    total = 0.0;
    frames = 0;
    ++windows;

    auto climbing = !cameDown && windows <= settings.climbWindows;
    auto next = level;

    if (average > settings.budget * settings.collapse)
        next = 0;
    else if (average > settings.budget)
        next = level > 0 ? level - 1 : 0;
    else if (climbing && average < settings.headroom && level + 1 < settings.levels)
        next = level + 1;

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
