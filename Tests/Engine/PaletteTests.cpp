#include "Render/Palette.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;

namespace
{
bool near(float a, float b)
{
    return std::abs(a - b) <= 1e-4f;
}
} // namespace

auto tToLinearKnownValues = test("Palette/toLinearKnownValues") = []
{
    check(near(Palette::toLinear(0.f), 0.f));
    check(near(Palette::toLinear(1.f), 1.f));
    check(near(Palette::toLinear(0.5f), 0.217638f));
};

auto tLinearFromHex = test("Palette/linearFromHex") = []
{
    auto white = Palette::linear(0xffffff);
    check(near(white.x, 1.f) && near(white.y, 1.f) && near(white.z, 1.f));

    auto black = Palette::linear(0x000000);
    check(near(black.x, 0.f) && near(black.y, 0.f) && near(black.z, 0.f));

    auto sun = Palette::linear(Palette::sun);
    check(near(sun.x, 1.f) && near(sun.y, 1.f) && near(sun.z, 0.f));

    auto grey = Palette::linear(0x808080);
    check(near(grey.x, std::pow(128.f / 255.f, 2.2f)));
    check(near(grey.x, grey.y) && near(grey.y, grey.z));
};
