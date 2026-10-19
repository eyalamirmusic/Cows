#pragma once

#include "Cow/CowSkin.h"
#include "Scene/FrameRate.h"
#include "Scene/Quality.h"

#include <Miro/Reflect.h>
#include <eacp/Core/Utils/FilePath.h>

#include <string>
#include <string_view>

namespace Cows
{
struct Settings final
{
    bool operator==(const Settings& other) const = default;

    CowSkin skin;
    QualityPreference quality;
    FrameRate frameRate = FrameRate::Auto;

    MIRO_REFLECT(skin, quality, frameRate)
};

std::string toJSON(const Settings& settings);

Settings settingsFromJSON(std::string_view text);

FilePath settingsFile();
FilePath settingsFile(const FilePath& directory);

Settings loadSettings(const FilePath& file);

bool saveSettings(const Settings& settings, const FilePath& file);
} // namespace Cows
