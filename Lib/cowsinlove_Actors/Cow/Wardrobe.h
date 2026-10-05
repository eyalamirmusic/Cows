#pragma once

#include "Cow/Cow.h"
#include "Cow/CowSkin.h"

namespace Cows
{
// The parts of `hat`, on the head bone so they nod with it.
void addHat(Vector<CowPart>& parts, Hat hat);

// The parts of `pants`, on the body bone with the legs.
void addPants(Vector<CowPart>& parts, Pants pants);

// The cow in its rest pose wearing `skin`.
Vector<CowPart> makeCowParts(const CowSkin& skin);
} // namespace Cows
