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
    return Platform::isMac() ? macTitleBarClearance : 0.f;
}
} // namespace

CowsApp::CowsApp()
    : scene {savedSettings.quality, Stages {firstStage()}, firstSeed()}
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
            scene.openEditor(dressEditor);
        else if (index == 2)
            scene.openEditor(settingsPanel);
        else
            showAbout();
    };

    dressEditor.setTitle(dressTitle);
    dressEditor.setRows(makeDressRows(savedSettings.skin));
    dressEditor.topClearance = titlebarClearance();
    menu.topClearance = dressEditor.topClearance;
    dressEditor.onRowValueStepped = [this](int row, int by)
    { dressCow(stepSkinItemChoice(savedSettings.skin, row, by)); };
    dressEditor.onDone = [this] { scene.closeEditor(); };
    dressEditor.onControl = [this](const ControlEvent& event)
    {
        scene.useHints(scene.pointerHints);
        scene.control(event);
    };

    settingsPanel.setTitle(settingsTitle);
    settingsPanel.topClearance = dressEditor.topClearance;
    settingsPanel.onRowValueStepped = [this](int row, int by)
    {
        if (row == 1)
        {
            chooseFrameRate(
                (FrameRate) (((int) savedSettings.frameRate + by + numFrameRates)
                             % numFrameRates));
            return;
        }

        auto next = ((int) scene.qualityPreference.chosen + by + numQualityChoices)
                    % numQualityChoices;
        scene.chooseQuality((QualityChoice) next);
    };
    settingsPanel.onDone = [this] { scene.closeEditor(); };
    settingsPanel.onControl = dressEditor.onControl;
    scene.frameRate = savedSettings.frameRate;
    showSettings();
    scene.wear(savedSettings.skin);
    scene.onQualityChanged = [this]
    {
        savedSettings.quality = scene.qualityPreference;
        writeSettingsFile();
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
        auto owner = scene.getInputRecipient();
        touchControls.showAgain = scene.game.state == Game::State::Found;
        menu.setShowing(owner == InputRecipient::Menu);
        menu.hovered = -1;
        menu.pressed = -1;
        for (auto* each: {&dressEditor, &settingsPanel})
        {
            each->setShowing(owner == InputRecipient::Editor
                             && scene.editor == each);
            each->hovered = {};
            each->pressed = {};
        }

        if (menu.isShowing())
            menu.showSelection = isGamepad(scene.hints);

        for (auto* each: {&dressEditor, &settingsPanel})
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
                showSettings();
                scene.editor->opacity = scene.editorOpacity();
                scene.editor->draw(hud);
            }
        }

        if (scene.getInputRecipient() != InputRecipient::Play)
            return;

        if (touchScreen && !isGamepad(scene.hints))
            touchControls.draw(hud);
    };

    if (touchScreen)
        root.addSubview(touchControls);

    root.addSubview(menu);
    root.addSubview(dressEditor);
    root.addSubview(settingsPanel);

    if (opensOnMenu())
        scene.openMenu(false);
    else
        menu.setShowing(false);

    if (opensOnMenu() && opensDressing())
        scene.openEditor(dressEditor);
    else if (opensOnMenu() && opensSettings())
        scene.openEditor(settingsPanel);
}

void CowsApp::showMainMenu()
{
    showingAbout = false;
    menu.setItems(makeMainMenuItems());
    menu.selected = 0;
    menu.hovered = -1;
    menu.pressed = -1;
}

void CowsApp::showAbout()
{
    showingAbout = true;
    menu.setItems(makeAboutItems());
    menu.selected = 0;
    menu.hovered = -1;
    menu.pressed = -1;
}

void CowsApp::dressCow(const CowSkin& next)
{
    savedSettings.skin = next;
    scene.wear(savedSettings.skin);
    showSkinInDressEditor();
    writeSettingsFile();
}

void CowsApp::writeSettingsFile() const
{
    if (!saveSettings(savedSettings, settingsFilePath))
        LOG("Cows: could not save ", settingsFilePath.str());
}

void CowsApp::chooseFrameRate(FrameRate rate)
{
    savedSettings.frameRate = rate;
    scene.frameRate = rate;
    showSettings();
    writeSettingsFile();
}

void CowsApp::showSettings()
{
    auto rows = makeSettingsRows(scene.qualityPreference.chosen,
                                 scene.quality,
                                 scene.measuring,
                                 scene.frameRate);
    auto& shown = settingsPanel.rows;

    if (shown.size() != rows.size() || shown[0].choices != rows[0].choices)
    {
        settingsPanel.setRows(rows);
        return;
    }

    for (auto index = 0; index < (int) rows.size(); ++index)
        shown[index].choice = rows[index].choice;
}

void CowsApp::showSkinInDressEditor()
{
    const auto& classes = itemClasses();

    for (auto index = 0; index < (int) classes.size(); ++index)
        dressEditor.rows[index].choice = classes[index].choice(savedSettings.skin);
}
} // namespace Cows
