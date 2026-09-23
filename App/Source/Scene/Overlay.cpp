#include "Overlay.h"

#include <string>

namespace Cows
{
namespace
{
constexpr auto footerText = "press q to quit  -  cowsinlove.com";
constexpr auto characterWidth = 7.8f;
constexpr auto bottomMargin = 16.f;
} // namespace

void FooterView::paint(Graphics::Context& g)
{
    auto bounds = getLocalBounds();
    auto text = std::string {footerText};
    auto width = (float) text.size() * characterWidth;
    auto position =
        Graphics::Point {(bounds.w - width) * 0.5f, bounds.h - bottomMargin};

    g.setColor(Graphics::Color {0.f, 0.16f, 0.f, 0.55f});
    g.drawText(text, {position.x + 1.f, position.y + 1.f}, font);

    g.setColor(Graphics::Color {0.92f, 1.f, 0.9f, 0.92f});
    g.drawText(text, position, font);
}

void RootView::resized()
{
    for (auto* child: getSubviews())
        child->setBounds(getLocalBounds());
}

void RootView::keyDown(const Graphics::KeyEvent& event)
{
    if (event.keyCode == Graphics::KeyCode::Q
        || event.keyCode == Graphics::KeyCode::Escape)
        Apps::quit();
}
} // namespace Cows
