#include "Render/Shading.h"

#include <NanoTest/NanoTest.h>

#include <algorithm>

using namespace nano;
using namespace Cows;

auto tLatticeSize = test("NoiseLattice/holdsFourPlanesPerCell") = []
{
    auto texels = makeNoiseLattice();
    check(texels.size() == NoiseLattice::size * NoiseLattice::size * 4);
};

auto tLatticeSpread = test("NoiseLattice/spreadsOverTheWholeRange") = []
{
    auto texels = makeNoiseLattice();

    for (auto plane = 0; plane < 4; ++plane)
    {
        auto lowest = 255;
        auto highest = 0;
        auto sum = 0.0;
        auto count = 0;

        for (auto at = plane; at < (int) texels.size(); at += 4)
        {
            lowest = std::min(lowest, (int) texels[at]);
            highest = std::max(highest, (int) texels[at]);
            sum += texels[at];
            ++count;
        }

        check(lowest < 25);
        check(highest > 230);
        check(std::abs(sum / count - 127.5) < 10.0);
    }
};

auto tLatticePlanesDiffer = test("NoiseLattice/planesAreIndependent") = []
{
    auto texels = makeNoiseLattice();
    auto same = 0;

    for (auto at = 0; at < (int) texels.size(); at += 4)
        same += texels[at] == texels[at + 1] ? 1 : 0;

    check(same < (int) texels.size() / 4 / 20);
};
