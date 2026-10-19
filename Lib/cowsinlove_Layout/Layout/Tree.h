#pragma once

#include <eacp/Graphics/Primitives/Primitives.h>

#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace Cows::Flex
{
// Width in points; supplied by the caller so this library never sees a font.
using Measure = std::function<float(std::string_view text, float pointSize)>;

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

// Across the parent's direction, Grow fills the parent.
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

struct Text final
{
    std::string text;
    float pointSize = 0.f;
    float lineHeight = 1.f; // times pointSize
    eacp::Vector<std::string> alsoFits = {};
};

// Nodes sharing a column all take the widest one's fit width, so cells line up.
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

struct Resolved final
{
    eacp::Graphics::Rect rect;
    bool overflow = false;
};

using NodeId = int;

struct Tree final
{
    static constexpr NodeId root = 0;

    explicit Tree(Node rootNode = {});

    NodeId add(NodeId parent, Node node);

    void resolve(eacp::Graphics::Rect available, const Measure& measure);

    const Node& node(NodeId id) const;
    eacp::Graphics::Rect rect(NodeId id) const;
    bool overflow(NodeId id) const;
    bool overflows() const;
    eacp::Graphics::Point fitSize() const;

private:
    struct Entry final
    {
        Node node;
        NodeId parent = -1;
        NodeId firstChild = -1;
        NodeId lastChild = -1;
        NodeId nextSibling = -1;
        eacp::Graphics::Point fit;
        float lengthAlongParent = 0.f;
        eacp::Graphics::Rect unsnappedRect;
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
