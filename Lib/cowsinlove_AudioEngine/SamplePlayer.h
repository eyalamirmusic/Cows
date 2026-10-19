#pragma once

#include <eacp/Core/Utils/Containers.h>

#include <MakeASound/MakeASound.h>

#include <cstddef>
#include <initializer_list>
#include <mutex>

namespace Cows
{
// `start` in seconds; `pan` -1 (left) to 1 (right); `muffle` 0 open to 1 muffled.
struct SampleVoice final
{
    float start = 0.f;
    float pitch = 1.f;
    float volume = 1.f;
    float pan = 0.f;
    float muffle = 0.f;
};

// `samples` is interleaved stereo, `frameCount` frames long.
void addVoice(float* samples,
              std::size_t frameCount,
              const eacp::Vector<float>& moo,
              const SampleVoice& voice);

// Silent with no device; stops while the app is in the background.
struct SamplePlayer final
{
    static constexpr auto sampleRate = 44100.0;
    static constexpr auto channels = 2;

    explicit SamplePlayer(float longestSeconds);
    ~SamplePlayer();

    SamplePlayer(const SamplePlayer&) = delete;
    SamplePlayer& operator=(const SamplePlayer&) = delete;

    void play(const eacp::Vector<float>& sample,
              std::initializer_list<SampleVoice> voices);

    void start();

    void render(MakeASound::AudioCallbackInfo& info);

    std::mutex mutex;
    eacp::Vector<float> mix;
    std::size_t usedFrames = 0;
    std::size_t playedFrames = 0;
    MakeASound::DeviceManager device;
    bool open = false;
};
} // namespace Cows
