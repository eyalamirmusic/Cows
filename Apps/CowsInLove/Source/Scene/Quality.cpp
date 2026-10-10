#include "Quality.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <cctype>

namespace Cows
{
namespace
{
std::string squeezed(std::string_view name)
{
    auto lower = std::string {};

    for (auto character: name)
        if (!std::isspace((unsigned char) character))
            lower += (char) std::tolower((unsigned char) character);

    return lower;
}

} // namespace

QualitySettings settingsFor(Quality quality)
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

std::string qualityName(Quality quality)
{
    switch (quality)
    {
        case Quality::Low:
            return "low";
        case Quality::Medium:
            return "medium";
        case Quality::High:
            return "high";
    }

    return "high";
}

std::optional<Quality> qualityNamed(std::string_view name)
{
    auto lower = squeezed(name);

    if (lower == "mid")
        return Quality::Medium;

    for (auto quality: {Quality::Low, Quality::Medium, Quality::High})
        if (lower == qualityName(quality))
            return quality;

    return std::nullopt;
}

std::string choiceLabel(QualityChoice choice)
{
    switch (choice)
    {
        case QualityChoice::Auto:
            return "Auto";
        case QualityChoice::Low:
            return "Low";
        case QualityChoice::Medium:
            return "Medium";
        case QualityChoice::High:
            return "High";
    }

    return "Auto";
}

std::optional<Quality> chosenQuality(QualityChoice choice)
{
    if (choice == QualityChoice::Auto)
        return std::nullopt;

    return (Quality) ((int) choice - 1);
}

std::optional<Quality> qualityToUse(std::optional<Quality> forced,
                                    QualityChoice chosen,
                                    std::optional<Quality> measured)
{
    if (forced.has_value())
        return forced;

    if (auto quality = chosenQuality(chosen))
        return quality;

    return measured;
}

std::optional<Quality> qualityOverride()
{
    return qualityNamed(getEnvValue("COWS_QUALITY"));
}

QualityPreference withKnownValues(QualityPreference preference)
{
    auto choice = (int) preference.chosen;

    if (choice < 0 || choice >= qualityChoices)
        preference.chosen = QualityChoice::Auto;

    if (preference.measured.has_value())
    {
        auto tier = (int) *preference.measured;

        if (tier < 0 || tier >= qualityLevels)
            preference.measured.reset();
    }

    return preference;
}
} // namespace Cows
