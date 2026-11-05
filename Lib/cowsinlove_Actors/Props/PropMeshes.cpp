#include "Props/PropMeshes.h"
#include "ObjLoader/ObjFile.h"

#include <ResEmbed/ResEmbed.h>

#include <array>
#include <map>
#include <string_view>

namespace Cows
{
namespace
{
// Where a Shape's mesh is: the group `group` of the embedded `file`.
struct PropMesh final
{
    Shape shape;
    std::string_view file;
    std::string_view group;
};

constexpr std::array<PropMesh, 9> propMeshes {{
    {Shape::BarnWall, "barn.obj", "wall"},
    {Shape::BarnTrim, "barn.obj", "trim"},
    {Shape::BarnRoof, "barn.obj", "roof"},
    {Shape::BarnWood, "barn.obj", "wood"},
    {Shape::BarnHay, "barn.obj", "hay"},
    {Shape::BarnMetal, "barn.obj", "metal"},
    {Shape::BarnTwine, "barn.obj", "twine"},
    {Shape::BaleStraw, "bale.obj", "straw"},
    {Shape::BaleTwine, "bale.obj", "twine"},
}};

const PropMesh* findPropMesh(Shape shape)
{
    for (const auto& mesh: propMeshes)
        if (mesh.shape == shape)
            return &mesh;

    return nullptr;
}

Vector<ObjGroup> readGroups(std::string_view file)
{
    return parseObj(ResEmbed::get(std::string(file), "Models").toStringView());
}

// Every prop file's groups, each file read once.
const Vector<ObjGroup>& groupsOf(std::string_view file)
{
    static const auto files = []
    {
        auto read = std::map<std::string_view, Vector<ObjGroup>> {};

        for (const auto& mesh: propMeshes)
            if (!read.contains(mesh.file))
                read.emplace(mesh.file, readGroups(mesh.file));

        return read;
    }();

    return files.at(file);
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

bool isPropShape(Shape shape)
{
    return findPropMesh(shape) != nullptr;
}

MeshData makePropMesh(Shape shape)
{
    const auto* mesh = findPropMesh(shape);
    return toMesh(findGroup(groupsOf(mesh->file), mesh->group));
}
} // namespace Cows
