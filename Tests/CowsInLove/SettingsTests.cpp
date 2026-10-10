#include "Settings.h"

#include <NanoTest/NanoTest.h>
#include <eacp/Core/Utils/Files.h>

#include <chrono>
#include <string>

using namespace nano;
using namespace Cows;

namespace
{
FilePath scratchDirectory()
{
    auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return FilePath::tempDirectory() / ("SettingsTests-" + std::to_string(stamp));
}

void writeText(const FilePath& file, const std::string& text)
{
    Files::createDirectories(file.parentDirectory());
    Files::writeFile(
        file, {reinterpret_cast<const std::uint8_t*>(text.data()), text.size()});
}

Settings dressedAndChosen()
{
    auto settings = Settings {};
    settings.skin.hat = Hat::WizardHat;
    settings.skin.pants = Pants::BackLegs;
    settings.quality.chosen = QualityChoice::Medium;
    settings.quality.measured = Quality::Low;
    return settings;
}
} // namespace

auto tSettingsDefaults = test("Settings/defaultsAreBareAndAuto") = []
{
    auto settings = Settings {};
    check(settings.skin == CowSkin {});
    check(settings.quality.chosen == QualityChoice::Auto);
    check(!settings.quality.measured.has_value());
};

auto tSettingsRoundTrip = test("Settings/saveThenLoadThroughTheFile") = []
{
    auto directory = scratchDirectory() / "Cows In Love";
    auto file = settingsFile(directory);

    check(file == directory / "settings.json");
    check(loadSettings(file) == Settings {});
    check(saveSettings(dressedAndChosen(), file));
    check(loadSettings(file) == dressedAndChosen());
    check(Files::readFile(file) == toJSON(dressedAndChosen()) + "\n");

    Files::removeAll(directory.parentDirectory());
};

auto tSavedNames = test("Settings/savesEnumeratorNames") = []
{
    auto json = toJSON(dressedAndChosen());

    check(json.find("\"hat\": \"WizardHat\"") != std::string::npos);
    check(json.find("\"chosen\": \"Medium\"") != std::string::npos);
    check(json.find("\"measured\": \"Low\"") != std::string::npos);
};

auto tUnmeasured = test("Settings/unmeasuredIsNull") = []
{
    auto settings = Settings {};
    check(toJSON(settings).find("\"measured\": null") != std::string::npos);
    check(settingsFromJSON(toJSON(settings)) == settings);
};

auto tUnknownNames = test("Settings/unknownNamesAreTheDefaults") = []
{
    auto settings = settingsFromJSON(
        R"({"skin": {"hat": "Sombrero", "pants": "BackLegs"},
            "quality": {"chosen": "Ultra", "measured": 7}})");

    check(settings.skin.hat == Hat::None);
    check(settings.skin.pants == Pants::BackLegs);
    check(settings.quality.chosen == QualityChoice::Auto);
    check(!settings.quality.measured.has_value());
};

auto tSettingsGarbage = test("Settings/garbageFileIsTheDefault") = []
{
    auto directory = scratchDirectory();
    auto file = settingsFile(directory);

    writeText(file, "\x01\xff{{{ moo");
    check(loadSettings(file) == Settings {});

    Files::removeAll(directory);
};

auto tSettingsAppFile = test("Settings/appFileIsInTheSupportDirectory") = []
{ check(settingsFile() == FilePath::appSupportDirectory() / "settings.json"); };
