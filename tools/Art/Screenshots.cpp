#include "Screenshots.h"
#include "Levels/LevelGenerator.h"
#include "MainMenu.h"
#include "Scene/CowsView.h"
#include "Templates.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <cmath>
#include <functional>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto step = 1.f / 60.f;
constexpr auto eyeHeight = 2.2f;
constexpr auto behind = -halfPi;
constexpr auto phoneTop = 62.f;
constexpr auto phoneBottom = 34.f;
constexpr auto menuStage = 0;
constexpr auto menuSeed = 3u;

struct Shot final
{
    std::string name;
    int stage = 0;
    std::uint32_t seed = 3;
    float seconds = 2.f;
    std::function<void(CowsView&)> setUp = [](CowsView&) {};
};

void settle(CowsView& view, float seconds)
{
    for (auto time = 0.f; time < seconds; time += step)
    {
        view.game.update(step, 0.f, 0.f, false);
        view.steerCamera(step, step);
        view.elapsed += step;
    }
}

void standAt(CowsView& view, Vec3 at, float heading)
{
    auto& game = view.game;
    game.player = at;
    game.playerHeading = heading;
    game.checkpoint = at;
    view.camera.target = at + Vec3 {0.f, eyeHeight, 0.f};
    view.camera.yaw = behind + heading;
}

float headingTo(Vec3 from, Vec3 to)
{
    return std::atan2(-(to.z - from.z), to.x - from.x);
}

Vector<Shot> shots()
{
    auto list = Vector<Shot> {};

    list.add({"1-meadow", 0, 3, 2.f});

    list.add({"2-moo",
              0,
              7,
              1.9f,
              [](CowsView& view)
              {
                  auto& game = view.game;
                  auto toward = headingTo(game.player, game.partner) + 0.25f;
                  standAt(view, game.player, toward);
                  view.callOut();
              }});

    list.add({"3-ravine",
              1,
              3,
              1.5f,
              [](CowsView& view)
              {
                  const auto& level = view.game.level;
                  const auto& gap = level.gaps.front();
                  auto x = level.criticalPath.center().x;
                  standAt(view, {x, 0.f, gap.center.y + gap.half.y + 5.f}, halfPi);
                  view.camera.pitch = 0.32f;
              }});

    list.add({"4-bridge",
              1,
              3,
              3.2f,
              [](CowsView& view)
              {
                  const auto& level = view.game.level;
                  const auto& gap = level.gaps.front();
                  auto x = level.criticalPath.center().x;
                  standAt(view, {x, 0.f, gap.center.y + 2.3f}, halfPi + 0.35f);
                  view.camera.pitch = 0.4f;
                  view.camera.distance = 13.f;
              }});

    list.add({"5-jump",
              1,
              3,
              0.f,
              [](CowsView& view)
              {
                  auto& game = view.game;
                  auto start = game.player;
                  standAt(view, start + Vec3 {0.f, 0.f, -15.3f}, halfPi);
                  settle(view, 1.f);
                  game.update(step, 1.f, 0.f, true);

                  for (auto frame = 0; frame < 16; ++frame)
                  {
                      game.update(step, 1.f, 0.f, false);
                      view.steerCamera(step, step);
                  }
              }});

    list.add({"6-found",
              0,
              openMeadowSeed(),
              0.f,
              [](CowsView& view)
              {
                  auto& game = view.game;
                  game.player = game.partner + Vec3 {3.f, 0.f, 0.f};
                  settle(view, 4.6f);

                  if (view.pointerHints == Hints::Touch)
                  {
                      view.framedPortrait = true;
                      view.camera.distance = 21.f;
                  }
              }});

    return list;
}

void render(const Shot& shot, const std::string& directory, const Screen& screen)
{
    auto phone = screen.phone;
    auto root = RootView {};
    auto scene =
        CowsView {{QualityChoice::High, {}}, Stages {shot.stage}, shot.seed};
    auto footer = Footer {};
    auto touch = TouchControls {};

    scene.frozen = true;
    scene.pointerHints = phone ? Hints::Touch : Hints::Keys;
    scene.hints = scene.pointerHints;
    footer.text = [&] { return footerText(scene.game, scene.hint, scene.hints); };
    root.addSubview(scene);

    if (phone)
    {
        root.addSubview(touch);
        root.setSafeAreaInsets({phoneTop, 0.f, phoneBottom, 0.f});
        footer.bottomInset = phoneBottom;
    }

    auto size = Graphics::Rect {0.f, 0.f, screen.width, screen.height};
    root.setBounds(size);
    root.resized();
    touch.resized();

    shot.setUp(scene);
    settle(scene, shot.seconds);
    touch.showAgain = scene.game.state == Game::State::Found;
    scene.drawHud = [&](Hud& hud)
    {
        footer.draw(hud);

        if (phone)
            touch.draw(hud);
    };

    auto image = root.renderToImage(screen.scale);
    auto path = directory + "/" + shot.name + ".png";

    if (!image.isValid())
    {
        LOG("could not render ", path);
        return;
    }

    image.save(FilePath {path});
    LOG(path);
}
} // namespace

std::uint32_t openMeadowSeed()
{
    for (auto seed = 1u;; ++seed)
        if (generate(meadowTemplate(), seed).hiding == Level::Hiding::Meadow)
            return seed;
}

void renderScreens(const std::string& directory, const std::string& only)
{
    enum class Screen
    {
        Menu,
        Dress,
        Settings,
        About
    };

    struct ScreenShape final
    {
        std::string name;
        float width = 0.f;
        float height = 0.f;
        Graphics::Insets safe;
        Hints hints = Hints::Keys;
    };

    struct Named final
    {
        Screen screen = Screen::Menu;
        std::string name;
    };

    constexpr auto phoneSides =
        Graphics::Insets {.left = 59.f, .bottom = 21.f, .right = 59.f};
    constexpr auto phoneUpright = Graphics::Insets {.top = 59.f, .bottom = 34.f};
    constexpr auto androidUpright = Graphics::Insets {.top = 24.f, .bottom = 24.f};
    constexpr auto tablet = Graphics::Insets {.top = 24.f, .bottom = 20.f};

    auto shapes = Vector<ScreenShape> {
        {"phone-430x932", 430.f, 932.f, phoneUpright, Hints::Touch},
        {"phone-932x430", 932.f, 430.f, phoneSides, Hints::Touch},
        {"phone-360x800", 360.f, 800.f, androidUpright, Hints::Touch},
        {"tablet-820x1180", 820.f, 1180.f, tablet, Hints::Touch},
        {"tablet-1180x820", 1180.f, 820.f, tablet, Hints::Touch},
        {"desktop-1280x800", 1280.f, 800.f, {}, Hints::Keys},
        {"desktop-1920x1080", 1920.f, 1080.f, {}, Hints::Keys},
        {"desktop-1720x720", 1720.f, 720.f, {}, Hints::Keys},
        {"desktop-650x800", 650.f, 800.f, {}, Hints::Keys},
        {"desktop-800x800", 800.f, 800.f, {}, Hints::Keys},
        {"desktop-1000x1000", 1000.f, 1000.f, {}, Hints::Keys},
        {"desktop-900x820", 900.f, 820.f, {}, Hints::Keys},
        {"desktop-820x900", 820.f, 900.f, {}, Hints::Keys},
        {"desktop-700x700", 700.f, 700.f, {}, Hints::Keys},
        {"desktop-500x500", 500.f, 500.f, {}, Hints::Keys},
        {"desktop-320x320", 320.f, 320.f, {}, Hints::Keys},
    };

    auto screens = Vector<Named> {{Screen::Menu, "menu"},
                                  {Screen::Dress, "dress"},
                                  {Screen::Settings, "settings"},
                                  {Screen::About, "about"}};

    for (const auto& screen: screens)
        for (const auto& shape: shapes)
        {
            if (!only.empty() && only != screen.name)
                continue;

            auto root = RootView {};
            auto scene =
                CowsView {{QualityChoice::High, {}}, Stages {menuStage}, menuSeed};
            auto menu = Menu {};
            auto editor = Editor {};
            auto footer = Footer {};
            auto clearance = shape.hints == Hints::Keys ? macTitleBarClearance : 0.f;

            scene.frozen = true;
            scene.menu = &menu;
            scene.pointerHints = shape.hints;
            scene.hints = scene.pointerHints;
            menu.setItems(screen.screen == Screen::About ? aboutItems()
                                                         : mainMenuItems());
            menu.topClearance = clearance;
            editor.topClearance = clearance;

            if (screen.screen == Screen::Dress)
                editor.rows = dressRows(CowSkin {});

            if (screen.screen == Screen::Settings)
            {
                editor.title = settingsTitle;
                editor.rows.add(
                    qualityRow(QualityChoice::Auto, Quality::High, false));
            }

            footer.text = [&]
            {
                if (scene.dressing)
                    return editorText(scene.hints);

                return footerText(scene.game, scene.hint, scene.hints);
            };
            root.addSubview(scene);
            root.addSubview(menu);
            root.addSubview(editor);
            root.setSafeAreaInsets(shape.safe);
            footer.bottomInset = shape.safe.bottom;
            root.setBounds({0.f, 0.f, shape.width, shape.height});
            root.resized();
            scene.viewAspect = shape.width / shape.height;
            scene.openMenu(false);

            if (screen.screen == Screen::Dress || screen.screen == Screen::Settings)
            {
                scene.openEditor(editor);
                scene.dressAmount = 1.f;
            }

            settle(scene, 2.f);
            scene.drawHud = [&](Hud& hud)
            {
                footer.draw(hud);
                menu.opacity = scene.menuOpacity();
                menu.draw(hud);

                if (scene.editor != nullptr)
                {
                    scene.editor->opacity = scene.editorOpacity();
                    scene.editor->draw(hud);
                }
            };

            // The first render lays the screen out, which picks its shape;
            // the camera frames the cow for that shape before the second, as
            // the app's swing does over the frames it draws.
            root.renderToImage(1.f);
            settle(scene, step);

            if (scene.dressing)
                scene.camera.setPose(scene.editorPose());

            auto image = root.renderToImage(1.f);
            auto path = directory + "/" + screen.name + "-" + shape.name + ".png";

            if (image.isValid())
                image.save(FilePath {path});
            else
                LOG("could not render ", path);

            if (scene.editor != nullptr && !scene.editor->placed.empty())
                LOG(path,
                    " text ",
                    scene.editor->placed.front().pointSize,
                    scene.editor->wide() ? " wide" : " tall");
            else
                LOG(path, " text ", menu.pointSize, menu.wide() ? " wide" : " tall");
        }
}

void renderScreenshots(const std::string& directory, const Screen& screen)
{
    for (const auto& shot: shots())
        render(shot, directory, screen);
}
} // namespace Cows
