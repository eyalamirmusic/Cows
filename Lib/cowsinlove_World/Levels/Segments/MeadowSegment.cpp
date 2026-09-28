#include "Levels/Segments/MeadowSegment.h"

namespace Cows
{
MeadowSegment::MeadowSegment(float lengthToUse)
{
    length = lengthToUse;
    biome = meadowBiome();
}
} // namespace Cows
