#include "UI/MenuStyle.h"
#include "Render/Palette.h"
#include "UI/Menu.h"

#include <algorithm>
#include <cmath>

namespace Cows
{
namespace
{
constexpr auto outlineShare = 0.05f;
constexpr auto outlineSteps = 16;
} // namespace

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
    auto outline = std::max(1.5f, font.pointSize * outlineShare);
    auto shadowAt = outline + offset;

    hud.drawText(
        text, {at.x + shadowAt, at.y + shadowAt}, font, faded(shadowColor, opacity));

    for (auto step = 0; step < outlineSteps; ++step)
    {
        auto angle = 2.f * Maths::pi * (float) step / (float) outlineSteps;
        hud.drawText(
            text,
            {at.x + outline * std::cos(angle), at.y + outline * std::sin(angle)},
            font,
            faded(outlineColor, opacity));
    }

    hud.drawText(text, at, font, color);
}
} // namespace Cows
