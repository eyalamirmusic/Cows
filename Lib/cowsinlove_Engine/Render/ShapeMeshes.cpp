#include "Render/ShapeMeshes.h"

namespace Cows
{
namespace
{
MeshData makeSolid(Shape shape, const std::function<MeshData(Shape)>& content)
{
    switch (shape)
    {
        case Shape::Sphere:
            return makeSphere(32, 48);
        case Shape::Capsule:
            return makeCapsule(0.14f, 32);
        case Shape::Horn:
            return makeHorn(24);
        case Shape::Barrel:
            return makeBarrel(48);
        case Shape::Box:
            return makeBox();
        case Shape::Wedge:
            return makeWedge();
        case Shape::Cylinder:
            return makeCylinder(32);
        case Shape::Cone:
            return makeCone(32);
        case Shape::Heart:
        case Shape::Belly:
        case Shape::BellyBand:
        case Shape::Seat:
        case Shape::SeatBand:
        case Shape::BucketCrown:
        case Shape::BucketBrim:
            break;
    }

    return content(shape);
}
} // namespace

ShapeMeshes::ShapeMeshes(const std::function<MeshData(Shape)>& content)
{
    for (auto index = 0; index < shapeCount; ++index)
        meshes.add(Mesh {makeSolid((Shape) index, content)});
}

const Mesh& ShapeMeshes::operator[](Shape shape) const
{
    return meshes[(int) shape];
}
} // namespace Cows
