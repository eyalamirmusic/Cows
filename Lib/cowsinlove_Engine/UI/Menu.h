#pragma once

#include "UI/Hud.h"

#include <functional>
#include <string>

namespace Cows
{
enum class Enabled
{
    No,
    Yes
};

struct MenuItem final
{
    std::string label;
    std::string note = {};
    float scale = 1.f;
    Enabled enabled = Enabled::Yes;
};

// The start menu over the scene: big pink choices in Comic Neue under room
// left at the top for the title, in a column beside the cow on a wide screen
// and below her on a tall one. A disabled choice is grey and never chosen. The mouse hovers and
// clicks, a finger taps, and keys or a controller move a selection ring.
struct Menu final : Graphics::View
{
    Menu();

    void draw(Hud& hud);

    // Where the title goes, in points: the top of the safe area, below
    // topClearance.
    Graphics::Rect titleArea() const;
    // Where the cow should stand on screen: right of the items when wide,
    // between the title and the items when tall.
    Graphics::Rect cowArea() const;
    Graphics::Rect itemArea() const;
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
        Graphics::Point center;
        float pointSize = 0.f;
        Graphics::Rect area;
    };

    // A column of labels, each on one line, never wrapped, shrunk to fit
    // itemArea.
    void layOut(Hud& hud);
    Graphics::Rect safeArea() const;
    float titleShare() const;
    Graphics::Rect belowTitle() const;
    float labelWidth(Hud& hud, int index, float pointSize) const;
    static Graphics::Rect
        targetFor(Graphics::Point center, float labelWidth, float pointSize);
    void drawItem(Hud& hud, int index);
    void choose(int index);
    void returnKeyFocus();

    std::function<void(int)> onChoose = [](int) {};

    Vector<MenuItem> items;
    Vector<Placed> placed;
    float opacity = 1.f;
    // Room kept at the top for the macOS title bar, which the safe area does
    // not count.
    float topClearance = 0.f;
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
