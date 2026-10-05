#pragma once

#include "Cow/CowSkin.h"
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

    RootView root;
    CowsView scene;
    Footer footer;
    TouchControls touchControls;
    Menu menu;
    Editor editor;
    FilePath skinFile = cowSkinFile();
    CowSkin skin = loadCowSkin(skinFile);
    Graphics::Window window;
    Graphics::GameInput gameInput {window};
};
} // namespace Cows
