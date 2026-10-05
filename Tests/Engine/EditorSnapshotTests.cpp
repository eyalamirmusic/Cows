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

bool overlaps(const Graphics::Rect& a, const Graphics::Rect& b)
{
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

Graphics::Point middleOf(const Graphics::Rect& rect)
{
    return {rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f};
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

void checkLayout(const Editor& editor, float width, float height)
{
    auto scene = editor.sceneArea();

    check(editor.placed.size() == editor.rows.size());

    for (auto index = 0; index < (int) editor.placed.size(); ++index)
    {
        const auto& place = editor.placed[index];

        for (const auto& area: {place.previous, place.value, place.next})
        {
            check(inside(area, width, height));
            check(!overlaps(area, scene));
            check(area.h >= 64.f && area.w >= 64.f);
        }

        check(!overlaps(place.previous, place.value));
        check(!overlaps(place.value, place.next));

        if (index > 0)
            check(editor.placed[index - 1].row.y < place.row.y);
    }

    check(inside(editor.done.value, width, height));
    check(editor.placed.back().value.y + editor.placed.back().value.h
          <= editor.done.value.y);
}
} // namespace

// Beside the cow on a wide window: every row and Done right of the scene's
// share, inside the window, each target a comfortable size.
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

// Below her on a portrait phone.
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
    editor.onStep = [&](int row, int by) { steps.add(row * 10 + by); };
    editor.onDone = [&] { ++finished; };

    check(editor.targetAt(middleOf(editor.placed[0].previous))
          == Editor::Target {0, Part::Previous});
    check(editor.targetAt(middleOf(editor.placed[1].next))
          == Editor::Target {1, Part::Next});
    check(editor.targetAt(middleOf(editor.done.value))
          == Editor::Target {2, Part::Done});
    check(editor.targetAt(middleOf(editor.sceneArea())).part == Part::None);

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
