#include "Render/ShapeMeshes.h"

namespace Cows
{
namespace
{
MeshData makeSolid(Shape shape,
                   const std::function<MeshData(Shape)>& content,
                   float detail)
{
    auto steps = [detail](int count) { return detailed(count, detail); };

    switch (shape)
    {
        case Shape::Sphere:
            return makeSphere(steps(32), steps(48));
        case Shape::Capsule:
            return makeCapsule(0.14f, steps(32));
        case Shape::Horn:
            return makeHorn(steps(24));
        case Shape::Barrel:
            return makeBarrel(steps(48));
        case Shape::Box:
            return makeBox();
        case Shape::Wedge:
            return makeWedge();
        case Shape::Cylinder:
            return makeCylinder(steps(32));
        case Shape::Cone:
            return makeCone(steps(32));
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
