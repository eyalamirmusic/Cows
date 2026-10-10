#pragma once

#include "Cow/CowSkin.h"
#include "Scene/Quality.h"

#include <Miro/Reflect.h>
#include <eacp/Core/Utils/FilePath.h>

#include <string>
#include <string_view>

namespace Cows
{
// Everything the game keeps between runs, in one document: settings.json in
// the app's support folder. A new setting is one field here.
struct Settings final
{
    bool operator==(const Settings& other) const = default;

    CowSkin skin;
    QualityPreference quality;

    MIRO_REFLECT(skin, quality)
};

std::string toJSON(const Settings& settings);

// The settings `text` describes; the default for anything it does not name.
Settings settingsFromJSON(std::string_view text);

// settings.json in `directory`, the app's support folder unless told otherwise.
FilePath settingsFile();
FilePath settingsFile(const FilePath& directory);

// The settings saved at `file`, or the defaults when there are none to read.
Settings loadSettings(const FilePath& file);

// Writes `settings` to `file`, creating its folder; false when it could not.
bool saveSettings(const Settings& settings, const FilePath& file);
} // namespace Cows
