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
struct MeshGroup final
{
    std::string name;
    eacp::Vector<eacp::Maths::Vec3> positions;
    eacp::Vector<eacp::Maths::Vec3> normals;
    eacp::Vector<std::uint32_t> indices;
};

// Empty when the blob is malformed or cut short.
eacp::Vector<MeshGroup> readMeshBlob(std::span<const std::byte> blob);

// An empty group when none is called `name`.
MeshGroup findGroup(const eacp::Vector<MeshGroup>& groups, std::string_view name);
} // namespace Cows
