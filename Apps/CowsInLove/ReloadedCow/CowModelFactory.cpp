#include "ReloadedCowModel.h"

#include <eacp/Core/Plugins/ReloadableEntry.h>

namespace
{
Cows::CowModel* makeCowModel()
{
    return new Cows::ReloadedCowModel;
}
} // namespace

EACP_RELOADABLE(makeCowModel)
