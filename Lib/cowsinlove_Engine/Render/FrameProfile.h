#pragma once

#include "Common.h"

#include <array>
#include <chrono>

namespace Cows
{
// COWS_PROFILE=1: once a second, logs the frame interval (average, p95, max),
// frames per second, and the CPU time of each part of a frame. Off, every call
// is one branch.
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
    void hudRepainted() { ++hudRepaints; }

    void report(Clock::time_point now);

    static constexpr auto maxFrames = 1024;

    std::array<double, maxFrames> intervals {};
    std::array<double, (int) Part::Count> totals {};
    int frames = 0;
    int hudRepaints = 0;
    double gpuMilliseconds = 0.0;
    int gpuSamples = 0;
    Clock::time_point lastFrame;
    Clock::time_point windowStart;
    bool started = false;
    bool enabled = false;
};
} // namespace Cows
