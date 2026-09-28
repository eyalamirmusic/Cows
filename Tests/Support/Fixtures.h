#pragma once

#include "Levels/LevelTemplate.h"

namespace Cows
{
// The two level templates the library tests are written against: they mirror
// the app's stages but belong to the tests, so the libraries ship no content.
LevelTemplate meadowFixture();
LevelTemplate meadowRavineFixture();
} // namespace Cows
