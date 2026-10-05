#pragma once

#include "UI/Hud.h"

#include <functional>
#include <string>
#include <string_view>

namespace Cows
{
// The footer, drawn over the scene: how to play while searching, how to play
// again once she is found. Breaks between phrases when it runs too wide.
struct Footer final
{
    void draw(Hud& hud) const;
    Vector<std::string>
        lines(const std::string& line,
              float width,
              const std::function<float(std::string_view)>& measure) const;

    std::function<std::string()> text = [] { return std::string {}; };
    float bottomInset = 0.f;

    Text::Font font {"Menlo", 13.f};
};

// Stacks its children over each other, quits on q, and on Escape unless
// onEscape takes it, and passes every other key on. Back is Escape that never
// quits: what onEscape leaves goes on to the system, which on Android leaves
// the app, and its repeats and release follow the press.
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
