#include "Platform.h"

namespace Cows
{
void Platform::attach(Graphics::View& root,
                      TouchControls& touchControls,
                      FooterView& footer)
{
    setUpAudioSession();
    makeSeeThrough(footer);
    makeSeeThrough(touchControls);
    root.addSubview(touchControls);

    surface = std::make_unique<TouchSurface>(root, touchControls);
    surface->onSafeAreaChanged = [&footer, &touchControls](Graphics::Insets insets)
    {
        touchControls.safeArea = insets;
        touchControls.resized();
        footer.bottomInset = insets.bottom;
        footer.repaint();
    };
}
} // namespace Cows
