#pragma once

#include <eacp/Core/Maths/Maths.h>
#include <eacp/Core/Utils/Containers.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace Cows
{
// The triangles of an OBJ file under one `usemtl`; faces under a material
// seen before join its group. Each vertex is a distinct position and normal
// pair.
struct ObjGroup final
{
    std::string name;
    eacp::Vector<eacp::Maths::Vec3> positions;
    eacp::Vector<eacp::Maths::Vec3> normals;
    eacp::Vector<std::uint32_t> indices;
};

// Reads an OBJ file as Blender exports it, with normals: `v`, `vn`, `usemtl`
// and faces of v//vn corners, n-gons fanned into triangles. Anything else in
// the file is skipped.
eacp::Vector<ObjGroup> parseObj(std::string_view text);

// The group called `name`, or an empty one.
ObjGroup findGroup(const eacp::Vector<ObjGroup>& groups, std::string_view name);
} // namespace Cows
