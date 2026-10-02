#include "FrameProfile.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace Cows
{
namespace
{
double millisecondsBetween(FrameProfile::Clock::time_point from,
                           FrameProfile::Clock::time_point to)
{
    return std::chrono::duration<double, std::milli>(to - from).count();
}

std::string fixed(double value, int decimals)
{
    auto text = std::ostringstream {};
    text << std::fixed << std::setprecision(decimals) << value;
    return text.str();
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
        { return fixed(totals[(size_t) part] / frames, 2); };
        auto gpu = gpuSamples > 0 ? fixed(gpuMilliseconds / gpuSamples, 2)
                                  : std::string {"-"};

        LOG("profile fps ",
            fixed(frames / seconds, 1),
            " frame avg ",
            fixed(sum / count, 2),
            " p95 ",
            fixed(p95, 2),
            " max ",
            fixed(sorted[(size_t) count - 1], 2),
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
            " | gpu ms ",
            gpu);
    }

    frames = 0;
    gpuMilliseconds = 0.0;
    gpuSamples = 0;
    totals = {};
    windowStart = now;
}
} // namespace Cows
