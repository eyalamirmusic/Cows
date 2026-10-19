#pragma once

#include "Terrain/Grass.h"

#include <Miro/Reflect.h>

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
enum class Quality
{
    Low,
    Medium,
    High
};

struct QualityAndLabel final
{
    Quality quality;
    std::string_view label;
};

constexpr auto qualitiesAndLabels = std::array {
    QualityAndLabel {Quality::Low, "low"},
    QualityAndLabel {Quality::Medium, "medium"},
    QualityAndLabel {Quality::High, "high"},
};

struct QualitySettings final
{
    int samples = 4;
    float renderScale = 1.f;

    float meshDetail = 1.f;

    GrassDensity grass;
    int bladeSegments = 5;
    bool quickGrassNoise = false;
    bool cheapGroundNoise = false;

    int shadowTaps = 16;
    int shadowResolution = 2048;
};

QualitySettings getSettingsForQuality(Quality quality);

std::string getQualityName(Quality quality);
std::optional<Quality> parseQualityName(std::string_view name);

enum class QualityChoice
{
    Auto,
    Low,
    Medium,
    High
};

struct QualityChoiceAndLabel final
{
    QualityChoice choice;
    std::string_view label;
    std::optional<Quality> quality;
};

constexpr auto qualityChoicesAndLabels = std::array {
    QualityChoiceAndLabel {QualityChoice::Auto, "Auto", std::nullopt},
    QualityChoiceAndLabel {QualityChoice::Low, "Low", Quality::Low},
    QualityChoiceAndLabel {QualityChoice::Medium, "Medium", Quality::Medium},
    QualityChoiceAndLabel {QualityChoice::High, "High", Quality::High},
};

std::string getQualityChoiceLabel(QualityChoice choice);

QualityChoice stepQualityChoice(QualityChoice choice, int steps);

std::optional<Quality> getChosenQuality(QualityChoice choice);

QualityChoice getQualityChoiceFor(Quality quality);

std::optional<Quality> chooseQualityToUse(std::optional<Quality> forced,
                                          QualityChoice chosen,
                                          std::optional<Quality> measured);

std::optional<Quality> readQualityFromEnvironment();

struct QualityPreference final
{
    bool operator==(const QualityPreference& other) const = default;

    QualityChoice chosen = QualityChoice::Auto;
    std::optional<Quality> measured;

    MIRO_REFLECT(chosen, measured)
};

QualityPreference replaceUnknownValuesWithDefaults(QualityPreference preference);
} // namespace Cows
