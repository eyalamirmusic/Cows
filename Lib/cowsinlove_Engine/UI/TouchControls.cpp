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

Graphics::Path circle(Point center, float radius)
{
    auto path = Graphics::Path {};
    path.addEllipse(
        {center.x - radius, center.y - radius, radius * 2.f, radius * 2.f});
    return path;
}

void strokeRing(Graphics::Context& g, Point center, float radius)
{
    auto path = circle(center, radius);
    g.setColor(shadowColor);
    g.setLineWidth(5.f);
    g.strokePath(path);
    g.setColor(lineColor);
    g.setLineWidth(2.5f);
    g.strokePath(path);
}

void fillDisc(Graphics::Context& g,
              Point center,
              float radius,
              Graphics::Color color)
{
    g.setColor(color);
    g.fillPath(circle(center, radius));
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
}

void TouchControls::resized()
{
    auto bounds = getLocalBounds();
    auto bottomLine = bounds.h - safeArea.bottom - footerSpace;

    stickHome = {safeArea.left + margin + ringRadius, bottomLine - ringRadius};
    jumpCenter = {bounds.w - safeArea.right - margin - jumpRadius,
                  bottomLine - jumpRadius};
    mooCenter = {jumpCenter.x + mooOffsetX, jumpCenter.y + mooOffsetY};

    if (!pressed(Role::Stick))
        stickCenter = knob = stickHome;

    repaint();
}

TouchControls::Role TouchControls::roleAt(Point position) const
{
    auto bounds = getLocalBounds();
    auto onJump = distance(position, jumpCenter) < jumpRadius + touchSlop;

    if (found())
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
            onJump();
            break;
        case Role::Moo:
            onMoo();
            break;
        case Role::Again:
            onRestart();
            break;
        case Role::Look:
            break;
    }

    repaint();
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
                onZoom(std::log(after / before));
        }
        else
        {
            onLook(position.x - pointer->position.x,
                   position.y - pointer->position.y);
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

    repaint();
}

void TouchControls::moveStick(Point position)
{
    auto offset = position - stickCenter;
    auto reach = std::hypot(offset.x, offset.y);

    if (reach > ringRadius)
        offset = {offset.x * ringRadius / reach, offset.y * ringRadius / reach};

    knob = stickCenter + offset;
    onStick(shaped(-offset.y / ringRadius), shaped(-offset.x / ringRadius));
    repaint();
}

void TouchControls::releaseStick()
{
    stickCenter = knob = stickHome;
    onStick(0.f, 0.f);
}

void TouchControls::paint(Graphics::Context& g)
{
    if (found())
    {
        drawButton(g, jumpCenter, jumpRadius, "Again", pressed(Role::Again));
        return;
    }

    drawStick(g);
    drawButton(g, mooCenter, mooRadius, "Moo", pressed(Role::Moo));
    drawButton(g, jumpCenter, jumpRadius, "Jump", pressed(Role::Jump));
}

void TouchControls::drawStick(Graphics::Context& g)
{
    strokeRing(g, stickCenter, ringRadius);
    fillDisc(g, knob, knobRadius, pressed(Role::Stick) ? pressedFill : knobFill);
    strokeRing(g, knob, knobRadius);
}

void TouchControls::drawButton(Graphics::Context& g,
                               Point center,
                               float radius,
                               const std::string& label,
                               bool down)
{
    if (down)
        fillDisc(g, center, radius, pressedFill);

    strokeRing(g, center, radius);

    const auto& font = radius > mooRadius ? largeLabel : smallLabel;
    auto width = Graphics::TextMetrics::measureWidth(label, font);
    auto baseline = center.y + Graphics::TextMetrics::getAscent(font) * 0.36f;
    auto position = Point {center.x - width * 0.5f, baseline};

    g.setColor(shadowColor);
    g.drawText(label, {position.x + 1.f, position.y + 1.f}, font);
    g.setColor(lineColor);
    g.drawText(label, position, font);
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
    onWheel(event);
}
} // namespace Cows
