#pragma once

namespace Cows::Choreography
{
// In seconds, measured off the original's frames.
constexpr auto swayPeriod = 5.72f;
constexpr auto hopPeriod = 0.4f;
constexpr auto heartbeatPeriod = 0.8f;
constexpr auto sunPulsePeriod = 0.67f;
constexpr auto titleWavePeriod = 1.7f;
constexpr auto burstDuration = 2.4f;

constexpr auto kissGap = 18.f;
constexpr auto apartGap = 26.f;

// 0 when furthest apart, 1 at the kiss.
float closeness(float seconds);

float kissTime(int kiss);
int latestKiss(float seconds);

// 0 on the ground, 1 at the top; `second` is half a hop out of phase.
float hopHeight(float seconds, bool second);

// 1 at touchdown, falling to 0 in the air.
float landingSquash(float seconds, bool second);

// -1 (the hollow ♡) to 1 (the big ❤).
float heartbeat(float seconds);

// 0 to 1.
float sunPulse(float seconds);
} // namespace Cows::Choreography
