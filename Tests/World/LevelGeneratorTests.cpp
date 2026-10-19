#include "Levels/LevelGenerator.h"
#include "Levels/Segments/JumpLineSegment.h"
#include "Levels/Segments/RavineSegment.h"
#include "Fixtures.h"

#include <NanoTest/NanoTest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
// Seeds 1 to 40. The draws are our own (randomUnit) and the targets build without
// FP contraction, so every platform generates the same level; values are rounded
// onto a grid before hashing so each libm's low bits in sin/cos do not change the
// hashes.
constexpr auto meadowGolden = std::to_array<std::uint64_t>(
    {14480292058536490485ull, 8015345250225959874ull,  8789866100485915750ull,
     16717649394203799234ull, 13763610983462542313ull, 9682631651030597886ull,
     9886301906639225336ull,  6084571856542902450ull,  12386465339021268291ull,
     15812591158408350634ull, 5591460055931223932ull,  12822614561260056047ull,
     10437677151532268105ull, 10995095398919459436ull, 6330020660465075333ull,
     8209104958805570139ull,  2751241026483976962ull,  11943908367227923581ull,
     8826563698048817916ull,  7317692226698534714ull,  1049854683543062534ull,
     12999828896507208182ull, 14692699536144887400ull, 9821701091982190698ull,
     1045014426940583875ull,  10805239616681081618ull, 12292938942928540222ull,
     9910400080323819483ull,  733317952517400094ull,   8129175193684434743ull,
     13864586895288654721ull, 9511476432751772560ull,  15676434753219874503ull,
     16977566862471103744ull, 4429653172043812491ull,  3908761681763050908ull,
     12535060082850075443ull, 14459583813896351785ull, 13227326960405452264ull,
     377148785936289669ull});

constexpr auto hashGridStepsPerUnit = 256.f;
constexpr auto fnvOffset = 1469598103934665603ull;
constexpr auto fnvPrime = 1099511628211ull;

std::uint64_t mixRoundedValueIntoHash(std::uint64_t hash, float value)
{
    auto valueOnGrid =
        static_cast<std::uint64_t>(std::llround(value * hashGridStepsPerUnit));
    return (hash ^ valueOnGrid) * fnvPrime;
}

using MeadowHashes = std::array<std::uint64_t, meadowGolden.size()>;

void printMeadowHashesAsTable(const MeadowHashes& hashes)
{
    constexpr auto hashesPerLine = 3;
    std::cout << "meadow golden hashes, seeds 1 to " << hashes.size() << ":\n";

    for (auto index = 0u; index < hashes.size(); ++index)
    {
        const auto* separator = (index + 1) % hashesPerLine == 0 ? "\n" : " ";
        std::cout << (index % hashesPerLine == 0 ? "    " : "") << hashes[index]
                  << "ull," << separator;
    }

    std::cout << std::endl;
}

std::uint64_t hashLevelLayout(const Level& level)
{
    auto hash = fnvOffset;

    for (const auto& c: level.colliders)
        for (auto v: {c.center.x, c.center.y, c.radius, c.top})
            hash = mixRoundedValueIntoHash(hash, v);

    for (const auto& b: level.blocks)
        for (auto v:
             {b.center.x, b.center.y, b.half.x, b.half.y, b.top, b.rise.x, b.rise.y})
            hash = mixRoundedValueIntoHash(hash, v);

    auto hidingPlace = static_cast<float>(level.hiding);

    for (auto v: {level.hideout.x, level.hideout.y, level.hideout.z, hidingPlace})
        hash = mixRoundedValueIntoHash(hash, v);

    for (const auto& list: level.batch.lists)
        for (const auto& i: list)
            for (auto v: {i.model3.x, i.model3.y, i.model3.z, i.model0.x, i.color.x})
                hash = mixRoundedValueIntoHash(hash, v);

    return hash;
}

struct Recorder final : Segment
{
    Recorder(float lengthToUse, Vector<Region>& builtToUse)
        : built(builtToUse)
    {
        length = lengthToUse;
    }

    void build(Level&, Layout&, Region region) const override { built.add(region); }

    Vector<Region>& built;
};

bool touchesPath(const Region& path, const Collider& collider)
{
    return path.contains(collider.center, collider.radius);
}

bool touchesPath(const Region& path, const Cows::Block& block)
{
    return block.center.x + block.half.x > path.min.x
           && block.center.x - block.half.x < path.max.x
           && block.center.y + block.half.y > path.min.y
           && block.center.y - block.half.y < path.max.y;
}
} // namespace

auto tMeadowGolden = test("LevelGenerator/meadowTemplateIsTodaysMeadow") = []
{
    auto hashes = MeadowHashes {};

    for (auto seed = 1u; seed <= hashes.size(); ++seed)
        hashes[seed - 1] = hashLevelLayout(generate(meadowFixture(), seed));

    if (hashes != meadowGolden)
        printMeadowHashesAsTable(hashes);

    check(hashes == meadowGolden);
};

auto tMeadowStart = test("LevelGenerator/meadowStartsAtTheOrigin") = []
{
    auto level = generate(meadowFixture(), 3);
    check(level.start.x == 0.f && level.start.y == 0.f && level.start.z == 0.f);
    check(level.startHeading == 0.f);
    check(level.criticalPath.isEmpty());
    check(level.gaps.empty() && level.movers.empty());
    check(level.killDepth == 0.f);
};

auto tInOrder = test("LevelGenerator/segmentsLaidInOrderAlongZ") = []
{
    auto built = Vector<Region> {};
    auto levelTemplate = LevelTemplate {};

    for (auto length: {30.f, 10.f, 60.f})
        levelTemplate.segments.add(std::make_shared<Recorder>(length, built));

    auto regions = segmentRegions(levelTemplate);
    generate(levelTemplate, 5);

    check(regions.size() == 3 && built.size() == 3);
    check(regions[0].max.y == 50.f && regions[2].min.y == -50.f);

    for (auto index = 0; index < regions.size(); ++index)
    {
        check(built[index].min.y == regions[index].min.y);
        check(built[index].max.y == regions[index].max.y);
        check(regions[index].min.x == -arenaSize * 0.5f);
        check(regions[index].max.x == arenaSize * 0.5f);

        if (index > 0)
            check(regions[index].max.y == regions[index - 1].min.y);
    }
};

auto tCriticalPath = test("LevelGenerator/criticalPathIsClear") = []
{
    auto levelTemplate = meadowRavineFixture();
    auto regions = segmentRegions(levelTemplate);

    for (auto seed = 1u; seed <= 20u; ++seed)
    {
        auto level = generate(levelTemplate, seed);
        const auto& path = level.criticalPath;

        check(!path.isEmpty());
        check(path.contains({level.start.x, level.start.z}));
        check(path.min.y == regions.back().max.y);

        for (const auto& collider: level.colliders)
            check(!touchesPath(path, collider));

        for (const auto& block: level.blocks)
        {
            if (!touchesPath(path, block))
                continue;

            auto owner = -1;

            for (auto index = 0; index < regions.size(); ++index)
                if (regions[index].contains(block.center))
                    owner = index;

            check(owner >= 0 && !levelTemplate.segments[owner]->biome);
        }
    }
};

auto tSegmentsStayInRegion =
    test("LevelGenerator/segmentsBuildOnlyInTheirRegion") = []
{
    auto region = Region {{-100.f, 20.f}, {100.f, 40.f}};
    auto layout = Layout {9u};
    layout.path = {{-3.f, -100.f}, {3.f, 100.f}};

    auto segments = SegmentList {
        std::make_shared<JumpLineSegment>(JumpLineSegment::Obstacle::Log),
        std::make_shared<JumpLineSegment>(JumpLineSegment::Obstacle::Fence),
        std::make_shared<RavineSegment>()};

    for (const auto& segment: segments)
    {
        auto level = Level {};
        segment->build(level, layout, region);

        for (const auto& block: level.blocks)
        {
            check(region.contains(block.center - block.half));
            check(region.contains(block.center + block.half));
        }

        for (const auto& gap: level.gaps)
        {
            check(region.contains(gap.center - gap.half));
            check(region.contains(gap.center + gap.half));
        }

        for (const auto& mover: level.movers)
            check(region.contains(mover.from) && region.contains(mover.to));
    }
};
