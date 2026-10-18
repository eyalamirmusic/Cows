#pragma once

#include <eacp/Core/Maths/Maths.h>
#include <eacp/Core/Utils/Containers.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace Cows
{
// One material's triangles: each vertex a position and a normal.
struct MeshGroup final
{
    std::string name;
    eacp::Vector<eacp::Maths::Vec3> positions;
    eacp::Vector<eacp::Maths::Vec3> normals;
    eacp::Vector<std::uint32_t> indices;
};

// The groups of a mesh blob, as tools/blender/meshkit.py writes it; nothing
// when the blob is not one or is cut short.
eacp::Vector<MeshGroup> readMeshBlob(std::span<const std::byte> blob);

// The group called `name`, or an empty one.
MeshGroup findGroup(const eacp::Vector<MeshGroup>& groups, std::string_view name);
} // namespace Cows
