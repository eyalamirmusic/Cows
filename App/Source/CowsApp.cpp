#include "CowsApp.h"

namespace Cows
{
CowsApp::CowsApp()
    : window {root, Platform::windowOptions()}
{
    root.addSubview(scene);
    root.addSubview(footer);

    root.onKeyDown = [this](const Graphics::KeyEvent& event)
    { scene.keyDown(event); };
    root.onKeyUp = [this](const Graphics::KeyEvent& event) { scene.keyUp(event); };

    footer.text = [this] { return scene.footerText(); };
    scene.onStateChanged = [this] { footer.repaint(); };

    platform.attach(*this);
}
} // namespace Cows
