#include "Snapshot.h"
#include "UI/Editor.h"
#include "UI/Hud.h"
#include "UI/Menu.h"

#include <NanoTest/NanoTest.h>

#include <algorithm>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;

namespace
{
constexpr auto smallestSide = 320.f;
constexpr auto widest = 1920.f;
constexpr auto tallest = 1080.f;
constexpr auto walkStep = 40.f;
constexpr auto titleBar = 28.f;
// The most a screen's text may grow or shrink between two windows a step
// apart: 1 / 0.8 when a shape gives way to one with text a quarter larger
// (largestShape's preference), times what a 40 point step adds to it (up to
// a tenth, 1.37 at worst for the editor from 1120x680 to 1120x720).
constexpr auto maxStepRatio = 1.4f;
constexpr auto startScale = 1.3f;
constexpr auto aboutScale = 0.75f;

struct Sizes final
{
    float menu = 0.f;
    float editor = 0.f;
};

Graphics::Rect contentOf(const Graphics::View& view)
{
    return frameOf(view, titleBar).contentArea();
}

bool smooth(float a, float b)
{
    return std::max(a, b) <= std::min(a, b) * maxStepRatio;
}
} // namespace

// From the smallest window to a large desktop one, a step at a time: the
// menu's text (About, its smallest item, included) and the editor's never go
// under the readable size, and never jump between neighbouring windows,
// whichever shape each takes.
auto tWalk = test("ScreenWalk/textStaysReadableAndChangesSmoothly") = []
{
    if (!hasDevice())
        return;

    auto hud = Hud {};
    auto menu = Menu {};
    auto editor = Editor {};
    menu.topClearance = titleBar;
    editor.topClearance = titleBar;
    menu.items.add({"Start", "", startScale});
    menu.items.add({"Dress Your Cow"});
    menu.items.add({"Settings"});
    menu.items.add({"About", "", aboutScale});
    editor.rows.add({"Hat",
                     {"None",
                      "Top Hat",
                      "Cowboy Hat",
                      "Party Hat",
                      "Beanie",
                      "Crown",
                      "Frog Hat",
                      "Cow Bucket Hat",
                      "Traffic Cone",
                      "Wizard Hat"},
                     0});
    editor.rows.add({"Pants", {"None", "Both Legs", "Back Legs"}, 0});

    auto columns = (int) ((widest - smallestSide) / walkStep) + 1;
    auto previousRow = Vector<Sizes> {};

    for (auto height = smallestSide; height <= tallest; height += walkStep)
    {
        auto row = Vector<Sizes> {};

        for (auto column = 0; column < columns; ++column)
        {
            auto width = smallestSide + walkStep * (float) column;
            menu.setBounds({0.f, 0.f, width, height});
            editor.setBounds({0.f, 0.f, width, height});
            menu.layOut(hud);
            editor.layOut(hud);

            auto sizes = Sizes {menu.pointSize, editor.placed.front().pointSize};
            auto readable = smallestReadableSize(contentOf(menu));

            check(sizes.menu * aboutScale >= readable);
            check(sizes.editor >= readable);

            if (column > 0)
            {
                check(smooth(sizes.menu, row.back().menu));
                check(smooth(sizes.editor, row.back().editor));
            }

            if (!previousRow.empty())
            {
                check(smooth(sizes.menu, previousRow[column].menu));
                check(smooth(sizes.editor, previousRow[column].editor));
            }

            row.add(sizes);
        }

        previousRow = row;
    }
};
