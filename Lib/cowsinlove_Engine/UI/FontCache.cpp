#include "UI/FontCache.h"

#include <eacp/Core/Utils/Singleton.h>

#include <algorithm>
#include <cmath>
#include <functional>

namespace Cows
{
float onFontGrid(float pointSize)
{
    return std::max(1.f, std::floor(pointSize / fontSizeStep)) * fontSizeStep;
}

std::size_t FontCache::KeyHash::operator()(const Key& key) const
{
    auto hash = std::hash<std::string> {}(key.family);
    hash ^= std::hash<int> {}(key.steps) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^=
        std::hash<int> {}((int) key.style) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

const Text::Font& FontCache::fontFor(std::string_view family,
                                     float pointSize,
                                     Text::FontStyle style)
{
    auto size = onFontGrid(pointSize);
    auto key =
        Key {std::string {family}, (int) std::lround(size / fontSizeStep), style};
    auto found = fonts.find(key);

    if (found != fonts.end())
        return found->second;

    auto font = Text::Font {key.family, size, style};
    return fonts.emplace(std::move(key), std::move(font)).first->second;
}

int FontCache::size() const
{
    return (int) fonts.size();
}

const Text::Font&
    uiFont(std::string_view family, float pointSize, Text::FontStyle style)
{
    return Singleton::get<FontCache>().fontFor(family, pointSize, style);
}
} // namespace Cows
