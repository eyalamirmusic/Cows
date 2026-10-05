#pragma once

#include "UI/Hud.h"

#include <string_view>

namespace Cows
{
// The start menu's look, shared by every screen drawn in its style: pink Comic
// Neue Bold with a soft drop shadow, grey when disabled.
constexpr Graphics::Color disabledColor {0.5f, 0.5f, 0.52f, 0.9f};
constexpr Graphics::Color shadowColor {0.f, 0.f, 0.f, 0.18f};

// The hearts' pink, `brighten` of the way to white.
Graphics::Color pink(float brighten = 0.f);
Graphics::Color faded(Graphics::Color color, float opacity);
Text::Font menuFont(float pointSize);
Graphics::Rect grown(const Graphics::Rect& rect, float by);

// `text` with its shadow a little below and to the right, as the menu draws
// every label.
void drawMenuText(Hud& hud,
                  std::string_view text,
                  Graphics::Point baselineLeft,
                  const Text::Font& font,
                  const Graphics::Color& color,
                  float opacity);
} // namespace Cows
