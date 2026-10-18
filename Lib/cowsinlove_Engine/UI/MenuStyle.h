#pragma once

#include "Layout/FitToSize.h"
#include "Layout/Tree.h"
#include "UI/FontCache.h"
#include "UI/Hud.h"

#include <string_view>

namespace Cows
{
// The start menu's look, shared by every screen drawn in its style: pink Comic
// Neue Bold with a white outline and a soft drop shadow, grey when disabled.
constexpr Graphics::Color disabledColor {0.5f, 0.5f, 0.52f, 0.9f};
constexpr Graphics::Color shadowColor {0.f, 0.f, 0.f, 0.18f};
constexpr Graphics::Color outlineColor {1.f, 1.f, 1.f, 1.f};

// Room kept at the top for the macOS title bar, which the safe area does not
// count.
constexpr auto macTitleBarClearance = 28.f;
// The smallest a finger's target is made, when the screen has room for it.
constexpr auto leastTarget = 64.f;

// The hearts' pink, `brighten` of the way to white.
Graphics::Color pink(float brighten = 0.f);
Graphics::Color faded(Graphics::Color color, float opacity);

// The menu's font at `pointSize` on the font grid, from the FontCache.
const Text::Font& menuFont(float pointSize);
// Widths of text in the menu's font, for laying it out.
Flex::Measure menuMeasure(Hud& hud);

// What a screen's layout depends on besides its own content: when none of it
// changes, the rects it made still stand.
struct ScreenFrame final
{
    // The bounds less the safe area, with the top at least topClearance down
    // and the footer's strip (Footer::reserveFor) kept clear at the bottom.
    Graphics::Rect contentArea() const;

    Graphics::Rect bounds;
    Graphics::Insets safe;
    float topClearance = 0.f;
};

ScreenFrame frameOf(const Graphics::View& view, float topClearance);
// Whether a layout made for `a` still stands in `b`.
bool sameFrame(const ScreenFrame& a, const ScreenFrame& b);

// The least a target is made in `content`: a finger's width, or a fifth of
// the content's short side in a window too small for that.
float tapTarget(const Graphics::Rect& content);

// The smallest size a screen's text is laid out at before the screen takes
// another shape: as tall as a button's label on the device, 18 pt on a phone
// to 22 pt on a desktop, from a finger's target (leastTarget).
float smallestReadableSize(const Graphics::Rect& content);

// One way to lay a screen out: the tree it makes at a point size, and the
// rect that tree must fit.
struct LayoutShape final
{
    Flex::TreeAtSize build;
    Graphics::Rect area;
};

struct ChosenShape final
{
    int index = 0;
    float pointSize = 0.f;
    // Whether it fits at the readable size.
    bool readable = false;
};

// Of `shapes`, in order of preference, the first whose text fits within a
// fifth of the largest any of them fits, between `readable` and `largest`.
// Text size is a continuous function of the window for each shape, so this
// never makes it jump by more than that fifth as the window changes. When none fits at `readable`,
// the one that fits largest below it, so nothing leaves the screen.
ChosenShape largestShape(const Vector<LayoutShape>& shapes,
                         const Flex::Measure& measure,
                         float readable,
                         float largest);

// `text` ringed in white so it reads over grass and sky, with its shadow a
// little below and to the right, as the menu draws every label.
void drawMenuText(Hud& hud,
                  std::string_view text,
                  Graphics::Point baselineLeft,
                  const Text::Font& font,
                  const Graphics::Color& color,
                  float opacity);
} // namespace Cows
