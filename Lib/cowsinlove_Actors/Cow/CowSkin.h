#pragma once

#include "Render/Common.h"

#include <Miro/Reflect.h>
#include <eacp/Core/Utils/FilePath.h>

#include <functional>
#include <string>
#include <string_view>

namespace Cows
{
enum class Hat
{
    None,
    TopHat,
    CowboyHat,
    PartyHat,
    Beanie,
    Crown
};

// What the player's cow wears: one choice per item class, saved as the
// enumerators' names so CowSkin.json stays readable by hand.
struct CowSkin final
{
    bool operator==(const CowSkin& other) const = default;

    Hat hat = Hat::None;

    MIRO_REFLECT(hat)
};

// One kind of thing to wear, as the editor shows it: `key` is its field in
// CowSkin.json, `choices` its enumerators spelled out ("Top Hat"), in order.
// `choice` is the skin's index into them, -1 when it holds no named value.
struct ItemClass final
{
    std::string key;
    std::string name;
    Vector<std::string> choices;
    std::function<int(const CowSkin&)> choice = [](const CowSkin&) { return -1; };
    std::function<void(CowSkin&, int)> choose = [](CowSkin&, int) {};
};

// Every item class CowSkin holds, in the order the editor lists them.
const Vector<ItemClass>& itemClasses();

// `skin` with item class `itemClass` moved `by` choices along, wrapping.
CowSkin stepped(CowSkin skin, int itemClass, int by);

// "TopHat" as "Top Hat".
std::string spelledOut(std::string_view enumeratorName);

std::string toJSON(const CowSkin& skin);

// The skin `text` describes; the default skin for text that is not a skin, and
// the default choice for an item it does not name.
CowSkin cowSkinFromJSON(std::string_view text);

// CowSkin.json in `directory`, the app's support folder unless told otherwise.
FilePath cowSkinFile();
FilePath cowSkinFile(const FilePath& directory);

// The skin saved at `file`, or the default one when there is none to read.
CowSkin loadCowSkin(const FilePath& file);

// Writes `skin` to `file`, creating its folder; false when it could not.
bool saveCowSkin(const CowSkin& skin, const FilePath& file);
} // namespace Cows
