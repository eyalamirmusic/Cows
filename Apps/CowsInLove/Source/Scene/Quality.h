#pragma once

#include "Terrain/Grass.h"

#include <Miro/Reflect.h>

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
// How much of the scene a GPU can afford. High is the game as designed; Medium
// and Low draw the same world with cheaper meshes, grass, noise and shadows,
// into fewer pixels than the panel has (renderScale), stretched over it.
enum class Quality
{
    Low,
    Medium,
    High
};

struct QualityAndName final
{
    Quality quality;
    std::string_view name;
};

// Every tier in enum order, with the name COWS_QUALITY and the log use.
constexpr auto qualitiesAndNames = std::array {
    QualityAndName {Quality::Low, "low"},
    QualityAndName {Quality::Medium, "medium"},
    QualityAndName {Quality::High, "high"},
};

constexpr auto numQualityLevels = static_cast<int>(qualitiesAndNames.size());

// Every knob a tier turns, in one place.
struct QualitySettings final
{
    int samples = 4;
    float renderScale = 1.f;

    // How finely the cows, props and hats are cut (see `detailed`).
    float meshDetail = 1.f;

    GrassDensity grass;
    // Rows up each blade (see makeBlade).
    int bladeSegments = 5;
    // The grass's noise hashed without sines (Shading::quickValueNoise).
    bool quickGrassNoise = false;
    // The ground's noise read from a NoiseLattice.
    bool cheapGroundNoise = false;

    int shadowTaps = 16;
    int shadowResolution = 2048;
};

QualitySettings getSettingsForQuality(Quality quality);

std::string getQualityName(Quality quality);
std::optional<Quality> parseQualityName(std::string_view name);

// What the player picked in Settings: a tier, or Auto, which is the tier
// measured on this device.
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
};

// Every choice in enum order, with the label the Settings row shows.
constexpr auto qualityChoicesAndLabels = std::array {
    QualityChoiceAndLabel {QualityChoice::Auto, "Auto"},
    QualityChoiceAndLabel {QualityChoice::Low, "Low"},
    QualityChoiceAndLabel {QualityChoice::Medium, "Medium"},
    QualityChoiceAndLabel {QualityChoice::High, "High"},
};

constexpr auto numQualityChoices = static_cast<int>(qualityChoicesAndLabels.size());

std::string getQualityChoiceLabel(QualityChoice choice);

// `choice` moved `steps` along the list, wrapping round: the Settings row's < >.
QualityChoice stepQualityChoice(QualityChoice choice, int steps);

// The tier a choice picks; none for Auto.
std::optional<Quality> getChosenQuality(QualityChoice choice);

// The choice that picks `quality`.
QualityChoice getQualityChoiceFor(Quality quality);

// The tier to draw at: `forced` (COWS_QUALITY), else the player's choice, else
// the measured tier; none means it is still to be measured.
std::optional<Quality> chooseQualityToUse(std::optional<Quality> forced,
                                          QualityChoice chosen,
                                          std::optional<Quality> measured);

// COWS_QUALITY=low|medium|high: that tier, and no measuring.
std::optional<Quality> readQualityFromEnvironment();

// The player's choice and, under Auto, the tier this device measured, as
// settings.json keeps them.
struct QualityPreference final
{
    bool operator==(const QualityPreference& other) const = default;

    QualityChoice chosen = QualityChoice::Auto;
    std::optional<Quality> measured;

    MIRO_REFLECT(chosen, measured)
};

// `preference` with anything that names no choice or tier put back to Auto
// and unmeasured.
QualityPreference replaceUnknownValuesWithDefaults(QualityPreference preference);
} // namespace Cows
