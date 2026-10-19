#include "Props/PropMeshes.h"
#include "MeshLoader/MeshBlob.h"

#include <ResEmbed/ResEmbed.h>

#include <array>
#include <map>
#include <span>
#include <string_view>

namespace Cows
{
namespace
{
struct PropMesh final
{
    Shape shape;
    std::string_view file;
    std::string_view group;
};

constexpr std::array<PropMesh, 9> propMeshes {{
    {Shape::BarnWall, "barn.mesh", "wall"},
    {Shape::BarnTrim, "barn.mesh", "trim"},
    {Shape::BarnRoof, "barn.mesh", "roof"},
    {Shape::BarnWood, "barn.mesh", "wood"},
    {Shape::BarnHay, "barn.mesh", "hay"},
    {Shape::BarnMetal, "barn.mesh", "metal"},
    {Shape::BarnTwine, "barn.mesh", "twine"},
    {Shape::BaleStraw, "bale.mesh", "straw"},
    {Shape::BaleTwine, "bale.mesh", "twine"},
}};

const PropMesh* findPropMesh(Shape shape)
{
    for (const auto& mesh: propMeshes)
        if (mesh.shape == shape)
            return &mesh;

    return nullptr;
}

Vector<MeshGroup> readGroups(std::string_view file)
{
    auto resource = ResEmbed::get(std::string(file), "Models");
    return readMeshBlob(std::as_bytes(std::span {resource.data(), resource.size()}));
}

const Vector<MeshGroup>& groupsOf(std::string_view file)
{
    static const auto files = []
    {
        auto read = std::map<std::string_view, Vector<MeshGroup>> {};

        for (const auto& mesh: propMeshes)
            if (!read.contains(mesh.file))
                read.emplace(mesh.file, readGroups(mesh.file));

        return read;
    }();

    return files.at(file);
}

MeshData toMesh(const MeshGroup& group)
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
