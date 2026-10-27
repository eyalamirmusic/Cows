#pragma once

#include "Cow/Cow.h"
#include "Cow/CowModel.h"
#include "Cow/CowSkin.h"
#include "Render/Mesh.h"

#include <functional>

namespace Cows
{
void addHat(Vector<CowPart>& parts, Hat hat);

void addPants(Vector<CowPart>& parts, Pants pants);

MeshData makePantsMesh(Shape shape, float detail = 1.f);

std::function<MeshData(Shape)> cowMeshes(float detail);

Vector<CowPart> makeDressedCowParts(const CowModel& model, const CowSkin& skin);
} // namespace Cows
