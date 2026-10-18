#pragma once

#include <eacp/Graphics/Primitives/Primitives.h>

#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace Cows::Flex
{
// The width of `text` set at `pointSize`, in points. The caller supplies it,
// so the library never sees a font or a GPU.
using Measure = std::function<float(std::string_view text, float pointSize)>;

// Which way a node lays its children out: side by side, or top to bottom.
enum class Direction
{
    Across,
    Down
};

enum class Align
{
    Start,
    Centre,
    End
};

constexpr auto unbounded = std::numeric_limits<float>::max();

// How a node is sized along one axis. Fit wraps its children or text (never
// less than `least`); Grow takes a share of what its siblings leave, between
// `least` and `most`, shrinking to nothing when there is no room; Fixed is
// `points`; Share is `fraction` of the parent's inner size. Across the parent's
// direction, Grow fills the parent (between `least` and `most`).
struct Size final
{
    enum class Kind
    {
        Fit,
        Grow,
        Fixed,
        Share
    };

    static Size fit(float least = 0.f);
    static Size grow(float least = 0.f, float most = unbounded);
    static Size fixed(float points);
    static Size share(float fraction);

    Kind kind = Kind::Fit;
    float least = 0.f;
    float most = unbounded;
    float value = 0.f;
};

// A line of text a leaf is sized for: its width through Measure, its height
// `lineHeight` times the point size. `alsoFits` are other strings the slot must
// hold too, such as every choice a value can step through.
struct Text final
{
    std::string text;
    float pointSize = 0.f;
    float lineHeight = 1.f;
    eacp::Vector<std::string> alsoFits = {};
};

// One box: how it sizes, its padding and the gap between its children, and how
// it aligns them along its direction (main) and across it (cross). A node with
// `text` is a leaf sized to it. Nodes sharing a `column` other than
// `noColumn` take one width, the widest of their fit widths, so cells line up
// across sibling rows.
struct Node final
{
    static constexpr auto noColumn = -1;

    Direction direction = Direction::Across;
    Size width;
    Size height;
    eacp::Graphics::Insets padding;
    float gap = 0.f;
    Align mainAlign = Align::Start;
    Align crossAlign = Align::Start;
    std::optional<Text> text;
    int column = noColumn;
};

// What a node came to: its rect in points, snapped to whole points so shared
// edges meet exactly, and whether what it holds (children and gaps, or text)
// is bigger than the room it got.
struct Resolved final
{
    eacp::Graphics::Rect rect;
    bool overflow = false;
};

using NodeId = int;

// A tree of nodes built for one layout pass and resolved to rects. add returns
// a node's id, which reads its rect back after resolve. Nodes live in one
// Vector, linked by index, so a pass allocates once as it grows.
struct Tree final
{
    static constexpr NodeId root = 0;

    explicit Tree(Node rootNode = {});

    NodeId add(NodeId parent, Node node);

    // Measures fit sizes bottom up, shares each parent's room out among its
    // children, and positions them, the root filling `available`.
    void resolve(eacp::Graphics::Rect available, const Measure& measure);

    const Node& node(NodeId id) const;
    eacp::Graphics::Rect rect(NodeId id) const;
    bool overflow(NodeId id) const;
    // Whether any node overflowed.
    bool overflows() const;
    // The root's fit size: what the tree needs, padding included.
    eacp::Graphics::Point fitSize() const;
    int size() const;

private:
    struct Entry final
    {
        Node node;
        NodeId parent = -1;
        NodeId firstChild = -1;
        NodeId lastChild = -1;
        NodeId nextSibling = -1;
        eacp::Graphics::Point fit;
        // This node's length along its parent's direction, set as the parent
        // places it.
        float main = 0.f;
        // Its rect before snapping, which its children are laid out in.
        eacp::Graphics::Rect exact;
        Resolved resolved;
    };

    void measureAll(const Measure& measure);
    eacp::Graphics::Point fitOf(NodeId id, const Measure& measure) const;
    void shareColumns();
    void place(NodeId id, eacp::Graphics::Rect area);
    void placeChildren(NodeId id);

    eacp::Vector<Entry> entries;
    eacp::Vector<float> columnWidths;
    eacp::Vector<Size> growSizes;
    eacp::Vector<float> grown;
};
} // namespace Cows::Flex
