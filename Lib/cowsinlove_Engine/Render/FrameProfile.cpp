#include "FrameProfile.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <cstdio>

namespace Cows
{
namespace
{
double millisecondsBetween(FrameProfile::Clock::time_point from,
                           FrameProfile::Clock::time_point to)
{
    return std::chrono::duration<double, std::milli>(to - from).count();
}

std::string formatted(const char* format, double value)
{
    char text[32];
    std::snprintf(text, sizeof(text), format, value);
    return text;
}
} // namespace

FrameProfile::Scope::Scope(FrameProfile& profileToUse, Part partToUse)
    : profile(profileToUse)
    , part(partToUse)
{
    if (profile.enabled)
        start = Clock::now();
}

FrameProfile::Scope::~Scope()
{
    if (profile.enabled)
        profile.add(part, millisecondsBetween(start, Clock::now()));
}

FrameProfile& FrameProfile::shared()
{
    static auto profile = []
    {
        auto made = FrameProfile {};
        made.enabled = !getEnvValue("COWS_PROFILE").empty();
        return made;
    }();

    return profile;
}

void FrameProfile::frameStarted()
{
    if (!enabled)
        return;

    auto now = Clock::now();

    if (!started)
    {
        started = true;
        lastFrame = now;
        windowStart = now;
        return;
    }

    if (frames < maxFrames)
        intervals[(size_t) frames] = millisecondsBetween(lastFrame, now);

    ++frames;
    lastFrame = now;

    auto& timings = Device::shared().lastFrameTimings();

    if (timings.frameIndex != 0)
    {
        gpuMilliseconds += timings.milliseconds;
        ++gpuSamples;
    }

    if (millisecondsBetween(windowStart, now) >= 1000.0)
        report(now);
}

void FrameProfile::add(Part part, double milliseconds)
{
    if (enabled)
        totals[(size_t) part] += milliseconds;
}

void FrameProfile::report(Clock::time_point now)
{
    auto count = std::min(frames, maxFrames);

    if (count > 0)
    {
        auto sorted = intervals;
        auto end = sorted.begin() + count;
        std::sort(sorted.begin(), end);

        auto sum = 0.0;
        for (auto at = sorted.begin(); at != end; ++at)
            sum += *at;

        auto p95 = sorted[(size_t) std::min(count - 1, (count * 95) / 100)];
        auto seconds = millisecondsBetween(windowStart, now) / 1000.0;
        auto perFrame = [&](Part part)
        { return formatted("%.2f", totals[(size_t) part] / frames); };
        auto gpu = gpuSamples > 0 ? formatted("%.2f", gpuMilliseconds / gpuSamples)
                                  : std::string {"-"};

        LOG("profile fps ",
            formatted("%.1f", frames / seconds),
            " frame avg ",
            formatted("%.2f", sum / count),
            " p95 ",
            formatted("%.2f", p95),
            " max ",
            formatted("%.2f", sorted[(size_t) count - 1]),
            " | cpu ms update ",
            perFrame(Part::Update),
            " gather ",
            perFrame(Part::Gather),
            " shadows ",
            perFrame(Part::Shadows),
            " scene ",
            perFrame(Part::Scene),
            " hud ",
            perFrame(Part::Hud),
            " hud repaints ",
            hudRepaints,
            " | gpu ms ",
            gpu);
    }

    frames = 0;
    hudRepaints = 0;
    gpuMilliseconds = 0.0;
    gpuSamples = 0;
    totals = {};
    windowStart = now;
}
} // namespace Cows
