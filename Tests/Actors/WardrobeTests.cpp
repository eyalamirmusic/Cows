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
        {
            auto cow = Cow {};
            auto view = SnapshotView {makeHeart()};
            cow.addTo(view.batch,
                      view.glows,
                      makeCowParts(wearing(hat)),
                      cow.freePose({}, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f));
            view.camera.target = {1.3f, 1.9f, 0.f};
            view.camera.yaw = angle.yaw;
            view.camera.pitch = 0.12f;
            view.camera.distance = 2.6f;

            auto name =
                "actors-hat-" + std::string {Miro::enumToString(hat)} + angle.name;
            check(snapshot(view, 360.f, 270.f, name).isValid());
        }
};
