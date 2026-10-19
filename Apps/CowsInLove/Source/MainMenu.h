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

inline Vector<MenuItem> makeAboutItems()
{
    auto items = Vector<MenuItem> {};
    items.add({backLabel, getVersionText()});
    return items;
}

inline Vector<EditorRow> makeDressRows(const CowSkin& skin)
{
    auto rows = Vector<EditorRow> {};

    for (const auto& item: itemClasses())
        rows.add({item.name, item.choices, item.choice(skin)});

    return rows;
}

inline EditorRow
    makeQualityRow(QualityChoice chosen, Quality measured, bool measuring)
{
    auto row = EditorRow {};
    row.name = qualityRowName;
    auto namesMeasuredTier = chosen == QualityChoice::Auto && !measuring;
    auto measuredLabel = getQualityChoiceLabel(getQualityChoiceFor(measured));

    for (const auto& entry: qualityChoicesAndLabels)
    {
        if (entry.choice == chosen)
            row.choice = row.choices.size();

        if (entry.choice == QualityChoice::Auto && namesMeasuredTier)
            row.choices.add(autoPrefix + measuredLabel);
        else
            row.choices.add(std::string {entry.label});
    }

    return row;
}

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
