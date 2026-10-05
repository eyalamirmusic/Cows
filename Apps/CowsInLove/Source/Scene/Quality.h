#pragma once

#include "Terrain/Grass.h"

#include <eacp/Core/Utils/FilePath.h>

#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
// How much of the scene a GPU can afford. High is the game as designed; Mid
// thins the far grass and reads the ground's noise from a texture; Low keeps
// the sky, the haze, the cows and the props and leaves only a sparse meadow
// round the player, with no MSAA and softer, cheaper shadows.
enum class Quality
{
    Low,
    Mid,
    High
};

constexpr auto qualityLevels = 3;

struct QualitySettings final
{
    int samples = 4;
    GrassDensity grass;
    bool cheapNoise = false;
    int shadowTaps = 16;
    int shadowResolution = 2048;
};

QualitySettings settingsFor(Quality quality);

std::string qualityName(Quality quality);
std::optional<Quality> qualityNamed(std::string_view name);

// COWS_QUALITY=low|mid|high: that tier, and no measuring.
std::optional<Quality> qualityOverride();

// Quality.txt beside CowSkin.json: the tier the first run measured.
FilePath qualityFile();
FilePath qualityFile(const FilePath& directory);
std::optional<Quality> loadQuality(const FilePath& file);
bool saveQuality(Quality quality, const FilePath& file);
} // namespace Cows
