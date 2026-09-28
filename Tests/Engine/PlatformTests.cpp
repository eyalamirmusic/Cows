#include "Platform.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

auto tPlatformHasNoTouch =
    test("Platform/macOSHasNoTouch") = [] { check(!Cows::Platform::touch); };

auto tPlatformAttachAddsNoSubview = test("Platform/attachAddsNoSubview") = []
{
    auto root = RootView {};
    auto touchControls = TouchControls {};
    auto footer = Footer {};

    auto before = root.getSubviews().size();
    auto platform = Cows::Platform {};
    platform.attach(root, touchControls, footer);

    check(root.getSubviews().size() == before);
    check(touchControls.getParent() == nullptr);
};
