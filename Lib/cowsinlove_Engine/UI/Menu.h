#pragma once

#include "Render/Common.h"
#include "UI/Hud.h"

#include <functional>
#include <string>

namespace Cows
{
struct MenuItem final
{
    std::string label;
    std::string note;
    bool enabled = true;
};

// The start menu over the scene: big pink choices in Comic Neue under room
// left at the top for the title, side by side on a wide screen and stacked on
// a tall one. A disabled choice is grey and never chosen. The mouse hovers and
// clicks, a finger taps, and keys or a controller move a selection ring.
struct Menu final : Graphics::View
{
    Menu();

    void draw(Hud& hud);

    // Where the title goes, in points: the top of the safe area.
    Graphics::Rect titleArea() const;
    bool wide() const;

    void selectNext();
    void selectPrevious();
    void chooseSelected();

    int itemAt(Graphics::Point position) const;

    // Hidden, it takes no mouse or touch, so they reach the views under it.
    // eacp finds a finger's view without calling hitTest, so this clears the
    // handles flags rather than overriding hitTest.
    void setShowing(bool shouldShow);
    bool isShowing() const;

    void mouseMoved(const Graphics::MouseEvent& event) override;
    void mouseExited(const Graphics::MouseEvent& event) override;
    void mouseDown(const Graphics::MouseEvent& event) override;
    void mouseUp(const Graphics::MouseEvent& event) override;
    void touchBegan(const Graphics::TouchEvent& event) override;
    void touchMoved(const Graphics::TouchEvent& event) override;
    void touchEnded(const Graphics::TouchEvent& event) override;

    struct Placed final
    {
        Vector<std::string> lines;
        Graphics::Point center;
        float pointSize = 0.f;
        Graphics::Rect area;
    };

    void layOut(Hud& hud);
    void drawItem(Hud& hud, int index);
    void choose(int index);
    void returnKeyFocus();

    std::function<void(int)> onChoose = [](int) {};

    Vector<MenuItem> items;
    Vector<Placed> placed;
    float opacity = 1.f;
    bool showSelection = false;
    int selected = 0;
    int hovered = -1;
    int pressed = -1;

private:
    bool showing = true;
};

// The family Comic Neue Bold registered under, embedded with the Engine; the
// platform's monospace face if it could not be registered.
const std::string& menuFontFamily();
} // namespace Cows
