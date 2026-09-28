#include "Cow/Moo.h"

#include <ResEmbed/ResEmbed.h>

#include <cstring>

namespace Cows
{
namespace
{
constexpr auto longestCall = 4.5f;
constexpr auto answerPitch = 1.2f;

Vector<float> loadMoo()
{
    auto resource = ResEmbed::get("moo.f32", "Sounds");
    auto count = resource.size() / sizeof(float);

    auto samples = Vector<float> {};
    samples.reserve(count);

    for (std::size_t index = 0; index < count; ++index)
    {
        auto sample = 0.f;
        std::memcpy(&sample, resource.data() + index * sizeof(float), sizeof(float));
        samples.add(sample);
    }

    return samples;
}
} // namespace

MooVoice::MooVoice()
    : moo(loadMoo())
    , player(longestCall)
{
}

void MooVoice::call(const MooAnswer& answer)
{
    player.play(
        moo,
        {{0.f, 1.f, 1.f, 0.f, 0.f},
         {mooAnswerDelay, answerPitch, answer.volume, answer.pan, answer.muffle}});
}
} // namespace Cows
