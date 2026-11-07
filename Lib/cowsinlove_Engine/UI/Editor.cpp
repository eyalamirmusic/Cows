#include "UI/Editor.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace Cows
{
namespace
{
using Graphics::Point;
using Graphics::Rect;

constexpr auto wideShape = Editor::Shape {true, 0.55f};
constexpr auto tallShape = Editor::Shape {false, 0.55f};
constexpr auto leastCowShare = 0.3f;
constexpr auto squeezeSteps = 5;
constexpr auto panelMargin = 0.04f;

constexpr auto lineSpacing = 1.05f;
constexpr auto hoverScale = 1.08f;
// Shares of the tap target, so a small window's padding shrinks with it.
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

// Shares of the point size.
constexpr auto nameGap = 0.4f;
constexpr auto rowPadding = 0.25f;
constexpr auto rowGap = 0.1f;
constexpr auto doneGap = 0.3f;

// A name and three targets per row.
constexpr auto targetWidthShare = 1.f / 6.f;
// Of the content's short side, the same for every shape so the cap never jumps.
constexpr auto largestShare = 0.08f;
constexpr auto leastLargest = 22.f;
constexpr auto mostLargest = 72.f;
// In tap targets.
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
        return {
            content.x, content.y, std::round(content.w * shape.cowShare), content.h};

    return {content.x, content.y, content.w, std::round(content.h * shape.cowShare)};
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
    return {from.wide, from.cowShare + (leastCowShare - from.cowShare) * along};
}

Flex::Node label(std::string text, float pointSize)
{
    auto node = Flex::Node {};
    node.text = Flex::Text {std::move(text), pointSize, lineSpacing};
    node.height = Flex::Size::grow();
    node.padding = {.top = pointSize * rowPadding, .bottom = pointSize * rowPadding};
    return node;
}

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

float linesFor(RowShape shape, int rows)
{
    auto perRow = shape == RowShape::Stacked ? stackedRowLines : 1.f;
    return perRow * (float) rows + 1.f + titleLines;
}

struct PanelContent final
{
    const std::string& title;
    const Vector<EditorRow>& rows;
    const std::string& doneLabel;
};

struct PanelSizing final
{
    RowShape shape = RowShape::Inline;
    float pointSize = 0.f;
    float target = 0.f;
    float margin = 0.f;
};

Flex::Node makeRowLine(const PanelSizing& sizing)
{
    auto stacked = sizing.shape == RowShape::Stacked;
    auto line = Flex::Node {};
    line.direction = stacked ? Flex::Direction::Down : Flex::Direction::Across;
    line.crossAlign = stacked ? Flex::Align::Centre : Flex::Align::Start;
    line.height = Flex::Size::fit(sizing.target);
    line.padding = {.left = sizing.target * rowSidePadding,
                    .right = sizing.target * rowSidePadding};
    return line;
}

Flex::Node makeRowName(const std::string& text, const PanelSizing& sizing)
{
    auto name = label(text, sizing.pointSize);
    name.column = nameColumn;

    if (sizing.shape == RowShape::Stacked)
        name.height = Flex::Size::fit();
    else
        name.padding.right = sizing.pointSize * nameGap;

    return name;
}

RowNodes addRowNodes(Flex::Tree& tree,
                     Flex::NodeId tableId,
                     const EditorRow& row,
                     const PanelSizing& sizing)
{
    auto lineId = tree.add(tableId, makeRowLine(sizing));
    auto nameId = tree.add(lineId, makeRowName(row.name, sizing));
    auto controls = lineId;

    if (sizing.shape == RowShape::Stacked)
    {
        auto group = Flex::Node {};
        group.height = Flex::Size::fit(sizing.target);
        controls = tree.add(lineId, group);
    }

    auto value = button({}, sizing.pointSize, sizing.target);
    value.text->alsoFits = row.choices;
    value.column = valueColumn;

    auto previousId =
        tree.add(controls, button(previousArrow, sizing.pointSize, sizing.target));
    auto valueId = tree.add(controls, value);
    auto nextId =
        tree.add(controls, button(nextArrow, sizing.pointSize, sizing.target));
    return {lineId, nameId, previousId, valueId, nextId};
}

Flex::Node makePanelBlock(const PanelSizing& sizing)
{
    auto block = Flex::Node {};
    block.direction = Flex::Direction::Down;
    block.mainAlign = Flex::Align::Centre;
    block.crossAlign = Flex::Align::Centre;
    block.padding = {.left = sizing.margin, .right = sizing.margin};
    block.gap = sizing.pointSize * doneGap;
    return block;
}

Flex::Tree makePanelTree(const PanelContent& content,
                         const PanelSizing& sizing,
                         PanelNodes& nodes)
{
    auto tree = Flex::Tree {makePanelBlock(sizing)};

    auto heading = Flex::Node {};
    heading.text = Flex::Text {
        content.title, onFontGrid(sizing.pointSize * titleScale), titleLine};
    nodes.title = tree.add(Flex::Tree::root, heading);

    auto table = Flex::Node {};
    table.direction = Flex::Direction::Down;
    table.gap = sizing.pointSize * rowGap;
    auto tableId = tree.add(Flex::Tree::root, table);

    nodes.rows.clear();

    for (const auto& row: content.rows)
        nodes.rows.add(addRowNodes(tree, tableId, row, sizing));

    auto done = button(content.doneLabel, sizing.pointSize, sizing.target);
    done.height = Flex::Size::fit(sizing.target);
    nodes.done = tree.add(Flex::Tree::root, done);

    return tree;
}

struct PanelFitting final
{
    Rect content;
    int numRows = 0;
    Flex::Measure measure;
    float readable = 0.f;
    float largest = 0.f;
};

float calculateTapTarget(const PanelFitting& fitting, const PanelShape& shape)
{
    auto panel = panelIn(fitting.content, shape.area);
    return std::min({tapTarget(fitting.content),
                     panel.h / linesFor(shape.rows, fitting.numRows),
                     panel.w * targetWidthShare});
}

PanelSizing calculatePanelSizing(const PanelFitting& fitting,
                                 const PanelShape& shape,
                                 float pointSize)
{
    auto panel = panelIn(fitting.content, shape.area);
    return {shape.rows,
            pointSize,
            calculateTapTarget(fitting, shape),
            panel.w * panelMargin};
}

struct FittedPanel final
{
    PanelShape shape;
    ChosenShape fitted;
};

FittedPanel chooseLargestPanel(const PanelContent& content,
                               const PanelFitting& fitting,
                               const Vector<PanelShape>& shapes)
{
    auto candidates = Vector<LayoutShape> {};

    for (const auto& each: shapes)
    {
        auto build = [content, fitting, each](float pointSize)
        {
            auto ignored = PanelNodes {};
            return makePanelTree(
                content, calculatePanelSizing(fitting, each, pointSize), ignored);
        };
        candidates.add({build, panelIn(fitting.content, each.area)});
    }

    auto best = largestShape(
        candidates, fitting.measure, {fitting.readable, fitting.largest});
    return {shapes[best.index], best};
}

FittedPanel chooseReadablePanel(const PanelContent& content,
                                const PanelFitting& fitting,
                                bool byAspect)
{
    auto result = FittedPanel {};
    auto preferred = byAspect ? wideShape : tallShape;
    auto other = byAspect ? tallShape : wideShape;

    for (auto rowShape: {RowShape::Inline, RowShape::Stacked})
    {
        for (auto step = 0; step <= squeezeSteps && !result.fitted.readable; ++step)
        {
            auto first = squeezed(preferred, step);
            auto second = squeezed(other, step);
            result = chooseLargestPanel(
                content, fitting, {{first, rowShape}, {second, rowShape}});
        }
    }

    return result;
}

float calculateLargestPointSize(Rect content)
{
    return std::clamp(
        std::min(content.w, content.h) * largestShare, leastLargest, mostLargest);
}

struct TextMetrics final
{
    float pointSize = 0.f;
    float ascent = 0.f;
    bool namesAboveValues = false;
};

float calculateBaselineIn(const Rect& area, const TextMetrics& metrics)
{
    return area.y + area.h * 0.5f - metrics.pointSize * lineSpacing * 0.5f
           + metrics.ascent * ascentShare;
}

Editor::Placed
    placeRow(const Flex::Tree& tree, const RowNodes& row, const TextMetrics& metrics)
{
    auto place = Editor::Placed {};
    place.pointSize = metrics.pointSize;
    place.row = tree.rect(row.row);
    place.name = tree.rect(row.name);
    place.previous = tree.rect(row.previous);
    place.value = tree.rect(row.value);
    place.next = tree.rect(row.next);
    place.baseline = calculateBaselineIn(
        metrics.namesAboveValues ? place.value : place.row, metrics);
    place.nameBaseline = metrics.namesAboveValues
                             ? calculateBaselineIn(place.name, metrics)
                             : place.baseline;
    return place;
}

struct PanelPlacement final
{
    Vector<Editor::Placed> rows;
    Editor::Placed done;
};

PanelPlacement placeRowsAndDone(const Flex::Tree& tree,
                                const PanelNodes& nodes,
                                const TextMetrics& metrics)
{
    auto placement = PanelPlacement {};

    for (const auto& row: nodes.rows)
        placement.rows.add(placeRow(tree, row, metrics));

    placement.done.pointSize = metrics.pointSize;
    placement.done.value = tree.rect(nodes.done);
    placement.done.baseline = calculateBaselineIn(placement.done.value, metrics);
    placement.done.row = placement.done.value;
    return placement;
}

} // namespace

Editor::Editor()
{
    setShowing(false);
}

Editor::Shape Editor::shape() const
{
    if (isLaidOutFor(frameOf(*this, topClearance)))
        return chosen;

    return aspectWide(*this) ? wideShape : tallShape;
}

bool Editor::isLaidOutFor(const ScreenFrame& frame) const
{
    return laidOutFor.has_value() && sameFrame(*laidOutFor, frame);
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
    auto upToDate = !contentChanged && isLaidOutFor(frame);

    if (upToDate)
        return;

    contentChanged = false;
    laidOutFor = frame;

    auto content = frame.contentArea();
    auto fitting = PanelFitting {content,
                                 (int) rows.size(),
                                 menuMeasure(hud),
                                 smallestReadableSize(content),
                                 calculateLargestPointSize(content)};
    auto panelContent = PanelContent {title, rows, doneLabel};
    auto choice = chooseReadablePanel(panelContent, fitting, aspectWide(*this));

    chosen = choice.shape.area;
    namesAboveValues = choice.shape.rows == RowShape::Stacked;
    auto size = choice.fitted.pointSize;

    auto nodes = PanelNodes {};
    auto tree = makePanelTree(
        panelContent, calculatePanelSizing(fitting, choice.shape, size), nodes);
    tree.resolve(panelIn(content, chosen), fitting.measure);

    titleBox = tree.rect(nodes.title);
    titleSize = tree.node(nodes.title).text->pointSize;
    auto placement = placeRowsAndDone(
        tree, nodes, {size, hud.ascent(menuFont(size)), namesAboveValues});
    placed = std::move(placement.rows);
    done = placement.done;
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
                 {titleFont, faded(pink(ringBrighten), opacity), opacity});

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
              {row.name,
               place.name,
               place.nameBaseline,
               place.pointSize,
               false,
               namesAboveValues});
    drawLabel(hud,
              {previousArrow,
               place.previous,
               place.baseline,
               place.pointSize,
               isLit({index, Part::Previous}),
               true});
    drawLabel(hud,
              {value,
               place.value,
               place.baseline,
               place.pointSize,
               isLit({index, Part::Value}),
               true});
    drawLabel(hud,
              {nextArrow,
               place.next,
               place.baseline,
               place.pointSize,
               isLit({index, Part::Next}),
               true});
}

void Editor::drawDone(Hud& hud)
{
    if (showSelection && selected == doneIndex())
        drawRing(hud, done.row);

    drawLabel(hud,
              {doneLabel,
               done.value,
               done.baseline,
               done.pointSize,
               isLit({doneIndex(), Part::Done}),
               true});
}

void Editor::drawLabel(Hud& hud, const LabelToDraw& label)
{
    const auto& text = label.text;
    const auto& area = label.area;
    auto baseline = label.baseline;
    auto pointSize = label.pointSize;
    auto lit = label.lit;
    auto centred = label.centred;
    const auto& font = menuFont(pointSize * (lit ? hoverScale : 1.f));
    auto size = font.pointSize;
    auto width = hud.measure(text, font);
    auto x = centred ? area.x + (area.w - width) * 0.5f : area.x;
    auto grownBy = (size - pointSize) * hoverDrop;

    drawMenuText(hud,
                 text,
                 {x, baseline + grownBy},
                 {font, faded(pink(lit ? litBrighten : 0.f), opacity), opacity});
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
        onRowValueStepped(selected, by);
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
            onRowValueStepped(target.row, -1);
            break;
        case Part::Value:
        case Part::Next:
            onRowValueStepped(target.row, 1);
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
