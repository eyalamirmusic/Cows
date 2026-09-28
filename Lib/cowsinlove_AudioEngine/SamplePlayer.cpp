#include "SamplePlayer.h"

#include <eacp/Core/Maths/Constants.h>

#include <algorithm>
#include <cmath>

using namespace eacp;
using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto sampleRate = SamplePlayer::sampleRate;
constexpr auto channels = SamplePlayer::channels;
constexpr auto openCutoff = 9000.f;
constexpr auto muffledCutoff = 900.f;

float sampleAt(const Vector<float>& moo, float position)
{
    auto index = (int) position;

    if (index + 1 >= moo.size())
        return 0.f;

    auto fraction = position - (float) index;
    return moo[index] + (moo[index + 1] - moo[index]) * fraction;
}

std::size_t mixFrames(float longestSeconds)
{
    return (std::size_t) (longestSeconds * (float) sampleRate);
}

MakeASound::SessionConfig ambientSession()
{
    auto session = MakeASound::SessionConfig {};
    session.category = MakeASound::SessionCategory::Ambient;
    session.options.mixWithOthers = true;
    return session;
}
} // namespace

void addVoice(float* samples,
              std::size_t frameCount,
              const Vector<float>& moo,
              const SampleVoice& voice)
{
    auto first = (std::size_t) (voice.start * (float) sampleRate);
    auto length = (std::size_t) ((float) moo.size() / voice.pitch);
    auto cutoff = openCutoff + (muffledCutoff - openCutoff) * voice.muffle;
    auto smoothing = 1.f - std::exp(-twoPi * cutoff / (float) sampleRate);
    auto left = std::sqrt(0.5f * (1.f - voice.pan)) * voice.volume;
    auto right = std::sqrt(0.5f * (1.f + voice.pan)) * voice.volume;
    auto low = 0.f;

    for (std::size_t index = 0; index < length && first + index < frameCount;
         ++index)
    {
        low += (sampleAt(moo, (float) index * voice.pitch) - low) * smoothing;

        auto* frame = samples + (first + index) * channels;
        frame[0] += low * left;
        frame[1] += low * right;
    }
}

SamplePlayer::SamplePlayer(float longestSeconds)
{
    mix.resize((int) (mixFrames(longestSeconds) * channels));

    auto config = device.getDefaultOutputConfig();

    if (!config.output)
        return;

    config.sampleRate = (int) sampleRate;
    device.setSessionConfig(ambientSession());
    open = device.start(
               config, [this](MakeASound::AudioCallbackInfo& info) { render(info); })
           == MakeASound::Error::NoError;
}

SamplePlayer::~SamplePlayer()
{
    device.stop();
}

void SamplePlayer::render(MakeASound::AudioCallbackInfo& info)
{
    auto output = info.getOutput();

    for (auto channel: output)
        channel.fill(0.f);

    auto lock = std::unique_lock {mutex, std::try_to_lock};

    if (!lock.owns_lock() || playedFrames >= usedFrames)
        return;

    auto frames = std::min((std::size_t) info.numSamples, usedFrames - playedFrames);
    auto outputs = output.getNumChannels();

    for (auto index = 0; index < outputs; ++index)
    {
        auto channel = output.getChannel(index);
        auto source = std::min(index, channels - 1);

        for (std::size_t frame = 0; frame < frames; ++frame)
            channel[(int) frame] = mix[(int) ((playedFrames + frame) * channels
                                              + (std::size_t) source)];
    }

    playedFrames += frames;
}

void SamplePlayer::play(const Vector<float>& sample,
                        std::initializer_list<SampleVoice> voices)
{
    if (sample.empty())
        return;

    auto lock = std::lock_guard {mutex};
    auto frameCount = (std::size_t) mix.size() / channels;
    std::fill(mix.begin(), mix.end(), 0.f);

    auto end = 0.f;

    for (const auto& voice: voices)
    {
        addVoice(mix.data(), frameCount, sample, voice);
        end = std::max(
            end,
            voice.start + (float) sample.size() / voice.pitch / (float) sampleRate);
    }

    usedFrames = std::min(frameCount, (std::size_t) (end * sampleRate));
    playedFrames = 0;
}
} // namespace Cows
