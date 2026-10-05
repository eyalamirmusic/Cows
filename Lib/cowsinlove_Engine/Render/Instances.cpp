#include "Render/Instances.h"
#include "Render/Mesh.h"

using namespace Maths;

namespace Cows
{
SurfaceInstance
    makeInstance(const Mat4& model, const Material& material, const Mat4& pattern)
{
    auto normals = normalMatrixFor(model);

    auto instance = SurfaceInstance {};
    instance.model0 = model.column(0);
    instance.model1 = model.column(1);
    instance.model2 = model.column(2);
    instance.model3 = model.column(3);
    instance.normal0 = normals.column(0);
    instance.normal1 = normals.column(1);
    instance.normal2 = normals.column(2);
    instance.pattern0 = pattern.column(0);
    instance.pattern1 = pattern.column(1);
    instance.pattern2 = pattern.column(2);
    instance.pattern3 = pattern.column(3);
    instance.color = {
        material.color.x, material.color.y, material.color.z, material.alpha};
    instance.material = {
        material.spots, material.emission, material.softness, material.gloss};
    instance.mist = material.mist;
    return instance;
}
} // namespace Cows
