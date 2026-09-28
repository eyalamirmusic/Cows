#include "HudLayer.h"
#include "Render/FrameProfile.h"

#include <cmath>

namespace Cows
{
namespace
{
void appendPoint(std::string& text, Graphics::Point point)
{
    text += std::to_string(point.x);
    text += ',';
    text += std::to_string(point.y);
    text += ';';
}

void paintChild(Graphics::SoftwareContext& canvas, Graphics::View& view)
{
    auto bounds = view.getBounds();

    canvas.saveState();
    canvas.translate(bounds.x, bounds.y);
    view.paint(canvas);
    canvas.restoreState();
}
} // namespace

HudLayer::HudLayer(TouchControls& controlsToUse, FooterView& footerToUse)
    : controls(controlsToUse)
    , footer(footerToUse)
{
}

// Everything the two views paint from, so an unchanged HUD is not repainted.
std::string HudLayer::snapshot() const
{
    auto text = footer.text();
    text += '|';
    text += controls.showAgain ? '1' : '0';

    for (const auto& pointer: controls.pointers)
    {
        text += std::to_string(pointer.id);
        text += ':';
        text += std::to_string((int) pointer.role);
        text += ';';
    }

    appendPoint(text, controls.stickCenter);
    appendPoint(text, controls.knob);
    appendPoint(text, controls.jumpCenter);
    appendPoint(text, controls.mooCenter);
    appendPoint(text, {footer.getBounds().w, footer.getBounds().h});
    appendPoint(text, {footer.bottomInset, 0.f});

    return text;
}

void HudLayer::repaint(int pixelWidth, int pixelHeight, float scale)
{
    if (canvas == nullptr || canvas->getImage().width() != pixelWidth
        || canvas->getImage().height() != pixelHeight)
    {
        canvas = std::make_unique<Graphics::SoftwareContext>(
            pixelWidth, pixelHeight, scale);
        texture.reset();
    }

    canvas->clear();

    if (controls.isVisible())
        paintChild(*canvas, controls);

    if (footer.isVisible())
        paintChild(*canvas, footer);

    const auto& image = canvas->getImage();

    if (texture && texture->isValid())
        texture->update(image.pixels().data());
    else
        texture = Device::shared().makeTexture(image);
}

void HudLayer::draw(GPUView& scene, RenderPass& pass)
{
    auto bounds = scene.getBounds();
    auto pixelWidth = pass.targetWidth();
    auto pixelHeight = pass.targetHeight();

    if (bounds.w <= 0.f || pixelWidth <= 0 || pixelHeight <= 0)
        return;

    auto scale = (float) pixelWidth / bounds.w;
    auto current = snapshot();
    auto sized = canvas != nullptr && canvas->getImage().width() == pixelWidth
                 && canvas->getImage().height() == pixelHeight;

    if (!sized || current != painted)
    {
        repaint(pixelWidth, pixelHeight, scale);
        painted = current;
        FrameProfile::shared().hudRepainted();
    }

    if (!texture || !texture->isValid())
        return;

    auto size = Graphics::Point {bounds.w, bounds.h};

    if (sprites == nullptr)
        sprites =
            std::make_unique<Sprites::SpriteRenderer>(size, scene.sampleCount());

    sprites->setLogicalSize(size);
    sprites->begin(pass);
    sprites->drawTexture(*texture, {0.f, 0.f, bounds.w, bounds.h});
    sprites->end();
}
} // namespace Cows
