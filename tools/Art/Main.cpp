#include "Ending.h"
#include "LogoView.h"
#include "Screenshots.h"
#include "Scene/CowsView.h"

#include <eacp/Core/Core.h>
#include <eacp/Core/Utils/Environment.h>

#include <cmath>
#include <functional>
#include <string>

using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto step = 1.f / 120.f;
constexpr auto clearAround = 14.f;
constexpr auto kissHeight = 1.9f;

struct Framing final
{
    float sinceFound = 3.4f;
    float yaw = 0.f;
    float pitch = 0.08f;
    float distance = 11.f;
    float lift = 0.f;
    float fieldOfView = 42.f;
    bool title = true;
    bool clutter = true;
};

void clearAroundStage(Level& level, Vec3 center, float reach)
{
    for (auto& list: level.batch.lists)
    {
        auto kept = Vector<SurfaceInstance> {};

        for (const auto& instance: list)
        {
            auto at = Vec2 {instance.model3.x, instance.model3.z};

            if (distance(at, Vec2 {center.x, center.z}) > reach)
                kept.add(instance);
        }

        list = kept;
    }
}

std::string outputPath(const std::string& name)
{
    return std::string(COWS_RENDERS_DIR) + "/" + name + ".png";
}

void save(const Graphics::Image& image, const std::string& name)
{
    if (!image.isValid())
    {
        LOG("could not render ", name);
        return;
    }

    image.save(FilePath {outputPath(name)});
    LOG(outputPath(name));
}

void playEnding(CowsView& view, float sinceFound)
{
    auto& game = view.game;
    view.frozen = true;
    game.player = game.partner + Vec3 {Ending::foundStartGap, 0.f, 0.f};
    game.update(step, 0.f, 0.f, false);

    while (game.sinceFound < sinceFound)
    {
        game.update(step, 0.f, 0.f, false);
        view.steerCamera(step);
        view.elapsed += step;
    }
}

void renderEnding(const std::string& name, int width, int height, Framing framing)
{
    auto view = CowsView {};
    view.setTitle(framing.title ? "cows in love" : "");
    playEnding(view, framing.sinceFound);
    clearAroundStage(view.game.level,
                     view.game.stageCenter,
                     framing.clutter ? clearAround : 1000.f);

    auto& camera = view.camera;
    camera.target =
        view.game.stageCenter + Vec3 {0.f, kissHeight + framing.lift, 0.f};
    camera.yaw = view.game.stageHeading + framing.yaw;
    camera.pitch = framing.pitch;
    camera.distance = framing.distance;
    camera.fieldOfView = radians(framing.fieldOfView);
    camera.leastWidth = radians(framing.fieldOfView);
    view.framedPortrait = true;

    view.setBounds({0.f, 0.f, (float) width, (float) height});
    save(view.renderToImage(1.f), name);
}

Graphics::Image renderLogo(int width, int height, Graphics::Color background)
{
    auto view = LogoView {"cows in love"};
    view.background = background;
    view.time = 0.35f;
    view.camera.target = {0.f, 0.75f, 0.f};
    view.camera.yaw = 0.f;
    view.camera.pitch = 0.05f;
    view.camera.distance = view.title.width * 1.05f;
    view.camera.fieldOfView = radians(40.f);
    view.camera.leastWidth = radians(62.f);
    view.setBounds({0.f, 0.f, (float) width, (float) height});
    return view.renderToImage(1.f);
}

// Two renders, over black and over white, give each pixel's coverage and its
// colour unblended from the background.
void saveLogo(const std::string& name, int width, int height)
{
    auto black = renderLogo(width, height, {0.f, 0.f, 0.f});
    auto white = renderLogo(width, height, {1.f, 1.f, 1.f});

    if (!black.isValid() || !white.isValid())
        return save({}, name);

    auto logo = Graphics::Image {width, height};

    for (auto y = 0; y < height; ++y)
        for (auto x = 0; x < width; ++x)
        {
            auto dark = black.at(x, y);
            auto light = white.at(x, y);
            auto alpha =
                1.f
                - ((light.r - dark.r) + (light.g - dark.g) + (light.b - dark.b))
                      / 3.f;
            alpha = std::clamp(alpha, 0.f, 1.f);

            if (alpha < 1.f / 255.f)
                continue;

            logo.set(x,
                     y,
                     {std::min(dark.r / alpha, 1.f),
                      std::min(dark.g / alpha, 1.f),
                      std::min(dark.b / alpha, 1.f),
                      alpha});
        }

    save(logo, name);
}

void renderAll()
{
    setEnv("COWS_SEED", std::to_string(openMeadowSeed()));
    setEnv("COWS_STAGE", "");

    auto icon = Framing {};
    icon.sinceFound = 1.05f;
    icon.distance = 3.4f;
    icon.lift = 0.15f;
    icon.pitch = 0.1f;
    icon.fieldOfView = 40.f;
    icon.title = false;
    icon.clutter = false;
    renderEnding("icon", 1024, 1024, icon);

    auto adaptive = icon;
    adaptive.distance = 5.6f;
    adaptive.lift = 0.3f;
    renderEnding("icon-adaptive", 1024, 1024, adaptive);

    auto wide = Framing {};
    wide.title = false;
    wide.distance = 12.f;
    wide.lift = 1.4f;
    wide.pitch = 0.12f;
    renderEnding("backdrop-wide", 3840, 2160, wide);

    auto tall = Framing {};
    tall.title = false;
    tall.distance = 12.f;
    tall.lift = 2.4f;
    tall.pitch = 0.14f;
    renderEnding("backdrop-tall", 2000, 2400, tall);

    auto hero = Framing {};
    hero.title = false;
    hero.distance = 16.f;
    hero.lift = 1.2f;
    hero.fieldOfView = 30.f;
    renderEnding("hero", 3840, 1240, hero);

    saveLogo("logo", 2560, 1440);
}
} // namespace

// CowsArt              renders the key art, icon and logo
// CowsArt steam <dir>    the Steam screenshots, 1920x1080
// CowsArt mac <dir>      the Mac App Store screenshots, 2880x1800
// CowsArt ios-6.9 <dir>  the App Store 6.9" iPhone screenshots, 1320x2868
// CowsArt ios-6.5 <dir>  the App Store 6.5" iPhone screenshots, 1284x2778
// CowsArt play-phone <dir>  Google Play phone screenshots, 1080x1920
// CowsArt play-7 <dir>      Google Play 7" tablet screenshots, 1440x2560
// CowsArt play-10 <dir>     Google Play 10" tablet screenshots, 2160x3840
int main(int argc, char* argv[])
{
    auto mode = std::string {argc > 1 ? argv[1] : "art"};
    auto directory = std::string {argc > 2 ? argv[2] : "."};

    Apps::run(
        [&]
        {
            if (mode == "art")
                renderAll();
            else if (mode == "steam")
                renderScreenshots(directory, desktopScreen);
            else if (mode == "mac")
                renderScreenshots(directory, macScreen);
            else if (mode == "ios-6.9")
                renderScreenshots(directory, iPhone69);
            else if (mode == "ios-6.5")
                renderScreenshots(directory, iPhone65);
            else if (mode == "play-phone")
                renderScreenshots(directory, playPhone);
            else if (mode == "play-7")
                renderScreenshots(directory, playTablet7);
            else if (mode == "play-10")
                renderScreenshots(directory, playTablet10);
            else
                LOG("unknown mode ", mode);
        });
    return 0;
}
