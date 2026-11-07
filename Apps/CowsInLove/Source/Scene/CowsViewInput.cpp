#include "CowsView.h"
#include "CowsViewTuning.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <span>

using namespace Maths;

namespace Cows
{
using namespace CowsViewTuning;

namespace
{
enum class MenuKeyAction
{
    None,
    Choose,
    SelectPrevious,
    SelectNext,
    StepBack,
    StepOn
};

struct KeyAndAction final
{
    std::uint16_t keyCode = 0;
    MenuKeyAction action = MenuKeyAction::None;
};

constexpr auto menuKeyActions = std::to_array<KeyAndAction>({
    {Graphics::KeyCode::Return, MenuKeyAction::Choose},
    {Graphics::KeyCode::KeypadEnter, MenuKeyAction::Choose},
    {Graphics::KeyCode::Space, MenuKeyAction::Choose},
    {Graphics::KeyCode::LeftArrow, MenuKeyAction::SelectPrevious},
    {Graphics::KeyCode::UpArrow, MenuKeyAction::SelectPrevious},
    {Graphics::KeyCode::A, MenuKeyAction::SelectPrevious},
    {Graphics::KeyCode::W, MenuKeyAction::SelectPrevious},
    {Graphics::KeyCode::RightArrow, MenuKeyAction::SelectNext},
    {Graphics::KeyCode::DownArrow, MenuKeyAction::SelectNext},
    {Graphics::KeyCode::D, MenuKeyAction::SelectNext},
    {Graphics::KeyCode::S, MenuKeyAction::SelectNext},
});

constexpr auto editorKeyActions = std::to_array<KeyAndAction>({
    {Graphics::KeyCode::Return, MenuKeyAction::Choose},
    {Graphics::KeyCode::KeypadEnter, MenuKeyAction::Choose},
    {Graphics::KeyCode::Space, MenuKeyAction::Choose},
    {Graphics::KeyCode::LeftArrow, MenuKeyAction::StepBack},
    {Graphics::KeyCode::A, MenuKeyAction::StepBack},
    {Graphics::KeyCode::RightArrow, MenuKeyAction::StepOn},
    {Graphics::KeyCode::D, MenuKeyAction::StepOn},
    {Graphics::KeyCode::UpArrow, MenuKeyAction::SelectPrevious},
    {Graphics::KeyCode::W, MenuKeyAction::SelectPrevious},
    {Graphics::KeyCode::DownArrow, MenuKeyAction::SelectNext},
    {Graphics::KeyCode::S, MenuKeyAction::SelectNext},
});

MenuKeyAction findMenuKeyAction(std::span<const KeyAndAction> keyActions,
                                const Graphics::KeyEvent& event)
{
    auto keyCode = event.keyCode;
    auto found = std::find_if(keyActions.begin(),
                              keyActions.end(),
                              [keyCode](const KeyAndAction& each)
                              { return each.keyCode == keyCode; });

    if (found == keyActions.end())
        return MenuKeyAction::None;

    return found->action;
}

bool isPlayActionKey(const Graphics::KeyEvent& event)
{
    return event.keyCode == Graphics::KeyCode::M
           || event.keyCode == Graphics::KeyCode::R;
}

int readStickStep(float stick)
{
    if (stick > menuStick)
        return -1;

    if (stick < -menuStick)
        return 1;

    return 0;
}

int readMenuStickStep(const PadControls& pad)
{
    if (pad.turn > menuStick || pad.ahead > menuStick)
        return -1;

    if (pad.turn < -menuStick || pad.ahead < -menuStick)
        return 1;

    return 0;
}

EditorStickSteps readEditorStickSteps(const PadControls& pad)
{
    auto sideways = std::abs(pad.turn) >= std::abs(pad.ahead);

    if (sideways)
        return {readStickStep(pad.turn), 0};

    return {0, readStickStep(pad.ahead)};
}
} // namespace

bool CowsView::menuKey(const Graphics::KeyEvent& event)
{
    if (game.state != Game::State::Menu || menu == nullptr)
        return false;

    if (dressing)
        return editorKey(event);

    switch (findMenuKeyAction(menuKeyActions, event))
    {
        case MenuKeyAction::Choose:
            chooseUnlessKeyRepeats(event, [this] { menu->chooseSelected(); });
            break;
        case MenuKeyAction::SelectPrevious:
            menu->selectPrevious();
            break;
        case MenuKeyAction::SelectNext:
            menu->selectNext();
            break;
        default:
            break;
    }

    return true;
}

bool CowsView::editorKey(const Graphics::KeyEvent& event)
{
    if (editor == nullptr)
        return true;

    switch (findMenuKeyAction(editorKeyActions, event))
    {
        case MenuKeyAction::Choose:
            chooseUnlessKeyRepeats(event, [this] { editor->chooseSelected(); });
            break;
        case MenuKeyAction::StepBack:
            editor->stepSelected(-1);
            break;
        case MenuKeyAction::StepOn:
            editor->stepSelected(1);
            break;
        case MenuKeyAction::SelectPrevious:
            editor->selectPrevious();
            break;
        case MenuKeyAction::SelectNext:
            editor->selectNext();
            break;
        default:
            break;
    }

    return true;
}

void CowsView::chooseUnlessKeyRepeats(const Graphics::KeyEvent& event,
                                      const std::function<void()>& choose)
{
    if (event.isRepeat)
        return;

    keyFromMenu = event.keyCode;
    choose();
}

void CowsView::keyDown(const Graphics::KeyEvent& event)
{
    useHints(pointerHints);

    if (menuKey(event))
        return;

    if (keyFromMenu == event.keyCode)
        return;

    if (isPlayActionKey(event))
    {
        if (!event.isRepeat)
            pressPlayActionKey(event);

        return;
    }

    input.setHeld(event.keyCode, true);
}

void CowsView::pressPlayActionKey(const Graphics::KeyEvent& event)
{
    if (event.keyCode == Graphics::KeyCode::M)
        callOut();
    else
        restart();
}

void CowsView::keyUp(const Graphics::KeyEvent& event)
{
    if (keyFromMenu == event.keyCode)
        keyFromMenu.reset();

    input.setHeld(event.keyCode, false);
}

void CowsView::control(const ControlEvent& event)
{
    switch (event.kind)
    {
        case ControlEvent::Kind::Steer:
            input.setStick(event.x, event.y);
            break;
        case ControlEvent::Kind::Jump:
            input.jump();
            break;
        case ControlEvent::Kind::Moo:
            callOut();
            break;
        case ControlEvent::Kind::Restart:
            restartOrAdvanceStage();
            break;
        case ControlEvent::Kind::Look:
            camera.orbit(event.x * orbitSpeed, event.y * orbitSpeed);
            break;
        case ControlEvent::Kind::Zoom:
            camera.zoom(event.x);
            break;
        case ControlEvent::Kind::Pause:
            openMenu();
            break;
    }
}

void CowsView::restartOrAdvanceStage()
{
    if (game.state == Game::State::Found)
        advanceStage();
    else
        restart();
}

void CowsView::readGameInput(float delta)
{
    if (gameInput == nullptr)
        return;

    usePad(readPad(gameInput->snapshot()), delta);
}

void CowsView::usePad(const PadControls& pad, float delta)
{
    if (!pad.jumping)
        padRestarted = false;

    if (pad.active)
        useHints(padHints(pad.family));

    if (pad.startPressed && getInputRecipient() == InputRecipient::Play)
    {
        openMenu();
        return;
    }

    if (game.state != Game::State::Menu)
        usePlayPad(pad, delta);
    else if (dressing)
        useEditorPad(pad, delta);
    else
        useMenuPad(pad);
}

void CowsView::usePlayPad(const PadControls& pad, float delta)
{
    pressPlayPadButtons(pad);

    if (orbitWithPadLook(pad, delta))
        lookHold = lookHoldTime;

    camera.zoom(pad.zoom * padZoomRate * delta);
    input.setPad(pad.ahead, pad.turn, pad.jumping && !padRestarted);
}

void CowsView::pressPlayPadButtons(const PadControls& pad)
{
    if (pad.jumpPressed)
        pressPadJump();

    if (pad.mooPressed)
        control({ControlEvent::Kind::Moo});

    if (pad.againPressed)
        control({ControlEvent::Kind::Restart});

    if (pad.recenterPressed && game.state == Game::State::Searching)
        camera.turnToward(game.playerHeading + startYaw, 1.f);
}

void CowsView::pressPadJump()
{
    if (game.state == Game::State::Found)
    {
        padRestarted = true;
        control({ControlEvent::Kind::Restart});
    }
    else
        control({ControlEvent::Kind::Jump});
}

bool CowsView::orbitWithPadLook(const PadControls& pad, float delta)
{
    if (pad.lookX == 0.f && pad.lookY == 0.f)
        return false;

    camera.orbit(pad.lookX * padYawRate * delta, -pad.lookY * padPitchRate * delta);
    return true;
}

void CowsView::useMenuPad(const PadControls& pad)
{
    input.setPad(0.f, 0.f, false);

    if (menu == nullptr)
        return;

    auto step = readMenuStickStep(pad);

    if (step != padMenuStep)
        moveMenuSelection(step);

    padMenuStep = step;

    if (pad.jumpPressed || pad.startPressed)
    {
        padRestarted = true;
        menu->showSelection = true;
        menu->chooseSelected();
    }
}

void CowsView::moveMenuSelection(int step)
{
    if (step < 0)
        menu->selectPrevious();

    if (step > 0)
        menu->selectNext();
}

void CowsView::useEditorPad(const PadControls& pad, float delta)
{
    input.setPad(0.f, 0.f, false);
    orbitWithPadLook(pad, delta);
    camera.zoom(pad.zoom * padZoomRate * delta);

    if (editor == nullptr)
        return;

    moveEditorSelection(readEditorStickSteps(pad));
    pressEditorPadButtons(pad);
}

void CowsView::moveEditorSelection(EditorStickSteps steps)
{
    if (steps.across != padAcross && steps.across != 0)
        editor->stepSelected(steps.across);

    if (steps.along != padAlong && steps.along < 0)
        editor->selectPrevious();

    if (steps.along != padAlong && steps.along > 0)
        editor->selectNext();

    padAcross = steps.across;
    padAlong = steps.along;
}

void CowsView::pressEditorPadButtons(const PadControls& pad)
{
    if (pad.jumpPressed)
    {
        padRestarted = true;
        editor->showSelection = true;
        editor->chooseSelected();
    }
    else if (pad.backPressed || pad.startPressed)
    {
        closeEditor();
    }
}

void CowsView::mouseDown(const Graphics::MouseEvent&)
{
    useHints(pointerHints);
    returnKeyFocus();
}

void CowsView::mouseUp(const Graphics::MouseEvent&)
{
    returnKeyFocus();
}

void CowsView::returnKeyFocus()
{
    if (auto* root = getParent())
        root->focus();
}

void CowsView::mouseDragged(const Graphics::MouseEvent& event)
{
    useHints(pointerHints);
    camera.orbit(event.delta.x * orbitSpeed, event.delta.y * orbitSpeed);
    lookHold = lookHoldTime;
}

void CowsView::mouseWheel(const Graphics::MouseEvent& event)
{
    useHints(pointerHints);
    camera.zoom(wheelZoom(event));
}
} // namespace Cows
