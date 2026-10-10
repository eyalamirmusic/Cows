#pragma once

#include "Render/Instances.h"
#include "Render/Mesh.h"

#include <functional>

namespace Cows
{
// A mesh for every Shape. The Engine makes the plain solids; `content` makes the
// rest (the heart, the pants), which belong to the game, and is never asked for
// a solid. `detail` coarsens the solids' steps (see `detailed`); the content
// is asked for its own at whatever detail it was made for.
struct ShapeMeshes final
{
    explicit ShapeMeshes(const std::function<MeshData(Shape)>& content,
                         float detail = 1.f);

    const Mesh& operator[](Shape shape) const;

private:
    Vector<Mesh> meshes;
};
} // namespace Cows
