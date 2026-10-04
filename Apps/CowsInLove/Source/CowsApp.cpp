#include "CowsApp.h"

namespace Cows
{
namespace
{
Graphics::WindowOptions windowOptions()
{
    auto options = Graphics::WindowOptions {};
    options.width = 1280;
    options.height = 800;
    options.minWidth = 640;
    options.minHeight = 400;
    options.title = "Cows In Love";
    return options;
}
} // namespace

CowsApp::CowsApp()
    : window {root, windowOptions()}
{
    root.addSubview(scene);
    scene.gameInput = &gameInput;

    root.onKeyDown = [this](const Graphics::KeyEvent& event)
    { scene.keyDown(event); };
    root.onKeyUp = [this](const Graphics::KeyEvent& event) { scene.keyUp(event); };

    footer.text = [this]
    { return footerText(scene.game, scene.hint, scene.touchHints); };
    touchControls.onControl = [this](const ControlEvent& event)
    { scene.control(event); };
    scene.onStateChanged = [this]
    { touchControls.showAgain = scene.game.state == Game::State::Found; };

    scene.touchHints = touchScreen;
    scene.drawHud = [this](Hud& hud)
    {
        footer.bottomInset = root.getSafeAreaInsets().bottom;
        footer.draw(hud);

        if (touchScreen)
            touchControls.draw(hud);
    };

    if (touchScreen)
        root.addSubview(touchControls);
}
} // namespace Cows
