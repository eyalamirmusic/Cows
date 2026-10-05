#include "Scene/Quality.h"

#include <NanoTest/NanoTest.h>

#include <eacp/Core/Utils/Files.h>

using namespace nano;
using namespace Cows;

auto tHighIsTheGame = test("Quality/highIsTheGameAsDesigned") = []
{
    auto high = settingsFor(Quality::High);
    auto design = GrassDensity {};

    check(high.samples == 4);
    check(!high.cheapNoise);
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
    auto mid = settingsFor(Quality::Mid);
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
    check(low.cheapNoise);
    check(low.shadowTaps == 4);
    check(low.grass.farShare == 0.f);
};

auto tNames = test("Quality/namesRoundTrip") = []
{
    for (auto quality: {Quality::Low, Quality::Mid, Quality::High})
        check(qualityNamed(qualityName(quality)) == quality);

    check(qualityNamed(" LOW\n") == Quality::Low);
    check(!qualityNamed("ultra").has_value());
    check(!qualityNamed("").has_value());
};

auto tSaved = test("Quality/savesAndLoadsBesideTheSkin") = []
{
    auto directory = FilePath::tempDirectory() / "CowsQualityTest";
    auto file = qualityFile(directory);

    check(file == directory / "Quality.txt");
    check(saveQuality(Quality::Mid, file));
    check(loadQuality(file) == Quality::Mid);

    Files::removeAll(directory);
    check(!loadQuality(file).has_value());
};
