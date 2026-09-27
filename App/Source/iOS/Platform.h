#pragma once

#include "Scene/TouchControls.h"
#include "TouchSurface.h"

#include <memory>

namespace Cows
{
struct CowsApp;

struct Platform final
{
    static Graphics::WindowOptions windowOptions();
    void attach(CowsApp& app);

    TouchControls hud;
    std::unique_ptr<TouchSurface> touch;
};
} // namespace Cows
