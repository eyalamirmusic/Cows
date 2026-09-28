#include "TouchSurface.h"

#include <eacp/Graphics/Window/Android.h>

namespace Cows
{
using Graphics::Android::TouchEvent;
using Graphics::Android::TouchPhase;

TouchSurface::TouchSurface(TouchControls& controls)
{
    Graphics::Android::setTouchHandler(
        [&controls](const TouchEvent& touch)
        {
            auto id = touch.pointerId + 1;

            switch (touch.phase)
            {
                case TouchPhase::Began:
                    controls.pointerDown(id, touch.position);
                    break;
                case TouchPhase::Moved:
                    controls.pointerMoved(id, touch.position);
                    break;
                case TouchPhase::Ended:
                case TouchPhase::Cancelled:
                    controls.pointerUp(id);
                    break;
            }

            return true;
        });

    Graphics::Android::setSafeAreaHandler([this](Graphics::Insets insets)
                                          { onSafeAreaChanged(insets); });

    Threads::callAsync(
        [this] { onSafeAreaChanged(Graphics::Android::getSafeAreaInsets()); });
}

TouchSurface::~TouchSurface()
{
    Graphics::Android::setTouchHandler({});
    Graphics::Android::setSafeAreaHandler({});
}
} // namespace Cows
