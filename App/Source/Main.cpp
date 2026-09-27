#include "Scene/CowsView.h"
#include "Scene/Overlay.h"
#include "Scene/TouchControls.h"

#include <TargetConditionals.h>

#if TARGET_OS_IOS
#include "Scene/TouchSurface.h"
#endif

namespace
{
Graphics::WindowOptions windowOptions()
{
    auto options = Graphics::WindowOptions {};
    options.width = 540;
    options.height = 960;
    options.minWidth = 360;
    options.minHeight = 640;
    options.title = "Cows in Love";
    return options;
}

struct CowsApp final
{
    CowsApp()
    {
        root.addSubview(scene);
        root.addSubview(footer);
        root.addSubview(hud);

        root.onKeyDown = [this](const Graphics::KeyEvent& event)
        { scene.keyDown(event); };
        root.onKeyUp = [this](const Graphics::KeyEvent& event)
        { scene.keyUp(event); };

        footer.text = [this] { return scene.footerText(); };
        scene.onStateChanged = [this]
        {
            footer.repaint();
            hud.repaint();
        };

        hud.found = [this]
        {
          return scene.game.state == Cows::Game::State::Found;
        };

        hud.onStick = [this](float ahead, float turn)
        {
          scene.setStick(ahead, turn);
        };
        hud.onJump = [this] { scene.jump(); };
        hud.onMoo = [this] { scene.callOut(); };
        hud.onRestart = [this] { scene.restart(); };
        hud.onLook = [this](float x, float y) { scene.look(x, y); };
        hud.onZoom = [this](float amount) { scene.camera.zoom(amount); };
        hud.onWheel = [this](const Graphics::MouseEvent& event) { scene.mouseWheel(event); };

#if TARGET_OS_IOS
        scene.touchHints = true;
        Cows::setUpAudioSession();
        Cows::makeSeeThrough(footer);
        Cows::makeSeeThrough(hud);
        touch = std::make_unique<Cows::TouchSurface>(root, hud);
        touch->onSafeAreaChanged = [this](Graphics::Insets insets)
        {
            hud.safeArea = insets;
            hud.resized();
            footer.bottomInset = insets.bottom;
            footer.repaint();
        };
#endif
    }

    Cows::RootView root;
    Cows::CowsView scene;
    Cows::FooterView footer;
    Cows::TouchControls hud;
    Graphics::Window window {root, windowOptions()};

#if TARGET_OS_IOS
    std::unique_ptr<Cows::TouchSurface> touch;
#endif
};
} // namespace

int main()
{
    return Apps::run<CowsApp>();
}
