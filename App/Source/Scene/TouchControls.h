#pragma once

#include "Common.h"

#include <functional>

namespace Cows
{
// The on-screen controls: a floating stick on the left to walk, Moo and Jump
// (or Again, once she is found) on the right, and a drag anywhere else to look
// round, or a pinch to zoom. Driven by numbered pointers so several fingers can
// work it at once; the mouse drives it as pointer 0.
struct TouchControls final : Graphics::View
{
    TouchControls();

    void pointerDown(int id, Graphics::Point position);
    void pointerMoved(int id, Graphics::Point position);
    void pointerUp(int id);

    void paint(Graphics::Context& g) override;
    void resized() override;

    void mouseDown(const Graphics::MouseEvent& event) override;
    void mouseDragged(const Graphics::MouseEvent& event) override;
    void mouseUp(const Graphics::MouseEvent& event) override;
    void mouseWheel(const Graphics::MouseEvent& event) override;

    enum class Role
    {
        Stick,
        Jump,
        Moo,
        Again,
        Look
    };

    struct Pointer final
    {
        int id = 0;
        Role role = Role::Look;
        Graphics::Point position;
    };

    Role roleAt(Graphics::Point position) const;
    Pointer* find(int id);
    Pointer* otherLooking(int id);
    bool pressed(Role role) const;
    void moveStick(Graphics::Point position);
    void releaseStick();
    void returnKeyFocus();

    void drawStick(Graphics::Context& g);
    void drawButton(Graphics::Context& g,
                    Graphics::Point center,
                    float radius,
                    const std::string& label,
                    bool down);

    std::function<bool()> found = [] { return false; };
    std::function<void(float ahead, float turn)> onStick = [](float, float) {};
    std::function<void()> onJump = [] {};
    std::function<void()> onMoo = [] {};
    std::function<void()> onRestart = [] {};
    std::function<void(float x, float y)> onLook = [](float, float) {};
    std::function<void(float amount)> onZoom = [](float) {};
    std::function<void(const Graphics::MouseEvent&)> onWheel =
        [](const Graphics::MouseEvent&) {};

    Graphics::Insets safeArea;
    Vector<Pointer> pointers;
    Graphics::Point stickHome;
    Graphics::Point stickCenter;
    Graphics::Point knob;
    Graphics::Point jumpCenter;
    Graphics::Point mooCenter;
    Graphics::Font smallLabel {
        Graphics::FontOptions().withName("Menlo-Bold").withSize(14.f)};
    Graphics::Font largeLabel {
        Graphics::FontOptions().withName("Menlo-Bold").withSize(17.f)};
};
} // namespace Cows
