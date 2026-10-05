#pragma once

#include "Render/Common.h"

#include <eacp/Text/TextRenderer.h>
#include <eacp/UI/Render/CoverageAtlas.h>
#include <eacp/UI/Render/GradientRamps.h>
#include <eacp/UI/Render/ShapeBatch.h>

#include <optional>
#include <string_view>

namespace Cows
{
// The footer and the touch controls, drawn over the scene inside its own pass:
// discs and rings through eacp's shape batch, text through its glyph atlas.
// Everything is in points, y down; every disc and ring lands under every label.
struct Hud final
{
    void begin(Frame& frame, RenderPass& pass, int samples);
    void end();

    void
        fillDisc(Graphics::Point center, float radius, const Graphics::Color& color);
    void strokeRing(Graphics::Point center,
                    float radius,
                    float width,
                    const Graphics::Color& color);
    void strokeRoundedRect(const Graphics::Rect& rect,
                           float width,
                           float cornerRadius,
                           const Graphics::Color& color);

    float drawText(std::string_view text,
                   Graphics::Point baselineLeft,
                   const Text::Font& font,
                   const Graphics::Color& color);

    float measure(std::string_view text, const Text::Font& font);
    float ascent(const Text::Font& font);

    Graphics::Point size;

private:
    UI::CoverageAtlas atlas;
    UI::GradientRamps ramps;
    std::optional<UI::ShapeBatch> shapes;
    Text::TextRenderer text;
    RenderPass* pass = nullptr;
    int builtForSamples = 0;
};
} // namespace Cows
