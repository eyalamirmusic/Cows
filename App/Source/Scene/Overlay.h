#pragma once

#include "Common.h"

#include <functional>

namespace Cows
{
// The footer line, painted over the scene: how to play while searching, how
// to play again once she is found.
struct FooterView final : Graphics::View
{
    void paint(Graphics::Context& g) override;

    std::function<bool()> isFound = [] { return false; };

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
