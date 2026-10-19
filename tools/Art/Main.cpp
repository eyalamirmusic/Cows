#include "Ending.h"
#include "LogoView.h"
#include "Props/Bale.h"
#include "Props/Barn.h"
#include "Screenshots.h"
#include "Scene/CowsView.h"

#include <eacp/Core/Core.h>

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
    game.player = Ending::foundStart(game);
    game.update(step, 0.f, 0.f, false);

    while (game.sinceFound < sinceFound)
    {
        game.update(step, 0.f, 0.f, false);
        view.steerCamera(step, step);
        view.elapsed += step;
    }
}

void renderEnding(const std::string& name, int width, int height, Framing framing)
{
    auto view = CowsView {{QualityChoice::High, {}}, Stages {}, openMeadowSeed()};
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

// Renders over black and white recover each pixel's coverage and true colour.
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

constexpr auto barnSeed = 10u;
constexpr auto barnRoom = 30.f;
constexpr auto settleSteps = 240;
constexpr auto barnShotWidth = 960;
constexpr auto barnShotHeight = 600;

// floorAt from here finds the highest surface over a point.
constexpr auto sky = 1000.f;
constexpr auto inFromTheDoor = 7.f;
constexpr auto alongTheLoft = 5.5f;

struct NamedSpot final
{
    std::string name;
    Vec3 at;
};

struct BarnLook final
{
    std::string name;
    float yaw = 0.f;
    float pitch = 0.3f;
};

void standBarnAt(Level& level, Vec2 center)
{
    clearAroundStage(level, {center.x, 0.f, center.y}, barnRoom);

    auto far = [&](Vec2 at) { return distance(at, center) > barnRoom; };
    auto blocks = Vector<Cows::Block> {};
    auto colliders = Vector<Collider> {};

    for (const auto& block: level.blocks)
        if (far(block.center))
            blocks.add(block);

    for (const auto& collider: level.colliders)
        if (far(collider.center))
            colliders.add(collider);

    level.blocks = blocks;
    level.colliders = colliders;
    addBarn(level, center, 0);
}

bool insideBarn(const Level& level, Vec3 eye)
{
    const auto& floor = level.clearings.back();
    auto at = Vec2 {eye.x, eye.z};
    auto offset = absolute(at - floor.center);
    return offset.x < floor.half.x && offset.y < floor.half.y
           && eye.y < level.floorAt(at, sky);
}

Vec3 stepped(const BarnSpot& spot, float reach, float feet)
{
    return spot.at
           + Vec3 {std::cos(spot.heading), 0.f, -std::sin(spot.heading)} * reach
           + Vec3 {0.f, feet - spot.at.y, 0.f};
}

constexpr auto ridgeSeed = 10u;
constexpr auto loftSeed = 8u;
constexpr auto endingShown = 4.f;

void renderBarnEnding(const std::string& directory,
                      const std::string& name,
                      std::uint32_t seed)
{
    auto view = CowsView {{QualityChoice::High, {}}, Stages {}, seed};
    view.game.start();
    playEnding(view, endingShown);
    view.setBounds({0.f, 0.f, (float) barnShotWidth, (float) barnShotHeight});
    LOG(name, " hiding ", (int) view.game.level.hiding);

    auto image = view.renderToImage(1.f);

    if (image.isValid())
        image.save(FilePath {directory + "/" + name + ".png"});
}

constexpr auto wideWidth = 1280;
constexpr auto wideHeight = 720;

struct EndingCase final
{
    std::string name;
    std::uint32_t seed;
};

void renderEndings(const std::string& directory)
{
    auto cases = std::initializer_list<EndingCase> {{"roof", 10u},
                                                    {"stack", 5u},
                                                    {"grove", 1u},
                                                    {"hedge", 2u},
                                                    {"meadow", 3u},
                                                    {"loft", 8u}};
    auto moments = std::initializer_list<float> {Ending::titleDelay
                                                     + Ending::titleRiseTime / 2.f,
                                                 Ending::settleTime,
                                                 Ending::loopTime - step};

    auto render = [&](const EndingCase& ending, float moment, int width, int height)
    {
        auto view = CowsView {{QualityChoice::High, {}}, Stages {}, ending.seed};
        view.setBounds({0.f, 0.f, (float) width, (float) height});
        view.framedPortrait = true;
        view.game.start();
        playEnding(view, moment);

        auto name = "ending-" + ending.name + "-"
                    + std::to_string((int) std::lround(moment * 10.f))
                    + (width < height ? "-portrait" : "");
        LOG(name, " hiding ", (int) view.game.level.hiding);
        auto image = view.renderToImage(1.f);

        if (image.isValid())
            image.save(FilePath {directory + "/" + name + ".png"});
    };

    for (const auto& ending: cases)
    {
        for (auto moment: moments)
            render(ending, moment, wideWidth, wideHeight);

        render(ending, Ending::loopTime - step, wideHeight, wideWidth);
    }
}

void renderBarn(const std::string& directory)
{
    auto view = CowsView {{QualityChoice::High, {}}, Stages {}, barnSeed};
    auto& game = view.game;
    auto center = Vec2 {game.partner.x, game.partner.z};
    game.start();
    game.partner = {center.x + barnRoom, 0.f, center.y + barnRoom};
    standBarnAt(game.level, center);
    view.setBounds({0.f, 0.f, (float) barnShotWidth, (float) barnShotHeight});

    auto barn = barnSpots(center, 0);
    auto spots = std::initializer_list<NamedSpot> {
        {"doorway", barn.doorway.at},
        {"floor", stepped(barn.doorway, inFromTheDoor, 0.f)},
        {"under-loft", stepped(barn.loft, 0.f, 0.f)},
        {"loft", barn.loft.at},
        {"loft-side", stepped(barn.loft, alongTheLoft, barn.loft.at.y)},
        {"ramp", barn.rampTop.at}};
    auto looks = std::initializer_list<BarnLook> {{"east", halfPi},
                                                  {"west", -halfPi},
                                                  {"north", 0.f},
                                                  {"south", pi},
                                                  {"up", 0.f, 1.2f}};

    for (const auto& spot: spots)
        for (const auto& look: looks)
        {
            game.player = spot.at;
            game.verticalSpeed = 0.f;
            view.camera.target = game.player;
            view.camera.yaw = look.yaw;
            view.camera.pitch = look.pitch;

            for (auto settle = 0; settle < settleSteps; ++settle)
            {
                game.update(step, 0.f, 0.f, false);
                view.camera.yaw = look.yaw;
                view.camera.pitch = look.pitch;
                view.steerCamera(step, step);
            }

            auto eye = view.camera.eye();
            auto name = "barn-" + spot.name + "-" + look.name;
            LOG(name,
                insideBarn(game.level, eye) ? " inside" : " OUTSIDE",
                " eye ",
                eye.x - center.x,
                " ",
                eye.y,
                " ",
                eye.z - center.y,
                " feet ",
                game.player.y);

            auto image = view.renderToImage(1.f);

            if (image.isValid())
                image.save(FilePath {directory + "/" + name + ".png"});
        }
}

constexpr auto baleShotWidth = 1920.f;
constexpr auto baleShotHeight = 1080.f;
constexpr auto baleLength = 1.7f;
constexpr auto baleRoom = 12.f;
constexpr auto stackAhead = 9.f;
constexpr auto cowBeside = 3.4f;
constexpr auto cowEyeHeight = 1.3f;
constexpr auto showcaseFieldOfView = 40.f;
constexpr auto ravineStage = 1;
constexpr auto ravineSeed = 2u;
constexpr auto rollingMoment = 0.3f;

void addBaleAt(Level& level, const Mat4& model)
{
    for (const auto& part: baleParts())
        level.batch.add(part.shape, makeInstance(model, part.material));
}

void addLyingBale(Level& level, Vec2 at, float lift, float heading)
{
    addBaleAt(level, lyingDrum(at, lift, heading, 0.f, baleRadius, baleLength));
}

void addStandingBale(Level& level, Vec2 at, float heading)
{
    addBaleAt(level,
              Mat4::translation({at.x, 0.f, at.y}) * Mat4::rotationY(heading)
                  * Mat4::scale({baleRadius * 2.f, baleLength, baleRadius * 2.f}));
}

void frame(CowsView& view, Vec3 target, float yaw, float pitch, float reach)
{
    auto& camera = view.camera;
    camera.target = target;
    camera.yaw = yaw;
    camera.pitch = pitch;
    camera.distance = reach;
    camera.fieldOfView = radians(showcaseFieldOfView);
    camera.leastWidth = radians(showcaseFieldOfView);
    view.setBounds({0.f, 0.f, baleShotWidth, baleShotHeight});
}

void saveTo(CowsView& view, const std::string& directory, const std::string& name)
{
    auto image = view.renderToImage(1.f);

    if (image.isValid())
        image.save(FilePath {directory + "/" + name + ".png"});

    LOG(directory, "/", name, ".png");
}

void renderBaleStack(const std::string& directory)
{
    constexpr auto stackYaw = 0.75f;
    constexpr auto stackPitch = 0.07f;
    constexpr auto stackReach = 12.5f;
    constexpr auto looseHeading = 0.9f;
    constexpr auto standingHeading = 0.4f;
    constexpr auto cowHeading = 2.6f;

    auto view = CowsView {{QualityChoice::High, {}}, Stages {}, openMeadowSeed()};
    auto& game = view.game;
    game.start();
    view.frozen = true;

    auto center = Vec2 {game.player.x + stackAhead, game.player.z};
    clearAroundStage(game.level, {center.x, 0.f, center.y}, baleRoom);

    auto across = baleRadius * 2.f;
    auto rest = baleRadius * std::sqrt(3.f);

    for (auto column: {-1.f, 0.f, 1.f})
        addLyingBale(game.level, center + Vec2 {column * across, 0.f}, 0.f, 0.f);

    for (auto column: {-0.5f, 0.5f})
        addLyingBale(game.level, center + Vec2 {column * across, 0.f}, rest, 0.f);

    addLyingBale(
        game.level, center + Vec2 {-across, across * 1.6f}, 0.f, looseHeading);
    addStandingBale(
        game.level, center + Vec2 {across * 2.3f, -across}, standingHeading);

    game.player = {center.x + cowBeside, 0.f, center.y + across * 1.4f};
    game.playerHeading = cowHeading;
    game.partner = {center.x - baleRoom * 4.f, 0.f, center.y - baleRoom * 4.f};

    frame(view,
          {center.x + cowBeside * 0.4f, cowEyeHeight, center.y + across * 0.5f},
          stackYaw,
          stackPitch,
          stackReach);
    saveTo(view, directory, "bale-ingame");
}

void renderRollingBale(const std::string& directory)
{
    constexpr auto rollingYaw = 0.8f;
    constexpr auto rollingPitch = 0.1f;
    constexpr auto rollingReach = 11.f;
    constexpr auto cowBack = 3.5f;

    auto view =
        CowsView {{QualityChoice::High, {}}, Stages {ravineStage}, ravineSeed};
    auto& game = view.game;
    game.start();
    view.frozen = true;

    if (game.level.movers.empty())
    {
        LOG("no movers on the ravine stage");
        return;
    }

    const auto& mover = game.level.movers.front();
    auto seconds = mover.period * rollingMoment;
    game.seconds = seconds;
    game.level.update(seconds);

    auto at = mover.positionAt(seconds);
    auto track = normalize(mover.to - mover.from);
    auto across = Vec2 {-track.y, track.x};
    auto cow = at + across * cowBack;
    game.player = {cow.x, mover.lift, cow.y};
    game.playerHeading = std::atan2(across.y, -across.x);

    auto heading = std::atan2(-track.y, track.x);
    frame(view,
          {at.x, mover.lift + cowEyeHeight, at.y},
          heading + rollingYaw,
          rollingPitch,
          rollingReach);
    saveTo(view, directory, "bale-rolling-ingame");
}

void run(const std::string& mode,
         const std::string& directory,
         const std::string& only)
{
    if (mode == "art")
        renderAll();
    else if (mode == "screens")
        renderScreens(directory, only);
    else if (mode == "endings")
        renderEndings(directory);
    else if (mode == "barn")
    {
        renderBarn(directory);
        renderBarnEnding(directory, "barn-ending-ridge", ridgeSeed);
        renderBarnEnding(directory, "barn-ending-loft", loftSeed);
    }
    else if (mode == "bale")
    {
        renderBaleStack(directory);
        renderRollingBale(directory);
    }
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
}
} // namespace

int main(int argc, char* argv[])
{
    auto mode = std::string {argc > 1 ? argv[1] : "art"};
    auto directory = std::string {argc > 2 ? argv[2] : "."};
    auto only = std::string {argc > 3 ? argv[3] : ""};
    auto exitCode = 0;

    Apps::run(
        [&]
        {
            try
            {
                run(mode, directory, only);
            }
            catch (const std::exception& error)
            {
                LOG("CowsArt ", mode, " failed: ", error.what());
                exitCode = 1;
            }
        });
    return exitCode;
}
