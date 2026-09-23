#include "Moo.h"

#include <ResEmbed/ResEmbed.h>

#include <algorithm>
#include <cmath>
#include <cstring>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto sampleRate = 44100.0;
constexpr auto channels = 2;
constexpr auto longestCall = 4.5f;
constexpr auto answerPitch = 1.2f;
constexpr auto openCutoff = 9000.f;
constexpr auto muffledCutoff = 900.f;

struct Voice final
{
    float start = 0.f;
    float pitch = 1.f;
    float volume = 1.f;
    float pan = 0.f;
    float muffle = 0.f;
};

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

float sampleAt(const Vector<float>& moo, float position)
{
    auto index = (int) position;

    if (index + 1 >= moo.size())
        return 0.f;

    auto fraction = position - (float) index;
    return moo[index] + (moo[index + 1] - moo[index]) * fraction;
}

void addVoice(float* samples,
              std::size_t frameCount,
              const Vector<float>& moo,
              const Voice& voice)
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

void finished(void*, AudioQueueRef, AudioQueueBufferRef) {}

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

UInt32 bufferBytes()
{
    return (UInt32) ((std::size_t) (longestCall * (float) sampleRate) * channels
                     * sizeof(float));
}
} // namespace

MooVoice::MooVoice()
    : moo(loadMoo())
{
    auto format = stereoFloat();

    if (AudioQueueNewOutput(&format, finished, nullptr, nullptr, nullptr, 0, &queue)
        != noErr)
    {
        queue = nullptr;
        return;
    }

    if (AudioQueueAllocateBuffer(queue, bufferBytes(), &buffer) != noErr)
        buffer = nullptr;
}

MooVoice::~MooVoice()
{
    if (queue != nullptr)
        AudioQueueDispose(queue, true);
}

void MooVoice::call(const MooAnswer& answer)
{
    if (queue == nullptr || buffer == nullptr || moo.empty())
        return;

    AudioQueueStop(queue, true);

    auto bytes = bufferBytes();
    auto frameCount = (std::size_t) bytes / (channels * sizeof(float));
    auto* samples = (float*) buffer->mAudioData;
    std::fill(samples, samples + frameCount * channels, 0.f);

    addVoice(samples, frameCount, moo, {0.f, 1.f, 1.f, 0.f, 0.f});
    addVoice(
        samples,
        frameCount,
        moo,
        {mooAnswerDelay, answerPitch, answer.volume, answer.pan, answer.muffle});

    auto answerEnd =
        mooAnswerDelay + (float) moo.size() / answerPitch / (float) sampleRate;
    auto usedFrames = std::min(frameCount, (std::size_t) (answerEnd * sampleRate));
    buffer->mAudioDataByteSize = (UInt32) (usedFrames * channels * sizeof(float));
    AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr);
    AudioQueueStart(queue, nullptr);
}
} // namespace Cows
