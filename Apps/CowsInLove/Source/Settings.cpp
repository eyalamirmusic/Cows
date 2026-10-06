#include "Settings.h"

#include <eacp/Core/Utils/Files.h>

#include <exception>

namespace Cows
{
namespace
{
constexpr auto fileName = "settings.json";
constexpr auto indent = 4;
} // namespace

std::string toJSON(const Settings& settings)
{
    return Miro::toJSONString(settings, indent);
}

Settings settingsFromJSON(std::string_view text)
{
    auto settings = Miro::createFromJSONString<Settings>(text);
    settings.skin = withNamedChoices(settings.skin);
    settings.quality = withKnownValues(settings.quality);
    return settings;
}

FilePath settingsFile()
{
    return settingsFile(FilePath::appSupportDirectory());
}

FilePath settingsFile(const FilePath& directory)
{
    return directory / fileName;
}

Settings loadSettings(const FilePath& file)
{
    try
    {
        return settingsFromJSON(Files::readFile(file));
    }
    catch (const std::exception&)
    {
        return {};
    }
}

bool saveSettings(const Settings& settings, const FilePath& file)
{
    auto text = toJSON(settings) + "\n";
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
