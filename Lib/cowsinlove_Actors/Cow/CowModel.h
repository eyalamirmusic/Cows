#pragma once

#include "Cow/Cow.h"
#include "Cow/CowPhysics.h"

#include <eacp/Core/Plugins/Reloadable.h>

namespace Cows
{
// The cow's parts and jump, held through a Plugins::Reloadable so a hot-reload
// build runs the module's latest code.
struct CowModel
{
    EACP_RELOADABLE Vector<CowPart> makeParts() const;
    EACP_RELOADABLE JumpPhysics getJumpPhysics() const;
};
} // namespace Cows
