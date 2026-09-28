#include "Animation/Choreography.h"
#include "Cow/KissHearts.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
struct Burst final
{
    explicit Burst(float seconds)
    {
        addKissHearts(batch, glows, seconds, {0.f, 3.f, 0.f});
    }

    int hearts() const { return (int) batch.lists[(int) Shape::Heart].size(); }

    SurfaceBatch batch;
    Vector<GlowInstance> glows;
};
} // namespace

auto tNoneBeforeFirstKiss = test("KissHearts/noneBeforeTheFirstKiss") = []
{
    for (auto seconds: {0.f, 0.5f, 1.f, 2.f})
    {
        auto burst = Burst {seconds};
        check(burst.hearts() == 0);
        check(burst.glows.empty());
    }
};

auto tAppearAtKiss = test("KissHearts/appearAroundEachKiss") = []
{
    for (auto kiss = 0; kiss < 4; ++kiss)
    {
        auto burst = Burst {Choreography::kissTime(kiss) + 0.5f};
        check(burst.hearts() > 0);
        check(!burst.glows.empty());
    }
};

auto tNoneWellBefore = test("KissHearts/noneWellBeforeAKiss") = []
{
    for (auto kiss = 1; kiss < 4; ++kiss)
    {
        auto burst = Burst {Choreography::kissTime(kiss) - 2.f};
        check(burst.hearts() == 0);
    }
};
