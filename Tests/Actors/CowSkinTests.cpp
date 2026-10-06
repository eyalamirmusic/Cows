#include "Cow/CowSkin.h"

#include <NanoTest/NanoTest.h>
#include <eacp/Core/Utils/Files.h>

#include <chrono>
#include <set>
#include <string>

using namespace nano;
using namespace Cows;

namespace
{
CowSkin wearing(Hat hat)
{
    auto skin = CowSkin {};
    skin.hat = hat;
    return skin;
}

FilePath scratchDirectory()
{
    auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return FilePath::tempDirectory() / ("CowSkinTests-" + std::to_string(stamp));
}

std::set<std::string> savedKeys()
{
    auto keys = std::set<std::string> {};
    auto json = Miro::toJSON(CowSkin {});

    for (const auto& [key, value]: json.asObject())
        keys.insert(key);

    return keys;
}
} // namespace

auto tDefaultSkin =
    test("CowSkin/defaultWearsNothing") = [] { check(CowSkin {}.hat == Hat::None); };

auto tRoundTrip = test("CowSkin/jsonRoundTrip") = []
{
    for (auto hat: {Hat::None,
                    Hat::TopHat,
                    Hat::CowboyHat,
                    Hat::PartyHat,
                    Hat::Beanie,
                    Hat::Crown,
                    Hat::FrogHat,
                    Hat::CowBucketHat,
                    Hat::TrafficCone,
                    Hat::WizardHat})
        check(cowSkinFromJSON(toJSON(wearing(hat))) == wearing(hat));
};

auto tSavesNames = test("CowSkin/savesEnumeratorNames") = []
{
    auto json = toJSON(wearing(Hat::TopHat));

    check(json.find("\"hat\": \"TopHat\"") != std::string::npos);
    check(json.find('\n') != std::string::npos);
};

auto tStableNames = test("CowSkin/hatNamesAreStable") = []
{
    auto names = Miro::enumNames<Hat>();

    check(names.size() == 10);
    check(names[0] == "None");
    check(names[1] == "TopHat");
    check(names[2] == "CowboyHat");
    check(names[3] == "PartyHat");
    check(names[4] == "Beanie");
    check(names[5] == "Crown");
    check(names[6] == "FrogHat");
    check(names[7] == "CowBucketHat");
    check(names[8] == "TrafficCone");
    check(names[9] == "WizardHat");
};

auto tStablePantsNames = test("CowSkin/pantsNamesAreStable") = []
{
    auto names = Miro::enumNames<Pants>();

    check(names.size() == 3);
    check(names[0] == "None");
    check(names[1] == "BothLegs");
    check(names[2] == "BackLegs");

    const auto& pants = itemClasses()[1];
    check(pants.key == "pants");
    check(pants.name == "Pants");
    check(pants.choices.size() == 3);
    check(pants.choices[1] == "Both Legs");
    check(pants.choices[2] == "Back Legs");
};

auto tPantsRoundTrip = test("CowSkin/pantsRoundTripBesideTheHat") = []
{
    auto skin = wearing(Hat::FrogHat);
    skin.pants = Pants::BackLegs;
    auto json = toJSON(skin);

    check(json.find("\"pants\": \"BackLegs\"") != std::string::npos);
    check(cowSkinFromJSON(json) == skin);
    check(stepped(skin, 1, 1).pants == Pants::None);
    check(stepped(skin, 1, 1).hat == Hat::FrogHat);

    auto onlyPants = cowSkinFromJSON(R"({"pants": "BothLegs", "hat": "Fez"})");
    check(onlyPants.pants == Pants::BothLegs);
    check(onlyPants.hat == Hat::None);
};

auto tLoadsNames = test("CowSkin/loadsByName") = []
{
    check(cowSkinFromJSON(R"({"hat": "Crown"})").hat == Hat::Crown);
    check(cowSkinFromJSON(R"({"hat": "Beanie", "later": "ignored"})").hat
          == Hat::Beanie);
};

auto tFallsBack = test("CowSkin/badInputIsTheDefault") = []
{
    for (auto text: {"",
                     "not json",
                     "{\"hat\": ",
                     "[1, 2, 3]",
                     "{}",
                     "{\"hat\": \"Sombrero\"}",
                     "{\"hat\": \"tophat\"}",
                     "{\"hat\": 99}",
                     "{\"hat\": -1}",
                     "{\"hat\": true}",
                     "{\"hat\": null}",
                     "{\"hat\": {\"name\": \"Crown\"}}",
                     "{\"pants\": \"Shorts\"}",
                     "{\"pants\": 7}"})
        check(cowSkinFromJSON(text) == CowSkin {});
};

auto tItemClassesCoverSkin = test("CowSkin/itemClassesCoverEveryField") = []
{
    auto keys = std::set<std::string> {};

    for (const auto& item: itemClasses())
    {
        check(!item.name.empty());
        check(!item.choices.empty());
        check(item.choice(CowSkin {}) == 0);
        keys.insert(item.key);
    }

    check((int) keys.size() == (int) itemClasses().size());
    check(keys == savedKeys());
};

auto tHatChoices = test("CowSkin/hatChoicesAreSpelledOut") = []
{
    const auto& hat = itemClasses()[0];

    check(hat.key == "hat");
    check(hat.name == "Hat");
    check(hat.choices.size() == 10);
    check(hat.choices[6] == "Frog Hat");
    check(hat.choices[7] == "Cow Bucket Hat");
    check(hat.choices[8] == "Traffic Cone");
    check(hat.choices[9] == "Wizard Hat");
    check(hat.choices[1] == "Top Hat");
    check(hat.choices[2] == "Cowboy Hat");
    check(hat.choice(wearing(Hat::PartyHat)) == 3);
};

auto tStepWraps = test("CowSkin/stepWrapsBothWays") = []
{
    check(stepped(CowSkin {}, 0, 1).hat == Hat::TopHat);
    check(stepped(CowSkin {}, 0, -1).hat == Hat::WizardHat);
    check(stepped(wearing(Hat::TrafficCone), 0, 1).hat == Hat::WizardHat);
    check(stepped(wearing(Hat::WizardHat), 0, 1).hat == Hat::None);
    check(stepped(CowSkin {}, 0, 11).hat == Hat::TopHat);
    check(stepped(wearing(Hat::Beanie), 5, 1) == wearing(Hat::Beanie));
    check(stepped(wearing(Hat::Beanie), -1, 1) == wearing(Hat::Beanie));
};

auto tSpelledOut = test("CowSkin/spelledOut") = []
{
    check(spelledOut("TopHat") == "Top Hat");
    check(spelledOut("None") == "None");
    check(spelledOut("") == "");
};

auto tFileRoundTrip = test("CowSkin/saveThenLoadThroughTheFile") = []
{
    auto directory = scratchDirectory() / "Cows In Love";
    auto file = cowSkinFile(directory);

    check(file == directory / "CowSkin.json");
    check(loadCowSkin(file) == CowSkin {});
    check(saveCowSkin(wearing(Hat::PartyHat), file));
    check(loadCowSkin(file) == wearing(Hat::PartyHat));
    check(Files::readFile(file) == toJSON(wearing(Hat::PartyHat)) + "\n");

    check(saveCowSkin(wearing(Hat::Crown), file));
    check(loadCowSkin(file) == wearing(Hat::Crown));

    check(saveCowSkin(wearing(Hat::WizardHat), file));
    check(Files::readFile(file).find("\"hat\": \"WizardHat\"") != std::string::npos);
    check(loadCowSkin(file) == wearing(Hat::WizardHat));

    Files::removeAll(directory.parentDirectory());
};

auto tGarbageFile = test("CowSkin/garbageFileIsTheDefault") = []
{
    auto directory = scratchDirectory();
    auto file = cowSkinFile(directory);
    auto garbage = std::string {"\x01\xff{{{ moo"};

    Files::createDirectories(directory);
    Files::writeFile(
        file,
        {reinterpret_cast<const std::uint8_t*>(garbage.data()), garbage.size()});
    check(loadCowSkin(file) == CowSkin {});

    Files::removeAll(directory);
};

auto tAppFile = test("CowSkin/appFileIsInTheSupportDirectory") = []
{ check(cowSkinFile() == FilePath::appSupportDirectory() / "CowSkin.json"); };
