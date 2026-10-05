#include "UI/Hud.h"

namespace Cows
{
void Hud::begin(Frame& frame, RenderPass& passToUse, int samples)
{
    size = frame.logicalSize();
    auto scale = frame.backingScale();

    if (!shapes.has_value() || builtForSamples != samples)
    {
        shapes.reset();
        shapes.emplace(atlas, ramps, size, scale, samples);
        builtForSamples = samples;
    }

    shapes->setLogicalSize(size);
    shapes->setPixelScale(scale);
    shapes->begin(passToUse);

    text.setSampleCount(samples);
    text.setViewport(size, scale);
    text.begin();

    pass = &passToUse;
}

void Hud::end()
{
    if (pass == nullptr)
        return;

    shapes->end();
    text.flush(*pass);
    pass = nullptr;
}

void Hud::fillDisc(Graphics::Point center,
                   float radius,
                   const Graphics::Color& color)
{
    shapes->fillRect(
        {center.x - radius, center.y - radius, radius * 2.f, radius * 2.f},
        color,
        radius);
}

void Hud::strokeRing(Graphics::Point center,
                     float radius,
                     float width,
                     const Graphics::Color& color)
{
    auto outer = radius + width * 0.5f;

    shapes->drawRect({center.x - outer, center.y - outer, outer * 2.f, outer * 2.f},
                     color,
                     width,
                     outer);
}

void Hud::strokeRoundedRect(const Graphics::Rect& rect,
                            float width,
                            float cornerRadius,
                            const Graphics::Color& color)
{
    shapes->drawRect(rect, color, width, cornerRadius);
}

float Hud::drawText(std::string_view line,
                    Graphics::Point baselineLeft,
                    const Text::Font& font,
                    const Graphics::Color& color)
{
    return text.draw(line, baselineLeft, color, font);
}

float Hud::measure(std::string_view line, const Text::Font& font)
{
    return text.measure(line, font);
}

float Hud::ascent(const Text::Font& font)
{
    return text.ascent(font);
}
} // namespace Cows
