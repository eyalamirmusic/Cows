#include "Screenshots.h"
#include "Levels/LevelGenerator.h"
#include "Scene/CowsView.h"
#include "Templates.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <cmath>
#include <cstdio>
#include <eacp/Core/Utils/Environment.h>
#include <cstdlib>
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

struct Shot final
{
    std::string name;
    int stage = 0;
    int seed = 3;
    float seconds = 2.f;
    std::function<void(CowsView&)> setUp = [](CowsView&) {};
};

void settle(CowsView& view, float seconds)
{
    for (auto time = 0.f; time < seconds; time += step)
    {
        view.game.update(step, 0.f, 0.f, false);
        view.steerCamera(step);
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
                      view.steerCamera(step);
                  }
              }});

    list.add({"6-found",
              0,
              (int) openMeadowSeed(),
              0.f,
              [](CowsView& view)
              {
                  auto& game = view.game;
                  game.player = game.partner + Vec3 {3.f, 0.f, 0.f};
                  settle(view, 4.6f);

                  if (view.touchHints)
                  {
                      view.framedPortrait = true;
                      view.camera.distance = 21.f;
                  }
              }});

    return list;
}

void setEnv(const char* name, int value)
{
    eacp::setEnv(name, std::to_string(value));
}

void render(const Shot& shot, const std::string& directory, const Screen& screen)
{
    setEnv("COWS_STAGE", shot.stage);
    setEnv("COWS_SEED", shot.seed);

    auto phone = screen.phone;
    auto root = RootView {};
    auto scene = CowsView {};
    auto footer = Footer {};
    auto touch = TouchControls {};

    scene.frozen = true;
    scene.touchHints = phone;
    footer.text = [&] { return footerText(scene.game, scene.hint, phone); };
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
        std::fprintf(stderr, "could not render %s\n", path.c_str());
        return;
    }

    image.save(FilePath {path});
    std::printf("%s\n", path.c_str());
}
} // namespace

std::uint32_t openMeadowSeed()
{
    for (auto seed = 1u;; ++seed)
        if (generate(meadowTemplate(), seed).hiding == Level::Hiding::Meadow)
            return seed;
}

void renderScreenshots(const std::string& directory, const Screen& screen)
{
    for (const auto& shot: shots())
        render(shot, directory, screen);
}
} // namespace Cows
