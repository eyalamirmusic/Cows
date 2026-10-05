#pragma once

#include "Render/Common.h"
#include "UI/ControlEvent.h"
#include "UI/Hud.h"

#include <functional>
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
// right on a wide screen and at the bottom of a tall one. The arrows step a
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
    bool wide() const;

    void selectNext();
    void selectPrevious();
    void stepSelected(int by);
    void chooseSelected();

    Target targetAt(Graphics::Point position) const;

    Graphics::View* hitTest(const Graphics::Point& point) override;
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
        Graphics::Rect name;
        Graphics::Rect previous;
        Graphics::Rect value;
        Graphics::Rect next;
        Graphics::Rect row;
    };

    Graphics::Rect contentArea() const;
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

    // Points at the top kept clear of anything clickable, for a title bar the
    // safe area does not count.
    float topClearance = 0.f;
    bool showing = false;
    float opacity = 1.f;
    bool showSelection = false;
    int selected = 0;
    Target hovered;
    Target pressed;
    int lookingTouch = -1;
    Graphics::Point lookedFrom;
};
} // namespace Cows
