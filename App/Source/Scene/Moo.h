#pragma once

#include "Common.h"

#include <AudioToolbox/AudioToolbox.h>

namespace Cows
{
// Where her answer comes from: `pan` -1 (left) to 1 (right), `volume` falling
// with distance, `muffle` 0 in front to 1 right behind.
struct MooAnswer final
{
    float pan = 0.f;
    float volume = 1.f;
    float muffle = 0.f;
};

// A synthesised moo, and the other cow mooing back from where she is.
struct MooVoice final
{
    MooVoice();
    ~MooVoice();

    MooVoice(const MooVoice&) = delete;
    MooVoice& operator=(const MooVoice&) = delete;

    void call(const MooAnswer& answer);

    AudioQueueRef queue = nullptr;
};

// How long after the player's moo she answers, in seconds.
constexpr auto mooAnswerDelay = 1.1f;
constexpr auto mooLength = 1.1f;
} // namespace Cows
