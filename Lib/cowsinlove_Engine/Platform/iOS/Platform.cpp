#include "Platform.h"

namespace Cows
{
void Platform::attach(Graphics::View& root, TouchControls& hud, FooterView& footer)
{
    setUpAudioSession();
    makeSeeThrough(footer);
    makeSeeThrough(hud);
    root.addSubview(hud);

    surface = std::make_unique<TouchSurface>(root, hud);
    surface->onSafeAreaChanged = [&footer, &hud](Graphics::Insets insets)
    {
        hud.safeArea = insets;
        hud.resized();
        footer.bottomInset = insets.bottom;
        footer.repaint();
    };
}
} // namespace Cows
