#include "Scene/Quality.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tHighIsTheGame = test("Quality/highIsTheGameAsDesigned") = []
{
    auto high = getSettingsForQuality(Quality::High);
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
    auto low = getSettingsForQuality(Quality::Low);
    auto mid = getSettingsForQuality(Quality::Medium);
    auto high = getSettingsForQuality(Quality::High);

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
    auto low = getSettingsForQuality(Quality::Low);
    check(low.samples == 1);
    check(low.cheapGroundNoise);
    check(low.shadowTaps == 4);
    check(low.grass.farShare == 0.f);
};

auto tNames = test("Quality/namesRoundTrip") = []
{
    for (auto quality: {Quality::Low, Quality::Medium, Quality::High})
        check(parseQualityName(getQualityName(quality)) == quality);

    check(parseQualityName(" LOW\n") == Quality::Low);
    check(parseQualityName("mid") == Quality::Medium);
    check(!parseQualityName("ultra").has_value());
    check(!parseQualityName("").has_value());
};

auto tChoices = test("Quality/choicesPickTheirTier") = []
{
    check(!getChosenQuality(QualityChoice::Auto).has_value());
    check(getChosenQuality(QualityChoice::Low) == Quality::Low);
    check(getChosenQuality(QualityChoice::Medium) == Quality::Medium);
    check(getChosenQuality(QualityChoice::High) == Quality::High);
    check(getQualityChoiceLabel(QualityChoice::Medium) == "Medium");
};

auto tToUse = test("Quality/forcedThenChosenThenMeasured") = []
{
    auto low = std::optional<Quality> {Quality::Low};
    auto none = std::optional<Quality> {};

    check(chooseQualityToUse(Quality::High, QualityChoice::Low, low)
          == Quality::High);
    check(chooseQualityToUse(none, QualityChoice::Medium, low) == Quality::Medium);
    check(chooseQualityToUse(none, QualityChoice::Auto, low) == Quality::Low);
    check(!chooseQualityToUse(none, QualityChoice::Auto, none).has_value());
};

auto tQualityStepping = test("Quality/steppingWrapsRoundBothEnds") = []
{
    check(stepQualityChoice(QualityChoice::Auto, 1) == QualityChoice::Low);
    check(stepQualityChoice(QualityChoice::High, 1) == QualityChoice::Auto);
    check(stepQualityChoice(QualityChoice::Auto, -1) == QualityChoice::High);
    check(getQualityChoiceFor(Quality::Medium) == QualityChoice::Medium);
};
