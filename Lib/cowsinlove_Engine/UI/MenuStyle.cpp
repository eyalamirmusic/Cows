#include "UI/MenuStyle.h"
#include "Render/Palette.h"
#include "UI/Menu.h"
#include "UI/Overlay.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace Cows
{
namespace
{
constexpr auto outlineShare = 0.05f;
constexpr auto outlineSteps = 16;
constexpr auto targetShare = 0.2f;
// Shares of leastTarget: 18 pt on a phone, 22 pt on a desktop.
constexpr auto phoneTextShare = 0.285f;
constexpr auto desktopTextShare = 0.345f;
constexpr auto phoneSide = 400.f;
constexpr auto desktopSide = 480.f;
constexpr auto smallestFallback = 6.f;
// Keeps the preferred shape unless that costs a fifth of the text's size.
constexpr auto preferenceShare = 0.8f;

bool sameRect(const Graphics::Rect& a, const Graphics::Rect& b)
{
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

bool sameInsets(const Graphics::Insets& a, const Graphics::Insets& b)
{
    return a.top == b.top && a.left == b.left && a.bottom == b.bottom
           && a.right == b.right;
}
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

const Text::Font& menuFont(float pointSize)
{
    return uiFont(menuFontFamily(), pointSize, Text::FontStyle::Bold);
}

Flex::Measure menuMeasure(Hud& hud)
{
    return [&hud](std::string_view text, float pointSize)
    { return hud.measure(text, menuFont(pointSize)); };
}

bool sameFrame(const ScreenFrame& a, const ScreenFrame& b)
{
    return sameRect(a.bounds, b.bounds) && sameInsets(a.safe, b.safe)
           && a.topClearance == b.topClearance;
}

Graphics::Rect ScreenFrame::contentArea() const
{
    auto top = std::max(safe.top, topClearance);
    auto bottom = bounds.h - safe.bottom - Footer::reserveFor(bounds.w);

    return {safe.left,
            top,
            std::max(bounds.w - safe.left - safe.right, 0.f),
            std::max(bottom - top, 0.f)};
}

ScreenFrame frameOf(const Graphics::View& view, float topClearance)
{
    return {view.getLocalBounds(), view.getSafeAreaInsets(), topClearance};
}

float tapTarget(const Graphics::Rect& content)
{
    return std::min(leastTarget, std::min(content.w, content.h) * targetShare);
}

float smallestReadableSize(const Graphics::Rect& content)
{
    auto side = std::min(content.w, content.h);
    auto along =
        std::clamp((side - phoneSide) / (desktopSide - phoneSide), 0.f, 1.f);
    auto share = phoneTextShare + (desktopTextShare - phoneTextShare) * along;
    return onFontGrid(leastTarget * share);
}

ChosenShape largestShape(const Vector<LayoutShape>& shapes,
                         const Flex::Measure& measure,
                         float readable,
                         float largest)
{
    auto pick = [&](float least, float most)
    {
        auto fits = Vector<ChosenShape> {};
        auto largestFit = 0.f;

        for (auto index = 0; index < shapes.size(); ++index)
        {
            const auto& shape = shapes[index];
            auto fitted = Flex::largestPointSize(
                shape.build, shape.area, measure, {least, most, fontSizeStep});

            if (!fitted.fits)
                continue;

            fits.add({index, fitted.pointSize, least >= readable});
            largestFit = std::max(largestFit, fitted.pointSize);
        }

        for (const auto& fit: fits)
            if (fit.pointSize >= largestFit * preferenceShare)
                return std::optional {fit};

        return std::optional<ChosenShape> {};
    };

    if (auto readableBest = pick(readable, std::max(readable, largest)))
        return *readableBest;

    if (auto smallBest = pick(smallestFallback, readable))
        return *smallBest;

    return {0, smallestFallback, false};
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
