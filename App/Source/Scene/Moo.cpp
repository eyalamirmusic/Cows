#include "Moo.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto sampleRate = 44100.0;
constexpr auto channels = 2;
constexpr auto tailLength = 0.3f;

struct Voice final
{
    float start = 0.f;
    float pitch = 1.f;
    float volume = 1.f;
    float pan = 0.f;
    float muffle = 0.f;
};

float envelope(float time)
{
    auto attack = std::clamp(time / 0.08f, 0.f, 1.f);
    auto release = std::clamp((mooLength - time) / 0.35f, 0.f, 1.f);
    return attack * release;
}

float fundamental(float time)
{
    auto rise = std::clamp(time / 0.2f, 0.f, 1.f);
    auto fall = std::clamp((time - 0.2f) / (mooLength - 0.2f), 0.f, 1.f);
    auto glide = 150.f + 22.f * rise - 65.f * fall;
    return glide * (1.f + 0.015f * std::sin(twoPi * 5.f * time));
}

float mouth(float time)
{
    auto open = std::sin(pi * std::clamp(time / mooLength, 0.f, 1.f));
    return 350.f + 650.f * open;
}

void addVoice(float* samples, std::size_t sampleCount, const Voice& voice)
{
    auto first = (std::size_t) (voice.start * (float) sampleRate);
    auto count = (std::size_t) (mooLength * (float) sampleRate);
    auto phase = 0.f;
    auto low = 0.f;
    auto lower = 0.f;
    auto left = std::sqrt(0.5f * (1.f - voice.pan));
    auto right = std::sqrt(0.5f * (1.f + voice.pan));

    for (std::size_t index = 0; index < count; ++index)
    {
        auto time = (float) index / (float) sampleRate;
        phase += fundamental(time) * voice.pitch / (float) sampleRate;
        phase -= std::floor(phase);

        auto saw = 2.f * phase - 1.f;
        auto cutoff = mouth(time) * (1.f - 0.6f * voice.muffle);
        auto smoothing = 1.f - std::exp(-twoPi * cutoff / (float) sampleRate);
        low += (saw - low) * smoothing;
        lower += (low - lower) * smoothing;

        auto sample = lower * envelope(time) * voice.volume * 0.5f;
        auto at = (first + index) * channels;

        if (at + 1 >= sampleCount)
            break;

        samples[at] += sample * left;
        samples[at + 1] += sample * right;
    }
}

void finished(void*, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
    AudioQueueFreeBuffer(queue, buffer);
}

AudioStreamBasicDescription stereoFloat()
{
    auto format = AudioStreamBasicDescription {};
    format.mSampleRate = sampleRate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    format.mChannelsPerFrame = channels;
    format.mBitsPerChannel = 32;
    format.mBytesPerFrame = channels * sizeof(float);
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = format.mBytesPerFrame;
    return format;
}
} // namespace

MooVoice::MooVoice()
{
    auto format = stereoFloat();

    if (AudioQueueNewOutput(&format, finished, nullptr, nullptr, nullptr, 0, &queue)
        != noErr)
        queue = nullptr;
}

MooVoice::~MooVoice()
{
    if (queue != nullptr)
        AudioQueueDispose(queue, true);
}

void MooVoice::call(const MooAnswer& answer)
{
    if (queue == nullptr)
        return;

    auto length = mooAnswerDelay + mooLength + tailLength;
    auto sampleCount = (std::size_t) (length * (float) sampleRate) * channels;
    auto bytes = (UInt32) (sampleCount * sizeof(float));
    auto buffer = AudioQueueBufferRef {};

    if (AudioQueueAllocateBuffer(queue, bytes, &buffer) != noErr)
        return;

    auto* samples = (float*) buffer->mAudioData;
    std::fill(samples, samples + sampleCount, 0.f);

    addVoice(samples, sampleCount, {0.f, 1.f, 1.f, 0.f, 0.f});
    addVoice(samples,
             sampleCount,
             {mooAnswerDelay, 1.3f, answer.volume, answer.pan, answer.muffle});

    buffer->mAudioDataByteSize = bytes;

    AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr);
    AudioQueueStart(queue, nullptr);
}
} // namespace Cows
