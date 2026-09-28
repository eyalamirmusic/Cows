#include "SamplePlayer.h"

#include <NanoTest/NanoTest.h>

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

using namespace nano;
using namespace Cows;

namespace
{
constexpr auto channels = (std::size_t) SamplePlayer::channels;

eacp::Vector<float> constant(int count, float value)
{
    auto sample = eacp::Vector<float> {};

    for (auto index = 0; index < count; ++index)
        sample.add(value);

    return sample;
}

std::vector<float> mix(std::size_t frames,
                       const eacp::Vector<float>& sample,
                       const SampleVoice& voice)
{
    auto samples = std::vector<float>(frames * channels, 0.f);
    addVoice(samples.data(), frames, sample, voice);
    return samples;
}

float left(const std::vector<float>& samples, std::size_t frame)
{
    return samples[frame * channels];
}

float right(const std::vector<float>& samples, std::size_t frame)
{
    return samples[frame * channels + 1];
}
} // namespace

auto tSilenceMixesToSilence = test("SamplePlayer/silenceMixesToSilence") = []
{
    auto samples = mix(1000, constant(800, 0.f), {});

    for (auto value: samples)
        check(value == 0.f);
};

auto tPanIsolatesChannels = test("SamplePlayer/panIsolatesChannels") = []
{
    auto hardLeft = mix(1000, constant(800, 1.f), {0.f, 1.f, 1.f, -1.f, 0.f});
    auto hardRight = mix(1000, constant(800, 1.f), {0.f, 1.f, 1.f, 1.f, 0.f});

    check(left(hardLeft, 400) > 0.5f);
    check(right(hardLeft, 400) == 0.f);
    check(right(hardRight, 400) > 0.5f);
    check(left(hardRight, 400) == 0.f);
};

auto tCentreIsEqualPower = test("SamplePlayer/centreIsEqualPower") = []
{
    auto samples = mix(1000, constant(800, 1.f), {});

    check(left(samples, 400) == right(samples, 400));
    check(left(samples, 400) > 0.6f && left(samples, 400) < 0.75f);
};

auto tStartDelaysTheVoice = test("SamplePlayer/startDelaysTheVoice") = []
{
    auto samples = mix(2000, constant(800, 1.f), {0.01f, 1.f, 1.f, 0.f, 0.f});

    for (std::size_t frame = 0; frame < 400; ++frame)
        check(left(samples, frame) == 0.f);

    check(left(samples, 800) > 0.f);
    check(left(samples, 1500) == 0.f);
};

auto tPitchShortensTheVoice = test("SamplePlayer/pitchShortensTheVoice") = []
{
    auto samples = mix(1000, constant(800, 1.f), {0.f, 2.f, 1.f, 0.f, 0.f});

    check(left(samples, 390) > 0.f);
    check(left(samples, 400) == 0.f);
};

auto tVoiceStopsAtTheBufferEnd = test("SamplePlayer/voiceStopsAtTheBufferEnd") = []
{
    constexpr auto frames = std::size_t {100};
    constexpr auto guard = 1234.f;

    auto samples = std::vector<float>((frames + 4) * channels, guard);
    std::fill(samples.begin(), samples.begin() + frames * channels, 0.f);

    addVoice(samples.data(), frames, constant(800, 1.f), {});

    check(left(samples, frames - 1) > 0.f);

    for (auto index = frames * channels; index < samples.size(); ++index)
        check(samples[index] == guard);
};

auto tBufferHoldsLongestSound = test("SamplePlayer/bufferHoldsLongestSound") = []
{
    auto player = SamplePlayer {2.f};

    check((std::size_t) player.mix.size()
          == 2 * (std::size_t) SamplePlayer::sampleRate * channels);
};

auto tEmptySampleIsANoOp = test("SamplePlayer/emptySampleIsANoOp") = []
{
    auto player = SamplePlayer {1.f};

    player.play({}, {{0.f, 1.f, 1.f, 0.f, 0.f}});

    check(player.usedFrames == 0);
};

auto tPlayMixesUpToTheLastVoice = test("SamplePlayer/playMixesUpToTheLastVoice") = []
{
    auto player = SamplePlayer {1.f};

    player.play(constant(441, 1.f),
                {{0.f, 1.f, 1.f, 0.f, 0.f}, {0.1f, 1.f, 1.f, 0.f, 0.f}});

    check(player.usedFrames >= 4850 && player.usedFrames <= 4851);
    check(player.mix[4849 * (int) channels] > 0.f);
};

auto tRenderPlaysTheMixOnce = test("SamplePlayer/renderPlaysTheMixOnce") = []
{
    auto player = SamplePlayer {1.f};
    player.device.stop();
    player.play(constant(441, 1.f), {{0.f, 1.f, 1.f, -1.f, 0.f}});

    auto block = std::vector<float>(2 * 512, -1.f);
    auto info = MakeASound::AudioCallbackInfo {};
    info.outputBuffer = block.data();
    info.numOutputs = 2;
    info.numSamples = 512;

    player.render(info);

    check(block[100] > 0.f);
    check(block[512 + 100] == 0.f);
    check(block[511] == 0.f);
    check(player.playedFrames == player.usedFrames);

    player.render(info);
    check(std::all_of(block.begin(), block.end(), [](float s) { return s == 0.f; }));
};

auto tDevicePullsTheMix = test("SamplePlayer/devicePullsTheMix") = []
{
    auto player = SamplePlayer {1.f};

    if (!player.open)
        return;

    player.play(constant(4410, 0.f), {{0.f, 1.f, 1.f, 0.f, 0.f}});

    for (auto wait = 0; wait < 100 && player.playedFrames < player.usedFrames;
         ++wait)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto lock = std::lock_guard {player.mutex};
    check(player.playedFrames == player.usedFrames);
};
