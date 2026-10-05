#include "UI/Overlay.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
using Graphics::KeyEventType;

Graphics::KeyEvent back(KeyEventType type, bool repeat = false)
{
    auto event = Graphics::KeyEvent {};
    event.keyCode = Graphics::KeyCode::Back;
    event.type = type;
    event.isRepeat = repeat;
    return event;
}

struct Root final
{
    explicit Root(bool takesEscape)
    {
        root.onEscape = [this, takesEscape]
        {
            ++escapes;
            return takesEscape;
        };
        root.onKeyDown = [this](const Graphics::KeyEvent&) { ++keysDown; };
        root.onKeyUp = [this](const Graphics::KeyEvent&) { ++keysUp; };
    }

    bool press(bool repeat = false)
    {
        return root.dispatchKeyEvent(back(KeyEventType::Down, repeat));
    }

    bool release() { return root.dispatchKeyEvent(back(KeyEventType::Up)); }

    RootView root;
    int escapes = 0;
    int keysDown = 0;
    int keysUp = 0;
};
} // namespace

auto tBackTakenIsKept = test("RootKey/backTakenByEscapeIsKept") = []
{
    auto root = Root {true};

    check(root.press());
    check(root.press(true));
    check(root.release());
    check(root.escapes == 1);
    check(root.keysDown == 0);
    check(root.keysUp == 0);
};

auto tBackLeftIsPassedOn = test("RootKey/backEscapeLeavesIsPassedOn") = []
{
    auto root = Root {false};

    check(!root.press());
    check(!root.press(true));
    check(!root.release());
    check(root.escapes == 1);
    check(root.keysDown == 0);
    check(root.keysUp == 0);
};

auto tReleaseFollowsPress = test("RootKey/backReleaseFollowsItsPress") = []
{
    auto taken = true;
    auto root = RootView {};
    root.onEscape = [&taken] { return taken; };

    check(root.dispatchKeyEvent(back(KeyEventType::Down)));
    taken = false;
    check(root.dispatchKeyEvent(back(KeyEventType::Up)));
    check(!root.dispatchKeyEvent(back(KeyEventType::Down)));
    taken = true;
    check(!root.dispatchKeyEvent(back(KeyEventType::Up)));
};
