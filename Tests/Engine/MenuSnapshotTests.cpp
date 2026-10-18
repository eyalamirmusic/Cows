#include "Snapshot.h"
#include "UI/Hud.h"
#include "UI/Menu.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;

namespace
{
constexpr auto bigItemScale = 1.3f;

void addItems(Menu& menu)
{
    menu.items.add({"Start", "", bigItemScale});
    menu.items.add(
        {.label = "Dress Your Cow", .note = "Coming Soon!", .enabled = Enabled::No});
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

// The menu in a column on the left of a wide window, the cow's room on the
// right: Start above the disabled choice, a selection ring round Start.
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

    auto cow = menu.cowArea();

    check(start.y + start.h <= dress.y);
    check(!start.intersects(dress));
    check(!start.intersects(cow) && !dress.intersects(cow));
    check(start.y >= title.y + title.h && dress.y >= title.y + title.h);
    check(inside(start, 1280.f, 800.f) && inside(dress, 1280.f, 800.f));
    check(anyDrawnIn(image, start) && anyDrawnIn(image, dress));
    check(!anyDrawnIn(image, title));
    check(menu.itemAt({start.x + start.w * 0.5f, start.y + start.h * 0.5f}) == 0);
};

// A portrait phone puts them below the cow's room: Start large and centred,
// the disabled choice smaller below it, every tap target at least 64 points
// tall.
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

    auto cow = menu.cowArea();

    check(start.area.y + start.area.h <= dress.area.y);
    check(start.area.y >= cow.y + cow.h);
    check(start.pointSize > dress.pointSize);
    check(start.area.h >= 64.f && start.area.w >= 64.f);
    check(inside(start.area, 390.f, 844.f) && inside(dress.area, 390.f, 844.f));
    check(anyDrawnIn(image, start.area) && anyDrawnIn(image, dress.area));
};

// The main menu's four choices never wrap or touch, in one column beside the
// cow's room on a wide window and below it on a narrow one, every one inside
// the window and below the title's room.
namespace
{
void checkMainMenu(float width, float height, const char* name)
{
    auto menu = Menu {};
    menu.items.add({"Start"});
    menu.items.add({"Dress Your Cow"});
    menu.items.add({"Settings"});
    menu.items.add({"About"});
    menu.showSelection = true;

    auto image = menuShot(menu, width, height, name);
    check(image.isValid());
    check(menu.placed.size() == 4);

    if (!image.isValid() || menu.placed.size() != 4)
        return;

    auto title = menu.titleArea();
    check(!menu.cowArea().intersects(title));

    for (auto index = 0; index < 4; ++index)
    {
        const auto& area = menu.placed[index].area;
        check(inside(area, width, height));
        check(area.y >= title.y + title.h);

        if (menu.shape().cow > 0.f)
            check(!area.intersects(menu.cowArea()));

        for (auto other = index + 1; other < 4; ++other)
            check(!area.intersects(menu.placed[other].area));
    }
}
} // namespace

auto tMainMenuSizes = test("MenuSnapshot/mainMenuNeverWrapsOrOverlaps") = []
{
    if (!hasDevice())
        return;

    checkMainMenu(1280.f, 800.f, "engine-menu-main-1280x800");
    checkMainMenu(853.f, 533.f, "engine-menu-main-853x533");
    checkMainMenu(1920.f, 1080.f, "engine-menu-main-1920x1080");
    checkMainMenu(640.f, 600.f, "engine-menu-main-640x600");
    checkMainMenu(500.f, 400.f, "engine-menu-main-500x400");
    checkMainMenu(390.f, 844.f, "engine-menu-main-390x844");
    checkMainMenu(650.f, 800.f, "engine-menu-main-650x800");
    checkMainMenu(800.f, 800.f, "engine-menu-main-800x800");
    checkMainMenu(320.f, 320.f, "engine-menu-main-320x320");
    checkMainMenu(320.f, 700.f, "engine-menu-main-320x700");
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
