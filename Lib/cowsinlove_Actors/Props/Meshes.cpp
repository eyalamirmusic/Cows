#include "Props/Meshes.h"
#include "Cow/Wardrobe.h"
#include "Props/BarnMesh.h"

namespace Cows
{
MeshData makeContentMesh(Shape shape)
{
    return contentMeshes(1.f)(shape);
}

std::function<MeshData(Shape)> contentMeshes(float detail)
{
    return [cow = cowMeshes(detail)](Shape shape)
    { return isBarnShape(shape) ? makeBarnMesh(shape) : cow(shape); };
}
} // namespace Cows
