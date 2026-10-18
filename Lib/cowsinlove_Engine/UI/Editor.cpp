#include "UI/Editor.h"
#include "Layout/FitToSize.h"

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

// The cow's share of the content, wide and tall, and the least it gives way
// to, a step at a time, before the rows' text goes under the readable size.
constexpr auto wideShape = Editor::Shape {true, 0.55f};
constexpr auto tallShape = Editor::Shape {false, 0.55f};
constexpr auto leastCowShare = 0.3f;
constexpr auto squeezeSteps = 5;
constexpr auto panelMargin = 0.04f;

constexpr auto lineSpacing = 1.05f;
constexpr auto hoverScale = 1.08f;
// Room either side of a button's label, and of a row, as shares of the tap
// target, so a small window's targets shrink with their padding.
constexpr auto buttonPadding = 14.f / leastTarget;
constexpr auto rowSidePadding = 7.f / leastTarget;
constexpr auto ringWidth = 4.f;
constexpr auto ringCorner = 28.f;
constexpr auto titleScale = 1.3f;
constexpr auto titleLine = 1.4f;
constexpr auto ascentShare = 0.9f;
constexpr auto hoverDrop = 0.3f;
constexpr auto litBrighten = 0.2f;
constexpr auto ringBrighten = 0.1f;

// Shares of the point size: the space after a name, a cell's padding top and
// bottom, the gap between rows and before Done.
constexpr auto nameGap = 0.4f;
constexpr auto rowPadding = 0.25f;
constexpr auto rowGap = 0.1f;
constexpr auto doneGap = 0.3f;

// A row is a name and three targets, so none is wider than this share of the
// panel.
constexpr auto targetWidthShare = 1.f / 6.f;
// The largest the rows' text goes, as a share of the content's short side,
// the same whatever shape is chosen so the cap never jumps between shapes.
constexpr auto largestShare = 0.08f;
constexpr auto leastLargest = 22.f;
constexpr auto mostLargest = 72.f;
// The title's share of the panel's height, in tap targets.
constexpr auto titleLines = 1.5f;

constexpr auto stackedRowLines = 1.5f;

constexpr auto nameColumn = 0;
constexpr auto valueColumn = 1;

constexpr auto previousArrow = "<";
constexpr auto nextArrow = ">";

bool aspectWide(const Graphics::View& view)
{
    auto bounds = view.getLocalBounds();
    return bounds.w >= bounds.h;
}

Rect cowIn(Rect content, Editor::Shape shape)
{
    if (shape.wide)
        return {content.x, content.y, std::round(content.w * shape.cow), content.h};

    return {content.x, content.y, content.w, std::round(content.h * shape.cow)};
}

Rect panelIn(Rect content, Editor::Shape shape)
{
    auto cow = cowIn(content, shape);

    if (shape.wide)
        return {cow.x + cow.w, content.y, content.w - cow.w, content.h};

    return {content.x, cow.y + cow.h, content.w, content.h - cow.h};
}

Editor::Shape squeezed(Editor::Shape from, int step)
{
    auto along = (float) step / (float) squeezeSteps;
    return {from.wide, from.cow + (leastCowShare - from.cow) * along};
}

// A cell of a row, as tall as the row, with room above and below its text.
Flex::Node label(std::string text, float pointSize)
{
    auto node = Flex::Node {};
    node.text = Flex::Text {std::move(text), pointSize, lineSpacing};
    node.height = Flex::Size::grow();
    node.padding = {.top = pointSize * rowPadding, .bottom = pointSize * rowPadding};
    return node;
}

// An arrow or Done: its label centred in at least a tap target.
Flex::Node button(std::string text, float pointSize, float target)
{
    auto node = label(std::move(text), pointSize);
    node.width = Flex::Size::fit(target);
    node.padding.left = target * buttonPadding;
    node.padding.right = target * buttonPadding;
    return node;
}

struct RowNodes final
{
    Flex::NodeId row = 0;
    Flex::NodeId name = 0;
    Flex::NodeId previous = 0;
    Flex::NodeId value = 0;
    Flex::NodeId next = 0;
};

struct PanelNodes final
{
    Flex::NodeId title = 0;
    Vector<RowNodes> rows;
    Flex::NodeId done = 0;
};

// How a row sits: its name beside its arrows and value, or over them when
// the panel is too narrow for both at a readable size.
enum class RowShape
{
    Inline,
    Stacked
};

struct PanelShape final
{
    Editor::Shape area;
    RowShape rows = RowShape::Inline;
};

// What the rows and Done need of the panel's height, in tap targets: a row
// each (and half again for a name over its row, a line of text rather than a
// target), Done, and the title.
float linesFor(RowShape shape, int rows)
{
    auto perRow = shape == RowShape::Stacked ? stackedRowLines : 1.f;
    return perRow * (float) rows + 1.f + titleLines;
}

// A centred block in the panel: the title, a table of rows (name, <, value,
// >), and Done under it.
Flex::Tree panelTree(const std::string& title,
                     const Vector<EditorRow>& rows,
                     const std::string& doneLabel,
                     RowShape shape,
                     float pointSize,
                     float target,
                     float margin,
                     PanelNodes* nodes)
{
    auto block = Flex::Node {};
    block.direction = Flex::Direction::Down;
    block.mainAlign = Flex::Align::Centre;
    block.crossAlign = Flex::Align::Centre;
    block.padding = {.left = margin, .right = margin};
    block.gap = pointSize * doneGap;
    auto tree = Flex::Tree {block};

    auto heading = Flex::Node {};
    heading.text = Flex::Text {title, onFontGrid(pointSize * titleScale), titleLine};
    auto titleId = tree.add(Flex::Tree::root, heading);

    auto table = Flex::Node {};
    table.direction = Flex::Direction::Down;
    table.gap = pointSize * rowGap;
    auto tableId = tree.add(Flex::Tree::root, table);

    auto made = PanelNodes {};
    made.title = titleId;

    for (const auto& row: rows)
    {
        auto stacked = shape == RowShape::Stacked;
        auto line = Flex::Node {};
        line.direction = stacked ? Flex::Direction::Down : Flex::Direction::Across;
        line.height = Flex::Size::fit(target);
        line.padding = {.left = target * rowSidePadding,
                        .right = target * rowSidePadding};
        auto lineId = tree.add(tableId, line);

        auto name = label(row.name, pointSize);
        name.column = nameColumn;

        if (stacked)
            name.height = Flex::Size::fit();
        else
            name.padding.right = pointSize * nameGap;

        auto nameId = tree.add(lineId, name);
        auto controls = lineId;

        if (stacked)
        {
            auto group = Flex::Node {};
            group.height = Flex::Size::fit(target);
            controls = tree.add(lineId, group);
        }

        auto value = button({}, pointSize, target);
        value.text->alsoFits = row.choices;
        value.column = valueColumn;

        auto previousId =
            tree.add(controls, button(previousArrow, pointSize, target));
        auto valueId = tree.add(controls, value);
        auto nextId = tree.add(controls, button(nextArrow, pointSize, target));
        made.rows.add({lineId, nameId, previousId, valueId, nextId});
    }

    auto done = button(doneLabel, pointSize, target);
    done.height = Flex::Size::fit(target);
    made.done = tree.add(Flex::Tree::root, done);

    if (nodes != nullptr)
        *nodes = made;

    return tree;
}

} // namespace

Editor::Editor()
{
    setShowing(false);
}

Editor::Shape Editor::shape() const
{
    if (laidOutFor.has_value()
        && sameFrame(*laidOutFor, frameOf(*this, topClearance)))
        return chosen;

    return aspectWide(*this) ? wideShape : tallShape;
}

bool Editor::wide() const
{
    return shape().wide;
}

Rect Editor::cowArea() const
{
    return cowIn(frameOf(*this, topClearance).contentArea(), shape());
}

Rect Editor::panelArea() const
{
    return panelIn(frameOf(*this, topClearance).contentArea(), shape());
}

void Editor::setRows(Vector<EditorRow> newRows)
{
    rows = std::move(newRows);
    contentChanged = true;
}

void Editor::setTitle(std::string newTitle)
{
    title = std::move(newTitle);
    contentChanged = true;
}

int Editor::doneIndex() const
{
    return (int) rows.size();
}

void Editor::layOut(Hud& hud)
{
    auto frame = frameOf(*this, topClearance);

    if (!contentChanged && laidOutFor.has_value() && sameFrame(*laidOutFor, frame))
        return;

    contentChanged = false;
    laidOutFor = frame;

    auto content = frame.contentArea();
    auto measure = menuMeasure(hud);
    auto byAspect = aspectWide(*this);

    auto targetFor = [&](const PanelShape& shape)
    {
        auto panel = panelIn(content, shape.area);
        return std::min({tapTarget(content),
                         panel.h / linesFor(shape.rows, rows.size()),
                         panel.w * targetWidthShare});
    };

    auto readable = smallestReadableSize(content);
    auto largest = std::clamp(
        std::min(content.w, content.h) * largestShare, leastLargest, mostLargest);

    auto choose = [&](const Vector<PanelShape>& shapes)
    {
        auto candidates = Vector<LayoutShape> {};

        for (const auto& each: shapes)
        {
            auto panel = panelIn(content, each.area);
            auto target = targetFor(each);
            auto margin = panel.w * panelMargin;
            candidates.add({[this, each, target, margin](float pointSize)
                            {
                                return panelTree(title,
                                                 rows,
                                                 doneLabel,
                                                 each.rows,
                                                 pointSize,
                                                 target,
                                                 margin,
                                                 nullptr);
                            },
                            panel});
        }

        auto best = largestShape(candidates, measure, readable, largest);
        return std::pair {shapes[best.index], best};
    };

    // The first step at which a shape holds the rows at the readable size, the
    // cow's room giving way a step at a time before the text shrinks. At each
    // step the aspect's shape comes first, rows inline before names over
    // their values, unless another makes the text a quarter larger.
    auto shape = PanelShape {};
    auto fitted = ChosenShape {};

    for (auto step = 0; step <= squeezeSteps && !fitted.readable; ++step)
    {
        auto first = squeezed(byAspect ? wideShape : tallShape, step);
        auto second = squeezed(byAspect ? tallShape : wideShape, step);
        std::tie(shape, fitted) = choose({{first, RowShape::Inline},
                                          {first, RowShape::Stacked},
                                          {second, RowShape::Inline},
                                          {second, RowShape::Stacked}});
    }

    chosen = shape.area;
    stacked = shape.rows == RowShape::Stacked;
    auto size = fitted.pointSize;

    auto panel = panelIn(content, chosen);
    auto nodes = PanelNodes {};
    auto tree = panelTree(title,
                          rows,
                          doneLabel,
                          shape.rows,
                          size,
                          targetFor(shape),
                          panel.w * panelMargin,
                          &nodes);
    tree.resolve(panel, measure);

    auto ascent = hud.ascent(menuFont(size));
    auto baselineIn = [&](const Rect& area)
    {
        return area.y + area.h * 0.5f - size * lineSpacing * 0.5f
               + ascent * ascentShare;
    };

    titleBox = tree.rect(nodes.title);
    titleSize = tree.node(nodes.title).text->pointSize;
    placed.clear();

    for (const auto& row: nodes.rows)
    {
        auto place = Placed {};
        place.pointSize = size;
        place.row = tree.rect(row.row);
        place.name = tree.rect(row.name);
        place.baseline = baselineIn(stacked ? tree.rect(row.value) : place.row);
        place.nameBaseline = stacked ? baselineIn(place.name) : place.baseline;
        place.previous = tree.rect(row.previous);
        place.value = tree.rect(row.value);
        place.next = tree.rect(row.next);
        placed.add(place);
    }

    done = {};
    done.pointSize = size;
    done.value = tree.rect(nodes.done);
    done.baseline = baselineIn(done.value);
    done.row = done.value;
}

void Editor::draw(Hud& hud)
{
    layOut(hud);

    if (opacity <= 0.f)
        return;

    const auto& titleFont = menuFont(titleSize);
    auto titleWidth = hud.measure(title, titleFont);
    auto titleBaseline = titleBox.y + (titleBox.h - titleSize * lineSpacing) * 0.5f
                         + hud.ascent(titleFont) * ascentShare;
    drawMenuText(hud,
                 title,
                 {titleBox.x + (titleBox.w - titleWidth) * 0.5f, titleBaseline},
                 titleFont,
                 faded(pink(ringBrighten), opacity),
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

    drawLabel(hud,
              row.name,
              place.name,
              place.nameBaseline,
              place.pointSize,
              false,
              false);
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
    const auto& font = menuFont(pointSize * (lit ? hoverScale : 1.f));
    auto size = font.pointSize;
    auto width = hud.measure(text, font);
    auto x = centred ? area.x + (area.w - width) * 0.5f : area.x;
    auto grownBy = (size - pointSize) * hoverDrop;

    drawMenuText(hud,
                 text,
                 {x, baseline + grownBy},
                 font,
                 faded(pink(lit ? litBrighten : 0.f), opacity),
                 opacity);
}

void Editor::drawRing(Hud& hud, const Rect& area)
{
    hud.strokeRoundedRect(area,
                          ringWidth,
                          std::min(area.h * 0.5f, ringCorner),
                          faded(pink(ringBrighten), opacity));
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
