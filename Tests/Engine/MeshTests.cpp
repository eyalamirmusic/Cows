#include "Render/Mesh.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
bool indicesInRange(const MeshData& mesh)
{
    for (auto index: mesh.indices)
        if (index >= (std::uint32_t) mesh.vertices.size())
            return false;

    return true;
}

bool normalsUnitLength(const MeshData& mesh)
{
    for (const auto& vertex: mesh.vertices)
        if (std::abs(length(vertex.normal) - 1.f) > 1e-4f)
            return false;

    return true;
}

bool facesOutward(const MeshData& mesh)
{
    for (auto first = 0; first + 2 < mesh.indices.size(); first += 3)
    {
        const auto& a = mesh.vertices[(int) mesh.indices[first]];
        const auto& b = mesh.vertices[(int) mesh.indices[first + 1]];
        const auto& c = mesh.vertices[(int) mesh.indices[first + 2]];

        auto face = cross(b.position - a.position, c.position - a.position);

        if (dot(face, a.normal + b.normal + c.normal) < 0.f)
            return false;
    }

    return true;
}

Vector<Vec2> cone()
{
    auto profile = Vector<Vec2> {};
    profile.add({0.f, 0.f});
    profile.add({1.f, 0.f});
    profile.add({0.f, 2.f});
    return profile;
}
} // namespace

auto tSphereCounts = test("Mesh/sphereCounts") = []
{
    auto mesh = makeSphere(8, 12);

    check(mesh.vertices.size() == 9 * 13);
    check(mesh.indices.size() == 8 * 12 * 6);
    check(indicesInRange(mesh));
    check(normalsUnitLength(mesh));
    check(facesOutward(mesh));
};

auto tSphereIsUnit = test("Mesh/sphereIsUnit") = []
{
    auto mesh = makeSphere(6, 10);

    for (const auto& vertex: mesh.vertices)
        check(std::abs(length(vertex.position) - 1.f) < 1e-4f);
};

auto tLatheCounts = test("Mesh/latheCounts") = []
{
    auto mesh = makeLathe(cone(), 16);

    check(mesh.vertices.size() == 3 * 17);
    check(mesh.indices.size() == 2 * 16 * 6);
    check(indicesInRange(mesh));
    check(normalsUnitLength(mesh));
};

auto tLatheShapes = test("Mesh/latheShapes") = []
{
    for (const auto& mesh: {makeCapsule(0.3f, 16), makeBarrel(16), makeHorn(16)})
    {
        check(!mesh.vertices.empty());
        check(mesh.indices.size() % 3 == 0);
        check(indicesInRange(mesh));
        check(normalsUnitLength(mesh));
    }
};

auto tBoxCounts = test("Mesh/boxCounts") = []
{
    auto mesh = makeBox();

    check(mesh.vertices.size() == 6 * 4);
    check(mesh.indices.size() == 6 * 6);
    check(indicesInRange(mesh));
    check(normalsUnitLength(mesh));
};

auto tAppendOffsetsIndices = test("Mesh/appendOffsetsIndices") = []
{
    auto mesh = makeBox();
    append(mesh, makeBox(), Mat4::translation({2.f, 0.f, 0.f}));

    check(mesh.vertices.size() == 48);
    check(mesh.indices.size() == 72);
    check(indicesInRange(mesh));
    check(mesh.indices[36] >= 24);
};

auto tCylinderShape = test("Mesh/cylinderIsClosedAndFlatTopped") = []
{
    auto mesh = makeCylinder(16);

    check(!mesh.indices.empty());
    check(indicesInRange(mesh));
    check(normalsUnitLength(mesh));
    check(facesOutward(mesh));

    auto tops = 0;

    for (const auto& vertex: mesh.vertices)
    {
        auto radius = std::hypot(vertex.position.x, vertex.position.z);
        check(radius < 0.5f + 1e-4f);
        check(vertex.position.y > -1e-4f && vertex.position.y < 1.f + 1e-4f);

        if (vertex.position.y > 1.f - 1e-4f && vertex.normal.y > 0.999f)
            ++tops;
    }

    check(tops >= 2 * 17);
};

auto tConeShape = test("Mesh/coneNarrowsToItsPoint") = []
{
    auto mesh = makeCone(16);

    check(!mesh.indices.empty());
    check(indicesInRange(mesh));
    check(normalsUnitLength(mesh));
    check(facesOutward(mesh));

    for (const auto& vertex: mesh.vertices)
    {
        auto radius = std::hypot(vertex.position.x, vertex.position.z);
        check(radius <= 0.5f * (1.f - vertex.position.y) + 1e-4f);
    }
};

auto tBarrelPatch = test("Mesh/barrelPatchLiesOnTheBarrel") = []
{
    auto lowerHalf =
        makeBarrelPatch({-halfPi, halfPi}, {-halfPi, halfPi}, 0.f, 16, 8);
    auto corner =
        makeBarrelPatch({-halfPi, 0.f}, {-0.75f * pi, 0.75f * pi}, 0.05f, 8, 12);

    for (const auto& mesh: {lowerHalf, corner})
    {
        check(!mesh.indices.empty());
        check(indicesInRange(mesh));
        check(normalsUnitLength(mesh));
        check(facesOutward(mesh));
    }

    for (const auto& vertex: lowerHalf.vertices)
    {
        check(vertex.position.x >= -1e-5f);
        check(vertex.position.y > -1e-5f && vertex.position.y < 1.f + 1e-5f);
        check(length(Vec2 {vertex.position.x, vertex.position.z}) < 0.5f + 1e-5f);
    }
};
