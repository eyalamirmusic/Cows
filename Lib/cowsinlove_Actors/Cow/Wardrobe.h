#pragma once

#include "Cow/Cow.h"
#include "Cow/CowSkin.h"
#include "Render/Mesh.h"

#include <functional>

namespace Cows
{
// The parts of `hat`, on the head bone so they nod with it.
void addHat(Vector<CowPart>& parts, Hat hat);

// The parts of `pants`, on the body bone with the legs.
void addPants(Vector<CowPart>& parts, Pants pants);

// The mesh of each pants Shape (Belly, Seat and their waistbands): pieces of the
// torso's Barrel, worn on its placement scaled out.
MeshData makePantsMesh(Shape shape, float detail = 1.f);

// The meshes ShapeMeshes leaves to the game: the heart, the hats and the pants.
MeshData makeCowMesh(Shape shape);

// makeCowMesh with every mesh's steps coarsened by `detail` (see `detailed`).
std::function<MeshData(Shape)> cowMeshes(float detail);

// The cow in its rest pose wearing `skin`.
Vector<CowPart> makeCowParts(const CowSkin& skin);
} // namespace Cows
