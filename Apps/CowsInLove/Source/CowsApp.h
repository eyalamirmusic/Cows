#pragma once

#include "Cow/CowSkin.h"
#include "Settings.h"
#include "Scene/CowsView.h"
#include "UI/Editor.h"
#include "UI/Menu.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <eacp/Core/Plugins/HotReload.h>
#include <eacp/Core/Plugins/Reloadable.h>

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
    Plugins::HotReloadedModule reloadedModule {Plugins::hotReloadTarget()};
};
} // namespace Cows
