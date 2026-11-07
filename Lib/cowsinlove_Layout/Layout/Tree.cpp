#include "Layout/Tree.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace Cows::Flex
{
namespace
{
using eacp::Vector;
using eacp::Graphics::Point;
using eacp::Graphics::Rect;

constexpr auto overflowRoundingTolerance = 0.01f;

// Rounds each edge, not the size, so neighbours meet exactly and never overlap.
Rect snapped(Rect exact)
{
    auto left = std::round(exact.x);
    auto top = std::round(exact.y);
    return {left,
            top,
            std::round(exact.x + exact.w) - left,
            std::round(exact.y + exact.h) - top};
}

float along(Point point, Direction direction)
{
    return direction == Direction::Across ? point.x : point.y;
}

float across(Point point, Direction direction)
{
    return direction == Direction::Across ? point.y : point.x;
}

const Size& mainSize(const Node& node, Direction direction)
{
    return direction == Direction::Across ? node.width : node.height;
}

const Size& crossSize(const Node& node, Direction direction)
{
    return direction == Direction::Across ? node.height : node.width;
}

// A Share is unknown until the parent is sized, so it asks nothing.
float basis(const Size& size, float fit)
{
    switch (size.kind)
    {
        case Size::Kind::Fit:
            return std::max(size.least, fit);
        case Size::Kind::Fixed:
            return size.value;
        case Size::Kind::Grow:
            return size.least;
        case Size::Kind::Share:
            return 0.f;
    }

    return fit;
}

float crossBasis(const Size& size, float fit)
{
    if (size.kind == Size::Kind::Grow)
        return std::max(size.least, fit);

    return basis(size, fit);
}

float offsetFor(Align align, float free)
{
    auto room = std::max(free, 0.f);

    switch (align)
    {
        case Align::Start:
            return 0.f;
        case Align::Centre:
            return room * 0.5f;
        case Align::End:
            return room;
    }

    return 0.f;
}

float sumLeasts(const Vector<Size>& sizes)
{
    auto leastSum = 0.f;

    for (const auto& size: sizes)
        leastSum += size.least;

    return leastSum;
}

void shareInProportionToLeasts(const Vector<Size>& sizes,
                               Vector<float>& shares,
                               float room,
                               float leastSum)
{
    for (auto index = 0; index < sizes.size(); ++index)
        shares[index] = room * sizes[index].least / leastSum;
}

struct EvenSplit final
{
    Vector<char> settled;
    float left = 0.f;
    int open = 0;
};

bool settleSharesOutsideTheirBounds(const Vector<Size>& sizes,
                                    Vector<float>& shares,
                                    EvenSplit& split)
{
    auto each = split.left / (float) split.open;
    auto changed = false;

    for (auto index = 0; index < sizes.size(); ++index)
    {
        if (split.settled[index])
            continue;

        auto clamped = std::clamp(each, sizes[index].least, sizes[index].most);

        if (clamped == each)
            continue;

        shares[index] = clamped;
        split.settled[index] = 1;
        split.left -= clamped;
        --split.open;
        changed = true;
    }

    return changed;
}

void shareWhatIsLeftAmongUnsettled(Vector<float>& shares, const EvenSplit& split)
{
    if (split.open == 0)
        return;

    auto each = std::max(split.left, 0.f) / (float) split.open;

    for (auto index = 0; index < shares.size(); ++index)
    {
        if (!split.settled[index])
            shares[index] = each;
    }
}

void shareEvenlyWithinBounds(const Vector<Size>& sizes,
                             Vector<float>& shares,
                             float room)
{
    auto count = static_cast<int>(sizes.size());
    auto split = EvenSplit {};
    split.settled.resize(count, 0);
    split.left = room;
    split.open = count;

    for (auto round = 0; round <= count && split.open > 0; ++round)
    {
        if (!settleSharesOutsideTheirBounds(sizes, shares, split))
            break;
    }

    shareWhatIsLeftAmongUnsettled(shares, split);
}

// Everyone's least first when there is room, else in proportion to the leasts.
void shareGrowth(const Vector<Size>& sizes, Vector<float>& shares, float room)
{
    auto leastSum = sumLeasts(sizes);

    if (room <= 0.f)
        std::fill(shares.begin(), shares.end(), 0.f);
    else if (room < leastSum)
        shareInProportionToLeasts(sizes, shares, room, leastSum);
    else
        shareEvenlyWithinBounds(sizes, shares, room);
}

float crossLengthFor(const Size& size, float fitAcross, float crossRoom)
{
    switch (size.kind)
    {
        case Size::Kind::Fixed:
            return size.value;
        case Size::Kind::Share:
            return size.value * crossRoom;
        case Size::Kind::Fit:
            return basis(size, fitAcross);
        case Size::Kind::Grow:
            return std::min(std::clamp(crossRoom, size.least, size.most), crossRoom);
    }

    return 0.f;
}

bool isSharingColumn(const Node& node)
{
    return node.column != Node::noColumn && node.width.kind == Size::Kind::Fit;
}

Point textFit(const Text& text, const Measure& measure)
{
    auto width = measure(text.text, text.pointSize);

    for (const auto& other: text.alsoFits)
        width = std::max(width, measure(other, text.pointSize));

    return {width, text.pointSize * text.lineHeight};
}

Point orientedPoint(Point mainAndCross, Direction direction)
{
    if (direction == Direction::Across)
        return mainAndCross;

    return {mainAndCross.y, mainAndCross.x};
}

Rect orientedRect(Point mainAndCrossStart,
                  Point mainAndCrossLength,
                  Direction direction)
{
    auto start = orientedPoint(mainAndCrossStart, direction);
    auto length = orientedPoint(mainAndCrossLength, direction);
    return {start.x, start.y, length.x, length.y};
}

float gapsTotal(float gap, int count)
{
    return gap * (float) std::max(count - 1, 0);
}
} // namespace

Size Size::fit(float least)
{
    return {Kind::Fit, least, unbounded, 0.f};
}

Size Size::grow(float least, float most)
{
    return {Kind::Grow, least, std::max(least, most), 0.f};
}

Size Size::fixed(float points)
{
    return {Kind::Fixed, 0.f, unbounded, points};
}

Size Size::share(float fraction)
{
    return {Kind::Share, 0.f, unbounded, fraction};
}

Tree::Tree(Node rootNode)
{
    auto entry = Entry {};
    entry.node = std::move(rootNode);
    entries.add(std::move(entry));
}

NodeId Tree::add(NodeId parent, Node node)
{
    assert(parent >= 0 && parent < entries.size());

    auto id = entries.size();
    auto entry = Entry {};
    entry.node = std::move(node);
    entry.parent = parent;
    entries.add(std::move(entry));

    auto& above = entries[parent];

    if (above.lastChild < 0)
        above.firstChild = id;
    else
        entries[above.lastChild].nextSibling = id;

    above.lastChild = id;
    return id;
}

const Node& Tree::node(NodeId id) const
{
    return entries[id].node;
}

Rect Tree::rect(NodeId id) const
{
    return entries[id].resolved.rect;
}

bool Tree::overflow(NodeId id) const
{
    return entries[id].resolved.overflow;
}

bool Tree::overflows() const
{
    return std::any_of(entries.begin(),
                       entries.end(),
                       [](const Entry& entry) { return entry.resolved.overflow; });
}

Point Tree::fitSize() const
{
    const auto& top = entries[root];
    return {basis(Size::fit(top.node.width.least), top.fit.x),
            basis(Size::fit(top.node.height.least), top.fit.y)};
}

void Tree::resolve(Rect available, const Measure& measure)
{
    measureAll(measure);

    place(root, snapped(available));
}

// Children come after their parent, so walking backwards measures children first.
void Tree::measureAll(const Measure& measure)
{
    columnWidths.clear();

    for (auto id = entries.size() - 1; id >= 0; --id)
        entries[id].fit = fitOf(id, measure);

    shareColumns();

    if (columnWidths.empty())
        return;

    for (auto id = entries.size() - 1; id >= 0; --id)
        entries[id].fit = fitOf(id, measure);
}

void Tree::shareColumns()
{
    for (const auto& entry: entries)
    {
        auto column = entry.node.column;

        if (column == Node::noColumn || entry.node.width.kind != Size::Kind::Fit)
            continue;

        if (column >= columnWidths.size())
            columnWidths.resize(column + 1, 0.f);

        columnWidths[column] =
            std::max(columnWidths[column], basis(entry.node.width, entry.fit.x));
    }
}

Point Tree::childrenFit(NodeId id) const
{
    const auto& entry = entries[id];
    auto direction = entry.node.direction;
    auto main = 0.f;
    auto cross = 0.f;
    auto count = 0;

    for (auto child = entry.firstChild; child >= 0;
         child = entries[child].nextSibling)
    {
        const auto& held = entries[child];
        main += basis(mainSize(held.node, direction), along(held.fit, direction));
        cross = std::max(cross,
                         crossBasis(crossSize(held.node, direction),
                                    across(held.fit, direction)));
        ++count;
    }

    main += gapsTotal(entry.node.gap, count);
    return orientedPoint({main, cross}, direction);
}

Point Tree::fitOf(NodeId id, const Measure& measure) const
{
    const auto& node = entries[id].node;
    auto padding = node.padding.size();
    auto content =
        node.text.has_value() ? textFit(*node.text, measure) : childrenFit(id);
    auto fit = Point {content.x + padding.x, content.y + padding.y};

    if (isSharingColumn(node) && node.column < columnWidths.size())
        fit.x = std::max(fit.x, columnWidths[node.column]);

    return fit;
}

void Tree::place(NodeId id, Rect area)
{
    auto& entry = entries[id];
    entry.unsnappedRect = area;
    entry.resolved.rect = snapped(area);
    entry.resolved.overflow =
        entry.node.text.has_value()
        && (entry.fit.x > area.w + overflowRoundingTolerance
            || entry.fit.y > area.h + overflowRoundingTolerance);

    if (entry.firstChild >= 0)
        placeChildren(id);
}

float Tree::measureLengthsAlongParent(NodeId id, float mainRoom)
{
    const auto& entry = entries[id];
    auto direction = entry.node.direction;
    auto count = 0;
    auto used = 0.f;
    growSizes.clear();

    for (auto child = entry.firstChild; child >= 0;
         child = entries[child].nextSibling)
    {
        auto& held = entries[child];
        const auto& size = mainSize(held.node, direction);
        ++count;

        if (size.kind == Size::Kind::Grow)
            growSizes.add(size);
        else if (size.kind == Size::Kind::Share)
            held.lengthAlongParent = size.value * mainRoom;
        else
            held.lengthAlongParent = basis(size, along(held.fit, direction));

        if (size.kind != Size::Kind::Grow)
            used += held.lengthAlongParent;
    }

    return used + gapsTotal(entry.node.gap, count);
}

float Tree::growChildren(NodeId id, float room)
{
    const auto& entry = entries[id];
    auto direction = entry.node.direction;
    grown.clear();
    grown.resize(growSizes.size(), 0.f);
    shareGrowth(growSizes, grown, room);

    auto growIndex = 0;
    auto used = 0.f;

    for (auto child = entry.firstChild; child >= 0;
         child = entries[child].nextSibling)
    {
        auto& held = entries[child];

        if (mainSize(held.node, direction).kind != Size::Kind::Grow)
            continue;

        held.lengthAlongParent = grown[growIndex++];
        used += held.lengthAlongParent;
    }

    return used;
}

void Tree::placeChildrenInRoom(NodeId id, Rect inner, float used)
{
    const auto& entry = entries[id];
    const auto& node = entry.node;
    auto direction = node.direction;
    auto room = orientedPoint({inner.w, inner.h}, direction);
    auto start = orientedPoint({inner.x, inner.y}, direction);
    auto cursor = start.x + offsetFor(node.mainAlign, room.x - used);

    for (auto child = entry.firstChild; child >= 0;
         child = entries[child].nextSibling)
    {
        const auto& held = entries[child];
        auto cross = crossLengthFor(
            crossSize(held.node, direction), across(held.fit, direction), room.y);

        if (cross > room.y + overflowRoundingTolerance)
            entries[id].resolved.overflow = true;

        auto main = held.lengthAlongParent;
        auto crossAt = start.y + offsetFor(node.crossAlign, room.y - cross);

        place(child, orientedRect({cursor, crossAt}, {main, cross}, direction));
        cursor += main + node.gap;
    }
}

// Works in unsnapped points throughout; only the rects handed back are snapped.
void Tree::placeChildren(NodeId id)
{
    const auto& node = entries[id].node;
    auto inner = entries[id].unsnappedRect.inset(node.padding);
    inner.w = std::max(inner.w, 0.f);
    inner.h = std::max(inner.h, 0.f);

    auto mainRoom = along({inner.w, inner.h}, node.direction);
    auto used = measureLengthsAlongParent(id, mainRoom);
    used += growChildren(id, mainRoom - used);

    if (used > mainRoom + overflowRoundingTolerance)
        entries[id].resolved.overflow = true;

    placeChildrenInRoom(id, inner, used);
}
} // namespace Cows::Flex
