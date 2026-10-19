#pragma once

#include "Render/Common.h"
#include "UI/ControlEvent.h"
#include "UI/Hud.h"

#include <functional>
#include <string_view>

namespace Cows
{
constexpr auto touchScreen = Platform::isIOS() || Platform::isAndroid();

// Each touch is its own pointer; the mouse is pointer 0.
struct TouchControls final : Graphics::View
{
    TouchControls();

    void pointerDown(int id, Graphics::Point position);
    void pointerMoved(int id, Graphics::Point position);
    void pointerUp(int id);

    void draw(Hud& hud) const;
    void resized() override;

    void mouseDown(const Graphics::MouseEvent& event) override;
    void mouseDragged(const Graphics::MouseEvent& event) override;
    void mouseUp(const Graphics::MouseEvent& event) override;
    void mouseWheel(const Graphics::MouseEvent& event) override;

    void touchBegan(const Graphics::TouchEvent& event) override;
    void touchMoved(const Graphics::TouchEvent& event) override;
    void touchEnded(const Graphics::TouchEvent& event) override;
    void safeAreaInsetsChanged() override;

    enum class Role
    {
        Stick,
        Jump,
        Moo,
        Again,
        Back,
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

    void drawStick(Hud& hud) const;
    void drawBackButton(Hud& hud) const;
    void drawButton(Hud& hud,
                    Graphics::Point center,
                    float radius,
                    std::string_view label,
                    bool down) const;

    std::function<void(const ControlEvent&)> onControl = [](const ControlEvent&) {};

    bool showAgain = false;
    // For a title bar the safe area does not count (macTitleBarClearance on macOS).
    float topClearance = 0.f;
    Vector<Pointer> pointers;
    Graphics::Point stickHome;
    Graphics::Point stickCenter;
    Graphics::Point knob;
    Graphics::Point jumpCenter;
    Graphics::Point mooCenter;
    Graphics::Point backCenter;
    static constexpr auto labelFamily = "Menlo";
    static constexpr auto smallLabelSize = 14.f;
    static constexpr auto largeLabelSize = 17.f;
};
} // namespace Cows
