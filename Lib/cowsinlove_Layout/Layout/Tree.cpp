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

// Room a float rounding error may take before a node counts as overflowing.
constexpr auto tolerance = 0.01f;

// The rect with each edge rounded to a whole point: neighbours that share an
// edge round the same number, so they meet exactly and never overlap.
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

// What a child asks of its parent along one axis before any room is shared
// out: its fit, its fixed size, a Grow's least; a Share is unknown until the
// parent is sized, so it asks nothing.
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

// Across its parent, a Grow child stretches to the parent but still asks room
// for what it holds.
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

// Shares `room` among Grow sizes as evenly as their least and most allow:
// everyone's least first when there is room for it, else the room in
// proportion to their leasts.
void shareGrowth(const Vector<Size>& sizes, Vector<float>& shares, float room)
{
    auto count = sizes.size();
    auto leastSum = 0.f;

    for (const auto& size: sizes)
        leastSum += size.least;

    if (room <= 0.f)
    {
        for (auto& share: shares)
            share = 0.f;

        return;
    }

    if (room < leastSum)
    {
        for (auto index = 0; index < count; ++index)
            shares[index] = room * sizes[index].least / leastSum;

        return;
    }

    auto settled = Vector<char> {};
    settled.resize(count, 0);
    auto left = room;
    auto open = count;

    for (auto round = 0; round <= count && open > 0; ++round)
    {
        auto each = left / (float) open;
        auto changed = false;

        for (auto index = 0; index < count; ++index)
        {
            if (settled[index])
                continue;

            auto clamped = std::clamp(each, sizes[index].least, sizes[index].most);

            if (clamped != each)
            {
                shares[index] = clamped;
                settled[index] = 1;
                left -= clamped;
                --open;
                changed = true;
            }
        }

        if (!changed)
            break;
    }

    if (open == 0)
        return;

    auto each = std::max(left, 0.f) / (float) open;

    for (auto index = 0; index < count; ++index)
        if (!settled[index])
            shares[index] = each;
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

int Tree::size() const
{
    return entries.size();
}

void Tree::resolve(Rect available, const Measure& measure)
{
    measureAll(measure);

    place(root, snapped(available));
}

// Children always come after their parent, so walking the entries backwards
// measures every child before the node that holds it. A second walk lets the
// shared columns' widths reach the nodes that hold them.
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

Point Tree::fitOf(NodeId id, const Measure& measure) const
{
    const auto& entry = entries[id];
    const auto& node = entry.node;
    auto padding = node.padding.size();
    auto content = Point {};

    if (node.text.has_value())
    {
        const auto& text = *node.text;
        content.x = measure(text.text, text.pointSize);

        for (const auto& other: text.alsoFits)
            content.x = std::max(content.x, measure(other, text.pointSize));

        content.y = text.pointSize * text.lineHeight;
    }
    else
    {
        auto main = 0.f;
        auto cross = 0.f;
        auto count = 0;

        for (auto child = entry.firstChild; child >= 0;
             child = entries[child].nextSibling)
        {
            const auto& held = entries[child];
            main += basis(mainSize(held.node, node.direction),
                          along(held.fit, node.direction));
            cross = std::max(cross,
                             crossBasis(crossSize(held.node, node.direction),
                                        across(held.fit, node.direction)));
            ++count;
        }

        main += node.gap * (float) std::max(count - 1, 0);
        content = node.direction == Direction::Across ? Point {main, cross}
                                                      : Point {cross, main};
    }

    auto fit = Point {content.x + padding.x, content.y + padding.y};
    auto column = node.column;

    if (column != Node::noColumn && column < columnWidths.size()
        && node.width.kind == Size::Kind::Fit)
        fit.x = std::max(fit.x, columnWidths[column]);

    return fit;
}

void Tree::place(NodeId id, Rect area)
{
    auto& entry = entries[id];
    entry.exact = area;
    entry.resolved.rect = snapped(area);
    entry.resolved.overflow =
        entry.node.text.has_value()
        && (entry.fit.x > area.w + tolerance || entry.fit.y > area.h + tolerance);

    if (entry.firstChild >= 0)
        placeChildren(id);
}

// Fixed, Share and Fit children take their sizes first, Grow children share
// what is left; then all are set along the main axis in order, with the gap
// between them and the free room placed by mainAlign, and each aligned across
// by crossAlign. All of it is worked out in exact points from the parent's
// exact rect; only the rects handed back are snapped.
void Tree::placeChildren(NodeId id)
{
    const auto& entry = entries[id];
    const auto& node = entry.node;
    auto direction = node.direction;
    auto inner = entry.exact.inset(node.padding);
    inner.w = std::max(inner.w, 0.f);
    inner.h = std::max(inner.h, 0.f);

    auto mainRoom = direction == Direction::Across ? inner.w : inner.h;
    auto crossRoom = direction == Direction::Across ? inner.h : inner.w;

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
        {
            growSizes.add(size);
            continue;
        }

        held.main = size.kind == Size::Kind::Share
                        ? size.value * mainRoom
                        : basis(size, along(held.fit, direction));
        used += held.main;
    }

    used += node.gap * (float) std::max(count - 1, 0);
    grown.clear();
    grown.resize(growSizes.size(), 0.f);
    shareGrowth(growSizes, grown, mainRoom - used);

    auto growIndex = 0;

    for (auto child = entry.firstChild; child >= 0;
         child = entries[child].nextSibling)
    {
        auto& held = entries[child];

        if (mainSize(held.node, direction).kind == Size::Kind::Grow)
        {
            held.main = grown[growIndex++];
            used += held.main;
        }
    }

    if (used > mainRoom + tolerance)
        entries[id].resolved.overflow = true;

    auto mainStart = direction == Direction::Across ? inner.x : inner.y;
    auto crossStart = direction == Direction::Across ? inner.y : inner.x;
    auto cursor = mainStart + offsetFor(node.mainAlign, mainRoom - used);

    for (auto child = entry.firstChild; child >= 0;
         child = entries[child].nextSibling)
    {
        const auto& held = entries[child];
        const auto& size = crossSize(held.node, direction);
        auto cross = 0.f;

        switch (size.kind)
        {
            case Size::Kind::Fixed:
                cross = size.value;
                break;
            case Size::Kind::Share:
                cross = size.value * crossRoom;
                break;
            case Size::Kind::Fit:
                cross = basis(size, across(held.fit, direction));
                break;
            case Size::Kind::Grow:
                cross = std::min(std::clamp(crossRoom, size.least, size.most),
                                 crossRoom);
                break;
        }

        if (cross > crossRoom + tolerance)
            entries[id].resolved.overflow = true;

        auto main = held.main;
        auto crossAt = crossStart + offsetFor(node.crossAlign, crossRoom - cross);

        place(child,
              direction == Direction::Across ? Rect {cursor, crossAt, main, cross}
                                             : Rect {crossAt, cursor, cross, main});
        cursor += main + node.gap;
    }
}
} // namespace Cows::Flex
