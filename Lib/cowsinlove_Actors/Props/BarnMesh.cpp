#include "Props/BarnMesh.h"
#include "ObjLoader/ObjFile.h"

#include <ResEmbed/ResEmbed.h>

namespace Cows
{
namespace
{
const char* groupName(Shape shape)
{
    switch (shape)
    {
        case Shape::BarnWall:
            return "wall";
        case Shape::BarnTrim:
            return "trim";
        case Shape::BarnRoof:
            return "roof";
        case Shape::BarnWood:
            return "wood";
        case Shape::BarnHay:
            return "hay";
        case Shape::BarnMetal:
            return "metal";
        case Shape::BarnTwine:
            return "twine";
        default:
            return "";
    }
}

const Vector<ObjGroup>& barnGroups()
{
    static const auto groups =
        parseObj(ResEmbed::get("barn.obj", "Models").toStringView());
    return groups;
}

MeshData toMesh(const ObjGroup& group)
{
    auto mesh = MeshData {};

    for (auto index = 0; index < (int) group.positions.size(); ++index)
        mesh.vertices.add({group.positions[index], group.normals[index]});

    mesh.indices = group.indices;
    return mesh;
}
} // namespace

bool isBarnShape(Shape shape)
{
    return *groupName(shape) != '\0';
}

MeshData makeBarnMesh(Shape shape)
{
    return toMesh(findGroup(barnGroups(), groupName(shape)));
}
} // namespace Cows
