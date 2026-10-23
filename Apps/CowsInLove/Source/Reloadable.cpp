#include "Reloadable.h"

#include <eacp/Core/Plugins/HotReload.h>

namespace Cows
{
HotReloadedModule::HotReloadedModule(std::string_view sourceDirectory)
{
    if constexpr (!Plugins::isHotReload())
        return;

    auto target = Plugins::hotReloadTarget();
    target.sourceDirectory = sourceDirectory;

    auto& started = loop.emplace(target, std::string {});
    started.onStateChanged = [this] { Console::print(loop->describeState()); };
    started.onReloaded = [this](Plugins::ReloadableLibrary& library)
    { rebindSymbols(library); };
}

void HotReloadedModule::addSymbolBinder(SymbolBinder binder)
{
    binders.add(std::move(binder));
}

void HotReloadedModule::rebindSymbols(Plugins::ReloadableLibrary& library)
{
    for (auto& bind: binders)
        bind(library);

    onReloaded();
}
} // namespace Cows
