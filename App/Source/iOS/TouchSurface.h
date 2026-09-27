#pragma once

#include "Scene/TouchControls.h"

#include <functional>
#include <memory>

namespace Cows
{
// iOS only. Lays a transparent multi-touch layer over `host` that feeds every
// finger to `controls` as its own pointer, and reports the screen's safe area.
struct TouchSurface final
{
    TouchSurface(Graphics::View& host, TouchControls& controls);
    ~TouchSurface();

    TouchSurface(const TouchSurface&) = delete;
    TouchSurface& operator=(const TouchSurface&) = delete;

    std::function<void(Graphics::Insets)> onSafeAreaChanged =
        [](Graphics::Insets) {};

    struct Native;
    std::unique_ptr<Native> native;
};

// eacp's iOS views are opaque, so an overlay painted over the scene hides it
// in black unless it is made see-through.
void makeSeeThrough(Graphics::View& view);

// Plays the moo alongside other apps' audio, quiet when the phone is silenced.
void setUpAudioSession();
} // namespace Cows
