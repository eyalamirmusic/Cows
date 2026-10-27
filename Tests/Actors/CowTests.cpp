#include "Animation/Choreography.h"
#include "Cow/Cow.h"
#include "Cow/Wardrobe.h"
#include "Snapshot.h"
#include "TestCow.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
bool finite(const Mat4& matrix)
{
    for (auto value: matrix.values)
        if (!std::isfinite(value))
            return false;

    return true;
}

bool finite(const SurfaceBatch& batch)
{
    for (const auto& list: batch.lists)
        for (const auto& instance: list)
            for (const auto& column: {instance.model0,
                                      instance.model1,
                                      instance.model2,
                                      instance.model3})
                if (!std::isfinite(column.x) || !std::isfinite(column.y)
                    || !std::isfinite(column.z) || !std::isfinite(column.w))
                    return false;

    return true;
}

int instanceCount(const SurfaceBatch& batch)
{
    auto count = 0;

    for (const auto& list: batch.lists)
        count += (int) list.size();

    return count;
}

int posedCount(const Cow& cow, const Vector<CowPart>& parts, const CowPose& pose)
{
    auto batch = SurfaceBatch {};
    auto glows = Vector<GlowInstance> {};
    getCowModelForTests().addPosedCowTo(batch, glows, cow, parts, pose);
    return instanceCount(batch);
}

CowPose standingStill(const Cow& cow)
{
    return getCowModelForTests().makeRoamingPose(
        cow, {}, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
}

bool isCowColour(const Graphics::Color& color)
{
    auto grey =
        std::abs(color.r - color.g) < 0.12f && std::abs(color.g - color.b) < 0.12f;
    return !isClearColor(color) && grey;
}
} // namespace

auto tFactoryMakesTheCow = test("Cow/theFactoryMakesTheCow") = []
{
    const auto& model = getCowModelForTests();
    auto jump = model.getJumpPhysics();

    check(std::abs(jump.launchSpeedMetresPerSecond - 12.f) < 1e-4f);
    check(std::abs(jump.gravityMetresPerSecondSquared - 30.f) < 1e-4f);
    check(model.makeParts().size() == 33);
};

auto tFreePoseFinite = test("Cow/freePoseIsFinite") = []
{
    const auto& model = getCowModelForTests();
    auto cow = Cow {};

    for (auto seconds = 0.f; seconds < 6.f; seconds += 0.37f)
    {
        auto pose = model.makeRoamingPose(
            cow, {3.f, 0.f, -2.f}, seconds, seconds, 1.f, seconds, seconds, 1.f);
        check(finite(pose.world));
        check(std::isfinite(pose.hop) && std::isfinite(pose.beat));
    }
};

auto tKissPoseFinite = test("Cow/kissPoseIsFinite") = []
{
    const auto& model = getCowModelForTests();

    for (auto side: {-1.f, 1.f})
    {
        auto cow = Cow {side, {}};

        for (auto kiss = 0; kiss < 3; ++kiss)
        {
            auto pose = model.makeEndingPose(cow, Choreography::kissTime(kiss), {});
            check(finite(pose.world));
            check(std::isfinite(pose.lean));

            auto batch = SurfaceBatch {};
            auto glows = Vector<GlowInstance> {};
            model.addPosedCowTo(batch, glows, cow, model.makeParts(), pose);
            check(finite(batch));
        }
    }
};

auto tConstantInstances = test("Cow/instanceCountIsConstant") = []
{
    const auto& model = getCowModelForTests();
    auto cow = Cow {};
    auto parts = model.makeParts();
    auto expected = (int) parts.size();

    check(expected > 0);

    for (auto seconds = 0.f; seconds < 12.f; seconds += 0.53f)
    {
        check(posedCount(cow, parts, model.makeEndingPose(cow, seconds, {}))
              == expected);
        check(posedCount(cow,
                         parts,
                         model.makeRoamingPose(
                             cow, {}, 1.f, seconds, 0.5f, seconds, seconds, 1.f))
              == expected);
    }
};

auto tCowSnapshot = test("Cow/snapshot") = []
{
    if (!hasDevice())
        return;

    const auto& model = getCowModelForTests();
    auto cow = Cow {};
    auto view = SnapshotView {cowMeshes(1.f)};
    model.addPosedCowTo(
        view.batch, view.glows, cow, model.makeParts(), standingStill(cow));
    view.camera.target = {0.f, 1.6f, 0.f};
    view.camera.yaw = 0.35f;
    view.camera.pitch = 0.15f;
    view.camera.distance = 6.5f;

    auto image = snapshot(view, 240.f, 180.f, "actors-cow");
    check(image.isValid());

    if (!image.isValid())
        return;

    auto width = image.width();
    auto height = image.height();
    auto cowPixels = 0;

    for (auto y = height * 3 / 8; y < height * 5 / 8; ++y)
        for (auto x = width * 3 / 8; x < width * 5 / 8; ++x)
            if (isCowColour(image.at(x, y)))
                ++cowPixels;

    check(cowPixels > 20);
    check(isClearColor(image.at(1, 1)));
    check(isClearColor(image.at(width - 2, 1)));
    check(isClearColor(image.at(1, height - 2)));
    check(isClearColor(image.at(width - 2, height - 2)));
};

namespace
{
bool isEyePink(const Graphics::Color& color)
{
    return color.r > 0.75f && color.g < 0.3f && color.b > 0.1f && color.b < 0.6f;
}

int eyePixels(const Graphics::Image& image)
{
    auto count = 0;

    for (auto y = 0; y < image.height(); ++y)
        for (auto x = 0; x < image.width(); ++x)
            if (isEyePink(image.at(x, y)))
                ++count;

    return count;
}

Graphics::Image cowSeenFrom(float distance, const std::string& name)
{
    const auto& model = getCowModelForTests();
    auto cow = Cow {};
    auto view = SnapshotView {cowMeshes(1.f)};
    model.addPosedCowTo(
        view.batch, view.glows, cow, model.makeParts(), standingStill(cow));
    view.camera.target = {1.2f, 1.6f, 0.f};
    view.camera.yaw = 1.15f;
    view.camera.pitch = 0.05f;
    view.camera.distance = distance;

    return snapshot(view, 960.f, 720.f, name);
}
} // namespace

auto tEyesInTheMist = test("Cow/eyesFadeIntoTheMist") = []
{
    if (!hasDevice())
        return;

    auto near = cowSeenFrom(6.5f, "actors-cow-mist-near");
    auto far = cowSeenFrom(45.f, "actors-cow-mist-far");
    check(near.isValid() && far.isValid());

    if (!near.isValid() || !far.isValid())
        return;

    check(eyePixels(near) > 20);
    check(eyePixels(far) == 0);
};
