#pragma once

#include "Render/Common.h"
#include "UI/ControlEvent.h"
#include "UI/Hud.h"
#include "UI/MenuStyle.h"

#include <algorithm>
#include <array>
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

inline int wrapChoiceIndex(int index, int numChoices)
{
    auto remainder = index % numChoices;
    return remainder < 0 ? remainder + numChoices : remainder;
}

template <typename Entry, std::size_t numEntries, typename IsCurrent>
const Entry& stepThroughEntries(const std::array<Entry, numEntries>& entries,
                                IsCurrent isCurrent,
                                int steps)
{
    auto current = std::ranges::find_if(entries, isCurrent);
    auto currentIndex = static_cast<int>(current - entries.begin());
    auto numChoices = static_cast<int>(numEntries);
    return entries[wrapChoiceIndex(currentIndex + steps, numChoices)];
}

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

    Graphics::Rect cowArea() const;
    Graphics::Rect panelArea() const;
    bool wide() const;

    void setRows(Vector<EditorRow> newRows);
    void setTitle(std::string newTitle);

    void selectNext();
    void selectPrevious();
    void stepSelected(int by);
    void chooseSelected();

    Target targetAt(Graphics::Point position) const;

    // eacp routes touches without hitTest, so hiding clears the handles flags.
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

    struct Shape final
    {
        bool wide = true;
        float cowShare = 0.f;
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

    std::function<void(int rowIndex, int steps)> onRowValueStepped = [](int, int) {};
    std::function<void()> onDone = [] {};
    std::function<void(const ControlEvent&)> onControl = [](const ControlEvent&) {};

    std::string title;
    std::string doneLabel = "Done";
    Vector<EditorRow> rows;
    Vector<Placed> placed;
    Placed done;
    Graphics::Rect titleBox;
    float titleSize = 0.f;
    bool namesAboveValues = false;

    // For a title bar the safe area does not count (macTitleBarClearance on macOS).
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
    // Until laid out, or in another window, the window's aspect decides.
    Shape chosen;
    std::optional<ScreenFrame> laidOutFor;
};
} // namespace Cows
