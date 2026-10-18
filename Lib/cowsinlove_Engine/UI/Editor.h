#pragma once

#include "Render/Common.h"
#include "UI/ControlEvent.h"
#include "UI/Hud.h"
#include "UI/MenuStyle.h"

#include <functional>
#include <optional>
#include <string>

namespace Cows
{
struct EditorRow final
{
    std::string name;
    Vector<std::string> choices;
    int choice = 0;
};

// Rows in the start menu's style over the live scene, for choosing one of
// several things per row: "Hat  <  Top Hat  >", then Done. They sit to the
// right on a wide screen and at the bottom of a tall one, as a table: the
// names in one column, the values in another as wide as the widest choice of
// any row, at the largest size that fits and never under the readable size:
// when a row can't hold it, the cow's room gives way, then each name goes
// over its value. Laid out through Flex trees, again only when the window, its
// safe area, the title or the rows (setRows) change. The arrows step a
// row back and on, the value steps it on; the mouse hovers and clicks, a
// finger taps, and keys or a controller move a selection ring. A drag or a
// wheel anywhere else is passed on through onControl as Look and Zoom, so the
// camera can go round the scene underneath.
struct Editor final : Graphics::View
{
    enum class Part
    {
        None,
        Previous,
        Value,
        Next,
        Done
    };

    struct Target final
    {
        bool operator==(const Target& other) const = default;

        int row = -1;
        Part part = Part::None;
    };

    Editor();

    void draw(Hud& hud);

    // Where the cow is framed, in points: left of the panel when wide, above
    // it when tall.
    Graphics::Rect cowArea() const;
    // Where the heading, the rows and Done sit.
    Graphics::Rect panelArea() const;
    // Whether the panel sits beside the cow rather than below her.
    bool wide() const;

    // Replace the rows, or the heading, and lay them out again on the next
    // draw. A row's choice alone changes nothing laid out.
    void setRows(Vector<EditorRow> newRows);
    void setTitle(std::string newTitle);

    void selectNext();
    void selectPrevious();
    void stepSelected(int by);
    void chooseSelected();

    Target targetAt(Graphics::Point position) const;

    // Hidden, it takes no mouse or touch, so they reach the views under it.
    // eacp finds a finger's view without calling hitTest, so this clears the
    // handles flags rather than overriding hitTest.
    void setShowing(bool shouldShow);
    bool isShowing() const;

    void mouseMoved(const Graphics::MouseEvent& event) override;
    void mouseExited(const Graphics::MouseEvent& event) override;
    void mouseDown(const Graphics::MouseEvent& event) override;
    void mouseUp(const Graphics::MouseEvent& event) override;
    void mouseDragged(const Graphics::MouseEvent& event) override;
    void mouseWheel(const Graphics::MouseEvent& event) override;
    void touchBegan(const Graphics::TouchEvent& event) override;
    void touchMoved(const Graphics::TouchEvent& event) override;
    void touchEnded(const Graphics::TouchEvent& event) override;

    struct Placed final
    {
        float pointSize = 0.f;
        float baseline = 0.f;
        float nameBaseline = 0.f;
        Graphics::Rect name;
        Graphics::Rect previous;
        Graphics::Rect value;
        Graphics::Rect next;
        Graphics::Rect row;
    };

    // The cow's share of the content: of its width when wide, of its height
    // when tall.
    struct Shape final
    {
        bool wide = true;
        float cow = 0.f;
    };

    Shape shape() const;
    void layOut(Hud& hud);
    void drawRow(Hud& hud, int index);
    void drawDone(Hud& hud);
    void drawLabel(Hud& hud,
                   const std::string& text,
                   const Graphics::Rect& area,
                   float baseline,
                   float pointSize,
                   bool lit,
                   bool centred);
    void drawRing(Hud& hud, const Graphics::Rect& area);
    bool isLit(Target target) const;
    int doneIndex() const;
    void act(Target target);
    void returnKeyFocus();

    std::function<void(int row, int by)> onStep = [](int, int) {};
    std::function<void()> onDone = [] {};
    std::function<void(const ControlEvent&)> onControl = [](const ControlEvent&) {};

    std::string title;
    std::string doneLabel = "Done";
    Vector<EditorRow> rows;
    Vector<Placed> placed;
    Placed done;
    Graphics::Rect titleBox;
    float titleSize = 0.f;
    // Each name over its arrows and value, rather than beside them.
    bool stacked = false;

    // Points at the top kept clear of anything clickable, for a title bar the
    // safe area does not count (macTitleBarClearance on macOS).
    float topClearance = 0.f;
    float opacity = 1.f;
    bool showSelection = false;
    int selected = 0;
    Target hovered;
    Target pressed;
    int lookingTouch = -1;
    Graphics::Point lookedFrom;

private:
    bool showing = false;
    bool contentChanged = true;
    // The shape the last layOut chose, for the window it was laid out for;
    // until then, or in another window, the window's aspect decides.
    Shape chosen;
    std::optional<ScreenFrame> laidOutFor;
};
} // namespace Cows
