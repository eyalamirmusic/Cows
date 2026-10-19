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

struct Menu final : Graphics::View
{
    Menu();

    void draw(Hud& hud);

    Graphics::Rect titleArea() const;
    Graphics::Rect cowArea() const;
    bool wide() const;

    void setItems(Vector<MenuItem> newItems);

    void selectNext();
    void selectPrevious();
    void chooseSelected();

    int itemAt(Graphics::Point position) const;

    // eacp routes touches without hitTest, so hiding clears the handles flags.
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

    struct Shape final
    {
        bool wide = true;
        float titleShare = 0.f;
        float cowShare = 0.f;
    };

    void layOut(Hud& hud);
    Shape shape() const;
    void drawItem(Hud& hud, int index);
    void choose(int index);
    void returnKeyFocus();

    std::function<void(int)> onChoose = [](int) {};

    Vector<MenuItem> items;
    Vector<Placed> placed;
    // At scale 1.
    float pointSize = 0.f;
    float opacity = 1.f;
    // For a title bar the safe area does not count (macTitleBarClearance on macOS).
    float topClearance = 0.f;
    bool showSelection = false;
    int selected = 0;
    int hovered = -1;
    int pressed = -1;

private:
    bool showing = true;
    bool itemsChanged = true;
    // Until laid out, or in another window, the window's aspect decides.
    Shape chosen;
    std::optional<ScreenFrame> laidOutFor;
};

// The platform's monospace face if Comic Neue could not be registered.
const std::string& menuFontFamily();
} // namespace Cows
