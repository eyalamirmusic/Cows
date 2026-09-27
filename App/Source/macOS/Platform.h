#pragma once

#include "Scene/Common.h"

namespace Cows
{
struct CowsApp;

struct Platform final
{
    static Graphics::WindowOptions windowOptions()
    {
        auto options = Graphics::WindowOptions {};
        options.width = 1280;
        options.height = 800;
        options.minWidth = 640;
        options.minHeight = 400;
        options.title = "Cows in Love";
        return options;
    }

    void attach(CowsApp&) {}
};
} // namespace Cows
