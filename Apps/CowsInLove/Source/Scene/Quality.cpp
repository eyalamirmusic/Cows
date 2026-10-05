#include "Quality.h"

#include <eacp/Core/Utils/Environment.h>
#include <eacp/Core/Utils/Files.h>

#include <algorithm>
#include <cctype>

namespace Cows
{
QualitySettings settingsFor(Quality quality)
{
    auto settings = QualitySettings {};

    if (quality == Quality::High)
        return settings;

    settings.cheapNoise = true;
    settings.renderScale = 0.75f;
    settings.grass.nearShare = 0.5f;
    settings.grass.farShare = 0.1f;
    settings.grass.thinFrom = 10.f;
    settings.grass.thinTo = 40.f;

    if (quality == Quality::Mid)
        return settings;

    settings.samples = 1;
    settings.renderScale = 0.5f;
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
        case Quality::Mid:
            return "mid";
        case Quality::High:
            return "high";
    }

    return "high";
}

std::optional<Quality> qualityNamed(std::string_view name)
{
    auto lower = std::string {};

    for (auto character: name)
        if (!std::isspace((unsigned char) character))
            lower += (char) std::tolower((unsigned char) character);

    for (auto quality: {Quality::Low, Quality::Mid, Quality::High})
        if (lower == qualityName(quality))
            return quality;

    return std::nullopt;
}

std::optional<Quality> qualityOverride()
{
    return qualityNamed(getEnvValue("COWS_QUALITY"));
}

FilePath qualityFile()
{
    return qualityFile(FilePath::appSupportDirectory());
}

FilePath qualityFile(const FilePath& directory)
{
    return directory / "Quality.txt";
}

std::optional<Quality> loadQuality(const FilePath& file)
{
    try
    {
        return qualityNamed(Files::readFile(file));
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

bool saveQuality(Quality quality, const FilePath& file)
{
    auto text = qualityName(quality) + "\n";
    auto bytes = Span<const std::uint8_t> {
        reinterpret_cast<const std::uint8_t*>(text.data()), text.size()};

    try
    {
        Files::createDirectories(file.parentDirectory());
        Files::writeFileAtomically(file, bytes);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}
} // namespace Cows
