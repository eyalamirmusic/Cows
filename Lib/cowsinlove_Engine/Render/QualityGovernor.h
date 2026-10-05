#pragma once

namespace Cows
{
// Chooses a quality level, 0 the lowest, from what frames cost the GPU.
//
// It averages a window of frames at a time - `window` frames, or fewer once
// they add up to `windowMilliseconds`, so a slow GPU is judged as quickly as a
// fast one. A window over `budget` takes a level off, or every level when it
// is over `budget * collapse`, sparing a GPU that cannot cope the levels in
// between. While the governor is still climbing (within `climbWindows` of
// starting, never having come down) a window under `headroom` adds one. Once
// it has come down it never goes back up, so it cannot see-saw. `settle`
// frames after a change are left out, since the GPU is still reporting
// frames drawn before it, and so are frames that report no GPU time: a device
// without timestamps keeps the level it started at.
struct QualityGovernor final
{
    struct Settings
    {
        int levels = 3;
        int start = 2;
        double budget = 25.0;
        double collapse = 3.0;
        double headroom = 8.0;
        int window = 30;
        double windowMilliseconds = 1000.0;
        int minimumWindow = 3;
        int settle = 5;
        int climbWindows = 2;
        int decideWindows = 3;
    };

    QualityGovernor() = default;
    explicit QualityGovernor(const Settings& settingsToUse);

    // Returns whether the level changed.
    bool addFrame(double gpuMilliseconds);

    // Whether the level has held for decideWindows windows in a row.
    bool decided() const;

    Settings settings;
    int level = settings.start;
    int toSettle = settings.settle;
    int frames = 0;
    double total = 0.0;
    int windows = 0;
    int steadyWindows = 0;
    bool cameDown = false;
};
} // namespace Cows
