#include "Quality.h"

#include "UI/Editor.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <cctype>

namespace Cows
{
namespace
{
const QualityAndLabel* findQuality(Quality quality)
{
    auto entry =
        std::ranges::find(qualitiesAndLabels, quality, &QualityAndLabel::quality);
    return entry == qualitiesAndLabels.end() ? nullptr : &*entry;
}

const QualityChoiceAndLabel* findQualityChoice(QualityChoice choice)
{
    auto entry = std::ranges::find(
        qualityChoicesAndLabels, choice, &QualityChoiceAndLabel::choice);
    return entry == qualityChoicesAndLabels.end() ? nullptr : &*entry;
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
    auto entry = findQuality(quality);
    return entry == nullptr ? std::string {} : std::string {entry->label};
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
    auto entry = findQualityChoice(choice);
    return entry == nullptr ? std::string {} : std::string {entry->label};
}

QualityChoice stepQualityChoice(QualityChoice choice, int steps)
{
    auto isCurrent = [choice](const QualityChoiceAndLabel& entry)
    { return entry.choice == choice; };
    return stepThroughEntries(qualityChoicesAndLabels, isCurrent, steps).choice;
}

std::optional<Quality> getChosenQuality(QualityChoice choice)
{
    auto entry = findQualityChoice(choice);
    return entry == nullptr ? std::nullopt : entry->quality;
}

QualityChoice getQualityChoiceFor(Quality quality)
{
    for (const auto& entry: qualityChoicesAndLabels)
        if (entry.quality == quality)
            return entry.choice;

    return QualityChoice::Auto;
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
    if (findQualityChoice(preference.chosen) == nullptr)
        preference.chosen = QualityChoice::Auto;

    auto measuredIsUnknown = preference.measured.has_value()
                             && findQuality(*preference.measured) == nullptr;

    if (measuredIsUnknown)
        preference.measured.reset();

    return preference;
}
} // namespace Cows
