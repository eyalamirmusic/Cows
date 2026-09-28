#include "Platform.h"

namespace Cows
{
void Platform::attach(Graphics::View& root,
                      TouchControls& touchControls,
                      Footer& footer)
{
    setUpAudioSession();
    makeSeeThrough(touchControls);
    root.addSubview(touchControls);

    surface = std::make_unique<TouchSurface>(root, touchControls);
    surface->onSafeAreaChanged = [&footer, &touchControls](Graphics::Insets insets)
    {
        touchControls.safeArea = insets;
        touchControls.resized();
        footer.bottomInset = insets.bottom;
    };
}
} // namespace Cows
