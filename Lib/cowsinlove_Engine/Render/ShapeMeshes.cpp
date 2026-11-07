#include "Render/ShapeMeshes.h"

#include <algorithm>
#include <array>

namespace Cows
{
namespace
{
constexpr auto capsuleRadius = 0.14f;
constexpr auto sphereRings = 32;
constexpr auto sphereSegments = 48;
constexpr auto roundSegments = 32;
constexpr auto hornSegments = 24;
constexpr auto barrelSegments = 48;

struct SolidMaker final
{
    Shape shape;
    MeshData (*make)(float detail);
};

const std::array solidMakers {
    SolidMaker {Shape::Sphere,
                [](float detail)
                {
                    return makeSphere(detailed(sphereRings, detail),
                                      detailed(sphereSegments, detail));
                }},
    SolidMaker {
        Shape::Capsule,
        [](float detail)
        { return makeCapsule(capsuleRadius, detailed(roundSegments, detail)); }},
    SolidMaker {Shape::Horn,
                [](float detail)
                { return makeHorn(detailed(hornSegments, detail)); }},
    SolidMaker {Shape::Barrel,
                [](float detail)
                { return makeBarrel(detailed(barrelSegments, detail)); }},
    SolidMaker {Shape::Box, [](float) { return makeBox(); }},
    SolidMaker {Shape::Wedge, [](float) { return makeWedge(); }},
    SolidMaker {Shape::Cylinder,
                [](float detail)
                { return makeCylinder(detailed(roundSegments, detail)); }},
    SolidMaker {Shape::Cone,
                [](float detail)
                { return makeCone(detailed(roundSegments, detail)); }},
};

MeshData makeSolid(Shape shape,
                   const std::function<MeshData(Shape)>& content,
                   float detail)
{
    auto isThisShape = [shape](const SolidMaker& maker)
    { return maker.shape == shape; };
    auto found = std::find_if(solidMakers.begin(), solidMakers.end(), isThisShape);

    if (found == solidMakers.end())
        return content(shape);

    return found->make(detail);
}
} // namespace

ShapeMeshes::ShapeMeshes(const std::function<MeshData(Shape)>& content, float detail)
{
    for (auto index = 0; index < shapeCount; ++index)
        meshes.add(Mesh {makeSolid((Shape) index, content, detail)});
}

const Mesh& ShapeMeshes::operator[](Shape shape) const
{
    return meshes[(int) shape];
}
} // namespace Cows
