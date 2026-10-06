#include "Cow/HeartMesh.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto fullOutlineSteps = 120;
constexpr auto fullInflateSteps = 28;
constexpr auto thickness = 0.5f;
constexpr Vec2 center {0.f, 0.05f};

Vec2 outline(float t)
{
    auto s = std::sin(t);
    auto x = 16.f * s * s * s;
    auto y = 13.f * std::cos(t) - 5.f * std::cos(2.f * t) - 2.f * std::cos(3.f * t)
             - std::cos(4.f * t);

    return Vec2 {x, y} / 16.f + Vec2 {0.f, 0.155f};
}

void smoothNormals(MeshData& mesh)
{
    for (auto& vertex: mesh.vertices)
        vertex.normal = {};

    for (auto first = 0; first + 2 < mesh.indices.size(); first += 3)
    {
        auto& a = mesh.vertices[(int) mesh.indices[first]];
        auto& b = mesh.vertices[(int) mesh.indices[first + 1]];
        auto& c = mesh.vertices[(int) mesh.indices[first + 2]];

        auto face = cross(b.position - a.position, c.position - a.position);
        a.normal += face;
        b.normal += face;
        c.normal += face;
    }

    auto outward = 0.f;

    for (auto& vertex: mesh.vertices)
    {
        vertex.normal = normalize(vertex.normal);
        outward += dot(vertex.normal, vertex.position);
    }

    if (outward < 0.f)
        for (auto& vertex: mesh.vertices)
            vertex.normal = -vertex.normal;
}
} // namespace

MeshData makeHeart(float detail)
{
    auto mesh = MeshData {};
    auto outlineSteps = detailed(fullOutlineSteps, detail);
    auto inflateSteps = detailed(fullInflateSteps, detail);

    for (auto layer = 0; layer <= inflateSteps; ++layer)
    {
        auto angle = pi * (float) layer / (float) inflateSteps;
        auto spread = std::sin(angle);
        auto depth = std::cos(angle) * thickness;

        for (auto step = 0; step <= outlineSteps; ++step)
        {
            auto t = twoPi * (float) step / (float) outlineSteps;
            auto edge = outline(t);
            auto reach = length(edge - center);
            auto point = center + (edge - center) * spread;
            auto puff = depth * std::sqrt(std::min(reach, 1.f));

            mesh.vertices.add(Vertex {{point.x, point.y, puff}, {}});
        }
    }

    for (auto layer = 0; layer < inflateSteps; ++layer)
        for (auto step = 0; step < outlineSteps; ++step)
        {
            auto a = (std::uint32_t) (layer * (outlineSteps + 1) + step);
            auto b = a + (std::uint32_t) (outlineSteps + 1);
            mesh.indices.add({a, a + 1, b + 1, a, b + 1, b});
        }

    smoothNormals(mesh);

    auto columns = outlineSteps + 1;

    for (auto step = 0; step < columns; ++step)
    {
        mesh.vertices[step].normal = {0.f, 0.f, 1.f};
        mesh.vertices[inflateSteps * columns + step].normal = {0.f, 0.f, -1.f};
    }

    windOutward(mesh);
    return mesh;
}
} // namespace Cows
