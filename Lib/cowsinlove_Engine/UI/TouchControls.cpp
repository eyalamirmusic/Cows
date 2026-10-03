#include "UI/TouchControls.h"

#include <algorithm>
#include <cmath>

namespace Cows
{
namespace
{
using Graphics::Point;

constexpr auto margin = 24.f;
constexpr auto footerSpace = 58.f;
constexpr auto ringRadius = 64.f;
constexpr auto knobRadius = 28.f;
constexpr auto jumpRadius = 44.f;
constexpr auto mooRadius = 30.f;
constexpr auto mooOffsetX = -64.f;
constexpr auto mooOffsetY = -66.f;
constexpr auto touchSlop = 12.f;
constexpr auto stickReach = 0.45f;
constexpr auto deadZone = 0.12f;

constexpr Graphics::Color lineColor {1.f, 1.f, 1.f, 0.9f};
constexpr Graphics::Color shadowColor {0.f, 0.12f, 0.f, 0.3f};
constexpr Graphics::Color pressedFill {1.f, 1.f, 1.f, 0.35f};
constexpr Graphics::Color knobFill {1.f, 1.f, 1.f, 0.22f};

float distance(Point a, Point b)
{
    return std::hypot(a.x - b.x, a.y - b.y);
}

void strokeRing(Hud& hud, Point center, float radius)
{
    hud.strokeRing(center, radius, 5.f, shadowColor);
    hud.strokeRing(center, radius, 2.5f, lineColor);
}

float shaped(float value)
{
    auto size = std::abs(value);
    if (size < deadZone)
        return 0.f;
    return std::copysign((size - deadZone) / (1.f - deadZone), value);
}
} // namespace

TouchControls::TouchControls()
{
    setHandlesMouseEvents(true);
    setHandlesTouchEvents(true);
}

void TouchControls::resized()
{
    auto bounds = getLocalBounds();
    auto safeArea = getSafeAreaInsets();
    auto bottomLine = bounds.h - safeArea.bottom - footerSpace;

    stickHome = {safeArea.left + margin + ringRadius, bottomLine - ringRadius};
    jumpCenter = {bounds.w - safeArea.right - margin - jumpRadius,
                  bottomLine - jumpRadius};
    mooCenter = {jumpCenter.x + mooOffsetX, jumpCenter.y + mooOffsetY};

    if (!pressed(Role::Stick))
        stickCenter = knob = stickHome;
}

TouchControls::Role TouchControls::roleAt(Point position) const
{
    auto bounds = getLocalBounds();
    auto onJump = distance(position, jumpCenter) < jumpRadius + touchSlop;

    if (showAgain)
        return onJump ? Role::Again : Role::Look;

    if (onJump)
        return Role::Jump;

    if (distance(position, mooCenter) < mooRadius + touchSlop)
        return Role::Moo;

    if (!pressed(Role::Stick) && position.x < bounds.w * stickReach
        && position.y > bounds.h / 3.f)
        return Role::Stick;

    return Role::Look;
}

TouchControls::Pointer* TouchControls::find(int id)
{
    for (auto& pointer: pointers)
        if (pointer.id == id)
            return &pointer;

    return nullptr;
}

TouchControls::Pointer* TouchControls::otherLooking(int id)
{
    for (auto& pointer: pointers)
        if (pointer.id != id && pointer.role == Role::Look)
            return &pointer;

    return nullptr;
}

bool TouchControls::pressed(Role role) const
{
    for (const auto& pointer: pointers)
        if (pointer.role == role)
            return true;

    return false;
}

void TouchControls::pointerDown(int id, Point position)
{
    pointerUp(id);

    auto role = roleAt(position);
    pointers.add(Pointer {id, role, position});

    switch (role)
    {
        case Role::Stick:
            stickCenter = knob = position;
            break;
        case Role::Jump:
            onControl({ControlEvent::Kind::Jump});
            break;
        case Role::Moo:
            onControl({ControlEvent::Kind::Moo});
            break;
        case Role::Again:
            onControl({ControlEvent::Kind::Restart});
            break;
        case Role::Look:
            break;
    }
}

void TouchControls::pointerMoved(int id, Point position)
{
    auto* pointer = find(id);
    if (pointer == nullptr)
        return;

    if (pointer->role == Role::Stick)
        moveStick(position);

    if (pointer->role == Role::Look)
    {
        if (auto* other = otherLooking(id))
        {
            auto before = distance(pointer->position, other->position);
            auto after = distance(position, other->position);
            if (before > 1.f && after > 1.f)
                onControl({ControlEvent::Kind::Zoom, std::log(after / before)});
        }
        else
        {
            onControl({ControlEvent::Kind::Look,
                       position.x - pointer->position.x,
                       position.y - pointer->position.y});
        }
    }

    pointer->position = position;
}

void TouchControls::pointerUp(int id)
{
    auto* pointer = find(id);
    if (pointer == nullptr)
        return;

    auto role = pointer->role;
    pointers.removeIndexesMatching([id](const Pointer& p) { return p.id == id; });

    if (role == Role::Stick)
        releaseStick();
}

void TouchControls::moveStick(Point position)
{
    auto offset = position - stickCenter;
    auto reach = std::hypot(offset.x, offset.y);

    if (reach > ringRadius)
        offset = {offset.x * ringRadius / reach, offset.y * ringRadius / reach};

    knob = stickCenter + offset;
    onControl({ControlEvent::Kind::Steer,
               shaped(-offset.y / ringRadius),
               shaped(-offset.x / ringRadius)});
}

void TouchControls::releaseStick()
{
    stickCenter = knob = stickHome;
    onControl({ControlEvent::Kind::Steer});
}

void TouchControls::draw(Hud& hud) const
{
    if (showAgain)
    {
        drawButton(hud, jumpCenter, jumpRadius, "Again", pressed(Role::Again));
        return;
    }

    drawStick(hud);
    drawButton(hud, mooCenter, mooRadius, "Moo", pressed(Role::Moo));
    drawButton(hud, jumpCenter, jumpRadius, "Jump", pressed(Role::Jump));
}

void TouchControls::drawStick(Hud& hud) const
{
    strokeRing(hud, stickCenter, ringRadius);
    hud.fillDisc(knob, knobRadius, pressed(Role::Stick) ? pressedFill : knobFill);
    strokeRing(hud, knob, knobRadius);
}

void TouchControls::drawButton(
    Hud& hud, Point center, float radius, std::string_view label, bool down) const
{
    if (down)
        hud.fillDisc(center, radius, pressedFill);

    strokeRing(hud, center, radius);

    const auto& font = radius > mooRadius ? largeLabel : smallLabel;
    auto width = hud.measure(label, font);
    auto baseline = center.y + hud.ascent(font) * 0.36f;
    auto position = Point {center.x - width * 0.5f, baseline};

    hud.drawText(label, {position.x + 1.f, position.y + 1.f}, font, shadowColor);
    hud.drawText(label, position, font, lineColor);
}

void TouchControls::returnKeyFocus()
{
    if (auto* root = getParent())
        root->focus();
}

void TouchControls::mouseDown(const Graphics::MouseEvent& event)
{
    returnKeyFocus();
    pointerDown(0, event.pos);
}

void TouchControls::mouseDragged(const Graphics::MouseEvent& event)
{
    pointerMoved(0, event.pos);
}

void TouchControls::mouseUp(const Graphics::MouseEvent&)
{
    pointerUp(0);
    returnKeyFocus();
}

void TouchControls::mouseWheel(const Graphics::MouseEvent& event)
{
    onControl({ControlEvent::Kind::Zoom, wheelZoom(event)});
}

void TouchControls::touchBegan(const Graphics::TouchEvent& event)
{
    pointerDown(event.id, event.pos);
}

void TouchControls::touchMoved(const Graphics::TouchEvent& event)
{
    pointerMoved(event.id, event.pos);
}

void TouchControls::touchEnded(const Graphics::TouchEvent& event)
{
    pointerUp(event.id);
}

void TouchControls::safeAreaInsetsChanged()
{
    resized();
}
} // namespace Cows
