#include "UI/TouchControls.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;

namespace
{
using Role = TouchControls::Role;

// A portrait phone: the stick's home bottom left, Jump bottom right and Moo up
// and left of it.
struct Phone final
{
    Phone()
    {
        controls.onControl = [this](const ControlEvent& event)
        {
            switch (event.kind)
            {
                case ControlEvent::Kind::Steer:
                    stickAhead = event.x;
                    stickTurn = event.y;
                    break;
                case ControlEvent::Kind::Jump:
                    ++jumps;
                    break;
                case ControlEvent::Kind::Moo:
                    ++moos;
                    break;
                case ControlEvent::Kind::Restart:
                    ++restarts;
                    break;
                case ControlEvent::Kind::Look:
                    lookX = event.x;
                    lookY = event.y;
                    break;
                case ControlEvent::Kind::Zoom:
                    zoom = event.x;
                    break;
            }
        };
        controls.setBounds({0.f, 0.f, 400.f, 800.f});

        // Off-screen, setBounds does not lay the view out; do it here.
        controls.resized();
    }

    TouchControls controls;
    int jumps = 0;
    int moos = 0;
    int restarts = 0;
    float stickAhead = 0.f;
    float stickTurn = 0.f;
    float lookX = 0.f;
    float lookY = 0.f;
    float zoom = 0.f;
};

bool same(Graphics::Point a, Graphics::Point b)
{
    return std::abs(a.x - b.x) < 1e-3f && std::abs(a.y - b.y) < 1e-3f;
}

const Graphics::Point leftLow {50.f, 600.f};
const Graphics::Point leftHigh {50.f, 100.f};
const Graphics::Point middle {300.f, 300.f};
} // namespace

auto tRoleAt = test("TouchControls/roleAt") = []
{
    auto phone = Phone {};
    const auto& controls = phone.controls;

    check(controls.roleAt(controls.jumpCenter) == Role::Jump);
    check(controls.roleAt(controls.mooCenter) == Role::Moo);
    check(controls.roleAt(leftLow) == Role::Stick);
    check(controls.roleAt(leftHigh) == Role::Look);
    check(controls.roleAt(middle) == Role::Look);
};

auto tRoleAtOnceFound = test("TouchControls/roleAtOnceFound") = []
{
    auto phone = Phone {};
    phone.controls.showAgain = true;
    const auto& controls = phone.controls;

    check(controls.roleAt(controls.jumpCenter) == Role::Again);
    check(controls.roleAt(controls.mooCenter) == Role::Look);
    check(controls.roleAt(leftLow) == Role::Look);
};

auto tButtonsFireOnDown = test("TouchControls/buttonsFireOnDown") = []
{
    auto phone = Phone {};
    auto& controls = phone.controls;

    controls.pointerDown(1, controls.jumpCenter);
    controls.pointerDown(2, controls.mooCenter);

    check(phone.jumps == 1);
    check(phone.moos == 1);
    check(controls.pointers.size() == 2);
    check(controls.pressed(Role::Jump));
    check(controls.pressed(Role::Moo));

    controls.pointerUp(1);
    controls.pointerUp(2);

    check(controls.pointers.empty());
    check(!controls.pressed(Role::Jump));
};

auto tAgainRestarts = test("TouchControls/againRestarts") = []
{
    auto phone = Phone {};
    phone.controls.showAgain = true;

    phone.controls.pointerDown(1, phone.controls.jumpCenter);

    check(phone.restarts == 1);
    check(phone.jumps == 0);
};

auto tStickFollowsAndReleases = test("TouchControls/stickFollowsAndReleases") = []
{
    auto phone = Phone {};
    auto& controls = phone.controls;
    auto home = controls.stickHome;

    controls.pointerDown(1, leftLow);
    check(controls.pressed(Role::Stick));
    check(same(controls.stickCenter, leftLow));

    // Only one finger holds the stick; a second on its side of the screen looks.
    check(controls.roleAt({leftLow.x + 10.f, leftLow.y}) == Role::Look);

    controls.pointerMoved(1, {leftLow.x, leftLow.y - 200.f});
    check(std::abs(phone.stickAhead - 1.f) < 1e-4f);
    check(std::abs(phone.stickTurn) < 1e-4f);

    controls.pointerUp(1);
    check(!controls.pressed(Role::Stick));
    check(same(controls.stickCenter, home));
    check(same(controls.knob, home));
    check(phone.stickAhead == 0.f && phone.stickTurn == 0.f);
};

auto tSameIdReplacesPointer = test("TouchControls/sameIdReplacesPointer") = []
{
    auto phone = Phone {};
    auto& controls = phone.controls;

    controls.pointerDown(1, leftLow);
    controls.pointerDown(1, middle);

    check(controls.pointers.size() == 1);
    check(controls.pointers[0].role == Role::Look);
    check(!controls.pressed(Role::Stick));
};

auto tDragLooks = test("TouchControls/dragLooks") = []
{
    auto phone = Phone {};
    auto& controls = phone.controls;

    controls.pointerDown(1, middle);
    controls.pointerMoved(1, {middle.x + 10.f, middle.y - 5.f});

    check(phone.lookX == 10.f);
    check(phone.lookY == -5.f);
};

auto tPinchZooms = test("TouchControls/pinchZooms") = []
{
    auto phone = Phone {};
    auto& controls = phone.controls;

    controls.pointerDown(1, {200.f, 200.f});
    controls.pointerDown(2, {300.f, 200.f});
    controls.pointerMoved(2, {400.f, 200.f});

    check(std::abs(phone.zoom - std::log(2.f)) < 1e-4f);
    check(phone.lookX == 0.f);
};

auto tNoTouchScreenOnMacOS =
    test("TouchControls/noTouchScreenOnMacOS") = [] { check(!touchScreen()); };

// Fingers reach the controls through eacp's view tree, each as its own pointer.
auto tTouchesArePointers = test("TouchControls/touchesArePointers") = []
{
    auto phone = Phone {};
    auto root = Graphics::View {};
    root.setBounds({0.f, 0.f, 400.f, 800.f});
    root.addSubview(phone.controls);

    auto touch = [&root](int id, Graphics::TouchPhase phase, Graphics::Point at)
    {
        auto event = Graphics::TouchEvent {};
        event.id = id;
        event.phase = phase;
        event.pos = at;
        root.dispatchTouchEvent(event);
    };

    touch(1, Graphics::TouchPhase::Began, phone.controls.jumpCenter);
    touch(2, Graphics::TouchPhase::Began, phone.controls.mooCenter);

    check(phone.jumps == 1);
    check(phone.moos == 1);
    check(phone.controls.pointers.size() == 2);

    touch(1, Graphics::TouchPhase::Ended, phone.controls.jumpCenter);
    touch(2, Graphics::TouchPhase::Cancelled, phone.controls.mooCenter);

    check(phone.controls.pointers.empty());
};

// The window's safe area lifts the controls clear of the home indicator.
auto tSafeAreaLiftsControls = test("TouchControls/safeAreaLiftsControls") = []
{
    auto phone = Phone {};
    auto root = Graphics::View {};
    root.setBounds({0.f, 0.f, 400.f, 800.f});
    root.addSubview(phone.controls);

    auto jumpBefore = phone.controls.jumpCenter;
    root.setSafeAreaInsets({.top = 59.f, .bottom = 34.f});

    check(same(phone.controls.jumpCenter, {jumpBefore.x, jumpBefore.y - 34.f}));
};
