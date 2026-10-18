#pragma once

#include "UI/Menu.h"

namespace Cows
{
constexpr auto startScale = 1.3f;
constexpr auto aboutScale = 0.75f;

// The start menu's choices, Start the biggest and About the smallest; CowsApp
// shows them and CowsArt renders them.
inline Vector<MenuItem> mainMenuItems()
{
    auto items = Vector<MenuItem> {};
    items.add({"Start", "", startScale});
    items.add({"Dress Your Cow"});
    items.add({"Settings"});
    items.add({"About", "", aboutScale});
    return items;
}
} // namespace Cows
