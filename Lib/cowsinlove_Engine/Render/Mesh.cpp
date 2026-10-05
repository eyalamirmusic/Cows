#include "Render/Mesh.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
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

void dropSlivers(MeshData& mesh)
{
    constexpr auto leastArea = 1e-7f;
    auto kept = Vector<std::uint32_t> {};

    for (auto first = 0; first + 2 < mesh.indices.size(); first += 3)
    {
        const auto& a = mesh.vertices[(int) mesh.indices[first]].position;
        const auto& b = mesh.vertices[(int) mesh.indices[first + 1]].position;
        const auto& c = mesh.vertices[(int) mesh.indices[first + 2]].position;

        if (length(cross(b - a, c - a)) > leastArea)
            kept.add({mesh.indices[first],
                      mesh.indices[first + 1],
                      mesh.indices[first + 2]});
    }

    mesh.indices = kept;
}

constexpr auto barrelRoundness = 2.f / 2.6f;

float signedPower(float value, float power)
{
    return std::copysign(std::pow(std::abs(value), power), value);
}

Vec2 barrelProfile(float angle)
{
    auto c = std::cos(angle);
    auto s = std::sin(angle);
    auto radius = 0.5f * std::pow(std::max(c, 0.f), barrelRoundness);
    auto height = 0.5f + 0.5f * signedPower(s, barrelRoundness);

    return {radius, height};
}

Vertex barrelPoint(float latitude, float longitude)
{
    auto profile = barrelProfile(latitude);
    auto slope = normalize(
        Vec2 {std::pow(std::max(std::cos(latitude), 0.f), 2.f - barrelRoundness),
              signedPower(std::sin(latitude), 2.f - barrelRoundness)});
    auto c = std::cos(longitude);
    auto s = std::sin(longitude);

    return {{profile.x * c, profile.y, profile.x * s},
            {slope.x * c, slope.y, slope.x * s}};
}

float barrelLatitudeAt(float along)
{
    auto height = std::clamp(2.f * along - 1.f, -1.f, 1.f);
    return std::asin(signedPower(height, 1.f / barrelRoundness));
}

Vec3 directionOr(Vec3 direction, Vec3 fallback)
{
    auto size = length(direction);
    return size > 1e-6f ? direction / size : fallback;
}

// The longitudes, from 0 towards pi, at which a ring `radius` from the axis
// lies between the planes x = across.x and x = across.y.
Vec2 longitudesAcross(Vec2 across, float radius)
{
    if (radius < 1e-6f)
        return across.x <= 0.f && across.y >= 0.f ? Vec2 {0.f, pi} : Vec2 {};

    auto at = [&](float x) { return std::acos(std::clamp(x / radius, -1.f, 1.f)); };
    return {at(across.y), at(across.x)};
}

void addBarrelHalf(MeshData& mesh, const BarrelPatch& patch, float side)
{
    auto base = (std::uint32_t) mesh.vertices.size();
    auto first = std::max(patch.latitude.x, barrelLatitudeAt(patch.along.x));
    auto last = std::min(patch.latitude.y, barrelLatitudeAt(patch.along.y));

    for (auto ring = 0; ring <= patch.rings; ++ring)
    {
        auto latitude = std::lerp(first, last, (float) ring / (float) patch.rings);
        auto kept = longitudesAcross(patch.across, barrelProfile(latitude).x);
        auto from = std::max(kept.x, patch.longitude.x);
        auto to = std::max(std::min(kept.y, patch.longitude.y), from);

        for (auto segment = 0; segment <= patch.segments; ++segment)
            mesh.vertices.add(barrelPoint(
                latitude,
                side
                    * std::lerp(
                        from, to, (float) segment / (float) patch.segments)));
    }

    for (auto ring = 0; ring < patch.rings; ++ring)
        for (auto segment = 0; segment < patch.segments; ++segment)
        {
            auto a = base + (std::uint32_t) (ring * (patch.segments + 1) + segment);
            auto b = a + (std::uint32_t) (patch.segments + 1);
            mesh.indices.add({a, a + 1, b + 1, a, b + 1, b});
        }
}

void cutHole(MeshData& mesh, Vec3 center, float radius)
{
    auto inside = [&](std::uint32_t index)
    { return length(mesh.vertices[(int) index].position - center) < radius; };
    auto kept = Vector<std::uint32_t> {};

    for (auto first = 0; first + 2 < mesh.indices.size(); first += 3)
        if (!inside(mesh.indices[first]) && !inside(mesh.indices[first + 1])
            && !inside(mesh.indices[first + 2]))
            kept.add({mesh.indices[first],
                      mesh.indices[first + 1],
                      mesh.indices[first + 2]});

    mesh.indices = kept;
}

using PointKey = std::array<int, 3>;

PointKey keyOf(Vec3 point)
{
    auto snap = [](float value) { return (int) std::lround(value * 1e5f); };
    return {snap(point.x), snap(point.y), snap(point.z)};
}

struct Edge final
{
    std::uint32_t from;
    std::uint32_t to;
    std::uint32_t opposite;
};

Vector<Edge> boundaryOf(const MeshData& mesh)
{
    auto uses = std::map<std::pair<PointKey, PointKey>, int> {};
    auto edges = Vector<Edge> {};
    auto keyed = [&](std::uint32_t a, std::uint32_t b)
    {
        auto ka = keyOf(mesh.vertices[(int) a].position);
        auto kb = keyOf(mesh.vertices[(int) b].position);
        return ka < kb ? std::pair {ka, kb} : std::pair {kb, ka};
    };

    for (auto pass = 0; pass < 2; ++pass)
        for (auto first = 0; first + 2 < mesh.indices.size(); first += 3)
            for (auto corner = 0; corner < 3; ++corner)
            {
                auto a = mesh.indices[first + corner];
                auto b = mesh.indices[first + (corner + 1) % 3];
                auto key = keyed(a, b);

                if (key.first == key.second)
                    continue;

                if (pass == 0)
                    ++uses[key];
                else if (uses[key] == 1)
                    edges.add({a, b, mesh.indices[first + (corner + 2) % 3]});
            }

    return edges;
}

void roundHole(MeshData& mesh, const Vector<Edge>& edges, Vec3 center, float radius)
{
    for (const auto& edge: edges)
        for (auto index: {edge.from, edge.to})
        {
            auto& position = mesh.vertices[(int) index].position;
            auto away = position - center;

            if (length(away) < 1.6f * radius)
                position = center + normalize(away) * radius;
        }
}

void addRims(MeshData& mesh, const Vector<Edge>& edges, float rim)
{
    for (const auto& edge: edges)
    {
        auto a = mesh.vertices[(int) edge.from];
        auto b = mesh.vertices[(int) edge.to];
        auto inward = mesh.vertices[(int) edge.opposite].position - a.position;
        auto along = b.position - a.position;
        auto normal = directionOr(cross(along, a.normal + b.normal), a.normal);

        if (dot(normal, inward) > 0.f)
            normal = -normal;

        auto base = (std::uint32_t) mesh.vertices.size();
        mesh.vertices.add({a.position, normal});
        mesh.vertices.add({b.position, normal});
        mesh.vertices.add({a.position - a.normal * rim, normal});
        mesh.vertices.add({b.position - b.normal * rim, normal});
        mesh.indices.add({base, base + 1, base + 3, base, base + 3, base + 2});
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
    auto profile = Vector<Vec2> {};
    auto steps = segments / 2;

    for (auto step = 0; step <= steps; ++step)
        profile.add(barrelProfile(-halfPi + pi * (float) step / (float) steps));

    return makeLathe(profile, segments);
}

MeshData makeBarrelPatch(const BarrelPatch& patch)
{
    auto mesh = MeshData {};
    addBarrelHalf(mesh, patch, 1.f);
    addBarrelHalf(mesh, patch, -1.f);
    dropSlivers(mesh);

    if (patch.holeRadius > 0.f)
        cutHole(mesh, patch.hole, patch.holeRadius);

    auto edges = boundaryOf(mesh);

    if (patch.holeRadius > 0.f)
        roundHole(mesh, edges, patch.hole, patch.holeRadius);

    addRims(mesh, edges, patch.rim);
    dropSlivers(mesh);
    windOutward(mesh);
    return mesh;
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

MeshData makeCylinder(int segments)
{
    auto profile = Vector<Vec2> {};
    profile.add({0.f, 0.f});
    profile.add({0.5f, 0.f});
    profile.add({0.5f, 0.f});
    profile.add({0.5f, 1.f});
    profile.add({0.5f, 1.f});
    profile.add({0.f, 1.f});

    auto mesh = makeLathe(profile, segments);
    dropSlivers(mesh);
    return mesh;
}

MeshData makeCone(int segments)
{
    auto profile = Vector<Vec2> {};
    profile.add({0.f, 0.f});
    profile.add({0.5f, 0.f});
    profile.add({0.5f, 0.f});
    profile.add({0.f, 1.f});

    auto mesh = makeLathe(profile, segments);
    dropSlivers(mesh);
    return mesh;
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
    , radius(boundingRadius(data))
{
}

float boundingRadius(const MeshData& data)
{
    auto furthest = 0.f;

    for (const auto& vertex: data.vertices)
        furthest = std::max(furthest, length(vertex.position));

    return furthest;
}
} // namespace Cows
