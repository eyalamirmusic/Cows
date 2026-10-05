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

Vector<std::string>
    wrapWords(const std::string& text,
              float width,
              const std::function<float(const std::string&)>& measure)
{
    auto lines = Vector<std::string> {};
    auto current = std::string {};
    auto start = std::size_t {0};

    while (start < text.size())
    {
        auto end = std::min(text.find(' ', start), text.size());
        auto word = text.substr(start, end - start);
        auto joined = current.empty() ? word : current + " " + word;

        if (!current.empty() && measure(joined) > width)
        {
            lines.add(current);
            current = word;
        }
        else
        {
            current = joined;
        }

        start = end + 1;
    }

    lines.add(current);
    return lines;
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

void Menu::layOut(Hud& hud)
{
    auto bounds = getLocalBounds();
    auto safe = getSafeAreaInsets();
    auto width = bounds.w - safe.left - safe.right;
    auto height = bounds.h - safe.top - safe.bottom;
    auto isWide = wide();
    auto count = (int) items.size();

    placed.clear();

    for (auto index = 0; index < count; ++index)
    {
        auto item = Placed {};
        auto column = 0.f;
        auto maxWidth = width * 0.9f;

        if (isWide)
        {
            auto share = count > 1 ? (float) index / (float) (count - 1) : 0.5f;
            item.pointSize = std::clamp(height * 0.1f, 30.f, 120.f);
            column = 0.2f + 0.58f * share;
            maxWidth = width * 0.38f;
            item.center = {safe.left + width * column, safe.top + height * 0.56f};
        }
        else
        {
            auto first = std::clamp(width * 0.2f, 40.f, 130.f);
            item.pointSize = index == 0 ? first : first * 0.55f;
            auto below = index == 0
                             ? 0.f
                             : first * 1.1f + item.pointSize * 1.2f * (float) index;
            item.center = {safe.left + width * 0.5f,
                           safe.top + height * 0.42f + below};
        }

        auto font = menuFont(item.pointSize);
        auto measure = [&](const std::string& line)
        { return hud.measure(line, font); };
        item.lines = wrapWords(items[index].label, maxWidth, measure);

        auto widest = 0.f;

        for (const auto& line: item.lines)
            widest = std::max(widest, measure(line));

        auto lineHeight = item.pointSize * lineSpacing;
        auto blockHeight = lineHeight * (float) item.lines.size();
        auto area = Rect {item.center.x - widest * 0.5f,
                          item.center.y - blockHeight * 0.5f,
                          widest,
                          blockHeight};
        area = grown(area, padding);

        if (area.h < leastTarget)
            area = {area.x, item.center.y - leastTarget * 0.5f, area.w, leastTarget};

        if (area.w < leastTarget)
            area = {item.center.x - leastTarget * 0.5f, area.y, leastTarget, area.h};

        item.area = area;
        placed.add(item);
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
    auto lineHeight = size * lineSpacing;
    auto top = place.center.y - lineHeight * (float) place.lines.size() * 0.5f;
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

    for (auto line = 0; line < (int) place.lines.size(); ++line)
    {
        const auto& text = place.lines[line];
        auto width = hud.measure(text, font);
        auto baseline = top + ascent * 0.9f + lineHeight * (float) line;
        auto at = Point {place.center.x - width * 0.5f, baseline};
        drawMenuText(hud, text, at, font, color, opacity);
    }
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
