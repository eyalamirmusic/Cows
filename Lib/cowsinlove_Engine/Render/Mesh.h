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

// A unit sphere.
MeshData makeSphere(int rings, int segments);

// A surface of revolution about y. `profile` runs bottom to top as (radius, y),
// and should start and end on the axis for the shape to be closed.
MeshData makeLathe(const Vector<Maths::Vec2>& profile, int segments);

// A capsule along y, from y = 0 to y = 1, a pill of the given radius.
MeshData makeCapsule(float radius, int segments);

// A rounded barrel along y from 0 to 1, half a unit in radius: a superellipse
// turned about its axis, flatter-ended than a sphere.
MeshData makeBarrel(int segments);

// A piece of makeBarrel's surface, its edges turned in `rim` along the normals
// so that, laid a little outside the barrel, it reads as cloth with a thickness.
struct BarrelPatch final
{
    // Radians, -pi/2 at the y = 0 end and pi/2 at the y = 1 end.
    Maths::Vec2 latitude {-Maths::halfPi, Maths::halfPi};
    // Radians about y, 0 at +x, turning towards +z; kept on both sides of x, so
    // (0, pi) is the whole way round.
    Maths::Vec2 longitude {0.f, Maths::pi};
    // Keeps what lies between the planes x = across.x and x = across.y.
    Maths::Vec2 across {-1.f, 1.f};
    // Leaves out a round hole about `hole`.
    Maths::Vec3 hole;
    float holeRadius = 0.f;
    float rim = 0.f;
    int rings = 24;
    int segments = 24;
};

MeshData makeBarrelPatch(const BarrelPatch& patch);

// A tapered, rounded cone along y from 0 to 1: a horn.
MeshData makeHorn(int segments);

// A flat-topped cylinder along y from 0 to 1, half a unit in radius; squashed,
// a disc.
MeshData makeCylinder(int segments);

// A cone along y, its base half a unit in radius at y = 0, its point at y = 1.
MeshData makeCone(int segments);

MeshData makePlane(float size);

// A unit box, x and z from -0.5 to 0.5, y from 0 to 1.
MeshData makeBox();

// A ramp on the same footprint, its top rising from y = 0 at x = -0.5 to y = 1
// at x = 0.5.
MeshData makeWedge();

void append(MeshData& mesh, const MeshData& part, const Maths::Mat4& transform = {});

// Turns every triangle to face the way its vertex normals do, so a procedural
// mesh can be built without minding its winding and still be back-face culled.
void windOutward(MeshData& mesh);

Maths::Mat4 normalMatrixFor(const Maths::Mat4& model);

struct Mesh final
{
    explicit Mesh(const MeshData& data);

    Buffer vertices;
    Buffer indices;
    int indexCount = 0;
};
} // namespace Cows
