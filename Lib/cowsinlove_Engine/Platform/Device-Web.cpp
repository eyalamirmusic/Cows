#include "Platform/Device.h"

#include <emscripten/em_js.h>

// A phone or tablet browser: a coarse pointer (a finger) and a touch screen. A
// laptop with a touch screen still has a fine pointer, so it keeps the mouse
// and keys.
EM_JS(bool, browserHasTouchScreen, (), {
    return navigator.maxTouchPoints > 0
           && window.matchMedia("(pointer: coarse)").matches;
});

namespace Cows
{
bool touchScreen()
{
    static const auto answer = browserHasTouchScreen();
    return answer;
}

bool canQuit()
{
    return false;
}
} // namespace Cows
