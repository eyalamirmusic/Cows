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

struct Phone final
{
    Phone()
    {
        menu.items.add({"Start"});
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

    void show(InputRecipient recipient)
    {
        menu.setShowing(recipient == InputRecipient::Menu);
        editor.setShowing(recipient == InputRecipient::Editor);
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
    phone.show(getInputRecipient(Game::State::Searching, false, false));
    pushStick(phone);

    check(phone.steered());
    check(phone.chosen == -1);
};

// The menu used to keep every touch until the camera swing landed.
auto tStickWalksWhileStartSwings = test("PlayTouch/stickWalksWhileStartSwings") = []
{
    auto phone = Phone {};
    phone.show(getInputRecipient(Game::State::Menu, false, true));
    pushStick(phone);

    check(phone.steered());
};

auto tMenuTakesTouchesBeforeStart =
    test("PlayTouch/menuTakesTouchesBeforeStart") = []
{
    auto phone = Phone {};
    phone.show(getInputRecipient(Game::State::Menu, false, false));
    pushStick(phone);

    check(!phone.steered());
    check(phone.events.empty());
};

auto tOwnerFollowsTheGame = test("PlayTouch/ownerFollowsTheGame") = []
{
    check(getInputRecipient(Game::State::Menu, false, false)
          == InputRecipient::Menu);
    check(getInputRecipient(Game::State::Menu, true, false)
          == InputRecipient::Editor);
    check(getInputRecipient(Game::State::Menu, false, true) == InputRecipient::Play);
    check(getInputRecipient(Game::State::Searching, false, false)
          == InputRecipient::Play);
    check(getInputRecipient(Game::State::Found, false, false)
          == InputRecipient::Play);
};
