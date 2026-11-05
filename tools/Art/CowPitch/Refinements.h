#pragma once

#include "Designs.h"

#include <functional>

namespace Cows::Pitch
{
// One small change to today's cow, shown before and after from `shot`.
struct Refinement final
{
    std::string name;
    Shot shot;
    std::function<void(Design&)> apply = [](Design&) {};
};

const Vector<Refinement>& refinements();

// Today's cow with the named refinements applied, in order.
Design refined(const Vector<std::string>& names);
} // namespace Cows::Pitch
