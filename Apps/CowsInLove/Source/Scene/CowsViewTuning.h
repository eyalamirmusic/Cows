#pragma once

#include "Camera/OrbitCamera.h"

#include <eacp/Core/Utils/Environment.h>

#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace Cows::CowsViewTuning
{
using namespace Maths;

inline constexpr auto orbitSpeed = 0.006f;
inline constexpr auto padYawRate = 2.6f;
inline constexpr auto padPitchRate = 1.5f;
inline constexpr auto padZoomRate = 1.5f;
inline constexpr auto lookHoldTime = 0.5f;

inline constexpr auto searchHeight = 2.2f;
inline const auto searchLeastWidth = OrbitCamera {}.leastWidth;
inline constexpr auto startYaw = -halfPi;
inline constexpr auto chaseRate = 4.f;
inline constexpr auto eyeMargin = 0.3f;
// Under a roof the target drops to here, so the eye is not clipped inside it.
inline constexpr auto bodyHeight = 0.6f;
inline constexpr auto nearestEye = 0.6f;
inline constexpr auto eyeReturnRate = 3.f;
inline constexpr auto contactReach = 0.4f;
inline constexpr auto flareTime = 2.6f;
inline constexpr auto flareHeight = 22.f;
inline constexpr auto flareHearts = 6;
inline constexpr auto quietest = 0.15f;
inline constexpr auto loudReach = 120.f;

inline constexpr auto portraitDistance = 6.5f;
inline constexpr auto portraitPitch = 0.26f;

inline constexpr auto menuTurn = 0.45f;
inline constexpr auto menuHeight = 1.3f;
inline constexpr auto menuPitch = 0.1f;
inline constexpr auto menuDistance = 9.5f;
inline constexpr auto menuCowHeight = 2.2f;
inline constexpr auto menuCowWidth = 2.f;
inline constexpr auto wideMenuCowShare = 0.5f;
inline constexpr auto tallMenuCowShare = 0.85f;
inline constexpr auto hatClearance = 0.8f;
inline constexpr auto nearestMenu = 4.f;
inline constexpr auto farthestMenu = 14.f;
inline constexpr auto menuStick = 0.5f;
inline constexpr auto swingLength = 1.1f;
inline constexpr auto dressLength = 0.6f;
inline constexpr auto editorHeight = 1.4f;
inline constexpr auto editorDistance = 7.5f;
inline constexpr auto editorTurn = 0.3f;
inline constexpr auto editorCowWidth = 3.2f;
inline constexpr auto tallEditorCowShare = 0.7f;
// Upright, the camera sinks into the grass unless it looks down more.
inline constexpr auto tallEditorPitch = 0.3f;

struct ProfileSettings final
{
    bool skips(std::string_view part) const
    {
        return skip.find(part) != std::string::npos;
    }

    int blades = std::numeric_limits<int>::max();
    std::optional<int> samples;
    std::optional<int> shadowResolution;
    std::optional<float> renderScale;
    std::optional<float> meshDetail;
    std::string skip;
};

inline const ProfileSettings& profileSettings()
{
    static auto settings = []
    {
        auto read = ProfileSettings {};

        if (auto blades = getEnv("COWS_BLADES"))
            read.blades = std::stoi(*blades);

        if (auto samples = getEnv("COWS_MSAA"))
            read.samples = std::stoi(*samples);

        if (auto scale = getEnv("COWS_RENDER_SCALE"))
            read.renderScale = std::stof(*scale);

        if (auto resolution = getEnv("COWS_SHADOW"))
            read.shadowResolution = std::stoi(*resolution);

        if (auto detail = getEnv("COWS_DETAIL"))
            read.meshDetail = std::stof(*detail);

        read.skip = getEnvValue("COWS_SKIP");
        return read;
    }();

    return settings;
}
} // namespace Cows::CowsViewTuning
