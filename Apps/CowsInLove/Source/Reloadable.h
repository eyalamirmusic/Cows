#pragma once

#include "Render/Common.h"

#include <eacp/Core/Plugins/HotReloadLoop.h>

#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
// The reload loop for the module eacp_add_hot_reload builds in the app's
// CMakeLists, watching sourceDirectory. In a production build it holds no loop
// and every Reloadable calls its compiled-in copy.
class HotReloadedModule
{
public:
    explicit HotReloadedModule(std::string_view sourceDirectory);

    using SymbolBinder = std::function<void(Plugins::ReloadableLibrary& library)>;
    void addSymbolBinder(SymbolBinder binder);

    std::function<void()> onReloaded = [] {};

private:
    void rebindSymbols(Plugins::ReloadableLibrary& library);

    Vector<SymbolBinder> binders;
    std::optional<Plugins::HotReloadLoop> loop;
};

// A function the module exports, called through the latest loaded generation.
template <auto compiledIn>
class Reloadable
{
public:
    Reloadable(HotReloadedModule& module, std::string symbolName)
        : name(std::move(symbolName))
    {
        module.addSymbolBinder([this](Plugins::ReloadableLibrary& library)
                               { bindTo(library); });
    }

    template <typename... Args>
    auto operator()(Args&&... args) const
    {
        return current(std::forward<Args>(args)...);
    }

private:
    using Function = decltype(compiledIn);

    void bindTo(Plugins::ReloadableLibrary& library)
    {
        auto loaded = library.findFunction<Function>(name);

        if (loaded == nullptr)
            Console::print("hot reload: no " + name
                           + ", using the compiled-in copy");

        current = loaded != nullptr ? loaded : compiledIn;
    }

    std::string name;
    Function current = compiledIn;
};
} // namespace Cows
