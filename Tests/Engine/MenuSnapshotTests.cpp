#include "Snapshot.h"
#include "UI/Hud.h"
#include "UI/Menu.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;

namespace
{
void addItems(Menu& menu)
{
    menu.items.add({"Start", "", true});
    menu.items.add({"Dress Your Cow", "Coming Soon!", false});
}

bool anyDrawnIn(const Graphics::Image& image, const Graphics::Rect& area)
{
    for (auto y = (int) area.y; y < (int) (area.y + area.h); ++y)
        for (auto x = (int) area.x; x < (int) (area.x + area.w); ++x)
            if (x >= 0 && y >= 0 && x < image.width() && y < image.height()
                && !isClearColor(image.at(x, y)))
                return true;

    return false;
}

bool overlaps(const Graphics::Rect& a, const Graphics::Rect& b)
{
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

bool inside(const Graphics::Rect& area, float width, float height)
{
    return area.x >= 0.f && area.y >= 0.f && area.x + area.w <= width
           && area.y + area.h <= height;
}

Graphics::Image menuShot(Menu& menu, float width, float height, const char* name)
{
    menu.setBounds({0.f, 0.f, width, height});

    auto hud = Hud {};
    auto view = SnapshotView {};
    view.drawOverlay = [&](Frame& frame, RenderPass& pass)
    {
        hud.begin(frame, pass, view.sampleCount());
        menu.draw(hud);
        hud.end();
    };

    return snapshot(view, width, height, name);
}
} // namespace

// The menu side by side on a wide window: Start on the left, the disabled
// choice on the right, both below the title's room and apart, with the
// selection ring round Start.
auto tMenuWide = test("MenuSnapshot/wide") = []
{
    if (!hasDevice())
        return;

    auto menu = Menu {};
    addItems(menu);
    menu.showSelection = true;

    auto image = menuShot(menu, 1280.f, 800.f, "engine-menu-wide");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(menu.wide());
    check(menu.placed.size() == 2);

    const auto& start = menu.placed[0].area;
    const auto& dress = menu.placed[1].area;
    auto title = menu.titleArea();

    check(start.x + start.w < dress.x);
    check(!overlaps(start, dress));
    check(start.y >= title.y + title.h && dress.y >= title.y + title.h);
    check(inside(start, 1280.f, 800.f) && inside(dress, 1280.f, 800.f));
    check(anyDrawnIn(image, start) && anyDrawnIn(image, dress));
    check(!anyDrawnIn(image, title));
    check(menu.itemAt({start.x + start.w * 0.5f, start.y + start.h * 0.5f}) == 0);
};

// A portrait phone stacks them: Start large and centred, the disabled choice
// smaller below it, every tap target at least 64 points tall.
auto tMenuTall = test("MenuSnapshot/tall") = []
{
    if (!hasDevice())
        return;

    auto menu = Menu {};
    addItems(menu);

    auto image = menuShot(menu, 390.f, 844.f, "engine-menu-tall");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!menu.wide());
    check(menu.placed.size() == 2);

    const auto& start = menu.placed[0];
    const auto& dress = menu.placed[1];

    check(start.area.y + start.area.h <= dress.area.y);
    check(start.pointSize > dress.pointSize);
    check(start.area.h >= 64.f && start.area.w >= 64.f);
    check(inside(start.area, 390.f, 844.f) && inside(dress.area, 390.f, 844.f));
    check(anyDrawnIn(image, start.area) && anyDrawnIn(image, dress.area));
};

// Only an enabled choice is ever chosen.
auto tMenuDisabled = test("Menu/aDisabledChoiceNeverFires") = []
{
    auto menu = Menu {};
    addItems(menu);
    auto chosen = -1;
    menu.onChoose = [&](int index) { chosen = index; };

    menu.selectNext();
    menu.chooseSelected();
    check(chosen == -1);
    check(menu.selected == 1);

    menu.selectNext();
    check(menu.selected == 1);

    menu.selectPrevious();
    menu.chooseSelected();
    check(chosen == 0);
};
