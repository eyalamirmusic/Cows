#pragma once

#include "Render/Common.h"

#include <Miro/Reflect.h>

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
    Crown,
    FrogHat,
    CowBucketHat,
    TrafficCone,
    WizardHat
};

enum class Pants
{
    None,
    BothLegs,
    BackLegs
};

// Saved as enumerator names so settings.json stays readable by hand.
struct CowSkin final
{
    bool operator==(const CowSkin& other) const = default;

    Hat hat = Hat::None;
    Pants pants = Pants::None;

    MIRO_REFLECT(hat, pants)
};

// `choice` is -1 when the skin holds no named value.
struct ItemClass final
{
    std::string key;
    std::string name;
    Vector<std::string> choices;
    std::function<int(const CowSkin&)> choice = [](const CowSkin&) { return -1; };
    std::function<void(CowSkin&, int)> choose = [](CowSkin&, int) {};
};

const Vector<ItemClass>& itemClasses();

CowSkin stepSkinItemChoice(CowSkin skin, int itemClass, int by);

std::string spellOutEnumeratorName(std::string_view enumeratorName);

std::string toJSON(const CowSkin& skin);

CowSkin cowSkinFromJSON(std::string_view text);

CowSkin replaceUnknownValuesWithDefaults(CowSkin skin);
} // namespace Cows
