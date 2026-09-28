#include "Platform.h"

namespace Cows
{
void Platform::attach(Graphics::View& root,
                      TouchControls& touchControls,
                      FooterView& footer)
{
    root.addSubview(touchControls);

    hud = std::make_unique<HudLayer>(touchControls, footer);
    surface = std::make_unique<TouchSurface>(touchControls);
    surface->onSafeAreaChanged = [&footer, &touchControls](Graphics::Insets insets)
    {
        touchControls.safeArea = insets;
        touchControls.resized();
        footer.bottomInset = insets.bottom;
    };
}

void Platform::drawOverlay(GPUView& scene, RenderPass& pass)
{
    if (hud != nullptr)
        hud->draw(scene, pass);
}
} // namespace Cows
