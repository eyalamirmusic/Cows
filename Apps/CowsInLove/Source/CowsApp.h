#pragma once

#include "Cow/CowSkin.h"
#include "Settings.h"
#include "Scene/CowsView.h"
#include "UI/Editor.h"
#include "UI/Menu.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

namespace Cows
{
struct CowsApp final
{
    CowsApp();

    void dressCow(const CowSkin& next);
    void showSkinInDressEditor();
    void showMainMenu();
    void showAbout();
    void showSettings();
    void chooseFrameRate(FrameRate rate);
    void writeSettingsFile() const;
    void chooseMenuItem(int itemIndex);
    void connectDressEditor();
    void controlSceneByPointer(const ControlEvent& event);
    void connectSettingsPanel();
    void stepSettingsRow(int rowIndex, int steps);
    void connectRootKeys();
    bool escape();
    void connectFooterAndTouchControls();
    std::string getFooterText() const;
    void showControlsForInputRecipient();
    void showSelectionRingsForGamepad();
    void useTouchHintsOnTouchScreens();
    void drawHud(Hud& hud);
    void drawMenuAndEditor(Hud& hud);
    void addOverlaysToRoot();
    void openFirstScreen();

    FilePath settingsFilePath = settingsFile();
    Settings savedSettings = loadSettings(settingsFilePath);
    RootView root;
    CowsView scene;
    Footer footer;
    TouchControls touchControls;
    Menu menu;
    Editor dressEditor;
    Editor settingsPanel;
    bool showingAbout = false;
    Graphics::Window window;
    Graphics::GameInput gameInput {window};
};
} // namespace Cows
