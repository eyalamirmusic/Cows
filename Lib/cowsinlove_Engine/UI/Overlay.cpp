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

Vector<std::string>
    Footer::lines(const std::string& line,
                  float width,
                  const std::function<float(std::string_view)>& measure) const
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

        if (!current.empty() && measure(joined) > width)
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

void Footer::draw(Hud& hud) const
{
    auto measure = [&](std::string_view line) { return hud.measure(line, font); };
    auto wrapped = lines(text(), hud.size.x - 2.f * sideMargin, measure);
    auto baseline = hud.size.y - bottomInset - bottomMargin
                    - lineHeight * (float) (wrapped.size() - 1);

    for (const auto& line: wrapped)
    {
        auto width = measure(line);
        auto position = Graphics::Point {(hud.size.x - width) * 0.5f, baseline};

        hud.drawText(line,
                     {position.x + 1.f, position.y + 1.f},
                     font,
                     Graphics::Color {0.f, 0.16f, 0.f, 0.55f});
        hud.drawText(
            line, position, font, Graphics::Color {0.92f, 1.f, 0.9f, 0.92f});

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
