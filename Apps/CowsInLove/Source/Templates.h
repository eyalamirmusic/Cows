#pragma once

#include "Levels/LevelTemplate.h"

namespace Cows
{
// One meadow over the whole arena; the player starts in its middle.
LevelTemplate meadowTemplate();

// A meadow, a log and a fence to jump, a ravine with one bridge and bales
// rolling across it, a fence and a log, and a second meadow where she hides.
// The player starts in the first meadow, facing the ravine.
LevelTemplate meadowRavineTemplate();
} // namespace Cows
