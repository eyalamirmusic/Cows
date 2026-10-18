#include "UI/FontCache.h"

#include <eacp/Core/Utils/Singleton.h>

#include <algorithm>
#include <cmath>

namespace Cows
{
namespace
{
bool matches(const Text::Font& font,
             std::string_view family,
             float pointSize,
             Text::FontStyle style)
{
    return font.pointSize == pointSize && font.style == style
           && font.family == family;
}
} // namespace

float onFontGrid(float pointSize)
{
    return std::max(1.f, std::floor(pointSize / fontSizeStep)) * fontSizeStep;
}

const Text::Font& FontCache::fontFor(std::string_view family,
                                     float pointSize,
                                     Text::FontStyle style)
{
    auto size = onFontGrid(pointSize);
    ++clock;

    for (auto& entry: entries)
        if (entry.used && matches(entry.font, family, size, style))
        {
            entry.lastUsed = clock;
            return entry.font;
        }

    auto& oldest = *std::min_element(entries.begin(),
                                     entries.end(),
                                     [](const Entry& a, const Entry& b)
                                     {
                                         if (a.used != b.used)
                                             return !a.used;

                                         return a.lastUsed < b.lastUsed;
                                     });

    oldest.font = {std::string {family}, size, style};
    oldest.lastUsed = clock;
    oldest.used = true;
    return oldest.font;
}

int FontCache::size() const
{
    return (int) std::count_if(entries.begin(),
                               entries.end(),
                               [](const Entry& entry) { return entry.used; });
}

bool FontCache::holds(std::string_view family,
                      float pointSize,
                      Text::FontStyle style) const
{
    auto size = onFontGrid(pointSize);
    return std::any_of(
        entries.begin(),
        entries.end(),
        [&](const Entry& entry)
        { return entry.used && matches(entry.font, family, size, style); });
}

const Text::Font&
    uiFont(std::string_view family, float pointSize, Text::FontStyle style)
{
    return Singleton::get<FontCache>().fontFor(family, pointSize, style);
}
} // namespace Cows
