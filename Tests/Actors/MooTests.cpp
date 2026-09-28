#include "Cow/Moo.h"

#include <ResEmbed/ResEmbed.h>

#include <NanoTest/NanoTest.h>

#include <cmath>
#include <cstring>

using namespace nano;
using namespace Cows;

auto tMooEmbedded = test("Moo/sampleIsEmbedded") = []
{
    auto resource = ResEmbed::get("moo.f32", "Sounds");
    auto count = resource.size() / sizeof(float);
    auto expected = (double) mooLength * SamplePlayer::sampleRate;

    check(count > 0);
    check(std::abs((double) count - expected) < expected * 0.02);

    auto inRange = true;
    auto loudest = 0.f;

    for (std::size_t index = 0; index < count; ++index)
    {
        auto sample = 0.f;
        std::memcpy(&sample, resource.data() + index * sizeof(float), sizeof(float));
        inRange = inRange && std::isfinite(sample) && std::abs(sample) <= 1.f;
        loudest = std::max(loudest, std::abs(sample));
    }

    check(inRange);
    check(loudest > 0.5f);
};
