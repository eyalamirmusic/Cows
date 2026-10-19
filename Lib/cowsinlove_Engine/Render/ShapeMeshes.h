#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

#include <functional>

namespace Cows
{
// detail coarsens the solids only; content meshes keep the detail they were made at.
struct ShapeMeshes final
{
    explicit ShapeMeshes(const std::function<MeshData(Shape)>& content,
                         float detail = 1.f);

    const Mesh& operator[](Shape shape) const;

private:
    Vector<Mesh> meshes;
};
} // namespace Cows
