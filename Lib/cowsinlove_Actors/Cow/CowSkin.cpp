#include "Cow/CowSkin.h"

#include <eacp/Core/Utils/Files.h>

#include <algorithm>
#include <cctype>
#include <exception>

namespace Cows
{
namespace
{
constexpr auto fileName = "CowSkin.json";
constexpr auto indent = 4;

template <typename Item>
ItemClass itemClass(std::string key, std::string name, Item CowSkin::* member)
{
    auto names = Miro::enumNames<Item>();

    auto item = ItemClass {};
    item.key = std::move(key);
    item.name = std::move(name);

    for (auto enumerator: names)
        item.choices.add(spelledOut(enumerator));

    item.choice = [member, names](const CowSkin& skin)
    {
        auto current = Miro::enumToString(skin.*member);

        for (auto index = 0; index < (int) names.size(); ++index)
            if (!current.empty() && names[index] == current)
                return index;

        return -1;
    };

    item.choose = [member, names](CowSkin& skin, int index)
    {
        if (index < 0 || index >= (int) names.size())
            return;

        if (auto chosen = Miro::enumFromString<Item>(names[index]))
            skin.*member = *chosen;
    };

    return item;
}

Vector<ItemClass> makeItemClasses()
{
    auto classes = Vector<ItemClass> {};
    classes.add(itemClass("hat", "Hat", &CowSkin::hat));
    return classes;
}

CowSkin withNamedChoices(CowSkin skin)
{
    const auto defaults = CowSkin {};

    for (const auto& item: itemClasses())
        if (item.choice(skin) < 0)
            item.choose(skin, item.choice(defaults));

    return skin;
}
} // namespace

const Vector<ItemClass>& itemClasses()
{
    static const auto classes = makeItemClasses();
    return classes;
}

CowSkin stepped(CowSkin skin, int itemClass, int by)
{
    const auto& classes = itemClasses();

    if (itemClass < 0 || itemClass >= (int) classes.size())
        return skin;

    const auto& item = classes[itemClass];
    auto count = (int) item.choices.size();

    if (count == 0)
        return skin;

    auto current = std::max(item.choice(skin), 0);
    item.choose(skin, ((current + by) % count + count) % count);
    return skin;
}

std::string spelledOut(std::string_view enumeratorName)
{
    auto text = std::string {};

    for (auto character: enumeratorName)
    {
        auto upper = std::isupper((unsigned char) character) != 0;

        if (upper && !text.empty())
            text += ' ';

        text += character;
    }

    return text;
}

std::string toJSON(const CowSkin& skin)
{
    return Miro::toJSONString(skin, indent);
}

CowSkin cowSkinFromJSON(std::string_view text)
{
    return withNamedChoices(Miro::createFromJSONString<CowSkin>(text));
}

FilePath cowSkinFile()
{
    return cowSkinFile(FilePath::appSupportDirectory());
}

FilePath cowSkinFile(const FilePath& directory)
{
    return directory / fileName;
}

CowSkin loadCowSkin(const FilePath& file)
{
    return cowSkinFromJSON(Files::readFile(file));
}

bool saveCowSkin(const CowSkin& skin, const FilePath& file)
{
    auto text = toJSON(skin) + "\n";
    auto bytes = Span<const std::uint8_t> {
        reinterpret_cast<const std::uint8_t*>(text.data()), text.size()};

    try
    {
        Files::createDirectories(file.parentDirectory());
        Files::writeFileAtomically(file, bytes);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}
} // namespace Cows
