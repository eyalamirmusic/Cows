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

UInt32 bufferBytes(float longestSeconds)
{
    return (UInt32) ((std::size_t) (longestSeconds * (float) sampleRate) * channels
                     * sizeof(float));
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
    : bytes(bufferBytes(longestSeconds))
{
    auto format = stereoFloat();

    if (AudioQueueNewOutput(&format, finished, nullptr, nullptr, nullptr, 0, &queue)
        != noErr)
    {
        queue = nullptr;
        return;
    }

    if (AudioQueueAllocateBuffer(queue, bytes, &buffer) != noErr)
        buffer = nullptr;
}

SamplePlayer::~SamplePlayer()
{
    if (queue != nullptr)
        AudioQueueDispose(queue, true);
}

void SamplePlayer::play(const Vector<float>& sample,
                        std::initializer_list<SampleVoice> voices)
{
    if (queue == nullptr || buffer == nullptr || sample.empty())
        return;

    AudioQueueStop(queue, true);

    auto frameCount = (std::size_t) bytes / (channels * sizeof(float));
    auto* samples = (float*) buffer->mAudioData;
    std::fill(samples, samples + frameCount * channels, 0.f);

    auto end = 0.f;

    for (const auto& voice: voices)
    {
        addVoice(samples, frameCount, sample, voice);
        end = std::max(
            end,
            voice.start + (float) sample.size() / voice.pitch / (float) sampleRate);
    }

    auto usedFrames = std::min(frameCount, (std::size_t) (end * sampleRate));
    buffer->mAudioDataByteSize = (UInt32) (usedFrames * channels * sizeof(float));
    AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr);
    AudioQueueStart(queue, nullptr);
}
} // namespace Cows
