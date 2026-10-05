#include "Game.h"
#include "UI/Editor.h"
#include "UI/Menu.h"
#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
using Graphics::TouchPhase;

// The app's stack on a phone: the touch controls, then the menu and the editor
// over them, all the root's size, shown as CowsApp shows them.
struct Phone final
{
    Phone()
    {
        menu.items.add({"Start", "", true});
        menu.onChoose = [this](int index) { chosen = index; };
        controls.onControl = [this](const ControlEvent& event)
        { events.add(event); };

        root.addSubview(controls);
        root.addSubview(menu);
        root.addSubview(editor);
        root.setBounds({0.f, 0.f, 400.f, 800.f});
        root.resized();

        // Off-screen, setBounds does not lay the view out; do it here.
        controls.resized();
    }

    void show(InputOwner owner)
    {
        menu.setShowing(owner == InputOwner::Menu);
        editor.setShowing(owner == InputOwner::Editor);
    }

    void touch(TouchPhase phase, Graphics::Point position, int id = 1)
    {
        root.dispatchTouchEvent({.pos = position, .id = id, .phase = phase});
    }

    bool steered() const
    {
        for (const auto& event: events)
            if (event.kind == ControlEvent::Kind::Steer && event.x > 0.5f)
                return true;

        return false;
    }

    RootView root;
    TouchControls controls;
    Menu menu;
    Editor editor;
    Vector<ControlEvent> events;
    int chosen = -1;
};

void pushStick(Phone& phone)
{
    auto home = phone.controls.stickHome;
    phone.touch(TouchPhase::Began, home);
    phone.touch(TouchPhase::Moved, {home.x, home.y - 60.f});
}
} // namespace

auto tStickWalksInPlay = test("PlayTouch/stickWalksInPlay") = []
{
    auto phone = Phone {};
    phone.show(inputOwner(Game::State::Searching, false, false));
    pushStick(phone);

    check(phone.steered());
    check(phone.chosen == -1);
};

// Start swings the camera down for over a second at 60 fps, and at the few
// frames a second of a slow phone for several, and the menu used to keep every
// touch until it landed: a thumb put on the stick in that time stayed the
// faded menu's, and the cow never walked however it was pushed.
auto tStickWalksWhileStartSwings = test("PlayTouch/stickWalksWhileStartSwings") = []
{
    auto phone = Phone {};
    phone.show(inputOwner(Game::State::Menu, false, true));
    pushStick(phone);

    check(phone.steered());
};

auto tMenuTakesTouchesBeforeStart =
    test("PlayTouch/menuTakesTouchesBeforeStart") = []
{
    auto phone = Phone {};
    phone.show(inputOwner(Game::State::Menu, false, false));
    pushStick(phone);

    check(!phone.steered());
    check(phone.events.empty());
};

auto tOwnerFollowsTheGame = test("PlayTouch/ownerFollowsTheGame") = []
{
    check(inputOwner(Game::State::Menu, false, false) == InputOwner::Menu);
    check(inputOwner(Game::State::Menu, true, false) == InputOwner::Editor);
    check(inputOwner(Game::State::Menu, false, true) == InputOwner::Play);
    check(inputOwner(Game::State::Searching, false, false) == InputOwner::Play);
    check(inputOwner(Game::State::Found, false, false) == InputOwner::Play);
};
