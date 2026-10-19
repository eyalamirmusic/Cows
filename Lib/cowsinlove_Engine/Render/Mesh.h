#pragma once

#include "Render/Common.h"

#include <cstdint>

namespace Cows
{
struct Vertex final
{
    Maths::Vec3 position;
    Maths::Vec3 normal;
};

struct MeshData final
{
    Vector<Vertex> vertices;
    Vector<std::uint32_t> indices;
};

MeshData makeSphere(int rings, int segments);

// profile is (radius, y) bottom to top; start and end on the axis to close it.
MeshData makeLathe(const Vector<Maths::Vec2>& profile, int segments);

// From y = 0 to y = 1.
MeshData makeCapsule(float radius, int segments);

// Along y from 0 to 1, radius 0.5: a superellipse turned about y.
MeshData makeBarrel(int segments);

struct BarrelPatch final
{
    // Radians; -pi/2 at y = 0, pi/2 at y = 1.
    Maths::Vec2 latitude {-Maths::halfPi, Maths::halfPi};
    // Radians from +x towards +z, mirrored across x: (0, pi) is the whole way round.
    Maths::Vec2 longitude {0.f, Maths::pi};
    // Keeps y between along.x and along.y.
    Maths::Vec2 along {0.f, 1.f};
    // Keeps x between across.x and across.y.
    Maths::Vec2 across {-1.f, 1.f};
    Maths::Vec3 hole;
    float holeRadius = 0.f;
    float rim = 0.f;
    int rings = 24;
    int segments = 24;
};

MeshData makeBarrelPatch(const BarrelPatch& patch);

// Along y from 0 to 1.
MeshData makeHorn(int segments);

// Along y from 0 to 1, radius 0.5.
MeshData makeCylinder(int segments);

// Base radius 0.5 at y = 0, point at y = 1.
MeshData makeCone(int segments);

MeshData makePlane(float size);

// detail 1 is as designed; a multiple of four, never under least.
int detailed(int count, float detail, int least = 8);

// x and z from -0.5 to 0.5, y from 0 to 1.
MeshData makeBox();

// Same footprint, top rising from y = 0 at x = -0.5 to y = 1 at x = 0.5.
MeshData makeWedge();

void append(MeshData& mesh, const MeshData& part, const Maths::Mat4& transform = {});

// So procedural meshes can ignore winding and still be back-face culled.
void windOutward(MeshData& mesh);

Maths::Mat4 normalMatrixFor(const Maths::Mat4& model);

struct Mesh final
{
    explicit Mesh(const MeshData& data);

    Buffer vertices;
    Buffer indices;
    int indexCount = 0;

    float radius = 0.f;
};

float boundingRadius(const MeshData& data);
} // namespace Cows
