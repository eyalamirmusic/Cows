#include "UI/Menu.h"
#include "UI/MenuStyle.h"
#include "Render/Palette.h"

#include <ResEmbed/ResEmbed.h>
#include <eacp/Text/GlyphRasterizer.h>

#include <algorithm>
#include <cmath>

namespace Cows
{
namespace
{
using Graphics::Point;
using Graphics::Rect;

constexpr auto titleShare = 0.34f;
constexpr auto lineSpacing = 1.05f;
constexpr auto noteScale = 0.4f;
constexpr auto hoverScale = 1.08f;
constexpr auto padding = 14.f;
constexpr auto leastTarget = 64.f;
constexpr auto ringWidth = 4.f;

constexpr auto gapShare = 0.8f;
constexpr auto fitShare = 0.9f;
constexpr auto stackTop = 0.42f;
constexpr auto stackBottom = 0.95f;
constexpr auto stackFirst = 1.1f;
constexpr auto stackRest = 0.55f;
constexpr auto stackStep = 1.2f;

float stackOffset(int index, float first)
{
    if (index == 0)
        return 0.f;

    return first * stackFirst + first * stackRest * stackStep * (float) index;
}

} // namespace

const std::string& menuFontFamily()
{
    static const auto family = []
    {
        auto font = ResEmbed::get("ComicNeue-Bold.ttf", "Fonts");
        auto registered = Text::registerMemoryFont(font.data(), (int) font.size());

        if (!registered.has_value())
            return std::string {Text::defaultMonospaceFamily()};

        return registered->family;
    }();

    return family;
}

Menu::Menu()
{
    setShowing(true);
}

bool Menu::wide() const
{
    auto bounds = getLocalBounds();
    return bounds.w >= bounds.h;
}

Rect Menu::titleArea() const
{
    auto bounds = getLocalBounds();
    auto safe = getSafeAreaInsets();
    auto height = bounds.h - safe.top - safe.bottom;

    return {
        safe.left, safe.top, bounds.w - safe.left - safe.right, height * titleShare};
}

float Menu::labelWidth(Hud& hud, int index, float pointSize) const
{
    const auto& item = items[index];
    auto width = hud.measure(item.label, menuFont(pointSize));

    if (!item.note.empty())
        width =
            std::max(width, hud.measure(item.note, menuFont(pointSize * noteScale)));

    return width;
}

Graphics::Rect Menu::targetFor(Point center, float labelWidth, float pointSize)
{
    auto height = pointSize * lineSpacing;
    auto area = grown(
        {center.x - labelWidth * 0.5f, center.y - height * 0.5f, labelWidth, height},
        padding);

    if (area.h < leastTarget)
        area = {area.x, center.y - leastTarget * 0.5f, area.w, leastTarget};

    if (area.w < leastTarget)
        area = {center.x - leastTarget * 0.5f, area.y, leastTarget, area.h};

    return area;
}

void Menu::layOut(Hud& hud)
{
    placed.clear();

    if (items.empty())
        return;

    if (wide())
        layOutInRow(hud);
    else
        layOutStacked(hud);
}

void Menu::layOutInRow(Hud& hud)
{
    auto bounds = getLocalBounds();
    auto safe = getSafeAreaInsets();
    auto width = bounds.w - safe.left - safe.right;
    auto height = bounds.h - safe.top - safe.bottom;
    auto count = (int) items.size();
    auto pointSize = std::clamp(height * 0.1f, 30.f, 120.f);

    auto rowWidth = [&](float size)
    {
        auto total = size * gapShare * (float) (count - 1);

        for (auto index = 0; index < count; ++index)
            total +=
                std::max(labelWidth(hud, index, size) * hoverScale + 2.f * padding,
                         leastTarget);

        return total;
    };

    auto room = width * fitShare;
    auto needed = rowWidth(pointSize);

    if (needed > room)
        pointSize *= room / needed;

    auto gap = pointSize * gapShare;
    auto x = safe.left + (width - rowWidth(pointSize)) * 0.5f;
    auto y = safe.top + height * 0.56f;

    for (auto index = 0; index < count; ++index)
    {
        auto label = labelWidth(hud, index, pointSize);
        auto slot = std::max(label * hoverScale + 2.f * padding, leastTarget);
        auto center = Point {x + slot * 0.5f, y};
        placed.add({center, pointSize, targetFor(center, label, pointSize)});
        x += slot + gap;
    }
}

void Menu::layOutStacked(Hud& hud)
{
    auto bounds = getLocalBounds();
    auto safe = getSafeAreaInsets();
    auto width = bounds.w - safe.left - safe.right;
    auto height = bounds.h - safe.top - safe.bottom;
    auto count = (int) items.size();
    auto sizeOf = [](int index, float first)
    { return index == 0 ? first : first * stackRest; };

    auto first = std::clamp(width * 0.2f, 40.f, 130.f);

    for (auto index = 0; index < count; ++index)
    {
        auto room = width * fitShare;
        auto needed = labelWidth(hud, index, sizeOf(index, first)) * hoverScale
                      + 2.f * padding;

        if (needed > room)
            first *= room / needed;
    }

    auto x = safe.left + width * 0.5f;
    auto bottom = safe.top + height * stackBottom;

    for (auto attempt = 0; attempt < 20; ++attempt, first *= 0.9f)
    {
        placed.clear();
        auto y = safe.top + height * stackTop;

        for (auto index = 0; index < count; ++index)
        {
            auto pointSize = sizeOf(index, first);
            auto label = labelWidth(hud, index, pointSize);
            auto area = targetFor({x, y}, label, pointSize);

            if (index > 0)
            {
                const auto& above = placed.back().area;
                auto step =
                    stackOffset(index, first) - stackOffset(index - 1, first);
                y = std::max(placed.back().center.y + step,
                             above.y + above.h + area.h * 0.5f);
                area = targetFor({x, y}, label, pointSize);
            }

            placed.add({{x, y}, pointSize, area});
        }

        if (placed.back().area.y + placed.back().area.h <= bottom)
            return;
    }
}

void Menu::draw(Hud& hud)
{
    layOut(hud);

    if (opacity <= 0.f)
        return;

    for (auto index = 0; index < (int) placed.size(); ++index)
        drawItem(hud, index);
}

void Menu::drawItem(Hud& hud, int index)
{
    const auto& item = items[index];
    const auto& place = placed[index];
    auto lit = item.enabled && (hovered == index || pressed == index);
    auto size = place.pointSize * (lit ? hoverScale : 1.f);
    auto font = menuFont(size);
    auto color =
        faded(item.enabled ? pink(lit ? 0.2f : 0.f) : disabledColor, opacity);
    auto top = place.center.y - size * lineSpacing * 0.5f;
    auto ascent = hud.ascent(font);

    if (showSelection && selected == index)
        hud.strokeRoundedRect(
            place.area,
            ringWidth,
            std::min(place.area.h * 0.5f, 28.f),
            faded(item.enabled ? pink(0.1f) : disabledColor, opacity));

    if (!item.note.empty())
    {
        auto noteFont = menuFont(place.pointSize * noteScale);
        auto width = hud.measure(item.note, noteFont);
        auto baseline = top - place.pointSize * 0.12f;
        hud.drawText(item.note,
                     {place.center.x - width * 0.5f, baseline},
                     noteFont,
                     faded(pink(), opacity));
    }

    auto width = hud.measure(item.label, font);
    auto at = Point {place.center.x - width * 0.5f, top + ascent * 0.9f};
    drawMenuText(hud, item.label, at, font, color, opacity);
}

int Menu::itemAt(Point position) const
{
    for (auto index = 0; index < (int) placed.size(); ++index)
        if (placed[index].area.contains(position))
            return index;

    return -1;
}

void Menu::selectNext()
{
    showSelection = true;
    selected = std::min(selected + 1, (int) items.size() - 1);
}

void Menu::selectPrevious()
{
    showSelection = true;
    selected = std::max(selected - 1, 0);
}

void Menu::chooseSelected()
{
    choose(selected);
}

void Menu::choose(int index)
{
    if (index < 0 || index >= (int) items.size() || !items[index].enabled)
        return;

    onChoose(index);
}

void Menu::setShowing(bool shouldShow)
{
    showing = shouldShow;
    setHandlesMouseEvents(shouldShow);
    setHandlesTouchEvents(shouldShow);
}

bool Menu::isShowing() const
{
    return showing;
}

void Menu::mouseMoved(const Graphics::MouseEvent& event)
{
    auto now = itemAt(event.pos);

    if (now != hovered)
        showSelection = false;

    hovered = now;
    setMouseCursor(now >= 0 && items[now].enabled
                       ? Graphics::MouseCursor::PointingHand
                       : Graphics::MouseCursor::Default);
}

void Menu::mouseExited(const Graphics::MouseEvent&)
{
    hovered = -1;
}

void Menu::mouseDown(const Graphics::MouseEvent& event)
{
    returnKeyFocus();
    pressed = itemAt(event.pos);
}

void Menu::mouseUp(const Graphics::MouseEvent& event)
{
    auto released = itemAt(event.pos);
    auto wasPressed = pressed;
    pressed = -1;
    returnKeyFocus();

    if (released == wasPressed)
        choose(released);
}

void Menu::touchBegan(const Graphics::TouchEvent& event)
{
    pressed = itemAt(event.pos);
}

void Menu::touchMoved(const Graphics::TouchEvent& event)
{
    if (pressed >= 0 && !placed[pressed].area.contains(event.pos))
        pressed = -1;
}

void Menu::touchEnded(const Graphics::TouchEvent& event)
{
    auto released = itemAt(event.pos);
    auto wasPressed = pressed;
    pressed = -1;

    if (released == wasPressed)
        choose(released);
}

void Menu::returnKeyFocus()
{
    if (auto* root = getParent())
        root->focus();
}
} // namespace Cows
