#pragma once

#include "Layout/FitToSize.h"
#include "Render/Common.h"

#include <eacp/Text/Font.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Cows
{
using Flex::fontSizeStep;

// `pointSize` rounded down to fontSizeStep, never below one step.
float onFontGrid(float pointSize);

// The UI's fonts, kept as Text::Font values so a frame never builds one (and
// copies its family) per label. A map, so a reference it hands out holds for
// as long as the cache does. Sizes land on the fontSizeStep grid between the
// readable floor and the size cap, about 109 per family and style, so it holds
// a few hundred fonts (a Font is 40 bytes, an entry about 110 bytes of heap:
// some 40 KB) however long a window is resized; a thousand would be about
// 110 KB. Reach it through `Singleton::get<FontCache>()`; `uiFont` does.
struct FontCache final
{
    // The font of `family` and `style` at `pointSize` on the grid.
    const Text::Font& fontFor(std::string_view family,
                              float pointSize,
                              Text::FontStyle style = Text::FontStyle::Regular);

    int size() const;

private:
    struct Key final
    {
        bool operator==(const Key& other) const = default;

        std::string family;
        int steps = 0;
        Text::FontStyle style = Text::FontStyle::Regular;
    };

    struct KeyHash final
    {
        std::size_t operator()(const Key& key) const;
    };

    std::unordered_map<Key, Text::Font, KeyHash> fonts;
};

// Singleton::get<FontCache>().fontFor(...).
const Text::Font& uiFont(std::string_view family,
                         float pointSize,
                         Text::FontStyle style = Text::FontStyle::Regular);
} // namespace Cows
