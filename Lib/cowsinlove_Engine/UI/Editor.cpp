#include "UI/Editor.h"
#include "UI/MenuStyle.h"

#include <algorithm>

namespace Cows
{
namespace
{
using Graphics::Point;
using Graphics::Rect;

constexpr auto lineSpacing = 1.05f;
constexpr auto hoverScale = 1.08f;
constexpr auto padding = 14.f;
constexpr auto leastTarget = 64.f;
constexpr auto ringWidth = 4.f;
constexpr auto titleScale = 1.3f;
constexpr auto wideSceneShare = 0.6f;
constexpr auto tallSceneShare = 0.6f;

constexpr auto previousArrow = "<";
constexpr auto nextArrow = ">";

float right(const Rect& rect)
{
    return rect.x + rect.w;
}

float bottom(const Rect& rect)
{
    return rect.y + rect.h;
}
} // namespace

Editor::Editor()
{
    setShowing(false);
}

bool Editor::wide() const
{
    auto bounds = getLocalBounds();
    return bounds.w >= bounds.h;
}

Rect Editor::contentArea() const
{
    auto bounds = getLocalBounds();
    auto safe = getSafeAreaInsets();
    auto top = std::max(safe.top, topClearance);

    return {safe.left,
            top,
            bounds.w - safe.left - safe.right,
            bounds.h - top - safe.bottom};
}

Rect Editor::sceneArea() const
{
    auto area = contentArea();

    if (wide())
        return {area.x, area.y, area.w * wideSceneShare, area.h};

    return {area.x, area.y, area.w, area.h * tallSceneShare};
}

int Editor::doneIndex() const
{
    return (int) rows.size();
}

void Editor::layOut(Hud& hud)
{
    auto area = contentArea();
    auto scene = sceneArea();
    auto panel = wide() ? Rect {right(scene),
                                area.y,
                                right(area) - right(scene) - area.w * 0.03f,
                                area.h}
                        : Rect {area.x + area.w * 0.04f,
                                bottom(scene),
                                area.w * 0.92f,
                                bottom(area) - bottom(scene)};

    auto size = wide() ? std::clamp(area.h * 0.06f, 22.f, 52.f)
                       : std::clamp(area.w * 0.1f, 22.f, 48.f);

    auto widest = [&](float pointSize, auto&& texts)
    {
        auto font = menuFont(pointSize);
        auto most = 0.f;

        for (const auto& text: texts)
            most = std::max(most, hud.measure(text, font));

        return most;
    };

    auto names = Vector<std::string> {};
    auto values = Vector<std::string> {};

    for (const auto& row: rows)
    {
        names.add(row.name);

        for (const auto& choice: row.choices)
            values.add(choice);
    }

    auto arrowWidth = [&](float pointSize)
    {
        auto arrows = Vector<std::string> {previousArrow, nextArrow};
        return std::max(leastTarget, widest(pointSize, arrows) + 2.f * padding);
    };

    auto gap = size * 0.4f;
    auto textWidth = widest(size, names) + widest(size, values);
    auto room = panel.w - 2.f * arrowWidth(size) - gap - 2.f * padding;

    if (textWidth > room && textWidth > 0.f)
        size *= std::max(room, 0.f) / textWidth;

    gap = size * 0.4f;
    titleSize = size * titleScale;

    auto nameWidth = widest(size, names);
    auto valueWidth = widest(size, values) + padding;
    auto arrow = arrowWidth(size);
    auto rowHeight = std::max(leastTarget, size * 1.5f);
    auto titleHeight = titleSize * 1.6f;
    auto blockWidth = nameWidth + gap + 2.f * arrow + valueWidth;
    auto blockHeight =
        titleHeight + rowHeight * (float) rows.size() + rowHeight * 1.3f;
    auto left = panel.x + (panel.w - blockWidth) * 0.5f;
    auto top = std::max(panel.y, panel.y + (panel.h - blockHeight) * 0.5f);
    auto ascent = hud.ascent(menuFont(size));
    auto baselineIn = [&](float rowTop)
    {
        return rowTop + rowHeight * 0.5f - size * lineSpacing * 0.5f + ascent * 0.9f;
    };

    titleBox = {panel.x, top, panel.w, titleHeight};
    placed.clear();

    for (auto index = 0; index < (int) rows.size(); ++index)
    {
        auto rowTop = top + titleHeight + rowHeight * (float) index;
        auto place = Placed {};
        place.pointSize = size;
        place.baseline = baselineIn(rowTop);
        place.name = {left, rowTop, nameWidth, rowHeight};
        place.previous = {right(place.name) + gap, rowTop, arrow, rowHeight};
        place.value = {right(place.previous), rowTop, valueWidth, rowHeight};
        place.next = {right(place.value), rowTop, arrow, rowHeight};
        place.row = grown({left, rowTop, blockWidth, rowHeight}, padding * 0.5f);
        placed.add(place);
    }

    auto doneTop = top + titleHeight + rowHeight * ((float) rows.size() + 0.3f);
    auto doneWidth = std::max(
        leastTarget, hud.measure(doneLabel, menuFont(size)) + 2.f * padding);

    done = {};
    done.pointSize = size;
    done.baseline = baselineIn(doneTop);
    done.value = {
        panel.x + (panel.w - doneWidth) * 0.5f, doneTop, doneWidth, rowHeight};
    done.row = done.value;
}

void Editor::draw(Hud& hud)
{
    layOut(hud);

    if (opacity <= 0.f)
        return;

    auto titleFont = menuFont(titleSize);
    auto titleWidth = hud.measure(title, titleFont);
    auto titleBaseline = titleBox.y + hud.ascent(titleFont) * 0.9f;
    drawMenuText(hud,
                 title,
                 {titleBox.x + (titleBox.w - titleWidth) * 0.5f, titleBaseline},
                 titleFont,
                 faded(pink(0.1f), opacity),
                 opacity);

    for (auto index = 0; index < (int) rows.size(); ++index)
        drawRow(hud, index);

    drawDone(hud);
}

void Editor::drawRow(Hud& hud, int index)
{
    const auto& row = rows[index];
    const auto& place = placed[index];
    auto value = row.choice >= 0 && row.choice < (int) row.choices.size()
                     ? row.choices[row.choice]
                     : std::string {};

    if (showSelection && selected == index)
        drawRing(hud, place.row);

    drawLabel(
        hud, row.name, place.name, place.baseline, place.pointSize, false, false);
    drawLabel(hud,
              previousArrow,
              place.previous,
              place.baseline,
              place.pointSize,
              isLit({index, Part::Previous}),
              true);
    drawLabel(hud,
              value,
              place.value,
              place.baseline,
              place.pointSize,
              isLit({index, Part::Value}),
              true);
    drawLabel(hud,
              nextArrow,
              place.next,
              place.baseline,
              place.pointSize,
              isLit({index, Part::Next}),
              true);
}

void Editor::drawDone(Hud& hud)
{
    if (showSelection && selected == doneIndex())
        drawRing(hud, done.row);

    drawLabel(hud,
              doneLabel,
              done.value,
              done.baseline,
              done.pointSize,
              isLit({doneIndex(), Part::Done}),
              true);
}

void Editor::drawLabel(Hud& hud,
                       const std::string& text,
                       const Rect& area,
                       float baseline,
                       float pointSize,
                       bool lit,
                       bool centred)
{
    auto size = pointSize * (lit ? hoverScale : 1.f);
    auto font = menuFont(size);
    auto width = hud.measure(text, font);
    auto x = centred ? area.x + (area.w - width) * 0.5f : area.x;
    auto grownBy = (size - pointSize) * 0.3f;

    drawMenuText(hud,
                 text,
                 {x, baseline + grownBy},
                 font,
                 faded(pink(lit ? 0.2f : 0.f), opacity),
                 opacity);
}

void Editor::drawRing(Hud& hud, const Rect& area)
{
    hud.strokeRoundedRect(
        area, ringWidth, std::min(area.h * 0.5f, 28.f), faded(pink(0.1f), opacity));
}

bool Editor::isLit(Target target) const
{
    return hovered == target || pressed == target;
}

Editor::Target Editor::targetAt(Point position) const
{
    for (auto index = 0; index < (int) placed.size(); ++index)
    {
        const auto& place = placed[index];

        if (place.previous.contains(position))
            return {index, Part::Previous};

        if (place.value.contains(position))
            return {index, Part::Value};

        if (place.next.contains(position))
            return {index, Part::Next};
    }

    if (done.value.contains(position))
        return {doneIndex(), Part::Done};

    return {};
}

void Editor::selectNext()
{
    showSelection = true;
    selected = std::min(selected + 1, doneIndex());
}

void Editor::selectPrevious()
{
    showSelection = true;
    selected = std::max(selected - 1, 0);
}

void Editor::stepSelected(int by)
{
    showSelection = true;

    if (selected < doneIndex())
        onStep(selected, by);
}

void Editor::chooseSelected()
{
    if (selected == doneIndex())
        act({selected, Part::Done});
    else
        act({selected, Part::Next});
}

void Editor::act(Target target)
{
    if (target.part == Part::None)
        return;

    selected = target.row;

    switch (target.part)
    {
        case Part::Previous:
            onStep(target.row, -1);
            break;
        case Part::Value:
        case Part::Next:
            onStep(target.row, 1);
            break;
        case Part::Done:
            onDone();
            break;
        case Part::None:
            break;
    }
}

void Editor::setShowing(bool shouldShow)
{
    showing = shouldShow;
    setHandlesMouseEvents(shouldShow);
    setHandlesTouchEvents(shouldShow);
}

bool Editor::isShowing() const
{
    return showing;
}

void Editor::mouseMoved(const Graphics::MouseEvent& event)
{
    auto now = targetAt(event.pos);

    if (now != hovered)
        showSelection = false;

    hovered = now;
    setMouseCursor(now.part != Part::None ? Graphics::MouseCursor::PointingHand
                                          : Graphics::MouseCursor::Default);
}

void Editor::mouseExited(const Graphics::MouseEvent&)
{
    hovered = {};
}

void Editor::mouseDown(const Graphics::MouseEvent& event)
{
    returnKeyFocus();
    pressed = targetAt(event.pos);
}

void Editor::mouseUp(const Graphics::MouseEvent& event)
{
    auto released = targetAt(event.pos);
    auto wasPressed = pressed;
    pressed = {};
    returnKeyFocus();

    if (released == wasPressed)
        act(released);
}

void Editor::mouseDragged(const Graphics::MouseEvent& event)
{
    if (pressed.part == Part::None)
        onControl({ControlEvent::Kind::Look, event.delta.x, event.delta.y});
}

void Editor::mouseWheel(const Graphics::MouseEvent& event)
{
    onControl({ControlEvent::Kind::Zoom, wheelZoom(event)});
}

void Editor::touchBegan(const Graphics::TouchEvent& event)
{
    auto target = targetAt(event.pos);

    if (target.part != Part::None)
    {
        pressed = target;
        return;
    }

    if (lookingTouch < 0)
    {
        lookingTouch = event.id;
        lookedFrom = event.pos;
    }
}

void Editor::touchMoved(const Graphics::TouchEvent& event)
{
    if (event.id == lookingTouch)
    {
        onControl({ControlEvent::Kind::Look,
                   event.pos.x - lookedFrom.x,
                   event.pos.y - lookedFrom.y});
        lookedFrom = event.pos;
        return;
    }

    if (targetAt(event.pos) != pressed)
        pressed = {};
}

void Editor::touchEnded(const Graphics::TouchEvent& event)
{
    if (event.id == lookingTouch)
    {
        lookingTouch = -1;
        return;
    }

    auto released = targetAt(event.pos);
    auto wasPressed = pressed;
    pressed = {};

    if (released == wasPressed)
        act(released);
}

void Editor::returnKeyFocus()
{
    if (auto* root = getParent())
        root->focus();
}
} // namespace Cows
