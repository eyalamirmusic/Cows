#pragma once

#include "Cow/CowSkin.h"
#include "Scene/FrameRate.h"
#include "Scene/Quality.h"
#include "UI/Editor.h"
#include "UI/Menu.h"

#include <string>

namespace Cows
{
constexpr auto startScale = 1.3f;
constexpr auto aboutScale = 0.75f;
constexpr auto startLabel = "Start";
constexpr auto dressLabel = "Dress Your Cow";
constexpr auto settingsLabel = "Settings";
constexpr auto aboutLabel = "About";
constexpr auto backLabel = "Back";
constexpr auto dressTitle = "Dress Your Cow";
constexpr auto settingsTitle = "Settings";
constexpr auto qualityRowName = "Quality";
constexpr auto frameRateRowName = "Frame rate";
constexpr auto autoPrefix = "Auto: ";

// The start menu's choices, Start the biggest and About the smallest, and the
// screens behind them; CowsApp shows them and CowsArt renders them.
inline Vector<MenuItem> mainMenuItems()
{
    auto items = Vector<MenuItem> {};
    items.add({startLabel, "", startScale});
    items.add({dressLabel});
    items.add({settingsLabel});
    items.add({aboutLabel, "", aboutScale});
    return items;
}

inline std::string versionText()
{
    return std::string {"Version " COWS_APP_VERSION " (Build " COWS_APP_BUILD ")"};
}

// About: Back, with the version over it.
inline Vector<MenuItem> aboutItems()
{
    auto items = Vector<MenuItem> {};
    items.add({backLabel, versionText()});
    return items;
}

// Dress Your Cow: a row per item class, on what `skin` wears.
inline Vector<EditorRow> dressRows(const CowSkin& skin)
{
    auto rows = Vector<EditorRow> {};

    for (const auto& item: itemClasses())
        rows.add({item.name, item.choices, item.choice(skin)});

    return rows;
}

// Settings' quality row: the quality choices, Auto naming the tier it measured
// once it has one.
inline EditorRow qualityRow(QualityChoice chosen, Quality measured, bool measuring)
{
    auto row = EditorRow {qualityRowName, {}, (int) chosen};

    for (auto index = 0; index < qualityChoices; ++index)
        row.choices.add(choiceLabel((QualityChoice) index));

    if (chosen == QualityChoice::Auto && !measuring)
        row.choices[0] =
            autoPrefix + choiceLabel((QualityChoice) ((int) measured + 1));

    return row;
}
inline EditorRow frameRateRow(FrameRate chosen)
{
    auto row = EditorRow {frameRateRowName, {}, (int) chosen};

    for (auto index = 0; index < frameRates; ++index)
        row.choices.add(frameRateLabel((FrameRate) index));

    return row;
}

// Settings: quality, then frame rate.
inline Vector<EditorRow> settingsRows(QualityChoice chosen,
                                      Quality measured,
                                      bool measuring,
                                      FrameRate frameRate)
{
    auto rows = Vector<EditorRow> {};
    rows.add(qualityRow(chosen, measured, measuring));
    rows.add(frameRateRow(frameRate));
    return rows;
}
} // namespace Cows
