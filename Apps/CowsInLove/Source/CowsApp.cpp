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

bool opensSettings()
{
    return getEnvValue("COWS_SETTINGS") == "1";
}

float titlebarClearance()
{
    return Platform::isMac() ? 28.f : 0.f;
}
} // namespace

CowsApp::CowsApp()
    : scene {saved.quality}
    , window {root, windowOptions()}
{
    root.addSubview(scene);
    scene.gameInput = &gameInput;
    scene.menu = &menu;

    menu.items.add({"Start", "", true});
    menu.items.add({"Dress Your Cow", "", true});
    menu.items.add({"Settings", "", true});
    menu.onChoose = [this](int index)
    {
        if (index == 0)
            scene.startGame();
        else if (index == 1)
            scene.openEditor(editor);
        else
            scene.openEditor(settingsPanel);
    };

    for (const auto& item: itemClasses())
        editor.rows.add({item.name, item.choices, item.choice(saved.skin)});

    editor.topClearance = titlebarClearance();
    editor.onStep = [this](int row, int by) { dress(stepped(saved.skin, row, by)); };
    editor.onDone = [this] { scene.closeEditor(); };
    editor.onControl = [this](const ControlEvent& event)
    {
        scene.useHints(scene.pointerHints);
        scene.control(event);
    };

    settingsPanel.title = "Settings";
    settingsPanel.rows.add({"Quality", {}, 0});
    settingsPanel.topClearance = editor.topClearance;
    settingsPanel.onStep = [this](int, int by)
    {
        auto next = ((int) scene.qualityPreference.chosen + by + qualityChoices)
                    % qualityChoices;
        scene.chooseQuality((QualityChoice) next);
    };
    settingsPanel.onDone = [this] { scene.closeEditor(); };
    settingsPanel.onControl = editor.onControl;
    showQuality();
    scene.wear(saved.skin);
    scene.onQualityChanged = [this]
    {
        saved.quality = scene.qualityPreference;
        save();
    };

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
        auto owner = scene.inputOwner();
        touchControls.showAgain = scene.game.state == Game::State::Found;
        menu.setShowing(owner == InputOwner::Menu);
        menu.hovered = -1;
        menu.pressed = -1;
        for (auto* each: {&editor, &settingsPanel})
        {
            each->setShowing(owner == InputOwner::Editor && scene.editor == each);
            each->hovered = {};
            each->pressed = {};
        }

        if (menu.isShowing())
            menu.showSelection = isGamepad(scene.hints);

        for (auto* each: {&editor, &settingsPanel})
            if (each->isShowing())
                each->showSelection = isGamepad(scene.hints);
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
            if (scene.editor != nullptr)
            {
                showQuality();
                scene.editor->opacity = scene.editorOpacity();
                scene.editor->draw(hud);
            }
        }

        if (scene.inputOwner() != InputOwner::Play)
            return;

        if (touchScreen && !isGamepad(scene.hints))
            touchControls.draw(hud);
    };

    if (touchScreen)
        root.addSubview(touchControls);

    root.addSubview(menu);
    root.addSubview(editor);
    root.addSubview(settingsPanel);

    if (opensOnMenu())
        scene.openMenu(false);
    else
        menu.setShowing(false);

    if (opensOnMenu() && opensDressing())
        scene.openEditor(editor);
    else if (opensOnMenu() && opensSettings())
        scene.openEditor(settingsPanel);
}

void CowsApp::dress(const CowSkin& next)
{
    saved.skin = next;
    scene.wear(saved.skin);
    showSkin();
    save();
}

void CowsApp::save()
{
    if (!saveSettings(saved, savedFile))
        LOG("Cows: could not save ", savedFile.str());
}

void CowsApp::showQuality()
{
    auto& row = settingsPanel.rows[0];
    row.choices.clear();

    for (auto index = 0; index < qualityChoices; ++index)
        row.choices.add(choiceLabel((QualityChoice) index));

    if (scene.qualityPreference.chosen == QualityChoice::Auto && !scene.measuring)
        row.choices[0] =
            "Auto: " + choiceLabel((QualityChoice) ((int) scene.quality + 1));

    row.choice = (int) scene.qualityPreference.chosen;
}

void CowsApp::showSkin()
{
    const auto& classes = itemClasses();

    for (auto index = 0; index < (int) classes.size(); ++index)
        editor.rows[index].choice = classes[index].choice(saved.skin);
}
} // namespace Cows
