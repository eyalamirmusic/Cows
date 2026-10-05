#include "Cow/Wardrobe.h"
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
                                     Hat::FrogHat,
                                     Hat::CowBucketHat,
                                     Hat::TrafficCone});

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

struct Shot final
{
    const char* name;
    Maths::Vec3 target;
    float yaw;
    float pitch;
    float distance;
    float lean = 0.f;
};

void shoot(const CowSkin& skin, const Shot& shot, const std::string& name)
{
    auto cow = Cow {};
    auto view = SnapshotView {makeCowMesh};
    auto pose = cow.freePose({}, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
    pose.lean = shot.lean;
    cow.addTo(view.batch, view.glows, makeCowParts(skin), pose);
    view.camera.target = shot.target;
    view.camera.yaw = shot.yaw;
    view.camera.pitch = shot.pitch;
    view.camera.distance = shot.distance;

    check(snapshot(view, 360.f, 270.f, name + shot.name).isValid());
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
            check(at.y > 0.f && at.y < 1.4f);
            check(std::abs(at.x) < 1.2f && std::abs(at.z) < 0.6f);
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

    auto head = Vec3 {1.3f, 1.9f, 0.f};
    auto body = Vec3 {0.f, 0.9f, 0.f};

    for (auto hat: hats)
        for (const auto& shot:
             {Shot {"", head, 0.9f, 0.12f, 2.6f},
              Shot {"-side", head, 0.f, 0.12f, 2.6f},
              Shot {"-front", head, halfPi, 0.12f, 2.6f},
              Shot {"-kiss", {1.25f, 1.75f, 0.f}, 1.2f, 0.08f, 2.6f, 1.f}})
            shoot(wearing(hat),
                  shot,
                  "actors-hat-" + std::string {Miro::enumToString(hat)});

    for (auto pants: {Pants::BothLegs, Pants::BackLegs})
        for (const auto& shot:
             {Shot {"-side", body, 0.f, 0.12f, 5.f},
              Shot {"", body, 0.6f, 0.12f, 5.f},
              Shot {"-low", {0.f, 0.7f, 0.f}, -0.8f, -0.02f, 3.6f},
              Shot {"-back", body, -halfPi, 0.1f, 4.f},
              Shot {"-high", {-0.6f, 1.f, 0.f}, -0.9f, 0.6f, 3.8f}})
            shoot(dressed(Hat::None, pants),
                  shot,
                  "actors-pants-" + std::string {Miro::enumToString(pants)});
};

auto tCoarserMeshes = test("Wardrobe/coarserMeshesHaveFewerTriangles") = []
{
    auto coarse = cowMeshes(0.5f);

    for (auto shape: {Shape::Heart,
                      Shape::Belly,
                      Shape::BellyBand,
                      Shape::Seat,
                      Shape::SeatBand,
                      Shape::BucketCrown,
                      Shape::BucketBrim})
    {
        auto fine = makeCowMesh(shape).indices.size();
        auto cut = coarse(shape).indices.size();
        check(cut > 0);
        check(cut < fine);
    }
};
