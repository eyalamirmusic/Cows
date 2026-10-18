#include "UI/Menu.h"
#include "Layout/FitToSize.h"

#include <ResEmbed/ResEmbed.h>
#include <eacp/Text/GlyphRasterizer.h>

#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>

namespace Cows
{
namespace
{
using Graphics::Point;
using Graphics::Rect;

// The shapes the menu takes, wide or tall, and the most the cow's and the
// title's room give way to the items, a step at a time, before the cow goes
// behind them (tightTallShape) in a window too small for anything else.
constexpr auto wideShape = Menu::Shape {true, 0.34f, 0.55f};
constexpr auto tallShape = Menu::Shape {false, 0.18f, 0.4f};
constexpr auto roomyWideShape = Menu::Shape {true, 0.24f, 0.4f};
constexpr auto roomyTallShape = Menu::Shape {false, 0.14f, 0.26f};
constexpr auto tightTallShape = Menu::Shape {false, 0.12f, 0.f};

constexpr auto lineSpacing = 1.05f;
constexpr auto rowStep = 1.35f;
constexpr auto noteScale = 0.4f;
constexpr auto noteLift = 0.12f;
constexpr auto hoverScale = 1.08f;
// Room each side of a label for it to grow by hoverScale, as a share of its
// point size.
constexpr auto hoverRoom = 0.25f;
constexpr auto padding = 14.f;
constexpr auto ringWidth = 4.f;
constexpr auto ringCorner = 28.f;
constexpr auto ascentShare = 0.9f;
constexpr auto litBrighten = 0.2f;
constexpr auto ringBrighten = 0.1f;

constexpr auto leastLargest = 24.f;
constexpr auto mostLargest = 110.f;
constexpr auto largestShare = 0.09f;

bool aspectWide(const Graphics::View& view)
{
    auto bounds = view.getLocalBounds();
    return bounds.w >= bounds.h;
}

Rect titleIn(Rect content, Menu::Shape shape)
{
    return {content.x, content.y, content.w, std::round(content.h * shape.title)};
}

Rect bodyIn(Rect content, Menu::Shape shape)
{
    auto title = std::round(content.h * shape.title);
    return {content.x, content.y + title, content.w, content.h - title};
}

Rect cowIn(Rect content, Menu::Shape shape)
{
    auto body = bodyIn(content, shape);

    if (shape.wide)
    {
        auto width = std::round(body.w * shape.cow);
        return {body.x + body.w - width, body.y, width, body.h};
    }

    // With no band of her own she stands behind the items.
    if (shape.cow <= 0.f)
        return body;

    return {body.x, body.y, body.w, std::round(content.h * shape.cow)};
}

Rect itemsIn(Rect content, Menu::Shape shape)
{
    auto body = bodyIn(content, shape);

    if (shape.wide)
        return {body.x, body.y, body.w - std::round(body.w * shape.cow), body.h};

    auto cow = std::round(content.h * shape.cow);
    return {body.x, body.y + cow, body.w, body.h - cow};
}

float smallestScale(const Vector<MenuItem>& items)
{
    auto least = 1.f;

    for (const auto& item: items)
        least = std::min(least, item.scale);

    return least;
}

constexpr auto squeezeSteps = 6;

Menu::Shape squeezed(Menu::Shape from, Menu::Shape to, int step)
{
    auto along = (float) step / (float) squeezeSteps;
    return {from.wide,
            from.title + (to.title - from.title) * along,
            from.cow + (to.cow - from.cow) * along};
}

struct ItemNodes final
{
    Flex::NodeId row = 0;
    Flex::NodeId label = 0;
};

float noteSizeFor(float labelSize, float leastNote)
{
    return onFontGrid(std::max(labelSize * noteScale, leastNote));
}

// One centred column of rows, each a label (with its note above it, never
// under `leastNote`) padded to at least a tap target, the labels at
// `pointSize` times their item's scale.
Flex::Tree itemTree(const Vector<MenuItem>& items,
                    float pointSize,
                    float target,
                    float leastNote,
                    Vector<ItemNodes>* nodes)
{
    auto column = Flex::Node {};
    column.direction = Flex::Direction::Down;
    column.mainAlign = Flex::Align::Centre;
    column.crossAlign = Flex::Align::Centre;
    auto tree = Flex::Tree {column};

    for (const auto& item: items)
    {
        auto size = onFontGrid(pointSize * item.scale);
        auto line = size * lineSpacing;
        auto rowPadding = line * (rowStep - 1.f) * 0.5f;
        auto side = padding + size * hoverRoom;

        auto row = Flex::Node {};
        row.direction = Flex::Direction::Down;
        row.width = Flex::Size::fit(target);
        row.height = Flex::Size::fit(target);
        row.padding = {
            .top = rowPadding, .left = side, .bottom = rowPadding, .right = side};
        row.mainAlign = Flex::Align::Centre;
        row.crossAlign = Flex::Align::Centre;
        auto rowId = tree.add(Flex::Tree::root, row);

        if (!item.note.empty())
        {
            auto note = Flex::Node {};
            note.text = Flex::Text {item.note, noteSizeFor(size, leastNote)};
            tree.add(rowId, note);
        }

        auto label = Flex::Node {};
        label.text = Flex::Text {item.label, size, lineSpacing};
        auto labelId = tree.add(rowId, label);

        if (nodes != nullptr)
            nodes->add({rowId, labelId});
    }

    return tree;
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

Menu::Shape Menu::shape() const
{
    if (laidOutFor.has_value()
        && sameFrame(*laidOutFor, frameOf(*this, topClearance)))
        return chosen;

    return aspectWide(*this) ? wideShape : tallShape;
}

bool Menu::wide() const
{
    return shape().wide;
}

Rect Menu::titleArea() const
{
    return titleIn(frameOf(*this, topClearance).contentArea(), shape());
}

Rect Menu::cowArea() const
{
    return cowIn(frameOf(*this, topClearance).contentArea(), shape());
}

Rect Menu::itemArea() const
{
    return itemsIn(frameOf(*this, topClearance).contentArea(), shape());
}

void Menu::setItems(Vector<MenuItem> newItems)
{
    items = std::move(newItems);
    itemsChanged = true;
}

void Menu::layOut(Hud& hud)
{
    auto frame = frameOf(*this, topClearance);

    if (!itemsChanged && laidOutFor.has_value() && sameFrame(*laidOutFor, frame))
        return;

    itemsChanged = false;
    laidOutFor = frame;
    placed.clear();

    if (items.empty())
        return;

    auto content = frame.contentArea();
    auto measure = menuMeasure(hud);
    auto byAspect = aspectWide(*this);
    auto leastNote = smallestReadableSize(content);
    auto targetIn = [&](Rect area)
    { return std::min(tapTarget(content), area.h / (float) items.size()); };

    auto choose = [&](const Vector<Shape>& shapes)
    {
        auto candidates = Vector<LayoutShape> {};

        for (const auto& each: shapes)
        {
            auto area = itemsIn(content, each);
            auto target = targetIn(area);
            candidates.add(
                {[this, target, leastNote](float size)
                 { return itemTree(items, size, target, leastNote, nullptr); },
                 area});
        }

        // The smallest item, About, is held to the readable size, so the rest
        // are larger.
        auto readable = smallestReadableSize(content) / smallestScale(items);
        auto largest =
            std::clamp(content.h * largestShare, leastLargest, mostLargest);
        auto best = largestShape(candidates, measure, readable, largest);
        return std::pair {shapes[best.index], best};
    };

    // The first step of squeeze at which either shape, the aspect's first,
    // holds the items at the readable size, so the text is near that size
    // either side of a step and the cow keeps as much room as it can.
    auto shapeChosen = Shape {};
    auto size = ChosenShape {};

    for (auto step = 0; step <= squeezeSteps && !size.readable; ++step)
    {
        auto wideStep = squeezed(wideShape, roomyWideShape, step);
        auto tallStep = squeezed(tallShape, roomyTallShape, step);
        std::tie(shapeChosen, size) =
            choose(byAspect ? Vector<Shape> {wideStep, tallStep}
                            : Vector<Shape> {tallStep, wideStep});
    }

    if (!size.readable)
        std::tie(shapeChosen, size) = choose({tightTallShape});

    chosen = shapeChosen;
    pointSize = size.pointSize;

    auto area = itemsIn(content, chosen);
    auto nodes = Vector<ItemNodes> {};
    auto tree = itemTree(items, pointSize, targetIn(area), leastNote, &nodes);
    tree.resolve(area, measure);

    for (auto index = 0; index < items.size(); ++index)
    {
        auto label = tree.rect(nodes[index].label);
        auto labelSize = tree.node(nodes[index].label).text->pointSize;
        placed.add({label.center(),
                    labelSize,
                    tree.rect(nodes[index].row),
                    noteSizeFor(labelSize, leastNote)});
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
    const auto& font = menuFont(place.pointSize * (lit ? hoverScale : 1.f));
    auto size = font.pointSize;
    auto color = faded(item.enabled == Enabled::Yes ? pink(lit ? litBrighten : 0.f)
                                                    : disabledColor,
                       opacity);
    auto top = place.center.y - size * lineSpacing * 0.5f;
    auto ascent = hud.ascent(font);

    if (showSelection && selected == index)
        hud.strokeRoundedRect(
            place.area,
            ringWidth,
            std::min(place.area.h * 0.5f, ringCorner),
            faded(item.enabled == Enabled::Yes ? pink(ringBrighten) : disabledColor,
                  opacity));

    if (!item.note.empty())
    {
        const auto& noteFont = menuFont(place.noteSize);
        auto width = hud.measure(item.note, noteFont);
        auto baseline = top - place.pointSize * noteLift;
        drawMenuText(hud,
                     item.note,
                     {place.center.x - width * 0.5f, baseline},
                     noteFont,
                     faded(pink(), opacity),
                     opacity);
    }

    auto width = hud.measure(item.label, font);
    auto at = Point {place.center.x - width * 0.5f, top + ascent * ascentShare};
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
