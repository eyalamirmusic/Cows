#pragma once

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
    Footer footer;
    TouchControls touchControls;
    Graphics::Window window;
    Graphics::GameInput gameInput {window};
};
} // namespace Cows
