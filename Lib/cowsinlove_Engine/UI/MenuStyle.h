#pragma once

#include "Layout/FitToSize.h"
#include "Layout/Tree.h"
#include "UI/FontCache.h"
#include "UI/Hud.h"

#include <string_view>

namespace Cows
{
constexpr Graphics::Color disabledColor {0.5f, 0.5f, 0.52f, 0.9f};
constexpr Graphics::Color shadowColor {0.f, 0.f, 0.f, 0.18f};
constexpr Graphics::Color outlineColor {1.f, 1.f, 1.f, 1.f};

// The safe area does not count the macOS title bar.
constexpr auto macTitleBarClearance = 28.f;
constexpr auto leastTarget = 64.f;

Graphics::Color pink(float brighten = 0.f);
Graphics::Color faded(Graphics::Color color, float opacity);

const Text::Font& menuFont(float pointSize);
Flex::Measure menuMeasure(Hud& hud);

struct ScreenFrame final
{
    // Less the safe area, topClearance and the footer's strip (Footer::reserveFor).
    Graphics::Rect contentArea() const;

    Graphics::Rect bounds;
    Graphics::Insets safe;
    float topClearance = 0.f;
};

ScreenFrame frameOf(const Graphics::View& view, float topClearance);
bool sameFrame(const ScreenFrame& a, const ScreenFrame& b);

float tapTarget(const Graphics::Rect& content);

// 18 pt on a phone to 22 pt on a desktop.
float smallestReadableSize(const Graphics::Rect& content);

struct LayoutShape final
{
    Flex::TreeAtSize build;
    Graphics::Rect area;
};

struct ChosenShape final
{
    int index = 0;
    float pointSize = 0.f;
    bool readable = false;
};

// First shape within a fifth of the best fit, so text never jumps more on resize.
ChosenShape largestShape(const Vector<LayoutShape>& shapes,
                         const Flex::Measure& measure,
                         float readable,
                         float largest);

void drawMenuText(Hud& hud,
                  std::string_view text,
                  Graphics::Point baselineLeft,
                  const Text::Font& font,
                  const Graphics::Color& color,
                  float opacity);
} // namespace Cows
