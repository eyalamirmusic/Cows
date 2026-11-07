#include "UI/Menu.h"

#include <ResEmbed/ResEmbed.h>
#include <eacp/Text/GlyphRasterizer.h>

#include <algorithm>
#include <cmath>
#include <utility>

namespace Cows
{
namespace
{
using Graphics::Point;
using Graphics::Rect;

// tightTallShape stands the cow behind the items when nothing else fits.
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
// Room each side for a label to grow by hoverScale, a share of its point size.
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
    return {
        content.x, content.y, content.w, std::round(content.h * shape.titleShare)};
}

Rect bodyIn(Rect content, Menu::Shape shape)
{
    auto title = std::round(content.h * shape.titleShare);
    return {content.x, content.y + title, content.w, content.h - title};
}

Rect cowIn(Rect content, Menu::Shape shape)
{
    auto body = bodyIn(content, shape);

    if (shape.wide)
    {
        auto width = std::round(body.w * shape.cowShare);
        return {body.x + body.w - width, body.y, width, body.h};
    }

    // With no band of her own she stands behind the items.
    if (shape.cowShare <= 0.f)
        return body;

    return {body.x, body.y, body.w, std::round(content.h * shape.cowShare)};
}

Rect itemsIn(Rect content, Menu::Shape shape)
{
    auto body = bodyIn(content, shape);

    if (shape.wide)
        return {
            body.x, body.y, body.w - std::round(body.w * shape.cowShare), body.h};

    auto cow = std::round(content.h * shape.cowShare);
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
            from.titleShare + (to.titleShare - from.titleShare) * along,
            from.cowShare + (to.cowShare - from.cowShare) * along};
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

struct ItemTreeSizes final
{
    float pointSize = 0.f;
    float target = 0.f;
    float leastNote = 0.f;
};

struct ItemTree final
{
    Flex::Tree tree;
    Vector<ItemNodes> nodes;
};

Flex::Node makeItemColumn()
{
    auto column = Flex::Node {};
    column.direction = Flex::Direction::Down;
    column.mainAlign = Flex::Align::Centre;
    column.crossAlign = Flex::Align::Centre;
    return column;
}

Flex::Node makeItemRow(float size, float target)
{
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
    return row;
}

ItemNodes
    addItemNodes(Flex::Tree& tree, const MenuItem& item, const ItemTreeSizes& sizes)
{
    auto size = onFontGrid(sizes.pointSize * item.scale);
    auto rowId = tree.add(Flex::Tree::root, makeItemRow(size, sizes.target));

    if (!item.note.empty())
    {
        auto note = Flex::Node {};
        note.text = Flex::Text {item.note, noteSizeFor(size, sizes.leastNote)};
        tree.add(rowId, note);
    }

    auto label = Flex::Node {};
    label.text = Flex::Text {item.label, size, lineSpacing};
    return {rowId, tree.add(rowId, label)};
}

ItemTree makeItemTree(const Vector<MenuItem>& items, const ItemTreeSizes& sizes)
{
    auto result = ItemTree {Flex::Tree {makeItemColumn()}, {}};

    for (const auto& item: items)
        result.nodes.add(addItemNodes(result.tree, item, sizes));

    return result;
}

Vector<Menu::Shape> orderShapesByAspect(bool byAspectWide,
                                        Menu::Shape wideStep,
                                        Menu::Shape tallStep)
{
    if (byAspectWide)
        return {wideStep, tallStep};

    return {tallStep, wideStep};
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
    if (isLaidOutFor(frameOf(*this, topClearance)))
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

void Menu::setItems(Vector<MenuItem> newItems)
{
    items = std::move(newItems);
    itemsChanged = true;
}

bool Menu::isLaidOutFor(const ScreenFrame& frame) const
{
    return laidOutFor.has_value() && sameFrame(*laidOutFor, frame);
}

float Menu::calculateTapTargetIn(Rect content, Rect area) const
{
    auto numItems = static_cast<float>(items.size());
    return std::min(tapTarget(content), area.h / numItems);
}

std::pair<Menu::Shape, ChosenShape> Menu::chooseLargestShape(
    const Vector<Shape>& shapes, Rect content, const Flex::Measure& measure) const
{
    auto leastNote = smallestReadableSize(content);
    auto candidates = Vector<LayoutShape> {};

    for (const auto& each: shapes)
    {
        auto area = itemsIn(content, each);
        auto sizes =
            ItemTreeSizes {0.f, calculateTapTargetIn(content, area), leastNote};
        candidates.add({[this, sizes](float size)
                        {
                            auto sized = sizes;
                            sized.pointSize = size;
                            return makeItemTree(items, sized).tree;
                        },
                        area});
    }

    // About, the smallest item, is held to the readable size.
    auto readable = leastNote / smallestScale(items);
    auto largest = std::clamp(content.h * largestShare, leastLargest, mostLargest);
    auto best = largestShape(candidates, measure, {readable, largest});
    return std::pair {shapes[best.index], best};
}

std::pair<Menu::Shape, ChosenShape>
    Menu::squeezeUntilReadable(Rect content, const Flex::Measure& measure) const
{
    auto byAspectWide = aspectWide(*this);
    auto result = std::pair<Shape, ChosenShape> {};

    for (auto step = 0; step <= squeezeSteps && !result.second.readable; ++step)
    {
        auto wideStep = squeezed(wideShape, roomyWideShape, step);
        auto tallStep = squeezed(tallShape, roomyTallShape, step);
        result = chooseLargestShape(
            orderShapesByAspect(byAspectWide, wideStep, tallStep), content, measure);
    }

    if (!result.second.readable)
        result = chooseLargestShape({tightTallShape}, content, measure);

    return result;
}

void Menu::placeItems(Rect content, const Flex::Measure& measure)
{
    auto leastNote = smallestReadableSize(content);
    auto area = itemsIn(content, chosen);
    auto sizes =
        ItemTreeSizes {pointSize, calculateTapTargetIn(content, area), leastNote};
    auto [tree, nodes] = makeItemTree(items, sizes);
    tree.resolve(area, measure);

    for (const auto& each: nodes)
    {
        auto labelSize = tree.node(each.label).text->pointSize;
        placed.add({tree.rect(each.label).center(),
                    labelSize,
                    tree.rect(each.row),
                    noteSizeFor(labelSize, leastNote)});
    }
}

void Menu::layOut(Hud& hud)
{
    auto frame = frameOf(*this, topClearance);

    if (!itemsChanged && isLaidOutFor(frame))
        return;

    itemsChanged = false;
    laidOutFor = frame;
    placed.clear();

    if (items.empty())
        return;

    auto content = frame.contentArea();
    auto measure = menuMeasure(hud);
    auto [shapeChosen, size] = squeezeUntilReadable(content, measure);
    chosen = shapeChosen;
    pointSize = size.pointSize;
    placeItems(content, measure);
}

void Menu::draw(Hud& hud)
{
    layOut(hud);

    if (opacity <= 0.f)
        return;

    for (auto index = 0; index < (int) placed.size(); ++index)
        drawItem(hud, index);
}

bool Menu::isEnabled(int index) const
{
    return items[index].enabled != Enabled::No;
}

bool Menu::isLit(int index) const
{
    return isEnabled(index) && (hovered == index || pressed == index);
}

Graphics::Color Menu::getLabelColor(int index) const
{
    if (!isEnabled(index))
        return faded(disabledColor, opacity);

    auto brighten = isLit(index) ? litBrighten : 0.f;
    return faded(pink(brighten), opacity);
}

Graphics::Color Menu::getRingColor(int index) const
{
    if (!isEnabled(index))
        return faded(disabledColor, opacity);

    return faded(pink(ringBrighten), opacity);
}

void Menu::drawSelectionRing(Hud& hud, int index)
{
    const auto& area = placed[index].area;
    hud.strokeRoundedRect(
        area, ringWidth, std::min(area.h * 0.5f, ringCorner), getRingColor(index));
}

void Menu::drawNote(Hud& hud, int index, float top)
{
    const auto& item = items[index];
    const auto& place = placed[index];
    const auto& noteFont = menuFont(place.noteSize);
    auto width = hud.measure(item.note, noteFont);
    auto baseline = top - place.pointSize * noteLift;
    drawMenuText(hud,
                 item.note,
                 {place.center.x - width * 0.5f, baseline},
                 {noteFont, faded(pink(), opacity), opacity});
}

void Menu::drawItem(Hud& hud, int index)
{
    const auto& item = items[index];
    const auto& place = placed[index];
    auto scale = isLit(index) ? hoverScale : 1.f;
    const auto& font = menuFont(place.pointSize * scale);
    auto top = place.center.y - font.pointSize * lineSpacing * 0.5f;

    if (showSelection && selected == index)
        drawSelectionRing(hud, index);

    if (!item.note.empty())
        drawNote(hud, index, top);

    auto width = hud.measure(item.label, font);
    auto at =
        Point {place.center.x - width * 0.5f, top + hud.ascent(font) * ascentShare};
    drawMenuText(hud, item.label, at, {font, getLabelColor(index), opacity});
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

bool Menu::isChoosable(int index) const
{
    auto numItems = static_cast<int>(items.size());
    auto isInRange = index >= 0 && index < numItems;
    return isInRange && isEnabled(index);
}

void Menu::choose(int index)
{
    if (!isChoosable(index))
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
