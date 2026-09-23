#include "Scene/CowsView.h"
#include "Scene/Overlay.h"

namespace
{
Graphics::WindowOptions windowOptions()
{
    auto options = Graphics::WindowOptions {};
    options.width = 1280;
    options.height = 800;
    options.minWidth = 640;
    options.minHeight = 400;
    options.title = "Cows in Love";
    return options;
}

struct CowsApp final
{
    CowsApp()
    {
        root.addSubview(scene);
        root.addSubview(footer);
    }

    Cows::RootView root;
    Cows::CowsView scene;
    Cows::FooterView footer;
    Graphics::Window window {root, windowOptions()};
};
} // namespace

int main()
{
    return Apps::run<CowsApp>();
}
