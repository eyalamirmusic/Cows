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

// The part of makeBarrel's surface between two latitudes (radians, -pi/2 at its
// y = 0 end, pi/2 at its y = 1 end) and two longitudes (radians about y, 0 at
// +x, turning towards +z, and may run past pi), each as (from, to). Every edge
// turns in `rim` along its normals, so laid a little outside the barrel it reads
// as cloth with a thickness rather than a painted line.
MeshData makeBarrelPatch(
    Maths::Vec2 latitude, Maths::Vec2 longitude, float rim, int rings, int segments);

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
