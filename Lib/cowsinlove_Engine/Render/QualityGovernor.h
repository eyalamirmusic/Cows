#pragma once

namespace Cows
{
struct QualityGovernor final
{
    struct Settings
    {
        int levels = 3;
        // Unknown hardware starts low and climbs once it proves headroom.
        int start = 0;
        double budget = 25.0;
        double collapse = 3.0;
        double headroom = 8.0;
        int window = 30;
        double windowMilliseconds = 1000.0;
        int minimumWindow = 3;
        // Skipped after a change: the GPU still reports frames drawn before it.
        int settle = 5;
        int climbWindows = 2;
        int decideWindows = 3;
    };

    QualityGovernor() = default;
    explicit QualityGovernor(const Settings& settingsToUse);

    // Returns whether the level changed.
    bool addFrame(double gpuMilliseconds);

    bool decided() const;

    Settings settings;
    int level = settings.start;
    int toSettle = settings.settle;
    int frames = 0;
    double total = 0.0;
    int windows = 0;
    int steadyWindows = 0;
    // Never climbs again after, so it cannot see-saw.
    bool cameDown = false;
};
} // namespace Cows
