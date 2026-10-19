#include "Quality.h"

#include "UI/Editor.h"

#include <eacp/Core/Utils/Environment.h>

#include <cctype>

namespace Cows
{
namespace
{
// The choices list Auto first, then the tiers in order.
constexpr auto firstTierChoiceIndex = 1;

constexpr bool qualitiesAreInEnumOrder()
{
    for (auto index = 0; index < numQualityLevels; ++index)
        if (static_cast<int>(qualitiesAndLabels[index].quality) != index)
            return false;

    for (auto index = 0; index < numQualityChoices; ++index)
        if (static_cast<int>(qualityChoicesAndLabels[index].choice) != index)
            return false;

    return numQualityChoices == numQualityLevels + firstTierChoiceIndex;
}

static_assert(qualitiesAreInEnumOrder());

int getQualityIndex(Quality quality)
{
    return static_cast<int>(quality);
}

int getQualityChoiceIndex(QualityChoice choice)
{
    return static_cast<int>(choice);
}

std::string lowercaseWithoutSpaces(std::string_view name)
{
    auto lowercase = std::string {};

    for (auto character: name)
    {
        auto asUnsigned = static_cast<unsigned char>(character);

        if (std::isspace(asUnsigned))
            continue;

        auto lowered = std::tolower(asUnsigned);
        lowercase += static_cast<char>(lowered);
    }

    return lowercase;
}

} // namespace

QualitySettings getSettingsForQuality(Quality quality)
{
    auto settings = QualitySettings {};

    if (quality == Quality::High)
        return settings;

    settings.renderScale = 0.75f;
    settings.meshDetail = 0.75f;
    settings.cheapGroundNoise = true;
    settings.grass.nearShare = 0.5f;
    settings.grass.farShare = 0.1f;
    settings.grass.thinFrom = 10.f;
    settings.grass.thinTo = 40.f;

    if (quality == Quality::Medium)
        return settings;

    settings.samples = 1;
    settings.renderScale = 0.5f;
    settings.meshDetail = 0.5f;
    settings.shadowTaps = 4;
    settings.shadowResolution = 1024;
    settings.grass.tilesAround = 1;
    settings.grass.nearShare = 0.15f;
    settings.grass.farShare = 0.f;
    settings.grass.thinFrom = 6.f;
    settings.grass.thinTo = 18.f;
    return settings;
}

std::string getQualityName(Quality quality)
{
    const auto& entry = qualitiesAndLabels[getQualityIndex(quality)];
    return std::string {entry.label};
}

std::optional<Quality> parseQualityName(std::string_view name)
{
    auto lowercaseName = lowercaseWithoutSpaces(name);

    if (lowercaseName == "mid")
        return Quality::Medium;

    for (const auto& entry: qualitiesAndLabels)
        if (lowercaseName == entry.label)
            return entry.quality;

    return std::nullopt;
}

std::string getQualityChoiceLabel(QualityChoice choice)
{
    const auto& entry = qualityChoicesAndLabels[getQualityChoiceIndex(choice)];
    return std::string {entry.label};
}

QualityChoice stepQualityChoice(QualityChoice choice, int steps)
{
    auto steppedIndex =
        wrapChoiceIndex(getQualityChoiceIndex(choice) + steps, numQualityChoices);
    return qualityChoicesAndLabels[steppedIndex].choice;
}

std::optional<Quality> getChosenQuality(QualityChoice choice)
{
    if (choice == QualityChoice::Auto)
        return std::nullopt;

    auto qualityIndex = getQualityChoiceIndex(choice) - firstTierChoiceIndex;
    return qualitiesAndLabels[qualityIndex].quality;
}

QualityChoice getQualityChoiceFor(Quality quality)
{
    auto choiceIndex = getQualityIndex(quality) + firstTierChoiceIndex;
    return qualityChoicesAndLabels[choiceIndex].choice;
}

std::optional<Quality> chooseQualityToUse(std::optional<Quality> forced,
                                          QualityChoice chosen,
                                          std::optional<Quality> measured)
{
    if (forced.has_value())
        return forced;

    if (auto quality = getChosenQuality(chosen))
        return quality;

    return measured;
}

std::optional<Quality> readQualityFromEnvironment()
{
    return parseQualityName(getEnvValue("COWS_QUALITY"));
}

QualityPreference replaceUnknownValuesWithDefaults(QualityPreference preference)
{
    auto choiceIndex = getQualityChoiceIndex(preference.chosen);

    if (choiceIndex < 0 || choiceIndex >= numQualityChoices)
        preference.chosen = QualityChoice::Auto;

    if (preference.measured.has_value())
    {
        auto qualityIndex = getQualityIndex(*preference.measured);

        if (qualityIndex < 0 || qualityIndex >= numQualityLevels)
            preference.measured.reset();
    }

    return preference;
}
} // namespace Cows
