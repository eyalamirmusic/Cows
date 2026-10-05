#include "Cow/Wardrobe.h"
#include "Cow/HeartMesh.h"
#include "Snapshot.h"

#include <NanoTest/NanoTest.h>

#include <string>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
constexpr auto hats = std::to_array({Hat::TopHat,
                                     Hat::CowboyHat,
                                     Hat::PartyHat,
                                     Hat::Beanie,
                                     Hat::Crown,
                                     Hat::FrogHat});

CowSkin wearing(Hat hat)
{
    auto skin = CowSkin {};
    skin.hat = hat;
    return skin;
}

Vector<CowPart> hatParts(Hat hat)
{
    auto parts = Vector<CowPart> {};
    addHat(parts, hat);
    return parts;
}
} // namespace

auto tNoHat = test("Wardrobe/noHatIsTheBareCow") = []
{
    check(hatParts(Hat::None).empty());
    check(makeCowParts(CowSkin {}).size() == makeCowParts().size());
};

auto tHatsOnHead = test("Wardrobe/everyHatSitsOnTheHead") = []
{
    for (auto hat: hats)
    {
        auto parts = hatParts(hat);
        check(!parts.empty());
        check(makeCowParts(wearing(hat)).size()
              == makeCowParts().size() + parts.size());

        for (const auto& part: parts)
        {
            auto at = transformPoint(part.transform, {});
            check(part.bone == Bone::Head);
            check(at.y > 1.3f && at.y < 2.6f);
            check(at.x > 1.f && at.x < 1.8f);
            check(std::abs(at.z) < 0.4f);
        }
    }
};

CowSkin dressed(Hat hat, Pants pants)
{
    auto skin = wearing(hat);
    skin.pants = pants;
    return skin;
}

void shoot(const CowSkin& skin,
           Maths::Vec3 target,
           float yaw,
           float distance,
           const std::string& name)
{
    auto cow = Cow {};
    auto view = SnapshotView {makeHeart()};
    cow.addTo(view.batch,
              view.glows,
              makeCowParts(skin),
              cow.freePose({}, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f));
    view.camera.target = target;
    view.camera.yaw = yaw;
    view.camera.pitch = 0.12f;
    view.camera.distance = distance;

    check(snapshot(view, 360.f, 270.f, name).isValid());
}

auto tPantsOnBody = test("Wardrobe/pantsSitOnTheBody") = []
{
    auto parts = Vector<CowPart> {};
    addPants(parts, Pants::None);
    check(parts.empty());

    for (auto pants: {Pants::BothLegs, Pants::BackLegs})
    {
        auto worn = Vector<CowPart> {};
        addPants(worn, pants);
        check(!worn.empty());

        for (const auto& part: worn)
        {
            auto at = transformPoint(part.transform, {});
            check(part.bone == Bone::Body);
            check(at.y > 0.2f && at.y < 1.4f);
        }
    }

    auto both = Vector<CowPart> {};
    auto back = Vector<CowPart> {};
    addPants(both, Pants::BothLegs);
    addPants(back, Pants::BackLegs);
    check(both.size() > back.size());
    check(makeCowParts(dressed(Hat::Crown, Pants::BothLegs)).size()
          == makeCowParts(wearing(Hat::Crown)).size() + both.size());
};

auto tHatSnapshots = test("Wardrobe/snapshots") = []
{
    if (!hasDevice())
        return;

    struct Angle final
    {
        const char* name;
        float yaw;
    };

    for (auto hat: hats)
        for (auto angle: {Angle {"", 0.9f},
                          Angle {"-side", 0.f},
                          Angle {"-front", Maths::halfPi}})
            shoot(wearing(hat),
                  {1.3f, 1.9f, 0.f},
                  angle.yaw,
                  2.6f,
                  "actors-hat-" + std::string {Miro::enumToString(hat)}
                      + angle.name);

    for (auto pants: {Pants::BothLegs, Pants::BackLegs})
        for (auto angle: {Angle {"-side", 0.f}, Angle {"", 0.6f}})
            shoot(dressed(Hat::None, pants),
                  {0.f, 0.9f, 0.f},
                  angle.yaw,
                  5.f,
                  "actors-pants-" + std::string {Miro::enumToString(pants)}
                      + angle.name);
};
