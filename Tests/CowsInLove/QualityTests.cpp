#include "Scene/Quality.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tHighIsTheGame = test("Quality/highIsTheGameAsDesigned") = []
{
    auto high = settingsFor(Quality::High);
    auto design = GrassDensity {};

    check(high.samples == 4);
    check(!high.cheapGroundNoise);
    check(!high.quickGrassNoise);
    check(high.bladeSegments == 5);
    check(high.shadowTaps == 16);
    check(high.shadowResolution == 2048);
    check(high.renderScale == 1.f);
    check(high.meshDetail == 1.f);
    check(high.grass.tilesAround == design.tilesAround);
    check(high.grass.nearShare == 1.f);
    check(high.grass.farShare == 1.f);
};

auto tLowerCostsLess = test("Quality/eachTierAsksNoMoreThanTheOneAbove") = []
{
    auto low = settingsFor(Quality::Low);
    auto mid = settingsFor(Quality::Medium);
    auto high = settingsFor(Quality::High);

    check(low.samples <= mid.samples && mid.samples <= high.samples);
    check(low.shadowTaps <= mid.shadowTaps && mid.shadowTaps <= high.shadowTaps);
    check(low.shadowResolution <= mid.shadowResolution);
    check(low.renderScale <= mid.renderScale);
    check(mid.renderScale <= high.renderScale);
    check(low.meshDetail <= mid.meshDetail);
    check(mid.meshDetail <= high.meshDetail);
    check(low.grass.tilesAround <= mid.grass.tilesAround);
    check(low.grass.nearShare <= mid.grass.nearShare);
    check(mid.grass.nearShare <= high.grass.nearShare);
    check(low.grass.farShare <= mid.grass.farShare);
    check(mid.grass.farShare <= high.grass.farShare);
};

auto tLowIsCheap = test("Quality/lowDropsMsaaAndTheFarGrass") = []
{
    auto low = settingsFor(Quality::Low);
    check(low.samples == 1);
    check(low.cheapGroundNoise);
    check(low.shadowTaps == 4);
    check(low.grass.farShare == 0.f);
};

auto tNames = test("Quality/namesRoundTrip") = []
{
    for (auto quality: {Quality::Low, Quality::Medium, Quality::High})
        check(qualityNamed(qualityName(quality)) == quality);

    check(qualityNamed(" LOW\n") == Quality::Low);
    check(qualityNamed("mid") == Quality::Medium);
    check(!qualityNamed("ultra").has_value());
    check(!qualityNamed("").has_value());
};

auto tChoices = test("Quality/choicesPickTheirTier") = []
{
    check(!chosenQuality(QualityChoice::Auto).has_value());
    check(chosenQuality(QualityChoice::Low) == Quality::Low);
    check(chosenQuality(QualityChoice::Medium) == Quality::Medium);
    check(chosenQuality(QualityChoice::High) == Quality::High);
    check(choiceLabel(QualityChoice::Medium) == "Medium");
};

auto tToUse = test("Quality/forcedThenChosenThenMeasured") = []
{
    auto low = std::optional<Quality> {Quality::Low};
    auto none = std::optional<Quality> {};

    check(qualityToUse(Quality::High, QualityChoice::Low, low) == Quality::High);
    check(qualityToUse(none, QualityChoice::Medium, low) == Quality::Medium);
    check(qualityToUse(none, QualityChoice::Auto, low) == Quality::Low);
    check(!qualityToUse(none, QualityChoice::Auto, none).has_value());
};
