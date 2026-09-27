#include "Mesh.h"

#include <cmath>
#include <initializer_list>

using namespace Maths;

namespace Cows
{
namespace
{
void addFace(MeshData& mesh, Vec3 inside, std::initializer_list<Vec3> corners)
{
    auto points = Vector<Vec3> {};

    for (const auto& corner: corners)
        points.add(corner);

    auto normal = normalize(cross(points[1] - points[0], points[2] - points[0]));
    auto center = Vec3 {};

    for (const auto& point: points)
        center += point / (float) points.size();

    if (dot(normal, center - inside) < 0.f)
        normal = -normal;

    auto base = (std::uint32_t) mesh.vertices.size();

    for (const auto& point: points)
        mesh.vertices.add({point, normal});

    for (auto corner = 1; corner + 1 < points.size(); ++corner)
        mesh.indices.add({base,
                          base + (std::uint32_t) corner,
                          base + (std::uint32_t) corner + 1});
}

Buffer makeVertexBuffer(const MeshData& data)
{
    auto bytes = (int) (sizeof(Vertex) * data.vertices.size());
    return Device::shared().makeBuffer(data.vertices.data(), bytes);
}

Buffer makeIndexBuffer(const MeshData& data)
{
    auto bytes = (int) (sizeof(std::uint32_t) * data.indices.size());
    return Device::shared().makeBuffer(
        data.indices.data(), bytes, BufferUsage::Index);
}

void addGrid(MeshData& mesh, int rows, int columns)
{
    for (auto row = 0; row < rows; ++row)
        for (auto column = 0; column < columns; ++column)
        {
            auto a = (std::uint32_t) (row * (columns + 1) + column);
            auto b = a + (std::uint32_t) (columns + 1);
            auto c = b + 1;
            auto d = a + 1;

            mesh.indices.add({a, d, c, a, c, b});
        }
}

Vec2 profileNormal(const Vector<Vec2>& profile, int index)
{
    auto last = profile.size() - 1;
    auto before = profile[std::max(index - 1, 0)];
    auto after = profile[std::min(index + 1, last)];
    auto tangent = after - before;

    return normalize(Vec2 {tangent.y, -tangent.x});
}
} // namespace

MeshData makeSphere(int rings, int segments)
{
    auto mesh = MeshData {};

    for (auto ring = 0; ring <= rings; ++ring)
    {
        auto polar = pi * (float) ring / (float) rings;

        for (auto segment = 0; segment <= segments; ++segment)
        {
            auto azimuth = twoPi * (float) segment / (float) segments;

            auto normal = Vec3 {std::sin(polar) * std::cos(azimuth),
                                std::cos(polar),
                                std::sin(polar) * std::sin(azimuth)};

            mesh.vertices.add({normal, normal});
        }
    }

    addGrid(mesh, rings, segments);
    windOutward(mesh);
    return mesh;
}

MeshData makeLathe(const Vector<Vec2>& profile, int segments)
{
    auto mesh = MeshData {};

    for (auto index = 0; index < profile.size(); ++index)
    {
        auto point = profile[index];
        auto normal = profileNormal(profile, index);

        for (auto segment = 0; segment <= segments; ++segment)
        {
            auto azimuth = twoPi * (float) segment / (float) segments;
            auto c = std::cos(azimuth);
            auto s = std::sin(azimuth);

            mesh.vertices.add(
                {{point.x * c, point.y, point.x * s},
                 normalize(Vec3 {normal.x * c, normal.y, normal.x * s})});
        }
    }

    addGrid(mesh, profile.size() - 1, segments);
    windOutward(mesh);
    return mesh;
}

MeshData makeCapsule(float radius, int segments)
{
    auto profile = Vector<Vec2> {};
    auto steps = segments / 4;

    for (auto step = 0; step <= steps; ++step)
    {
        auto angle = -halfPi + halfPi * (float) step / (float) steps;
        profile.add({radius * std::cos(angle), radius + radius * std::sin(angle)});
    }

    for (auto step = 0; step <= steps; ++step)
    {
        auto angle = halfPi * (float) step / (float) steps;
        profile.add(
            {radius * std::cos(angle), 1.f - radius + radius * std::sin(angle)});
    }

    return makeLathe(profile, segments);
}

MeshData makeBarrel(int segments)
{
    constexpr auto roundness = 2.f / 2.6f;
    auto profile = Vector<Vec2> {};
    auto steps = segments / 2;

    for (auto step = 0; step <= steps; ++step)
    {
        auto angle = -halfPi + pi * (float) step / (float) steps;
        auto c = std::cos(angle);
        auto s = std::sin(angle);
        auto radius = 0.5f * std::pow(std::max(c, 0.f), roundness);
        auto height =
            0.5f + 0.5f * std::copysign(std::pow(std::abs(s), roundness), s);

        profile.add({radius, height});
    }

    return makeLathe(profile, segments);
}

MeshData makeHorn(int segments)
{
    auto profile = Vector<Vec2> {};
    profile.add({0.f, 0.f});
    profile.add({0.5f, 0.f});

    for (auto step = 1; step <= 8; ++step)
    {
        auto along = (float) step / 8.f;
        profile.add({0.5f - 0.36f * std::pow(along, 1.3f), 0.85f * along});
    }

    for (auto step = 1; step <= 6; ++step)
    {
        auto angle = halfPi * (float) step / 6.f;
        profile.add({0.14f * std::cos(angle), 0.85f + 0.14f * std::sin(angle)});
    }

    return makeLathe(profile, segments);
}

MeshData makeBox()
{
    auto mesh = MeshData {};

    addFace(mesh,
            {0.f, 0.5f, 0.f},
            {{-0.5f, 1.f, 0.5f},
             {0.5f, 1.f, 0.5f},
             {0.5f, 1.f, -0.5f},
             {-0.5f, 1.f, -0.5f}});
    addFace(mesh,
            {0.f, 0.5f, 0.f},
            {{-0.5f, 0.f, -0.5f},
             {0.5f, 0.f, -0.5f},
             {0.5f, 0.f, 0.5f},
             {-0.5f, 0.f, 0.5f}});
    addFace(mesh,
            {0.f, 0.5f, 0.f},
            {{0.5f, 0.f, -0.5f},
             {0.5f, 1.f, -0.5f},
             {0.5f, 1.f, 0.5f},
             {0.5f, 0.f, 0.5f}});
    addFace(mesh,
            {0.f, 0.5f, 0.f},
            {{-0.5f, 0.f, 0.5f},
             {-0.5f, 1.f, 0.5f},
             {-0.5f, 1.f, -0.5f},
             {-0.5f, 0.f, -0.5f}});
    addFace(mesh,
            {0.f, 0.5f, 0.f},
            {{-0.5f, 0.f, 0.5f},
             {0.5f, 0.f, 0.5f},
             {0.5f, 1.f, 0.5f},
             {-0.5f, 1.f, 0.5f}});
    addFace(mesh,
            {0.f, 0.5f, 0.f},
            {{0.5f, 0.f, -0.5f},
             {-0.5f, 0.f, -0.5f},
             {-0.5f, 1.f, -0.5f},
             {0.5f, 1.f, -0.5f}});

    windOutward(mesh);
    return mesh;
}

MeshData makeWedge()
{
    auto mesh = MeshData {};

    addFace(mesh,
            {0.2f, 0.3f, 0.f},
            {{-0.5f, 0.f, 0.5f},
             {0.5f, 1.f, 0.5f},
             {0.5f, 1.f, -0.5f},
             {-0.5f, 0.f, -0.5f}});
    addFace(mesh,
            {0.2f, 0.3f, 0.f},
            {{-0.5f, 0.f, -0.5f},
             {0.5f, 0.f, -0.5f},
             {0.5f, 0.f, 0.5f},
             {-0.5f, 0.f, 0.5f}});
    addFace(mesh,
            {0.2f, 0.3f, 0.f},
            {{0.5f, 0.f, -0.5f},
             {0.5f, 1.f, -0.5f},
             {0.5f, 1.f, 0.5f},
             {0.5f, 0.f, 0.5f}});
    addFace(mesh,
            {0.2f, 0.3f, 0.f},
            {{-0.5f, 0.f, 0.5f}, {0.5f, 0.f, 0.5f}, {0.5f, 1.f, 0.5f}});
    addFace(mesh,
            {0.2f, 0.3f, 0.f},
            {{0.5f, 0.f, -0.5f}, {-0.5f, 0.f, -0.5f}, {0.5f, 1.f, -0.5f}});

    windOutward(mesh);
    return mesh;
}

MeshData makePlane(float size)
{
    auto mesh = MeshData {};
    auto half = size * 0.5f;
    auto up = Vec3 {0.f, 1.f, 0.f};

    mesh.vertices.add({{-half, 0.f, half}, up});
    mesh.vertices.add({{half, 0.f, half}, up});
    mesh.vertices.add({{half, 0.f, -half}, up});
    mesh.vertices.add({{-half, 0.f, -half}, up});

    mesh.indices.add({0u, 1u, 2u, 0u, 2u, 3u});

    return mesh;
}

void append(MeshData& mesh, const MeshData& part, const Mat4& transform)
{
    auto base = (std::uint32_t) mesh.vertices.size();
    auto normals = normalMatrixFor(transform);

    for (const auto& vertex: part.vertices)
        mesh.vertices.add({transformPoint(transform, vertex.position),
                           normalize(transformDirection(normals, vertex.normal))});

    for (auto index: part.indices)
        mesh.indices.add(base + index);
}

void windOutward(MeshData& mesh)
{
    for (auto first = 0; first + 2 < mesh.indices.size(); first += 3)
    {
        auto& a = mesh.indices[first];
        auto& b = mesh.indices[first + 1];
        auto& c = mesh.indices[first + 2];

        const auto& va = mesh.vertices[(int) a];
        const auto& vb = mesh.vertices[(int) b];
        const auto& vc = mesh.vertices[(int) c];

        auto face = cross(vb.position - va.position, vc.position - va.position);
        auto normal = va.normal + vb.normal + vc.normal;

        if (dot(face, normal) < 0.f)
            std::swap(b, c);
    }
}

Mat4 normalMatrixFor(const Mat4& model)
{
    return model.inverted().transposed();
}

Mesh::Mesh(const MeshData& data)
    : vertices(makeVertexBuffer(data))
    , indices(makeIndexBuffer(data))
    , indexCount(data.indices.size())
{
}
} // namespace Cows
