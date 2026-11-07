#include "CowsView.h"
#include "CowsViewTuning.h"
#include "Ending.h"

#include <algorithm>

using namespace Maths;

namespace Cows
{
using namespace CowsViewTuning;

namespace
{
Graphics::Point centreInClipSpace(Graphics::Rect area, Graphics::Rect bounds)
{
    return {2.f * (area.x + area.w * 0.5f) / bounds.w - 1.f,
            1.f - 2.f * (area.y + area.h * 0.5f) / bounds.h};
}

bool isEmptyRect(Graphics::Rect rect)
{
    return rect.w <= 0.f || rect.h <= 0.f;
}
} // namespace

void CowsView::steerCamera(float delta, float wallDelta)
{
    if (swinging || game.state == Game::State::Menu)
        camera.reach = unlimitedReach;

    if (swinging)
    {
        swingCamera(wallDelta);
        return;
    }

    camera.leastWidth =
        game.state == Game::State::Found ? Ending::leastWidth : searchLeastWidth;

    if (game.state == Game::State::Menu)
        steerMenuCamera();
    else if (game.state == Game::State::Searching)
        steerSearchingCamera(delta);
    else
        steerFoundCamera(delta);
}

void CowsView::steerMenuCamera()
{
    if (!dressing)
        camera.setPose(menuPose());
    else if (editorFramingChanged())
        frameEditor();
}

void CowsView::steerSearchingCamera(float delta)
{
    camera.follow(game.player + Vec3 {0.f, searchHeight, 0.f}, delta);
    camera.swayYaw = 0.f;
    camera.swayPitch = 0.f;

    auto walking = input.walkAhead() != 0.f || input.walkTurn() != 0.f;

    if (walking && lookHold <= 0.f)
        camera.turnToward(game.playerHeading + startYaw,
                          std::min(1.f, delta * chaseRate));

    pullCameraInFrontOfBlockingProps(delta);
}

void CowsView::steerFoundCamera(float delta)
{
    auto staging = Ending::stagingFor(game.level.hiding);
    camera.follow(game.stageCenter + Vec3 {0.f, staging.cameraHeight, 0.f}, delta);
    camera.drift(game.sinceFound);

    if (game.sinceFound <= Ending::settleTime)
    {
        auto amount = std::min(1.f, delta * Ending::settleRate);
        camera.turnToward(game.stageHeading, amount);
        camera.pitch += (staging.cameraPitch - camera.pitch) * amount;
        camera.distance += (staging.cameraDistance - camera.distance) * amount;
    }

    pullCameraInFrontOfBlockingProps(delta);
}

void CowsView::pullCameraInFrontOfBlockingProps(float delta)
{
    auto body = game.player + Vec3 {0.f, bodyHeight, 0.f};
    camera.target = game.level.clearReach(body, camera.target, eyeMargin);

    auto held = camera.reach;
    camera.reach = unlimitedReach;
    auto clear = game.level.clearReach(camera.target, camera.eye(), eyeMargin);
    auto wanted = std::max(length(clear - camera.target), nearestEye);

    camera.reach =
        wanted < held
            ? wanted
            : held + (wanted - held) * std::min(1.f, delta * eyeReturnRate);
}

CameraPose CowsView::menuPose() const
{
    auto sway = driftAt(elapsed);

    auto pose = CameraPose {};
    pose.target = game.player + Vec3 {0.f, menuHeight, 0.f};
    pose.yaw = game.playerHeading + startYaw + pi + menuTurn;
    pose.pitch = menuPitch;
    pose.distance = menuDistance;
    pose.swayYaw = sway.x;
    pose.swayPitch = sway.y;

    if (menu == nullptr)
        return pose;

    auto wide = menu->wide();
    return framedCow(pose,
                     menu->cowArea(),
                     menu->getLocalBounds(),
                     {menuCowWidth,
                      wide ? wideMenuCowShare : tallMenuCowShare,
                      wide ? 0.f : hatClearance});
}

CameraPose CowsView::editorPose() const
{
    auto pose = CameraPose {};
    pose.target = game.player + Vec3 {0.f, editorHeight, 0.f};
    pose.yaw = game.playerHeading + startYaw + pi + editorTurn;
    pose.pitch = editor != nullptr && !editor->wide() ? tallEditorPitch : menuPitch;
    pose.distance = editorDistance;

    if (editor == nullptr)
        return pose;

    return framedCow(pose,
                     editor->cowArea(),
                     editor->getLocalBounds(),
                     {editorCowWidth,
                      editor->wide() ? wideMenuCowShare : tallEditorCowShare,
                      hatClearance});
}

bool CowsView::editorFramingChanged() const
{
    if (editor == nullptr)
        return false;

    auto area = editor->cowArea();
    return area.x != editorFramedArea.x || area.y != editorFramedArea.y
           || area.w != editorFramedArea.w || area.h != editorFramedArea.h
           || viewAspect != editorFramedAspect;
}

void CowsView::frameEditor()
{
    camera.setPose(editorPose());

    if (editor != nullptr)
        editorFramedArea = editor->cowArea();

    editorFramedAspect = viewAspect;
}

CameraPose CowsView::framedCow(CameraPose pose,
                               Graphics::Rect area,
                               Graphics::Rect bounds,
                               CowFraming framing) const
{
    if (isEmptyRect(bounds) || isEmptyRect(area))
        return pose;

    auto fov = camera.verticalFieldOfView(viewAspect);
    auto above = framing.clearanceAboveHead;
    pose.target.y += above * 0.5f;
    auto heightShare = framing.shareOfArea * area.h / bounds.h;
    auto widthShare = framing.shareOfArea * area.w / bounds.h;
    auto fitted = std::max(distanceToFit(menuCowHeight + above, heightShare, fov),
                           distanceToFit(framing.cowWidth, widthShare, fov));
    pose.distance = std::clamp(fitted, nearestMenu, farthestMenu);
    return placeTargetOnScreen(
        pose, centreInClipSpace(area, bounds), fov, viewAspect);
}

CameraPose CowsView::playPose() const
{
    auto pose = CameraPose {};

    if (game.getStateBehindMenu() == Game::State::Found)
    {
        auto sway = driftAt(game.sinceFound);
        auto staging = Ending::stagingFor(game.level.hiding);
        pose.target = game.stageCenter + Vec3 {0.f, staging.cameraHeight, 0.f};
        pose.yaw = game.stageHeading;
        pose.pitch = staging.cameraPitch;
        pose.distance = staging.cameraDistance;
        pose.swayYaw = sway.x;
        pose.swayPitch = sway.y;
        return pose;
    }

    pose.target = game.player + Vec3 {0.f, searchHeight, 0.f};
    pose.yaw = game.playerHeading + startYaw;
    pose.pitch = playPitch;
    pose.distance = playDistance;
    return pose;
}

CameraPose CowsView::getSwingGoalPose() const
{
    if (swingGoal == SwingGoal::Play)
        return playPose();

    if (swingGoal == SwingGoal::Dress)
        return editorPose();

    return menuPose();
}

void CowsView::swingCamera(float delta)
{
    swingTime = std::min(swingTime + delta, swingLength);
    auto amount = easeOut(swingTime / swingLength);
    camera.setPose(blend(swingFrom, getSwingGoalPose(), amount));

    if (swingTime < swingLength)
        return;

    swinging = false;

    if (swingGoal == SwingGoal::Dress)
        frameEditor();

    if (swingGoal == SwingGoal::Play)
    {
        game.start();
        onStateChanged();
    }
}

float CowsView::menuOpacity() const
{
    auto behindEditor = 1.f - editorOpacity();
    auto aroundCow =
        swingGoal == SwingGoal::Dress || swingGoal == SwingGoal::Undress;

    if (!swinging || aroundCow)
        return game.state == Game::State::Menu ? behindEditor : 0.f;

    auto amount = easeOut(swingTime / swingLength);
    return swingGoal == SwingGoal::Play ? 1.f - amount : amount;
}

float CowsView::editorOpacity() const
{
    return game.state == Game::State::Menu ? easeOut(dressAmount) : 0.f;
}

void CowsView::openMenu(bool swing)
{
    if (isSwingingIntoPlay())
    {
        swingTo(SwingGoal::Menu);
        onStateChanged();
        return;
    }

    if (game.state == Game::State::Menu)
        return;

    playPitch = camera.pitch;
    playDistance = camera.distance;
    game.openMenu();
    input = Input {};

    swingTo(SwingGoal::Menu);
    swinging = swing;

    if (!swing)
        camera.setPose(menuPose());

    onStateChanged();
}

bool CowsView::isMenuSettled() const
{
    return game.state == Game::State::Menu && !swinging && !dressing;
}

void CowsView::startGame()
{
    if (!isMenuSettled())
        return;

    swingTo(SwingGoal::Play);
    onStateChanged();
}

bool CowsView::isSwingingIntoPlay() const
{
    return swinging && swingGoal == SwingGoal::Play;
}

InputRecipient CowsView::getInputRecipient() const
{
    return Cows::getInputRecipient(game.state, dressing, isSwingingIntoPlay());
}

void CowsView::swingTo(SwingGoal goal)
{
    swingFrom = camera.pose();
    swingTime = 0.f;
    swinging = true;
    swingGoal = goal;
}

void CowsView::openEditor(Editor& which)
{
    if (!isMenuSettled())
        return;

    editor = &which;
    dressing = true;
    swingTo(SwingGoal::Dress);

    if (editor != nullptr)
        editor->selected = 0;

    onStateChanged();
}

void CowsView::closeEditor()
{
    if (!dressing)
        return;

    dressing = false;
    swingTo(SwingGoal::Undress);
    onStateChanged();
}

bool CowsView::escape()
{
    switch (escapeAction(getInputRecipient(), swinging))
    {
        case EscapeAction::Swallow:
            return true;
        case EscapeAction::CloseEditor:
            closeEditor();
            return true;
        case EscapeAction::OpenMenu:
            openMenu();
            return true;
        case EscapeAction::PassOn:
            return false;
    }

    return false;
}

void CowsView::framePortrait(float aspect)
{
    if (framedPortrait || aspect >= 1.f)
        return;

    framedPortrait = true;
    playPitch = portraitPitch;
    playDistance = portraitDistance;

    if (game.state == Game::State::Menu)
        return;

    camera.distance = portraitDistance;
    camera.pitch = portraitPitch;
}
} // namespace Cows
