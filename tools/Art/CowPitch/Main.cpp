// CowPitch <dir> [seeds]: renders the cow redesign pitch's mockups (docs/design/cow)
// through the tests' SnapshotView, in the game's lighting; `seeds` renders the
// plush and storybook cows at eight spot seeds instead. Never shipped.

#include "Designs.h"
#include "Refinements.h"

#include "Animation/Choreography.h"
#include "Cow/Wardrobe.h"
#include "Render/Palette.h"
#include "Snapshot.h"

#include <eacp/Core/Core.h>

#include <cmath>
#include <map>
#include <string>

using namespace Cows;
using namespace Cows::Pitch;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
constexpr Vec3 todaysCrown {1.45f, 2.f, 0.f};
constexpr auto kissReach = 1.98f;
constexpr auto todaysMuzzleTip = 2.01f;
constexpr auto tileWidth = 600.f;
constexpr auto tileHeight = 450.f;
constexpr auto farWidth = 200.f;
constexpr auto farHeight = 150.f;
constexpr Vec3 secondSeed {3.7f, 1.3f, 5.1f};

constexpr auto front = Shot {{0.6f, 1.3f, 0.f}, halfPi, 0.1f, 6.f};
constexpr auto side = Shot {{0.45f, 1.2f, 0.f}, 0.f, 0.06f, 7.f};
constexpr auto threeQuarter = Shot {{0.5f, 1.25f, 0.f}, 0.8f, 0.15f, 6.5f};
constexpr auto behind = Shot {{0.f, 1.05f, 0.f}, -0.85f, 0.2f, 6.f};
constexpr auto kiss = Shot {{0.f, 1.5f, 0.f}, 0.22f, 0.08f, 9.5f};
constexpr auto far = Shot {{0.5f, 1.2f, 0.f}, 0.8f, 0.12f, 16.f};

Mat4 about(Vec3 pivot, const Mat4& rotation)
{
    return Mat4::translation(pivot) * rotation * Mat4::translation(-pivot);
}

// Cow::addTo with the design's own pivots, at hop 0 and no moo.
void addCow(SnapshotView& view,
            const Design& design,
            const Vector<CowPart>& parts,
            const CowPose& pose,
            float facing,
            Vec3 spotSeed,
            bool silhouette)
{
    auto nod = 0.06f * (pose.hop - 0.5f) - 0.12f * pose.lean;
    auto head = about(design.neckPivot,
                      Mat4::rotationX(0.2f * pose.lean) * Mat4::rotationZ(nod));
    auto swish = std::sin(twoPi * pose.seconds / 1.3f + facing);
    auto tail =
        about(design.tailPivot,
              Mat4::rotationX(0.45f * swish) * Mat4::rotationZ(0.1f * swish));
    auto eye = Mat4::scale(1.f + 0.24f * pose.beat);
    auto seed = Mat4::translation(spotSeed);

    for (const auto& part: parts)
    {
        auto local = part.transform;

        if (part.bone == Bone::Head)
            local = head * part.transform;
        else if (part.bone == Bone::Tail)
            local = tail * part.transform;
        else if (part.bone == Bone::Eye)
            local = head * part.transform * eye;

        auto material = part.material;

        if (silhouette)
        {
            material = Material {};
            material.emission = 1.f;
            material.mist = 0.f;
        }
        else if (part.bone == Bone::Eye)
        {
            material.emission = 1.2f + 0.8f * std::max(pose.beat, 0.f);
            auto center = transformPoint(pose.world * local, {0.f, 0.f, 0.3f});
            auto glow =
                Palette::linear(Palette::heart) * (0.22f + 0.16f * pose.beat);
            view.glows.add(makeGlow(center, 0.24f + 0.05f * pose.beat, glow));
        }

        view.batch.add(
            part.shape,
            makeInstance(pose.world * local,
                         material,
                         seed * Mat4::scale(part.spotScale) * part.transform));
    }
}

int nearestLeg(const std::array<Mat4, 4>& legs, Vec3 at)
{
    auto best = 0;
    auto bestDistance = 1e9f;

    for (auto index = 0; index < 4; ++index)
    {
        auto foot = legs[(size_t) index].column(3);
        auto gap = std::hypot(foot.x - at.x, foot.z - at.z);

        if (gap < bestDistance)
        {
            bestDistance = gap;
            best = index;
        }
    }

    return best;
}

// The game's hats and pants moved onto `design`: the hats by where its crown is
// and how big its head is, the pants by its torso and legs against today's.
Vector<CowPart> dressed(const Design& design, Hat hat, Pants pants)
{
    auto parts = design.parts;

    auto hatParts = Vector<CowPart> {};
    addHat(hatParts, hat);
    auto seat = Mat4::translation(design.crown) * Mat4::scale(design.headScale)
                * Mat4::translation(-todaysCrown);

    for (auto part: hatParts)
    {
        part.transform = seat * part.transform;
        parts.add(part);
    }

    if (!design.barrelBody)
        return parts;

    auto back = legPlacements(LegPair::Back);
    auto frontLegs = legPlacements(LegPair::Front);
    auto todaysLegs =
        std::array<Mat4, 4> {back[0], back[1], frontLegs[0], frontLegs[1]};
    auto torso = design.torso * torsoPlacement().inverted();

    auto pantsParts = Vector<CowPart> {};
    addPants(pantsParts, pants);

    for (auto part: pantsParts)
    {
        if (part.shape == Shape::Capsule)
        {
            auto leg = nearestLeg(todaysLegs, part.transform.column(3).xyz());
            part.transform = design.legs[(size_t) leg]
                             * todaysLegs[(size_t) leg].inverted() * part.transform;
        }
        else
            part.transform = torso * part.transform;

        parts.add(part);
    }

    return parts;
}

Material skyBackdrop(bool silhouette)
{
    auto material =
        Material {Palette::linear(silhouette ? Palette::hide : Palette::sky)};
    material.emission = 1.f;
    material.mist = 0.f;
    return material;
}

void stage(SnapshotView& view, const Shot& shot, bool silhouette)
{
    view.camera.target = shot.target;
    view.camera.yaw = shot.yaw;
    view.camera.pitch = shot.pitch;
    view.camera.distance = shot.distance;

    auto eye = view.camera.eye();
    auto away = normalize(Vec3 {shot.target.x - eye.x, 0.f, shot.target.z - eye.z});
    auto backdrop = shot.target + away * 150.f - Vec3 {0.f, 100.f, 0.f};
    view.batch.add(Shape::Box,
                   makeInstance(Mat4::translation(backdrop) * Mat4::scale(200.f),
                                skyBackdrop(silhouette)));

    if (silhouette)
        return;

    auto grass = Material {Palette::linear(0x5fa83a)};
    grass.gloss = 0.05f;
    view.batch.add(Shape::Cylinder,
                   makeInstance(Mat4::translation({0.f, -0.05f, 0.f})
                                    * Mat4::scale({120.f, 0.05f, 120.f}),
                                grass));
}

void addContactShadow(SnapshotView& view, Vec3 at, float length, float width)
{
    auto shade = Material {Palette::linear(0x0b2a08)};
    shade.alpha = 0.4f;
    shade.gloss = 0.f;
    view.translucent.add(Shape::Sphere,
                         makeInstance(Mat4::translation({at.x, 0.005f, at.z})
                                          * Mat4::scale({length, 0.004f, width}),
                                      shade));
}

CowPose restPose()
{
    auto pose = CowPose {};
    pose.hop = 0.f;
    return pose;
}

void save(const Graphics::Image& image, const std::string& path)
{
    if (!image.isValid())
    {
        LOG("could not render ", path);
        return;
    }

    image.save(FilePath {path});
    LOG(path);
}

Graphics::Image render(SnapshotView& view, float width, float height)
{
    view.setBounds({0.f, 0.f, width, height});
    return view.renderToImage(1.f);
}

void single(const Design& design,
            const Vector<CowPart>& parts,
            const Shot& shot,
            const std::string& path,
            bool silhouette = false,
            float width = tileWidth,
            float height = tileHeight)
{
    auto view = SnapshotView {makeCowMesh};
    stage(view, shot, silhouette);

    if (!silhouette)
        addContactShadow(view, {0.1f, 0.f, 0.f}, 1.5f, 0.75f);

    addCow(view, design, parts, restPose(), -1.f, design.spotSeed, silhouette);
    save(render(view, width, height), path);
}

void kissing(const Design& design, const std::string& path)
{
    auto view = SnapshotView {makeCowMesh};
    stage(view, kiss, false);

    auto reach = design.muzzleTip - (todaysMuzzleTip - kissReach);
    auto at = Choreography::kissTime(0);

    for (auto facing: {-1.f, 1.f})
    {
        auto cow = Cow {facing, {}};
        auto pose = cow.pose(at);
        pose.world =
            Mat4::translation({facing * (reach - kissReach), 0.f, 0.f}) * pose.world;
        addContactShadow(view, pose.world.column(3).xyz(), 1.5f, 0.75f);
        addCow(view,
               design,
               dressed(design, Hat::None, Pants::None),
               pose,
               facing,
               facing < 0.f ? design.spotSeed : design.spotSeed + secondSeed,
               false);
    }

    save(render(view, tileWidth, tileHeight), path);
}

int triangles(Shape shape)
{
    static auto counts = std::map<Shape, int> {};

    if (auto found = counts.find(shape); found != counts.end())
        return found->second;

    auto mesh = MeshData {};

    switch (shape)
    {
        case Shape::Sphere:
            mesh = makeSphere(32, 48);
            break;
        case Shape::Capsule:
            mesh = makeCapsule(0.14f, 32);
            break;
        case Shape::Horn:
            mesh = makeHorn(24);
            break;
        case Shape::Barrel:
            mesh = makeBarrel(48);
            break;
        case Shape::Box:
            mesh = makeBox();
            break;
        case Shape::Cylinder:
            mesh = makeCylinder(32);
            break;
        case Shape::Cone:
            mesh = makeCone(32);
            break;
        default:
            mesh = makeCowMesh(shape);
            break;
    }

    auto count = (int) mesh.indices.size() / 3;
    counts[shape] = count;
    return count;
}

int triangles(const Vector<CowPart>& parts)
{
    auto total = 0;

    for (const auto& part: parts)
        total += triangles(part.shape);

    return total;
}

void renderDesign(const Design& design, const std::string& directory)
{
    auto at = [&](const std::string& view)
    { return directory + "/" + design.name + "-" + view + ".png"; };

    single(design, design.parts, front, at("front"));
    single(design, design.parts, side, at("side"));
    single(design, design.parts, threeQuarter, at("three-quarter"));
    single(design, design.parts, side, at("silhouette"), true);
    single(design, design.parts, far, at("far"), false, farWidth, farHeight);
    kissing(design, at("kiss"));
    single(design,
           dressed(design, Hat::CowboyHat, Pants::BothLegs),
           threeQuarter,
           at("wardrobe"));
    single(design,
           dressed(design, Hat::WizardHat, Pants::BackLegs),
           behind,
           at("wardrobe-back"));

    LOG(design.name,
        ": ",
        (int) design.parts.size(),
        " parts, ",
        triangles(design.parts),
        " triangles");
}

// The refinements worth doing, together.
const auto chosen = Vector<std::string> {"ears", "muzzle", "tail-tuft"};

void renderRefinements(const std::string& directory)
{
    auto today = currentCow();

    for (const auto& refinement: refinements())
    {
        auto after = refined({refinement.name});
        single(today,
               today.parts,
               refinement.shot,
               directory + "/before-" + refinement.name + ".png");
        single(after,
               after.parts,
               refinement.shot,
               directory + "/after-" + refinement.name + ".png");
        LOG(refinement.name,
            ": ",
            triangles(after.parts) - triangles(today.parts),
            " triangles");
    }

    renderDesign(refined(chosen), directory);
}
} // namespace

int main(int argc, char* argv[])
{
    auto directory = std::string {argc > 1 ? argv[1] : "."};
    auto exitCode = 0;

    Apps::run(
        [&]
        {
            if (!hasDevice())
            {
                LOG("CowPitch: no GPU");
                exitCode = 1;
                return;
            }

            if (argc > 2 && std::string {argv[2]} == "seeds")
            {
                for (auto design: {plushCow(), storybookCow()})
                    for (auto index = 0; index < 8; ++index)
                    {
                        design.spotSeed = {1.7f * (float) index,
                                           0.9f * (float) index,
                                           2.3f * (float) index};
                        single(design,
                               design.parts,
                               threeQuarter,
                               directory + "/seed-" + design.name + "-"
                                   + std::to_string(index) + ".png");
                    }
                return;
            }

            for (const auto& design:
                 {currentCow(), plushCow(), storybookCow(), terminalCow()})
                renderDesign(design, directory);

            renderRefinements(directory);
        });
    return exitCode;
}
