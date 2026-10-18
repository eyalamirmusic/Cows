#pragma once

#include "UI/Hud.h"
#include "UI/MenuStyle.h"

#include <functional>
#include <optional>
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
// and below her on a tall one. A disabled choice is grey and never chosen. The
// mouse hovers and clicks, a finger taps, and keys or a controller move a
// selection ring. Its labels are laid out through Flex trees, again only when
// the window, its safe area or the items change.
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
    // Whether the items sit in a column beside the cow rather than below her.
    bool wide() const;

    // Replaces the items and lays them out again on the next draw.
    void setItems(Vector<MenuItem> newItems);

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
        float noteSize = 0.f;
    };

    // How the content area is shared out. Wide: the title across the top
    // (`title` of the height), then the items' column and the cow's (`cow` of
    // the width) side by side. Tall: title, cow (`cow` of the height) and
    // items, top to bottom.
    struct Shape final
    {
        bool wide = true;
        float title = 0.f;
        float cow = 0.f;
    };

    // A column of labels, each on one line, never wrapped, at the largest
    // size that fits the items' area, in the first shape that holds every label at
    // the readable size: the cow and the title give way before the text.
    void layOut(Hud& hud);
    Shape shape() const;
    void drawItem(Hud& hud, int index);
    void choose(int index);
    void returnKeyFocus();

    std::function<void(int)> onChoose = [](int) {};

    Vector<MenuItem> items;
    Vector<Placed> placed;
    // The size an item of scale 1 is laid out at.
    float pointSize = 0.f;
    float opacity = 1.f;
    // Room kept at the top for a title bar the safe area does not count
    // (macTitleBarClearance on macOS).
    float topClearance = 0.f;
    bool showSelection = false;
    int selected = 0;
    int hovered = -1;
    int pressed = -1;

private:
    bool showing = true;
    bool itemsChanged = true;
    // The shape the last layOut chose, for the window it was laid out for;
    // until then, or in another window, the window's aspect decides.
    Shape chosen;
    std::optional<ScreenFrame> laidOutFor;
};

// The family Comic Neue Bold registered under, embedded with the Engine; the
// platform's monospace face if it could not be registered.
const std::string& menuFontFamily();
} // namespace Cows
