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
    bool operator==(const EditorRow& other) const = default;

    std::string name;
    Vector<std::string> choices;
    int choice = 0;
};

// Rows in the start menu's style over the live scene, for choosing one of
// several things per row: "Hat  <  Top Hat  >", then Done. They sit to the
// right on a wide screen and at the bottom of a tall one, as a table: the
// names in one column, the values in another as wide as the widest choice of
// any row, at the largest size that fits. Laid out through Flex trees, again
// only when the window, its safe area or the rows change. The arrows step a
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

    // The part of the screen the rows leave for the scene, in points.
    Graphics::Rect sceneArea() const;
    // Whether the rows sit beside the scene rather than below it: whichever
    // shape (and rows inline or with each name over its arrows) lets the text
    // be largest, wide on a tie when the window is.
    bool wide() const;

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

    // The scene's band and the panel the rows sit in, which depend on the
    // window alone.
    struct Bands final
    {
        Graphics::Rect scene;
        Graphics::Rect panel;
    };

    const Bands& bands() const;
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

    std::string title = "Dress Your Cow";
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
    // Everything the rows' layout depends on besides the window.
    struct Content final
    {
        std::string title;
        std::string doneLabel;
        Vector<EditorRow> rows;
    };

    bool showing = false;
    mutable std::optional<ScreenFrame> bandsFrame;
    mutable bool bandsWide = false;
    mutable Bands bandsMade;
    std::optional<bool> chosenWide;
    std::optional<ScreenFrame> placedFrame;
    Content placedContent;
};
} // namespace Cows
