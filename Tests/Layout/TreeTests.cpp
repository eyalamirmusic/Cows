#include "Layout/Tree.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows::Flex;
using eacp::Vector;
using eacp::Graphics::Rect;

namespace
{
// Every character half the point size wide, as a stand-in for a font.
constexpr auto characterShare = 0.5f;
constexpr auto closeEnough = 0.001f;

float measureByCharacters(std::string_view text, float pointSize)
{
    return (float) text.size() * pointSize * characterShare;
}

bool near(float a, float b)
{
    return std::abs(a - b) < closeEnough;
}

bool same(const Rect& a, const Rect& b)
{
    return near(a.x, b.x) && near(a.y, b.y) && near(a.w, b.w) && near(a.h, b.h);
}

Node fixedBox(float width, float height)
{
    auto node = Node {};
    node.width = Size::fixed(width);
    node.height = Size::fixed(height);
    return node;
}

Node textLeaf(std::string text, float pointSize)
{
    auto node = Node {};
    node.text = Text {std::move(text), pointSize};
    return node;
}
} // namespace

auto tFixed = test("Tree/fixedChildrenSitInOrder") = []
{
    auto tree = Tree {};
    auto first = tree.add(Tree::root, fixedBox(30.f, 10.f));
    auto second = tree.add(Tree::root, fixedBox(20.f, 40.f));
    tree.resolve({0.f, 0.f, 100.f, 50.f}, measureByCharacters);

    check(same(tree.rect(Tree::root), {0.f, 0.f, 100.f, 50.f}));
    check(same(tree.rect(first), {0.f, 0.f, 30.f, 10.f}));
    check(same(tree.rect(second), {30.f, 0.f, 20.f, 40.f}));
    check(!tree.overflows());
};

auto tFitText = test("Tree/fitWrapsTextAndChildren") = []
{
    constexpr auto pointSize = 20.f;
    constexpr auto lineHeight = 1.5f;

    auto column = Node {};
    column.direction = Direction::Down;
    column.width = Size::fit();
    column.height = Size::fit();
    column.padding = {.top = 2.f, .left = 3.f, .bottom = 4.f, .right = 5.f};

    auto tree = Tree {};
    auto holder = tree.add(Tree::root, column);
    auto leaf = Node {};
    leaf.text = Text {"abcd", pointSize, lineHeight};
    auto label = tree.add(holder, leaf);
    tree.resolve({0.f, 0.f, 200.f, 200.f}, measureByCharacters);

    check(same(tree.rect(label), {3.f, 2.f, 40.f, 30.f}));
    check(same(tree.rect(holder), {0.f, 0.f, 48.f, 36.f}));
};

auto tFitLeast = test("Tree/fitNeverGoesBelowItsLeast") = []
{
    constexpr auto least = 64.f;

    auto leaf = textLeaf("a", 10.f);
    leaf.width = Size::fit(least);
    leaf.height = Size::fit(least);

    auto tree = Tree {};
    auto id = tree.add(Tree::root, leaf);
    tree.resolve({0.f, 0.f, 200.f, 200.f}, measureByCharacters);

    check(near(tree.rect(id).w, least) && near(tree.rect(id).h, least));
};

auto tAlsoFits = test("Tree/aSlotFitsItsWidestAlternative") = []
{
    auto leaf = textLeaf("ab", 10.f);
    leaf.text->alsoFits = {"abcdef", "a"};

    auto tree = Tree {};
    auto id = tree.add(Tree::root, leaf);
    tree.resolve({0.f, 0.f, 200.f, 200.f}, measureByCharacters);

    check(near(tree.rect(id).w, 30.f));
};

auto tAlignment = test("Tree/alignsAlongAndAcross") = []
{
    auto check3 = [](Align main, Align cross, Rect expected)
    {
        auto root = Node {};
        root.mainAlign = main;
        root.crossAlign = cross;

        auto tree = Tree {root};
        auto id = tree.add(Tree::root, fixedBox(20.f, 10.f));
        tree.resolve({0.f, 0.f, 100.f, 50.f}, measureByCharacters);
        check(same(tree.rect(id), expected));
    };

    check3(Align::Start, Align::Start, {0.f, 0.f, 20.f, 10.f});
    check3(Align::Centre, Align::Centre, {40.f, 20.f, 20.f, 10.f});
    check3(Align::End, Align::End, {80.f, 40.f, 20.f, 10.f});
};

auto tGapPadding = test("Tree/gapAndPaddingSpaceChildren") = []
{
    auto root = Node {};
    root.direction = Direction::Down;
    root.gap = 5.f;
    root.padding = {.top = 10.f, .left = 7.f};

    auto tree = Tree {root};
    auto first = tree.add(Tree::root, fixedBox(10.f, 10.f));
    auto second = tree.add(Tree::root, fixedBox(10.f, 10.f));
    tree.resolve({0.f, 0.f, 100.f, 100.f}, measureByCharacters);

    check(same(tree.rect(first), {7.f, 10.f, 10.f, 10.f}));
    check(same(tree.rect(second), {7.f, 25.f, 10.f, 10.f}));
};

auto tShare = test("Tree/shareTakesItsFractionAndGrowTheRest") = []
{
    auto band = Node {};
    band.width = Size::share(0.25f);
    band.height = Size::grow();
    auto rest = Node {};
    rest.width = Size::grow();
    rest.height = Size::share(0.5f);

    auto tree = Tree {};
    auto left = tree.add(Tree::root, band);
    auto right = tree.add(Tree::root, rest);
    tree.resolve({0.f, 0.f, 200.f, 100.f}, measureByCharacters);

    check(same(tree.rect(left), {0.f, 0.f, 50.f, 100.f}));
    check(same(tree.rect(right), {50.f, 0.f, 150.f, 50.f}));
};

auto tGrowClamp = test("Tree/growSharesWithinItsLeastAndMost") = []
{
    auto capped = Node {};
    capped.width = Size::grow(0.f, 20.f);
    auto open = Node {};
    open.width = Size::grow();
    auto floored = Node {};
    floored.width = Size::grow(60.f);

    auto tree = Tree {};
    auto a = tree.add(Tree::root, capped);
    auto b = tree.add(Tree::root, open);
    auto c = tree.add(Tree::root, floored);
    tree.resolve({0.f, 0.f, 120.f, 10.f}, measureByCharacters);

    check(near(tree.rect(a).w, 20.f));
    check(near(tree.rect(b).w, 40.f));
    check(near(tree.rect(c).w, 60.f));
    check(!tree.overflows());
};

auto tGrowShrinks = test("Tree/growShrinksToNothingRatherThanOverflow") = []
{
    auto open = Node {};
    open.width = Size::grow(40.f);

    auto tree = Tree {};
    auto fixed = tree.add(Tree::root, fixedBox(90.f, 10.f));
    auto grown = tree.add(Tree::root, open);
    tree.resolve({0.f, 0.f, 100.f, 10.f}, measureByCharacters);

    check(near(tree.rect(fixed).w, 90.f));
    check(near(tree.rect(grown).w, 10.f));
    check(!tree.overflow(Tree::root));

    auto tight = Tree {};
    tight.add(Tree::root, fixedBox(100.f, 10.f));
    auto squeezed = tight.add(Tree::root, open);
    tight.resolve({0.f, 0.f, 100.f, 10.f}, measureByCharacters);

    check(near(tight.rect(squeezed).w, 0.f));
    check(!tight.overflows());
};

auto tOverflow = test("Tree/overflowIsReportedWhereItHappens") = []
{
    auto tree = Tree {};
    auto first = tree.add(Tree::root, fixedBox(80.f, 10.f));
    auto second = tree.add(Tree::root, fixedBox(80.f, 10.f));
    tree.resolve({0.f, 0.f, 100.f, 10.f}, measureByCharacters);

    check(tree.overflow(Tree::root));
    check(!tree.overflow(first) && !tree.overflow(second));
    check(tree.overflows());
    check(near(tree.rect(second).x, 80.f));

    auto text = Tree {};
    auto leaf = textLeaf("abcdefghij", 20.f);
    leaf.width = Size::fixed(50.f);
    auto label = text.add(Tree::root, leaf);
    text.resolve({0.f, 0.f, 200.f, 200.f}, measureByCharacters);

    check(text.overflow(label));
};

auto tColumns = test("Tree/sharedColumnsTakeTheWidestWidth") = []
{
    constexpr auto nameColumn = 0;
    constexpr auto valueColumn = 1;

    auto table = Node {};
    table.direction = Direction::Down;
    auto tree = Tree {table};
    auto names = Vector<NodeId> {};
    auto values = Vector<NodeId> {};

    for (const auto& [name, value]:
         {std::pair {"ab", "abcdef"}, std::pair {"abcd", "a"}})
    {
        auto row = tree.add(Tree::root, Node {});
        auto nameCell = textLeaf(name, 10.f);
        nameCell.column = nameColumn;
        auto valueCell = textLeaf(value, 10.f);
        valueCell.column = valueColumn;
        names.add(tree.add(row, nameCell));
        values.add(tree.add(row, valueCell));
    }

    tree.resolve({0.f, 0.f, 300.f, 100.f}, measureByCharacters);

    for (auto index = 0; index < 2; ++index)
    {
        check(near(tree.rect(names[index]).w, 20.f));
        check(near(tree.rect(values[index]).w, 30.f));
        check(near(tree.rect(values[index]).x, 20.f));
    }
};

auto tSnapping = test("Tree/edgesSnapToWholePointsAndNeverOverlap") = []
{
    auto tree = Tree {};
    auto ids = Vector<NodeId> {};

    for (auto index = 0; index < 3; ++index)
    {
        auto third = Node {};
        third.width = Size::grow();
        third.height = Size::grow();
        ids.add(tree.add(Tree::root, third));
    }

    tree.resolve({0.3f, 0.6f, 100.f, 10.f}, measureByCharacters);

    for (auto index = 0; index < 3; ++index)
    {
        auto area = tree.rect(ids[index]);
        check(area.x == std::round(area.x) && area.w == std::round(area.w));
        check(area.y == std::round(area.y) && area.h == std::round(area.h));
    }

    for (auto index = 1; index < 3; ++index)
        check(tree.rect(ids[index - 1]).right() == tree.rect(ids[index]).x);

    check(tree.rect(ids[0]).x == 0.f);
    check(tree.rect(ids[2]).right() == 100.f);
};
