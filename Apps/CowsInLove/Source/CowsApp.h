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

    void dress(const CowSkin& next);
    void showSkin();
    void showQuality();
    void save();

    FilePath savedFile = settingsFile();
    Settings saved = loadSettings(savedFile);
    RootView root;
    CowsView scene;
    Footer footer;
    TouchControls touchControls;
    Menu menu;
    Editor editor;
    Editor settingsPanel;
    Graphics::Window window;
    Graphics::GameInput gameInput {window};
};
} // namespace Cows
