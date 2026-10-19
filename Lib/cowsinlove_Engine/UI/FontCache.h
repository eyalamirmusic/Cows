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

float onFontGrid(float pointSize);

// A map so references stay valid; a few hundred fonts on the step grid, about 40 KB.
struct FontCache final
{
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

const Text::Font& uiFont(std::string_view family,
                         float pointSize,
                         Text::FontStyle style = Text::FontStyle::Regular);
} // namespace Cows
