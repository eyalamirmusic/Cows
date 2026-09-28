#include "Platform.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tPlatformHasNoTouch =
    test("Platform/macOSHasNoTouch") = [] { check(!Cows::Platform::touch); };

auto tPlatformAttachAddsNoSubview = test("Platform/attachAddsNoSubview") = []
{
    auto root = RootView {};
    auto hud = TouchControls {};
    auto footer = FooterView {};
    root.addSubview(footer);

    auto before = root.getSubviews().size();
    auto platform = Cows::Platform {};
    platform.attach(root, hud, footer);

    check(root.getSubviews().size() == before);
    check(hud.getParent() == nullptr);
};
