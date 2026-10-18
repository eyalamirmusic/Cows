#include "CowsApp.h"
#include "MainMenu.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <string>

namespace Cows
{
namespace
{
constexpr auto defaultWidth = 1280;
constexpr auto defaultHeight = 800;
constexpr auto smallestSide = 320;

struct WindowSize final
{
    int width = defaultWidth;
    int height = defaultHeight;
};

// COWS_WINDOW=<width>x<height> in points, else the default.
WindowSize firstWindowSize()
{
    auto value = getEnvValue("COWS_WINDOW");
    auto by = value.find('x');

    if (by == std::string::npos)
        return {};

    try
    {
        return {std::max(smallestSide, std::stoi(value.substr(0, by))),
                std::max(smallestSide, std::stoi(value.substr(by + 1)))};
    }
    catch (const std::exception&)
    {
        return {};
    }
}

Graphics::WindowOptions windowOptions()
{
    auto size = firstWindowSize();
    auto options = Graphics::WindowOptions {};
    options.width = size.width;
    options.height = size.height;
    options.minWidth = smallestSide;
    options.minHeight = smallestSide;
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

int firstStage()
{
    auto stage = getEnvValue("COWS_STAGE");
    if (stage.empty())
        return 0;
    return std::stoi(stage);
}

std::uint32_t firstSeed()
{
    auto seed = getEnvValue("COWS_SEED");
    if (seed.empty())
        return clockSeed();
    return (std::uint32_t) std::stoul(seed);
}

float titlebarClearance()
{
    return Platform::isMac() ? 28.f : 0.f;
}

std::string versionText()
{
    return std::string {"Version " COWS_APP_VERSION " (Build " COWS_APP_BUILD ")"};
}
} // namespace

CowsApp::CowsApp()
    : scene {saved.quality, Stages {firstStage()}, firstSeed()}
    , window {root, windowOptions()}
{
    root.addSubview(scene);
    scene.gameInput = &gameInput;
    scene.menu = &menu;

    showMainMenu();
    menu.onChoose = [this](int index)
    {
        if (showingAbout)
        {
            showMainMenu();
            return;
        }

        if (index == 0)
            scene.startGame();
        else if (index == 1)
            scene.openEditor(editor);
        else if (index == 2)
            scene.openEditor(settingsPanel);
        else
            showAbout();
    };

    for (const auto& item: itemClasses())
        editor.rows.add({item.name, item.choices, item.choice(saved.skin)});

    editor.topClearance = titlebarClearance();
    menu.topClearance = editor.topClearance;
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
    root.onEscape = [this]
    {
        if (showingAbout)
        {
            showMainMenu();
            return true;
        }

        return scene.escape();
    };

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

void CowsApp::showMainMenu()
{
    showingAbout = false;
    menu.items = mainMenuItems();
    menu.selected = 0;
    menu.hovered = -1;
    menu.pressed = -1;
}

void CowsApp::showAbout()
{
    showingAbout = true;
    menu.items.clear();
    menu.items.add({"Back", versionText()});
    menu.selected = 0;
    menu.hovered = -1;
    menu.pressed = -1;
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
