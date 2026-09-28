#include "Levels/Segments/MoverSpawner.h"
#include "Props/Bale.h"

namespace Cows
{
void MoverSpawner::spawn(Level& level) const
{
    level.movers.add(makeRollingBale(from, to, period, phase));
}
} // namespace Cows
