#pragma once

namespace Cows::Choreography
{
// Every rhythm of the original, measured off its frames, in seconds.
constexpr auto swayPeriod = 5.72f;
constexpr auto hopPeriod = 0.4f;
constexpr auto heartbeatPeriod = 0.8f;
constexpr auto sunPulsePeriod = 0.67f;
constexpr auto titleWavePeriod = 1.7f;
constexpr auto burstDuration = 2.4f;

// The two cows' gap at its closest and at its widest, in the original's ratio.
constexpr auto kissGap = 18.f;
constexpr auto apartGap = 26.f;

// 0 when the cows are furthest apart, 1 at the kiss.
float closeness(float seconds);

// The kiss is the middle of every sway cycle.
float kissTime(int kiss);
int latestKiss(float seconds);

// 0 on the ground, 1 at the top of a hop; `second` is the other cow, half a
// hop out of phase.
float hopHeight(float seconds, bool second);

// How hard a cow is landing: 1 at touchdown, falling to 0 in the air.
float landingSquash(float seconds, bool second);

// The heart eyes' beat, -1 (the hollow ♡) to 1 (the big ❤).
float heartbeat(float seconds);

// The sun's pulse, 0 to 1.
float sunPulse(float seconds);
} // namespace Cows::Choreography
