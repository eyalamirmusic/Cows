#include "UI/Overlay.h"

#include <algorithm>
#include <string>
#include <string_view>

namespace Cows
{
namespace
{
constexpr auto bottomMargin = 16.f;
constexpr auto sideMargin = 16.f;
constexpr auto lineHeight = 18.f;
constexpr std::string_view separator = "  -  ";
} // namespace

Vector<std::string> FooterView::lines(const std::string& line, float width) const
{
    auto result = Vector<std::string> {};
    auto current = std::string {};
    auto start = std::size_t {0};

    while (start <= line.size())
    {
        auto end = std::min(line.find(separator, start), line.size());
        auto phrase = line.substr(start, end - start);
        auto joined =
            current.empty() ? phrase : current + std::string(separator) + phrase;

        if (!current.empty()
            && Graphics::TextMetrics::measureWidth(joined, font) > width)
        {
            result.add(current);
            current = phrase;
        }
        else
        {
            current = joined;
        }

        start = end + separator.size();
    }

    result.add(current);
    return result;
}

void FooterView::paint(Graphics::Context& g)
{
    auto bounds = getLocalBounds();
    auto wrapped = lines(text(), bounds.w - 2.f * sideMargin);
    auto baseline = bounds.h - bottomInset - bottomMargin
                    - lineHeight * (float) (wrapped.size() - 1);

    for (const auto& line: wrapped)
    {
        auto width = Graphics::TextMetrics::measureWidth(line, font);
        auto position = Graphics::Point {(bounds.w - width) * 0.5f, baseline};

        g.setColor(Graphics::Color {0.f, 0.16f, 0.f, 0.55f});
        g.drawText(line, {position.x + 1.f, position.y + 1.f}, font);

        g.setColor(Graphics::Color {0.92f, 1.f, 0.9f, 0.92f});
        g.drawText(line, position, font);

        baseline += lineHeight;
    }
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
    {
        Apps::quit();
        return;
    }

    onKeyDown(event);
}

void RootView::keyUp(const Graphics::KeyEvent& event)
{
    onKeyUp(event);
}
} // namespace Cows
