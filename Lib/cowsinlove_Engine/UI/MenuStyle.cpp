#include "UI/MenuStyle.h"
#include "Render/Palette.h"
#include "UI/Menu.h"

#include <algorithm>

namespace Cows
{
Graphics::Color pink(float brighten)
{
    auto color = Palette::display(Palette::heart);
    return {color.r + (1.f - color.r) * brighten,
            color.g + (1.f - color.g) * brighten,
            color.b + (1.f - color.b) * brighten,
            1.f};
}

Graphics::Color faded(Graphics::Color color, float opacity)
{
    color.a *= opacity;
    return color;
}

Text::Font menuFont(float pointSize)
{
    return {menuFontFamily(), pointSize, Text::FontStyle::Bold};
}

Graphics::Rect grown(const Graphics::Rect& rect, float by)
{
    return {rect.x - by, rect.y - by, rect.w + 2.f * by, rect.h + 2.f * by};
}

void drawMenuText(Hud& hud,
                  std::string_view text,
                  Graphics::Point baselineLeft,
                  const Text::Font& font,
                  const Graphics::Color& color,
                  float opacity)
{
    auto at = baselineLeft;
    auto offset = std::max(1.f, font.pointSize * 0.03f);

    hud.drawText(
        text, {at.x + offset, at.y + offset}, font, faded(shadowColor, opacity));
    hud.drawText(text, at, font, color);
}
} // namespace Cows
