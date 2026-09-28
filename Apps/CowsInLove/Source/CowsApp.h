#pragma once

#include "Platform.h"
#include "Scene/CowsView.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

namespace Cows
{
struct CowsApp final
{
    CowsApp();

    RootView root;
    CowsView scene;
    FooterView footer;
    TouchControls hud;
    Graphics::Window window;
    Platform platform;
};
} // namespace Cows
