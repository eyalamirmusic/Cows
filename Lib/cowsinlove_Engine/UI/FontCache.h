#pragma once

#include "Render/Common.h"

#include <eacp/Text/Font.h>

#include <array>
#include <cstdint>
#include <string_view>

namespace Cows
{
// Every point size the UI asks a font for lands on this grid, so a window
// being resized asks the glyph atlas for a few faces rather than hundreds
// (eacp keeps a face for every size it is ever asked for).
constexpr auto fontSizeStep = 0.5f;

// `pointSize` rounded down to fontSizeStep, never below one step.
float onFontGrid(float pointSize);

// The UI's fonts, kept as plain Text::Font values so a frame never builds one
// (and copies its family) per label. A least-recently-used set of `capacity`:
// a frame uses a handful of sizes (the menu's four items, notes, the editor's
// rows, heading, hover sizes, the footer, the touch labels), and a resize
// walks through more, so a few dozen keeps a whole frame's set while bounding
// what a long session of resizing keeps. Reach it through
// `Singleton::get<FontCache>()`; `uiFont` does.
struct FontCache final
{
    static constexpr auto capacity = 48;

    // The font of `family` and `style` at `pointSize` on the grid. The
    // reference holds until `capacity` other fonts have been asked for.
    const Text::Font& fontFor(std::string_view family,
                              float pointSize,
                              Text::FontStyle style = Text::FontStyle::Regular);

    int size() const;
    bool holds(std::string_view family,
               float pointSize,
               Text::FontStyle style = Text::FontStyle::Regular) const;

private:
    struct Entry final
    {
        Text::Font font;
        std::uint64_t lastUsed = 0;
        bool used = false;
    };

    std::array<Entry, capacity> entries;
    std::uint64_t clock = 0;
};

// Singleton::get<FontCache>().fontFor(...).
const Text::Font& uiFont(std::string_view family,
                         float pointSize,
                         Text::FontStyle style = Text::FontStyle::Regular);
} // namespace Cows
