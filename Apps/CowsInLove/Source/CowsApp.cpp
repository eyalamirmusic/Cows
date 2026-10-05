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

bool opensDressing()
{
    return getEnvValue("COWS_DRESS") == "1";
}

float titlebarClearance()
{
    return Platform::isMac() ? 28.f : 0.f;
}
} // namespace

CowsApp::CowsApp()
    : window {root, windowOptions()}
{
    root.addSubview(scene);
    scene.gameInput = &gameInput;
    scene.menu = &menu;
    scene.editor = &editor;

    menu.items.add({"Start", "", true});
    menu.items.add({"Dress Your Cow", "", true});
    menu.onChoose = [this](int index)
    {
        if (index == 0)
            scene.startGame();
        else
            scene.openEditor();
    };

    for (const auto& item: itemClasses())
        editor.rows.add({item.name, item.choices, item.choice(skin)});

    editor.topClearance = titlebarClearance();
    editor.onStep = [this](int row, int by) { dress(stepped(skin, row, by)); };
    editor.onDone = [this] { scene.closeEditor(); };
    editor.onControl = [this](const ControlEvent& event)
    {
        scene.useHints(scene.pointerHints);
        scene.control(event);
    };
    scene.wear(skin);

    root.onKeyDown = [this](const Graphics::KeyEvent& event)
    { scene.keyDown(event); };
    root.onKeyUp = [this](const Graphics::KeyEvent& event) { scene.keyUp(event); };
    root.onEscape = [this] { return scene.escape(); };

    footer.text = [this]
    {
        if (scene.dressing)
            return editorText(scene.hints);

        return footerText(scene.game, scene.hint, scene.hints);
    };
    touchControls.onControl = [this](const ControlEvent& event)
    {
        scene.useHints(scene.pointerHints);
        scene.control(event);
    };
    scene.onStateChanged = [this]
    {
        auto inMenu = scene.game.state == Game::State::Menu;
        touchControls.showAgain = scene.game.state == Game::State::Found;
        menu.setShowing(inMenu && !scene.dressing);
        menu.hovered = -1;
        menu.pressed = -1;
        editor.setShowing(inMenu && scene.dressing);
        editor.hovered = {};
        editor.pressed = {};

        if (menu.isShowing())
            menu.showSelection = isGamepad(scene.hints);

        if (editor.isShowing())
            editor.showSelection = isGamepad(scene.hints);
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
            editor.opacity = scene.editorOpacity();
            editor.draw(hud);
            return;
        }

        if (touchScreen && !isGamepad(scene.hints))
            touchControls.draw(hud);
    };

    if (touchScreen)
        root.addSubview(touchControls);

    root.addSubview(menu);
    root.addSubview(editor);

    if (opensOnMenu())
        scene.openMenu(false);
    else
        menu.setShowing(false);

    if (opensOnMenu() && opensDressing())
        scene.openEditor();
}

void CowsApp::dress(const CowSkin& next)
{
    skin = next;
    scene.wear(skin);
    showSkin();

    if (!saveCowSkin(skin, skinFile))
        LOG("Cows: could not save ", skinFile.str());
}

void CowsApp::showSkin()
{
    const auto& classes = itemClasses();

    for (auto index = 0; index < (int) classes.size(); ++index)
        editor.rows[index].choice = classes[index].choice(skin);
}
} // namespace Cows
