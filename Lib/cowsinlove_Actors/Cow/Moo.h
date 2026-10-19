#pragma once

#include "Render/Common.h"
#include "SamplePlayer.h"

namespace Cows
{
// `pan` -1 (left) to 1 (right); `muffle` 0 in front to 1 right behind.
struct MooAnswer final
{
    float pan = 0.f;
    float volume = 1.f;
    float muffle = 0.f;
};

struct MooVoice final
{
    MooVoice();

    void call(const MooAnswer& answer);

    Vector<float> moo;
    SamplePlayer player;
};

// In seconds.
constexpr auto mooAnswerDelay = 1.7f;
constexpr auto mooLength = 1.5f;
} // namespace Cows
