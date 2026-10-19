#pragma once

#include "UI/Hud.h"

#include <functional>
#include <string>
#include <string_view>

namespace Cows
{
struct Footer final
{
    void draw(Hud& hud) const;
    Vector<std::string>
        lines(const std::string& line,
              float width,
              const std::function<float(std::string_view)>& measure) const;

    static constexpr auto bottomMargin = 16.f;
    static constexpr auto lineHeight = 18.f;
    static constexpr auto reserve = bottomMargin + lineHeight;
    // Below this the menu's and the editor's footers wrap to two lines.
    static constexpr auto oneLineWidth = 400.f;
    static float reserveFor(float width);

    std::function<std::string()> text = [] { return std::string {}; };
    float bottomInset = 0.f;

    static constexpr auto fontFamily = "Menlo";
    static constexpr auto fontSize = 13.f;
};

// Back never quits: what onEscape declines goes on to the system.
struct RootView final : Graphics::View
{
    void resized() override;
    void keyDown(const Graphics::KeyEvent& event) override;
    void keyUp(const Graphics::KeyEvent& event) override;

    std::function<void(const Graphics::KeyEvent&)> onKeyDown =
        [](const Graphics::KeyEvent&) {};
    std::function<void(const Graphics::KeyEvent&)> onKeyUp =
        [](const Graphics::KeyEvent&) {};
    std::function<bool()> onEscape = [] { return false; };

private:
    void backKey(const Graphics::KeyEvent& event);

    bool passingBack = false;
};
} // namespace Cows
