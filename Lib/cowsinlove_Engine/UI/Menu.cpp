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

constexpr auto wideTitleShare = 0.34f;
constexpr auto tallTitleShare = 0.18f;
constexpr auto lineSpacing = 1.05f;
constexpr auto noteScale = 0.4f;
constexpr auto hoverScale = 1.08f;
constexpr auto padding = 14.f;
constexpr auto leastTarget = 64.f;
constexpr auto ringWidth = 4.f;

constexpr auto fitShare = 0.9f;
constexpr auto wideAspect = 1.f;
constexpr auto columnShare = 0.45f;
constexpr auto tallCowShare = 0.4f;
constexpr auto bottomShare = 0.06f;
constexpr auto rowStep = 1.35f;

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
    return bounds.w >= bounds.h * wideAspect;
}

Rect Menu::safeArea() const
{
    auto bounds = getLocalBounds();
    auto safe = getSafeAreaInsets();
    auto top = std::max(safe.top, topClearance);

    return {safe.left,
            top,
            bounds.w - safe.left - safe.right,
            bounds.h - top - safe.bottom};
}

float Menu::titleShare() const
{
    return wide() ? wideTitleShare : tallTitleShare;
}

Rect Menu::titleArea() const
{
    auto area = safeArea();
    return {area.x, area.y, area.w, area.h * titleShare()};
}

Rect Menu::belowTitle() const
{
    auto area = safeArea();
    auto top = area.y + area.h * titleShare();
    return {area.x, top, area.w, area.y + area.h * (1.f - bottomShare) - top};
}

Rect Menu::itemArea() const
{
    auto area = belowTitle();

    if (wide())
        return {area.x, area.y, area.w * columnShare, area.h};

    auto cow = safeArea().h * tallCowShare;
    return {area.x, area.y + cow, area.w, area.h - cow};
}

Rect Menu::cowArea() const
{
    auto area = belowTitle();

    if (wide())
    {
        auto left = area.w * columnShare;
        return {area.x + left, area.y, area.w - left, area.h};
    }

    return {area.x, area.y, area.w, safeArea().h * tallCowShare};
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
    auto area = Graphics::Rect {center.x - labelWidth * 0.5f,
                                center.y - height * 0.5f,
                                labelWidth,
                                height}
                    .inset(-padding);

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

    auto area = itemArea();
    auto count = (int) items.size();
    auto pointSize = std::clamp(safeArea().h * 0.09f, 24.f, 110.f);

    auto rowHeight = [&](int index, float size)
    {
        auto line = size * items[index].scale * lineSpacing;
        return std::max({line * rowStep, line + 2.f * padding, leastTarget});
    };

    auto totalHeight = [&](float size)
    {
        auto total = 0.f;

        for (auto index = 0; index < count; ++index)
            total += rowHeight(index, size);

        return total;
    };

    for (auto index = 0; index < count; ++index)
    {
        auto needed =
            labelWidth(hud, index, pointSize * items[index].scale) * hoverScale
            + 2.f * padding;

        if (needed > area.w * fitShare)
            pointSize *= area.w * fitShare / needed;
    }

    auto spaced = 0.f;

    for (auto index = 0; index < count; ++index)
        spaced += pointSize * items[index].scale * lineSpacing * rowStep;

    pointSize = std::min(pointSize, pointSize * area.h / spaced);

    if (totalHeight(pointSize) > area.h && totalHeight(0.f) < area.h)
    {
        auto low = 0.f;
        auto high = pointSize;

        for (auto attempt = 0; attempt < 20; ++attempt)
        {
            auto middle = (low + high) * 0.5f;
            (totalHeight(middle) > area.h ? high : low) = middle;
        }

        pointSize = low;
    }

    auto total = totalHeight(pointSize);
    auto squeeze = std::min(1.f, area.h / total);
    total *= squeeze;

    auto x = area.x + area.w * 0.5f;
    auto y = area.y + (area.h - total) * 0.5f;

    for (auto index = 0; index < count; ++index)
    {
        auto size = pointSize * items[index].scale;
        auto step = rowHeight(index, pointSize) * squeeze;
        auto center = Point {x, y + step * 0.5f};
        auto target = targetFor(center, labelWidth(hud, index, size), size);
        auto rowBottom = y + step;
        target.y = std::max(target.y, y);
        target.h = std::min(target.y + target.h, rowBottom) - target.y;
        placed.add({center, size, target});
        y += step;
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
    auto lit =
        item.enabled == Enabled::Yes && (hovered == index || pressed == index);
    auto size = place.pointSize * (lit ? hoverScale : 1.f);
    auto font = menuFont(size);
    auto color =
        faded(item.enabled == Enabled::Yes ? pink(lit ? 0.2f : 0.f) : disabledColor,
              opacity);
    auto top = place.center.y - size * lineSpacing * 0.5f;
    auto ascent = hud.ascent(font);

    if (showSelection && selected == index)
        hud.strokeRoundedRect(
            place.area,
            ringWidth,
            std::min(place.area.h * 0.5f, 28.f),
            faded(item.enabled == Enabled::Yes ? pink(0.1f) : disabledColor,
                  opacity));

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
    if (index < 0 || index >= (int) items.size()
        || items[index].enabled == Enabled::No)
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
    setMouseCursor(now >= 0 && items[now].enabled == Enabled::Yes
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
