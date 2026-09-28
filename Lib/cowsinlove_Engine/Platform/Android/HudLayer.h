#pragma once

#include "UI/Overlay.h"
#include "UI/TouchControls.h"

#include <eacp/Sprites/SpriteRenderer.h>

#include <memory>
#include <optional>
#include <string>

namespace Cows
{
// The footer and the touch controls, painted on the CPU through eacp's
// SoftwareContext and drawn over the scene as one texture. Repainted only when
// what they show has changed.
struct HudLayer final
{
    HudLayer(TouchControls& controlsToUse, FooterView& footerToUse);

    void draw(GPUView& scene, RenderPass& pass);

    std::string snapshot() const;
    void repaint(int pixelWidth, int pixelHeight, float scale);

    TouchControls& controls;
    FooterView& footer;

    std::string painted;
    std::unique_ptr<Graphics::SoftwareContext> canvas;
    std::optional<Texture> texture;
    std::unique_ptr<Sprites::SpriteRenderer> sprites;
};
} // namespace Cows
