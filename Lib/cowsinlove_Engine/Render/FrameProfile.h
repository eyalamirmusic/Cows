#pragma once

#include "Common.h"

#include <array>
#include <chrono>
#include <string>

namespace Cows
{
// COWS_PROFILE=1: once a second, logs the frame interval (average, p95, max),
// frames per second, the CPU time of each part of a frame, and the GPU time
// of the frame and of each labelled pass. Off, every call is one branch.
struct FrameProfile final
{
    enum class Part
    {
        Update,
        Gather,
        Shadows,
        Scene,
        Hud,
        Count
    };

    using Clock = std::chrono::steady_clock;

    struct Scope final
    {
        Scope(FrameProfile& profileToUse, Part partToUse);
        ~Scope();

        FrameProfile& profile;
        Part part;
        Clock::time_point start;
    };

    static FrameProfile& shared();

    void frameStarted();
    void add(Part part, double milliseconds);

    void report(Clock::time_point now);
    std::string passesText() const;

    static constexpr auto maxFrames = 1024;
    static constexpr auto maxPasses = 4;

    std::array<double, maxFrames> intervals {};
    std::array<double, (int) Part::Count> totals {};
    int frames = 0;
    double gpuMilliseconds = 0.0;
    std::array<double, maxPasses> gpuPasses {};
    std::array<std::string, maxPasses> gpuPassLabels {};
    int gpuSamples = 0;
    Clock::time_point lastFrame;
    Clock::time_point windowStart;
    bool started = false;
    bool enabled = false;
};
} // namespace Cows
