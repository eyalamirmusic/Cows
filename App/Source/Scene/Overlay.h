#pragma once

#include "Common.h"

namespace Cows
{
// The original's footer line, painted over the scene.
struct FooterView final : Graphics::View
{
    void paint(Graphics::Context& g) override;

    Graphics::Font font {Graphics::FontOptions().withName("Menlo").withSize(13.f)};
};

// Stacks its children over each other, and quits on q or Escape.
struct RootView final : Graphics::View
{
    void resized() override;
    void keyDown(const Graphics::KeyEvent& event) override;
};
} // namespace Cows
