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
inline Vector<MenuItem> makeMainMenuItems()
{
    auto items = Vector<MenuItem> {};
    items.add({startLabel, "", startScale});
    items.add({dressLabel});
    items.add({settingsLabel});
    items.add({aboutLabel, "", aboutScale});
    return items;
}

inline std::string getVersionText()
{
    return std::string {"Version " COWS_APP_VERSION " (Build " COWS_APP_BUILD ")"};
}

// About: Back, with the version over it.
inline Vector<MenuItem> makeAboutItems()
{
    auto items = Vector<MenuItem> {};
    items.add({backLabel, getVersionText()});
    return items;
}

// Dress Your Cow: a row per item class, on what `skin` wears.
inline Vector<EditorRow> makeDressRows(const CowSkin& skin)
{
    auto rows = Vector<EditorRow> {};

    for (const auto& item: itemClasses())
        rows.add({item.name, item.choices, item.choice(skin)});

    return rows;
}

// Settings' quality row: the quality choices, Auto naming the tier it measured
// once it has one.
inline EditorRow
    makeQualityRow(QualityChoice chosen, Quality measured, bool measuring)
{
    auto row = EditorRow {};
    row.name = qualityRowName;

    for (const auto& entry: qualityChoicesAndLabels)
    {
        if (entry.choice == chosen)
            row.choice = row.choices.size();

        row.choices.add(std::string {entry.label});
    }

    if (chosen == QualityChoice::Auto && !measuring)
    {
        auto measuredChoice = getQualityChoiceFor(measured);
        row.choices[0] = autoPrefix + getQualityChoiceLabel(measuredChoice);
    }

    return row;
}

// Settings' frame rate row: Auto, 60, Max.
inline EditorRow makeFrameRateRow(FrameRate chosen)
{
    auto row = EditorRow {};
    row.name = frameRateRowName;

    for (const auto& entry: frameRatesAndLabels)
    {
        if (entry.rate == chosen)
            row.choice = row.choices.size();

        row.choices.add(std::string {entry.label});
    }

    return row;
}

// Settings: quality, then frame rate.
inline Vector<EditorRow> makeSettingsRows(QualityChoice chosen,
                                          Quality measured,
                                          bool measuring,
                                          FrameRate frameRate)
{
    auto rows = Vector<EditorRow> {};
    rows.add(makeQualityRow(chosen, measured, measuring));
    rows.add(makeFrameRateRow(frameRate));
    return rows;
}
} // namespace Cows
