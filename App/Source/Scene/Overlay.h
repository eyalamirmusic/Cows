#pragma once

#include "Common.h"

#include <functional>
#include <string>

namespace Cows
{
// The footer, painted over the scene: how to play while searching, how to
// play again once she is found. Breaks between phrases when it runs too wide.
struct FooterView final : Graphics::View
{
    void paint(Graphics::Context& g) override;
    Vector<std::string> lines(const std::string& line, float width) const;

    std::function<std::string()> text = [] { return std::string {}; };
    float bottomInset = 0.f;

    Graphics::Font font {Graphics::FontOptions().withName("Menlo").withSize(13.f)};
};

// Stacks its children over each other, quits on q or Escape, and passes every
// other key on.
struct RootView final : Graphics::View
{
    void resized() override;
    void keyDown(const Graphics::KeyEvent& event) override;
    void keyUp(const Graphics::KeyEvent& event) override;

    std::function<void(const Graphics::KeyEvent&)> onKeyDown =
        [](const Graphics::KeyEvent&) {};
    std::function<void(const Graphics::KeyEvent&)> onKeyUp =
        [](const Graphics::KeyEvent&) {};
};
} // namespace Cows
