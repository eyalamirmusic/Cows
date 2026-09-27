#pragma once

#include "Platform.h"
#include "Scene/CowsView.h"
#include "Scene/Overlay.h"

namespace Cows
{
struct CowsApp final
{
    CowsApp();

    RootView root;
    CowsView scene;
    FooterView footer;
    Graphics::Window window;
    Platform platform;
};
} // namespace Cows
