#include "Snapshot.h"
#include "UI/Editor.h"
#include "UI/Hud.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;

namespace
{
using Part = Editor::Part;

void addRows(Editor& editor)
{
    editor.rows.add({"Hat", {"None", "Top Hat", "Cowboy Hat", "Frog Hat"}, 2});
    editor.rows.add({"Glasses", {"None", "Shades"}, 0});
}

bool inside(const Graphics::Rect& area, float width, float height)
{
    return area.x >= 0.f && area.y >= 0.f && area.x + area.w <= width
           && area.y + area.h <= height;
}

Graphics::Image
    editorShot(Editor& editor, float width, float height, const char* name)
{
    editor.setBounds({0.f, 0.f, width, height});

    auto hud = Hud {};
    auto view = SnapshotView {};
    view.drawOverlay = [&](Frame& frame, RenderPass& pass)
    {
        hud.begin(frame, pass, view.sampleCount());
        editor.draw(hud);
        hud.end();
    };

    return snapshot(view, width, height, name);
}

bool within(const Graphics::Rect& inner, const Graphics::Rect& outer)
{
    constexpr auto slack = 0.5f;
    return inner.x >= outer.x - slack && inner.y >= outer.y - slack
           && inner.x + inner.w <= outer.x + outer.w + slack
           && inner.y + inner.h <= outer.y + outer.h + slack;
}

void checkPanel(const Editor& editor)
{
    auto content = frameOf(editor, editor.topClearance).contentArea();
    auto panel = editor.panelArea();
    auto readable = smallestReadableSize(content);

    check(!panel.intersects(editor.cowArea()));
    check(within(panel, content));
    check(within(editor.titleBox, panel));
    check(within(editor.done.value, panel));
    check(editor.titleSize >= readable);

    const auto& first = editor.placed.front();

    for (auto index = 0; index < (int) editor.placed.size(); ++index)
    {
        const auto& place = editor.placed[index];
        check(place.pointSize >= readable);
        check(within(place.row, panel));
        check(editor.titleBox.y + editor.titleBox.h <= place.row.y + 0.5f);
        check(place.row.y + place.row.h <= editor.done.value.y + 0.5f);
        check(place.name.x == first.name.x);
        check(place.value.x == first.value.x && place.value.w == first.value.w);

        if (index > 0)
        {
            const auto& above = editor.placed[index - 1].row;
            check(above.y + above.h <= place.row.y + 0.5f);
        }
    }
}

void checkLayout(const Editor& editor, float width, float height)
{
    auto scene = editor.cowArea();
    checkPanel(editor);

    check(editor.placed.size() == editor.rows.size());

    for (auto index = 0; index < (int) editor.placed.size(); ++index)
    {
        const auto& place = editor.placed[index];

        for (const auto& area: {place.previous, place.value, place.next})
        {
            check(inside(area, width, height));
            check(!area.intersects(scene));
            check(area.h >= 64.f && area.w >= 64.f);
        }

        check(!place.previous.intersects(place.value));
        check(!place.value.intersects(place.next));

        if (index > 0)
            check(editor.placed[index - 1].row.y < place.row.y);
    }

    check(inside(editor.done.value, width, height));
    check(editor.placed.back().value.y + editor.placed.back().value.h
          <= editor.done.value.y);
}
} // namespace

auto tEditorWide = test("EditorSnapshot/wide") = []
{
    if (!hasDevice())
        return;

    auto editor = Editor {};
    addRows(editor);
    editor.topClearance = 28.f;
    editor.showSelection = true;

    auto image = editorShot(editor, 1280.f, 800.f, "engine-editor-wide");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(editor.wide());
    checkLayout(editor, 1280.f, 800.f);
};

auto tEditorTall = test("EditorSnapshot/tall") = []
{
    if (!hasDevice())
        return;

    auto editor = Editor {};
    addRows(editor);

    auto image = editorShot(editor, 390.f, 844.f, "engine-editor-tall");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!editor.wide());
    checkLayout(editor, 390.f, 844.f);
};

auto tEditorTargets = test("Editor/clicksStepAndFinish") = []
{
    if (!hasDevice())
        return;

    auto editor = Editor {};
    addRows(editor);
    editorShot(editor, 1280.f, 800.f, "engine-editor-targets");

    auto steps = Vector<int> {};
    auto finished = 0;
    editor.onRowValueStepped = [&](int row, int by) { steps.add(row * 10 + by); };
    editor.onDone = [&] { ++finished; };

    check(editor.targetAt(editor.placed[0].previous.center())
          == Editor::Target {0, Part::Previous});
    check(editor.targetAt(editor.placed[1].next.center())
          == Editor::Target {1, Part::Next});
    check(editor.targetAt(editor.done.value.center())
          == Editor::Target {2, Part::Done});
    check(editor.targetAt(editor.cowArea().center()).part == Part::None);

    editor.stepSelected(-1);
    editor.selectNext();
    editor.chooseSelected();
    editor.selectNext();
    editor.selectNext();
    check(editor.selected == 2);
    editor.stepSelected(1);
    editor.chooseSelected();

    check(steps.size() == 2);
    check(steps[0] == -1);
    check(steps[1] == 11);
    check(finished == 1);
};

namespace
{
void checkEveryShape(const std::string& title, const Vector<EditorRow>& rows)
{
    struct Shape final
    {
        float width = 0.f;
        float height = 0.f;
        Graphics::Insets safe;
        float topClearance = 0.f;
    };

    constexpr auto titleBar = 28.f;
    auto shapes = Vector<Shape> {
        {430.f, 932.f, {.top = 59.f, .bottom = 34.f}},
        {932.f, 430.f, {.left = 59.f, .bottom = 21.f, .right = 59.f}},
        {360.f, 800.f, {.top = 24.f, .bottom = 24.f}},
        {820.f, 1180.f, {.top = 24.f, .bottom = 20.f}},
        {1180.f, 820.f, {.top = 24.f, .bottom = 20.f}},
        {1280.f, 800.f, {}, titleBar},
        {1920.f, 1080.f, {}, titleBar},
        {1720.f, 720.f, {}, titleBar},
        {650.f, 800.f, {}, titleBar},
        {800.f, 800.f, {}, titleBar},
        {500.f, 500.f, {}, titleBar},
        {320.f, 320.f, {}, titleBar},
    };

    auto hud = Hud {};

    for (const auto& shape: shapes)
    {
        auto root = Graphics::View {};
        auto editor = Editor {};
        editor.setTitle(title);
        editor.setRows(rows);
        editor.topClearance = shape.topClearance;
        root.addSubview(editor);
        root.setSafeAreaInsets(shape.safe);
        root.setBounds({0.f, 0.f, shape.width, shape.height});
        editor.setBounds({0.f, 0.f, shape.width, shape.height});
        editor.layOut(hud);

        check(editor.placed.size() == rows.size());

        if (editor.placed.size() == rows.size())
            checkPanel(editor);
    }
}

Vector<EditorRow> makeExampleSettingsRows()
{
    auto rows = Vector<EditorRow> {};
    rows.add({"Quality", {"Auto: High", "Low", "Medium", "High"}});
    rows.add({"Frame rate", {"Auto", "60", "Max"}});
    return rows;
}
} // namespace

auto tEditorShapes = test("EditorLayout/holdsAtEveryShape") = []
{
    if (!hasDevice())
        return;

    auto rows = Vector<EditorRow> {};
    rows.add({"Hat", {"None", "Top Hat", "Cow Bucket Hat", "Wizard Hat"}});
    rows.add({"Pants", {"None", "Both Legs", "Back Legs"}});
    checkEveryShape("Dress Your Cow", rows);
};

auto tSettingsShapes = test("EditorLayout/settingsHoldAtEveryShape") = []
{
    if (!hasDevice())
        return;

    checkEveryShape("Settings", makeExampleSettingsRows());
};

auto tSettingsShot = test("EditorSnapshot/settings") = []
{
    if (!hasDevice())
        return;

    auto editor = Editor {};
    editor.setTitle("Settings");
    editor.setRows(makeExampleSettingsRows());
    editor.topClearance = macTitleBarClearance;

    auto image = editorShot(editor, 1280.f, 800.f, "engine-editor-settings");
    check(image.isValid());

    if (!image.isValid())
        return;

    checkLayout(editor, 1280.f, 800.f);
};
