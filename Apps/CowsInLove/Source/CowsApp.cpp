#include "CowsApp.h"

#include <eacp/Core/Utils/Environment.h>

namespace Cows
{
namespace
{
Graphics::WindowOptions windowOptions()
{
    auto options = Graphics::WindowOptions {};
    options.width = 1280;
    options.height = 800;
    options.minWidth = 640;
    options.minHeight = 400;
    options.title = "Cows In Love";
    options.flags.add(Graphics::WindowFlags::FullSizeContentView);
    options.titlebarTransparent = true;
    options.showTitle = !Platform::isMac();
    options.showTitlebarSeparator = false;
    return options;
}

bool opensOnMenu()
{
    return getEnvValue("COWS_MENU") != "0" && getEnvValue("COWS_FOUND").empty();
}
} // namespace

CowsApp::CowsApp()
    : window {root, windowOptions()}
{
    root.addSubview(scene);
    scene.gameInput = &gameInput;
    scene.menu = &menu;

    menu.items.add({"Start", "", true});
    menu.items.add({"Dress Your Cow", "Coming Soon!", false});
    menu.onChoose = [this](int index)
    {
        if (index == 0)
            scene.startGame();
    };

    root.onKeyDown = [this](const Graphics::KeyEvent& event)
    { scene.keyDown(event); };
    root.onKeyUp = [this](const Graphics::KeyEvent& event) { scene.keyUp(event); };
    root.onEscape = [this] { return scene.escape(); };

    footer.text = [this] { return footerText(scene.game, scene.hint, scene.hints); };
    touchControls.onControl = [this](const ControlEvent& event)
    {
        scene.useHints(scene.pointerHints);
        scene.control(event);
    };
    scene.onStateChanged = [this]
    {
        auto inMenu = scene.game.state == Game::State::Menu;
        touchControls.showAgain = scene.game.state == Game::State::Found;
        menu.showing = inMenu;
        menu.hovered = -1;
        menu.pressed = -1;

        if (inMenu)
            menu.showSelection = isGamepad(scene.hints);
    };

    if (touchScreen)
    {
        scene.pointerHints = Hints::Touch;
        scene.hints = Hints::Touch;
    }

    scene.drawHud = [this](Hud& hud)
    {
        footer.bottomInset = root.getSafeAreaInsets().bottom;
        footer.draw(hud);

        if (scene.game.state == Game::State::Menu)
        {
            menu.opacity = scene.menuOpacity();
            menu.draw(hud);
            return;
        }

        if (touchScreen && !isGamepad(scene.hints))
            touchControls.draw(hud);
    };

    if (touchScreen)
        root.addSubview(touchControls);

    root.addSubview(menu);

    if (opensOnMenu())
        scene.openMenu(false);
    else
        menu.showing = false;
}
} // namespace Cows
