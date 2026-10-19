#include "Props/Barn.h"
#include "Props/Props.h"

#include <array>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
// Must match the numbers at the top of tools/blender/barn.py; the front faces -z.
constexpr auto barnHalfWidth = 8.f;
constexpr auto barnHalfDepth = 11.f;
constexpr auto barnWallThickness = 0.3f;
constexpr auto barnEaves = 6.2f;
constexpr auto barnRidge = 11.5f;
constexpr auto barnDoorHalfWidth = 2.5f;
constexpr auto barnDoorHeight = 4.5f;
constexpr auto barnDoorThickness = 0.15f;
constexpr auto barnTrimWidth = 0.25f;
constexpr auto barnTrimDepth = 0.06f;

constexpr auto barnDoorLeafWidth = barnDoorHalfWidth + 0.1f;

constexpr auto barnRoofBreakX = 4.8f;
constexpr auto barnRoofBreakHeight = 8.6f;
constexpr auto barnEavesOverhang = 0.4f;
constexpr auto barnGableOverhang = 0.5f;

constexpr auto barnLoftHeight = 5.f;
constexpr auto barnLoftThickness = 0.3f;
constexpr auto barnLoftDepth = 8.f;
constexpr auto barnRampWidth = 2.4f;
constexpr auto barnRampFootZ = 6.f;
constexpr auto barnLoftPostX = 3.f;
constexpr auto barnLoftPostRadius = 0.2f;

constexpr auto barnHayDoorTop = 8.4f;
constexpr auto barnHayLeafWidth = barnDoorHalfWidth;

constexpr auto barnLoftSpotInset = 1.4f;

constexpr std::array<float, 4> barnStallZs {-8.5f, -5.5f, -2.5f, 0.5f};
constexpr auto barnStallDepth = 2.4f;
constexpr auto barnStallHeight = 1.2f;
constexpr auto barnStallBoard = 0.15f;
constexpr auto barnTroughWidth = 0.8f;
constexpr auto barnTroughLength = 1.6f;
constexpr auto barnTroughHeight = 0.8f;

constexpr auto barnBaleWidth = 1.3f;
constexpr auto barnBaleHeight = 0.7f;
constexpr auto barnBaleLength = 2.5f;

// Each stack a jump above the last, the highest just under the eaves' edge.
constexpr auto barnClimbFront = -10.f;
constexpr auto barnClimbRampLength = 5.f;
constexpr auto barnClimbRampLayers = 3;
constexpr std::array<int, 4> barnClimbLayers {3, 5, 7, 8};

constexpr auto barnLoftStackLayers = 2;

// Towards the back, leaving the rest of the ridge to walk along.
constexpr auto barnCupolaZ = 7.5f;
constexpr auto barnCupolaHalf = 0.7f;
constexpr auto barnCupolaTop = 13.2f;

constexpr std::uint32_t wallColor = 0xb5473a;
constexpr std::uint32_t trimColor = 0xf1e8d6;
constexpr std::uint32_t roofColor = 0x5a4b4f;
constexpr std::uint32_t woodColor = 0xa9824f;
constexpr std::uint32_t hayColor = 0xe3bf5a;
constexpr std::uint32_t metalColor = 0x3d3b40;
constexpr std::uint32_t twineColor = 0x8c6a3c;
constexpr auto barnSoftness = 0.1f;
constexpr auto barnGloss = 0.1f;

Vec2 turned(Vec2 offset, int quarterTurns)
{
    for (auto turn = 0; turn < quarterTurns; ++turn)
        offset = {-offset.y, offset.x};

    return offset;
}

float roofHeightAt(float x)
{
    auto across = std::abs(x);

    if (across <= barnRoofBreakX)
        return barnRidge
               - across * (barnRidge - barnRoofBreakHeight) / barnRoofBreakX;

    auto lowerSlope =
        (barnRoofBreakHeight - barnEaves) / (barnHalfWidth - barnRoofBreakX);
    return barnRoofBreakHeight - (across - barnRoofBreakX) * lowerSlope;
}

struct BarnPart final
{
    Shape shape;
    std::uint32_t color;
};

constexpr std::array<BarnPart, 7> barnParts {{{Shape::BarnWall, wallColor},
                                              {Shape::BarnTrim, trimColor},
                                              {Shape::BarnRoof, roofColor},
                                              {Shape::BarnWood, woodColor},
                                              {Shape::BarnHay, hayColor},
                                              {Shape::BarnMetal, metalColor},
                                              {Shape::BarnTwine, twineColor}}};

constexpr auto inside = barnHalfWidth - barnWallThickness;
constexpr auto backInside = barnHalfDepth - barnWallThickness;
constexpr auto loftEdge = -barnHalfDepth + barnLoftDepth;
constexpr auto frontInside = -barnHalfDepth + barnWallThickness;
constexpr auto roofReach = barnHalfWidth + barnEavesOverhang;
constexpr auto roofDepth = barnHalfDepth + barnGableOverhang;
// So a point on the ridge is on both upper pitches.
constexpr auto ridgeOverlap = 0.05f;
constexpr auto upperSlope = (barnRidge - barnRoofBreakHeight) / barnRoofBreakX;
constexpr auto climbOut = barnHalfWidth + 2.f * barnBaleWidth;
constexpr auto rampTopInset = 0.5f;

struct Piece final
{
    Vec2 from;
    Vec2 to;
    float top = 0.f;
    float bottom = 0.f;
    Vec2 rise;
};

Piece standing(Vec2 from, Vec2 to, float top)
{
    return {from, to, top, 0.f, {}};
}

Piece floating(Vec2 from, Vec2 to, float bottom, float top)
{
    return {from, to, top, bottom, {}};
}

Piece ramp(Vec2 from, Vec2 to, float top, Vec2 rise)
{
    return {from, to, top, 0.f, rise};
}

Piece pitch(Vec2 from, Vec2 to, float bottom, float top, Vec2 rise)
{
    return {from, to, top, bottom, rise};
}

struct Placer final
{
    void add(const Piece& piece) const
    {
        auto middle = (piece.from + piece.to) * 0.5f;
        auto half = absolute(piece.to - piece.from) * 0.5f;

        auto block = Block {};
        block.center = center + turned(middle, quarterTurns);
        block.half = absolute(turned(half, quarterTurns));
        block.top = piece.top;
        block.bottom = piece.bottom;
        block.rise = turned(piece.rise, quarterTurns);
        scenery.blocks.add(block);
    }

    Scenery& scenery;
    Vec2 center;
    int quarterTurns = 0;
};

// Up to the roof over them: nothing pokes through it, nothing inside is jumped.
void addWalls(const Placer& place)
{
    auto front = -barnHalfDepth;
    auto sideTop = roofHeightAt(roofReach);
    auto outerGable = roofHeightAt(inside);

    for (auto side: {-1.f, 1.f})
    {
        place.add(standing(
            {side * inside, front}, {side * barnHalfWidth, barnHalfDepth}, sideTop));
        place.add(standing({side * barnRoofBreakX, front},
                           {side * inside, frontInside},
                           outerGable));
        place.add(standing({side * barnDoorHalfWidth, front},
                           {side * barnRoofBreakX, frontInside},
                           barnRoofBreakHeight));
        place.add(standing({side * barnRoofBreakX, backInside},
                           {side * inside, barnHalfDepth},
                           outerGable));
    }

    place.add(standing({-barnRoofBreakX, backInside},
                       {barnRoofBreakX, barnHalfDepth},
                       barnRoofBreakHeight));
    place.add(floating({-barnDoorHalfWidth, front},
                       {barnDoorHalfWidth, frontInside},
                       barnDoorHeight,
                       barnLoftHeight));
    place.add(floating({-barnDoorHalfWidth, front},
                       {barnDoorHalfWidth, frontInside},
                       barnHayDoorTop,
                       barnRoofBreakHeight));
}

void addDoors(const Placer& place)
{
    auto hinge = barnDoorHalfWidth + barnTrimWidth;
    auto out = -barnHalfDepth - barnTrimDepth - barnDoorThickness;

    auto face = -barnHalfDepth - barnTrimDepth;

    for (auto side: {-1.f, 1.f})
    {
        place.add(standing({side * hinge, out},
                           {side * (hinge + barnDoorLeafWidth), -barnHalfDepth},
                           barnDoorHeight));
        place.add(floating({side * hinge, face - barnHayLeafWidth},
                           {side * (hinge + barnDoorThickness), face},
                           barnLoftHeight,
                           barnHayDoorTop));
    }
}

void addLoft(const Placer& place)
{
    auto loftBottom = barnLoftHeight - barnLoftThickness;
    auto rampInside = inside - barnRampWidth;
    auto baleBack = frontInside + barnBaleLength;

    place.add(floating(
        {-inside, frontInside}, {inside, loftEdge}, loftBottom, barnLoftHeight));
    place.add(ramp({rampInside, loftEdge},
                   {inside, barnRampFootZ},
                   barnLoftHeight,
                   {0.f, -1.f}));
    place.add(
        floating({-inside, frontInside},
                 {-inside + 2.f * barnBaleWidth, baleBack},
                 barnLoftHeight,
                 barnLoftHeight + (float) barnLoftStackLayers * barnBaleHeight));
    place.add(floating({inside - barnBaleWidth, frontInside},
                       {inside, baleBack},
                       barnLoftHeight,
                       barnLoftHeight + barnBaleHeight));

    for (auto side: {-1.f, 1.f})
        place.scenery.colliders.add(Collider {
            place.center
                + turned({side * barnLoftPostX, loftEdge}, place.quarterTurns),
            barnLoftPostRadius,
            loftBottom});
}

void addStalls(const Placer& place)
{
    auto out = -inside + barnStallDepth;

    for (auto z: barnStallZs)
        place.add(standing({-inside, z - barnStallBoard / 2.f},
                           {out, z + barnStallBoard / 2.f},
                           barnStallHeight));

    auto middle = (barnStallZs[0] + barnStallZs[1]) / 2.f;
    place.add(standing({-inside, middle - barnTroughLength / 2.f},
                       {-inside + barnTroughWidth, middle + barnTroughLength / 2.f},
                       barnTroughHeight));
}

void addClimb(const Placer& place)
{
    auto rampBack = barnClimbFront + barnClimbRampLength;

    place.add(ramp({-climbOut, barnClimbFront},
                   {-barnHalfWidth, rampBack},
                   (float) barnClimbRampLayers * barnBaleHeight,
                   {0.f, 1.f}));

    for (auto index = 0; index < (int) barnClimbLayers.size(); ++index)
    {
        auto from = rampBack + (float) index * barnBaleLength;
        place.add(
            standing({-climbOut, from},
                     {-barnHalfWidth, from + barnBaleLength},
                     (float) barnClimbLayers[(std::size_t) index] * barnBaleHeight));
    }
}

void addRoof(const Placer& place)
{
    for (auto side: {-1.f, 1.f})
    {
        auto inward = Vec2 {-side, 0.f};
        place.add(pitch({side * barnRoofBreakX, -roofDepth},
                        {side * roofReach, roofDepth},
                        roofHeightAt(roofReach),
                        barnRoofBreakHeight,
                        inward));
        auto pastRidge = barnRidge + ridgeOverlap * upperSlope;
        place.add(pitch({-side * ridgeOverlap, -roofDepth},
                        {side * barnRoofBreakX, roofDepth},
                        barnRoofBreakHeight,
                        pastRidge,
                        inward));
    }

    place.add(floating({-barnCupolaHalf, barnCupolaZ - barnCupolaHalf},
                       {barnCupolaHalf, barnCupolaZ + barnCupolaHalf},
                       roofHeightAt(barnCupolaHalf),
                       barnCupolaTop));
}

void addMeshes(Scenery& scenery, Vec2 center, int quarterTurns)
{
    auto heading = -(float) quarterTurns * halfPi;
    auto model =
        Mat4::translation({center.x, 0.f, center.y}) * Mat4::rotationY(heading);

    for (const auto& part: barnParts)
        scenery.batch.add(
            part.shape,
            makeInstance(model, matte(part.color, barnSoftness, barnGloss)));
}

float facing(Vec2 direction, int quarterTurns)
{
    auto turnedDirection = turned(direction, quarterTurns);
    return std::atan2(-turnedDirection.y, turnedDirection.x);
}
} // namespace

BarnSpots barnSpots(Vec2 center, int quarterTurns)
{
    auto spot = [&](Vec2 at, float feet, Vec2 direction)
    {
        auto placed = center + turned(at, quarterTurns);
        return BarnSpot {{placed.x, feet, placed.y},
                         facing(direction, quarterTurns)};
    };

    auto rampTopZ = loftEdge + rampTopInset;
    auto rampRise =
        barnLoftHeight * (barnRampFootZ - rampTopZ) / (barnRampFootZ - loftEdge);

    return {
        spot({0.f, 0.f}, barnRidge, {0.f, 1.f}),
        spot({0.f, frontInside + barnLoftSpotInset}, barnLoftHeight, {-1.f, 0.f}),
        spot({0.f, -barnHalfDepth + barnWallThickness / 2.f}, 0.f, {0.f, 1.f}),
        spot({inside - barnRampWidth / 2.f, rampTopZ}, rampRise, {0.f, -1.f})};
}

float barnReach()
{
    return length(Vec2 {climbOut, roofDepth});
}

void addBarn(Scenery& scenery, Vec2 center, int quarterTurns)
{
    auto place = Placer {scenery, center, quarterTurns};

    addWalls(place);
    addDoors(place);
    addLoft(place);
    addStalls(place);
    addClimb(place);
    addRoof(place);
    scenery.clearings.add(
        Clearing {center, absolute(turned({inside, backInside}, quarterTurns))});
    addMeshes(scenery, center, quarterTurns);
}
} // namespace Cows
