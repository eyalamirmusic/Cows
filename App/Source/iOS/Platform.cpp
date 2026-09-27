#include "Platform.h"

#include "CowsApp.h"

namespace Cows
{
Graphics::WindowOptions Platform::windowOptions()
{
    auto options = Graphics::WindowOptions {};
    options.width = 540;
    options.height = 960;
    options.minWidth = 360;
    options.minHeight = 640;
    options.title = "Cows in Love";
    return options;
}

void Platform::attach(CowsApp& app)
{
    auto& scene = app.scene;
    auto& footer = app.footer;

    scene.touchHints = true;
    setUpAudioSession();
    makeSeeThrough(footer);
    makeSeeThrough(hud);
    app.root.addSubview(hud);

    scene.onStateChanged = [&footer, this]
    {
        footer.repaint();
        hud.repaint();
    };

    hud.found = [&scene] { return scene.game.state == Game::State::Found; };
    hud.onStick = [&scene](float ahead, float turn) { scene.setStick(ahead, turn); };
    hud.onJump = [&scene] { scene.jump(); };
    hud.onMoo = [&scene] { scene.callOut(); };
    hud.onRestart = [&scene] { scene.restart(); };
    hud.onLook = [&scene](float x, float y) { scene.look(x, y); };
    hud.onZoom = [&scene](float amount) { scene.camera.zoom(amount); };
    hud.onWheel = [&scene](const Graphics::MouseEvent& event)
    { scene.mouseWheel(event); };

    touch = std::make_unique<TouchSurface>(app.root, hud);
    touch->onSafeAreaChanged = [&footer, this](Graphics::Insets insets)
    {
        hud.safeArea = insets;
        hud.resized();
        footer.bottomInset = insets.bottom;
        footer.repaint();
    };
}
} // namespace Cows
