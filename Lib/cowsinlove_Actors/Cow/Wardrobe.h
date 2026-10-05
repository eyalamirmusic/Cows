#pragma once

#include "Cow/Cow.h"
#include "Cow/CowSkin.h"
#include "Render/Mesh.h"

namespace Cows
{
// The parts of `hat`, on the head bone so they nod with it.
void addHat(Vector<CowPart>& parts, Hat hat);

// The parts of `pants`, on the body bone with the legs.
void addPants(Vector<CowPart>& parts, Pants pants);

// The mesh of each pants Shape (Belly, Seat and their waistbands): pieces of the
// torso's Barrel, worn on its placement scaled out.
MeshData makePantsMesh(Shape shape);

// The meshes ShapeMeshes leaves to the game: the heart and the pants.
MeshData makeCowMesh(Shape shape);

// The cow in its rest pose wearing `skin`.
Vector<CowPart> makeCowParts(const CowSkin& skin);
} // namespace Cows
