#include "UI/Editor.h"
#include "UI/Menu.h"
#include "UI/Overlay.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
using Graphics::TouchPhase;

// The app's stack: the menu, then the editor over it, both the root's size.
struct Screen final
{
    Screen()
    {
        menu.items.add({"Start", "", true});
        menu.onChoose = [this](int index) { chosen = index; };

        root.addSubview(menu);
        root.addSubview(editor);
        root.setBounds({0.f, 0.f, 400.f, 800.f});
        root.resized();

        // Off-screen nothing draws, so place Start by hand.
        auto start = Menu::Placed {};
        start.area = {100.f, 300.f, 200.f, 100.f};
        menu.placed.add(start);
    }

    void tap(Graphics::Point position, int id = 1)
    {
        for (auto phase: {TouchPhase::Began, TouchPhase::Ended})
            root.dispatchTouchEvent({.pos = position, .id = id, .phase = phase});
    }

    RootView root;
    Menu menu;
    Editor editor;
    int chosen = -1;
};
} // namespace

// eacp hands a finger to the topmost view that handles touches without
// asking hitTest, so the hidden editor over the menu took every tap on iOS.
auto tTapReachesMenuUnderHiddenEditor =
    test("MenuTouch/tapReachesMenuUnderHiddenEditor") = []
{
    auto screen = Screen {};
    screen.tap({200.f, 350.f});

    check(screen.chosen == 0);
    check(screen.editor.lookingTouch == -1);
};

auto tShownEditorTakesTheTap = test("MenuTouch/shownEditorTakesTheTap") = []
{
    auto screen = Screen {};
    screen.menu.setShowing(false);
    screen.editor.setShowing(true);

    screen.root.dispatchTouchEvent(
        {.pos = {200.f, 350.f}, .id = 1, .phase = TouchPhase::Began});

    check(screen.chosen == -1);
    check(screen.editor.lookingTouch == 1);
};

auto tHiddenMenuTakesNoTap = test("MenuTouch/hiddenMenuTakesNoTap") = []
{
    auto screen = Screen {};
    screen.menu.setShowing(false);
    screen.tap({200.f, 350.f});

    check(screen.chosen == -1);
};
