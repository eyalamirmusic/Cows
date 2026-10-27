#include "TestCow.h"

#include <eacp/Core/Plugins/Reloadable.h>

#include <stdexcept>

namespace Cows
{
namespace
{
struct CowForTests final
{
    Plugins::Reloadable cowCode {};
    OwningPointer<CowModel> model = cowCode.create<CowModel>();
};
} // namespace

const CowModel& getCowModelForTests()
{
    static auto cow = CowForTests {};

    if (!cow.model)
        throw std::runtime_error("ReloadedCow's makeCowModel made no cow");

    return *cow.model;
}
} // namespace Cows
