#include "Animation/Choreography.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;

namespace
{
bool near(float a, float b, float tolerance = 1e-3f)
{
    return std::abs(a - b) < tolerance;
}

template <typename Function>
bool everyStep(float from, float to, Function function)
{
    for (auto seconds = from; seconds < to; seconds += 0.013f)
        if (!function(seconds))
            return false;

    return true;
}
} // namespace

auto tKissSpacing = test("Choreography/kissesAreOneSwayApart") = []
{
    for (auto kiss = 0; kiss < 10; ++kiss)
        check(near(Choreography::kissTime(kiss + 1) - Choreography::kissTime(kiss),
                   Choreography::swayPeriod));
};

auto tKissIsClosest = test("Choreography/kissIsClosest") = []
{
    for (auto kiss = 0; kiss < 5; ++kiss)
    {
        auto time = Choreography::kissTime(kiss);
        check(near(Choreography::closeness(time), 1.f));
        check(Choreography::latestKiss(time + 0.01f) == kiss);
        check(Choreography::latestKiss(time - 0.01f) == kiss - 1);
    }

    check(near(Choreography::closeness(0.f), 0.f));
};

auto tHopsInRange = test("Choreography/hopsStayInRange") = []
{
    for (auto second: {false, true})
    {
        check(everyStep(0.f,
                        20.f,
                        [&](float seconds)
                        {
                            auto hop = Choreography::hopHeight(seconds, second);
                            auto squash =
                                Choreography::landingSquash(seconds, second);
                            return hop >= 0.f && hop <= 1.f && squash >= 0.f
                                   && squash <= 1.f;
                        }));
    }
};

auto tHopsOutOfPhase = test("Choreography/secondCowHopsHalfAHopLater") = []
{
    auto half = Choreography::hopPeriod * 0.5f;

    check(everyStep(0.f,
                    5.f,
                    [&](float seconds)
                    {
                        return near(Choreography::hopHeight(seconds, true),
                                    Choreography::hopHeight(seconds + half, false));
                    }));
};

auto tHeartbeat = test("Choreography/heartbeatIsPeriodicInRange") = []
{
    check(everyStep(0.f,
                    10.f,
                    [](float seconds)
                    {
                        auto beat = Choreography::heartbeat(seconds);
                        auto next = Choreography::heartbeat(
                            seconds + Choreography::heartbeatPeriod);
                        return beat >= -1.f && beat <= 1.f
                               && near(beat, next, 1e-2f);
                    }));
};

auto tSunPulse = test("Choreography/sunPulseIsPeriodicInRange") = []
{
    check(everyStep(0.f,
                    10.f,
                    [](float seconds)
                    {
                        auto pulse = Choreography::sunPulse(seconds);
                        auto next = Choreography::sunPulse(
                            seconds + Choreography::sunPulsePeriod);
                        return pulse >= 0.f && pulse <= 1.f
                               && near(pulse, next, 1e-2f);
                    }));
};

auto tPeriods = test("Choreography/periodsArePositive") = []
{
    check(Choreography::titleWavePeriod > 0.f);
    check(Choreography::burstDuration < Choreography::swayPeriod);
    check(Choreography::kissGap < Choreography::apartGap);
};
