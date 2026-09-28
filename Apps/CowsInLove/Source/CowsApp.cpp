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
    options.title = "Cows in Love";
    return options;
}
} // namespace

CowsApp::CowsApp()
    : window {root, windowOptions()}
{
    root.addSubview(scene);
    root.addSubview(footer);

    root.onKeyDown = [this](const Graphics::KeyEvent& event)
    { scene.keyDown(event); };
    root.onKeyUp = [this](const Graphics::KeyEvent& event) { scene.keyUp(event); };

    footer.text = [this]
    { return footerText(scene.game, scene.hint, scene.touchHints); };
    scene.onStateChanged = [this]
    {
        footer.repaint();
        hud.repaint();
    };

    hud.found = [this] { return scene.game.state == Game::State::Found; };
    hud.onStick = [this](float ahead, float turn)
    { scene.input.setStick(ahead, turn); };
    hud.onJump = [this] { scene.input.jump(); };
    hud.onMoo = [this] { scene.callOut(); };
    hud.onRestart = [this] { scene.restart(); };
    hud.onLook = [this](float x, float y) { scene.look(x, y); };
    hud.onZoom = [this](float amount) { scene.camera.zoom(amount); };
    hud.onWheel = [this](const Graphics::MouseEvent& event)
    { scene.mouseWheel(event); };

    scene.touchHints = Platform::touch;
    platform.attach(root, hud, footer);
}
} // namespace Cows
