#pragma once

#include "Cow/Cow.h"
#include "Cow/CowSkin.h"

namespace Cows
{
// The parts of `hat`, on the head bone so they nod with it.
void addHat(Vector<CowPart>& parts, Hat hat);

// The cow in its rest pose wearing `skin`.
Vector<CowPart> makeCowParts(const CowSkin& skin);
} // namespace Cows
