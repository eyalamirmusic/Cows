#pragma once

#include <eacp/Core/Utils/Containers.h>

#include <MakeASound/MakeASound.h>

#include <cstddef>
#include <initializer_list>
#include <mutex>

namespace Cows
{
// One playing of a mono sample in a stereo mix: from `start` seconds, at
// `pitch`, `pan` -1 (left) to 1 (right), `muffle` 0 open to 1 low-passed.
struct SampleVoice final
{
    float start = 0.f;
    float pitch = 1.f;
    float volume = 1.f;
    float pan = 0.f;
    float muffle = 0.f;
};

// Mixes `voice` into interleaved stereo `samples`, `frameCount` frames long.
void addVoice(float* samples,
              std::size_t frameCount,
              const eacp::Vector<float>& moo,
              const SampleVoice& voice);

// A mono float sample played through the output device, once per voice, in a
// buffer long enough for `longestSeconds` of sound. With no device it stays
// silent.
struct SamplePlayer final
{
    static constexpr auto sampleRate = 44100.0;
    static constexpr auto channels = 2;

    explicit SamplePlayer(float longestSeconds);
    ~SamplePlayer();

    SamplePlayer(const SamplePlayer&) = delete;
    SamplePlayer& operator=(const SamplePlayer&) = delete;

    // Stops whatever is playing and plays the voices of `sample` together.
    void play(const eacp::Vector<float>& sample,
              std::initializer_list<SampleVoice> voices);

    // Fills the device's block from the mix, silence when there is none.
    void render(MakeASound::AudioCallbackInfo& info);

    std::mutex mutex;
    eacp::Vector<float> mix;
    std::size_t usedFrames = 0;
    std::size_t playedFrames = 0;
    MakeASound::DeviceManager device;
    bool open = false;
};
} // namespace Cows
