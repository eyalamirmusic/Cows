#include "Levels/LevelGenerator.h"
#include "Levels/Segments/JumpLineSegment.h"
#include "Levels/Segments/MeadowSegment.h"
#include "Levels/Segments/RavineSegment.h"
#include "Fixtures.h"

#include <NanoTest/NanoTest.h>

#include <eacp/Core/Platform/Platform.h>

#include <array>
#include <bit>
#include <cstdint>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
// makeMeadow(seed) for seeds 1 to 40, hashed before levels were generated from
// templates. MSVC's random distributions and maths differ in the low bits, so
// Windows has its own table, taken from the same template.
constexpr auto msvcMeadowGolden = std::to_array<std::uint64_t>({
    11244291730218905906ull, 10343436613414068082ull, 11684292388551233939ull,
    10329903429551420818ull, 16146641879865093557ull, 7326419559184624660ull,
    4355824712913150694ull,  10898734205573658670ull, 8818754057994609395ull,
    1616347605941072906ull,  16613073689392090795ull, 10584052646619483228ull,
    17113113472130015665ull, 15291949470523204505ull, 3034540995451892651ull,
    2044409613191734294ull,  4565468061624003720ull,  13613761503097584892ull,
    2476776270068140728ull,  15730384985877249127ull, 1316048311950576528ull,
    9871167729732207224ull,  7689513085613183989ull,  288985286394413559ull,
    590907176555942712ull,   4324252589140702740ull,  8084594379953467524ull,
    14261557273372356691ull, 315700554286914712ull,   15558760483485305734ull,
    11436925280209628553ull, 6057135330542983343ull,  2782188834484876774ull,
    12657607272428366434ull, 17213503771893865822ull, 8966284539395114966ull,
    8864771927408114120ull,  3431518799143032867ull,  10065454127548761700ull,
    13022393425606858233ull,
});

constexpr auto otherMeadowGolden = std::to_array<std::uint64_t>({
    16968748991025543715ull, 2400430729910096927ull,  14610855147932081439ull,
    3777630825141125684ull,  7498380083352017341ull,  7414868895855385629ull,
    794469300712865879ull,   15683707823397588847ull, 2315092222490707705ull,
    15043323818370707806ull, 13241651763179972440ull, 16695739769091679493ull,
    357954411715264113ull,   5545740502393386506ull,  13471382694833766619ull,
    12404924417128179067ull, 13821898116458693668ull, 18101947618990065113ull,
    12290862214621283750ull, 4040615275994709974ull,  16900446058299264158ull,
    7979796828104393026ull,  7191565459983691127ull,  4588108163046949683ull,
    17815757282466108957ull, 13692107801021102329ull, 8286943516381848608ull,
    1197238106289244543ull,  8430704876180868099ull,  10276022316847053093ull,
    12798085215607819704ull, 11915057734903774126ull, 15703319507716496811ull,
    7364493285580767347ull,  9813079974151635243ull,  13028706465663847859ull,
    18198534311345766270ull, 1290066189693251099ull,  17844490471936195330ull,
    6065830062698745729ull,
});

// x86-64 Linux (CI's runner) differs in the low bits again. arm64 Linux matches
// none of the three.
constexpr auto linuxMeadowGolden = std::to_array<std::uint64_t>({
    1151177960577254616ull,  13738730589382266549ull, 15197971795123286052ull,
    2383273968527815807ull,  2428477535599350953ull,  12072876086045465495ull,
    12574481065660919751ull, 13273426080093746865ull, 2784937808278090082ull,
    12063927317693631913ull, 12430887636966795118ull, 3640544014187368960ull,
    11061612428955070813ull, 5439260746834344065ull,  16289257021953624838ull,
    10532436916426050197ull, 10350579454875552547ull, 14238544770294048238ull,
    353906853697121291ull,   9491961147817210717ull,  7171937648199247643ull,
    12022711299925237007ull, 2365108853393691589ull,  3183307993517245238ull,
    14694277993699789234ull, 9164553411196599106ull,  14674563222518210054ull,
    9300472995833050578ull,  5550271528308836203ull,  16661157769857622003ull,
    3807964431777502747ull,  12774910309662990339ull, 2285899882437085956ull,
    9282679620146109723ull,  5392606008903866686ull,  10959089563826635870ull,
    2324174887429392242ull,  1276525791139328517ull,  3056490746189571217ull,
    5678711752673766838ull,
});

constexpr auto& meadowGolden = Platform::isWindows() ? msvcMeadowGolden
                               : Platform::isLinux() ? linuxMeadowGolden
                                                     : otherMeadowGolden;

std::uint64_t mixIn(std::uint64_t hash, float value)
{
    return (hash ^ std::bit_cast<std::uint32_t>(value)) * 1099511628211ull;
}

std::uint64_t levelHash(const Level& level)
{
    auto hash = 1469598103934665603ull;

    for (const auto& c: level.colliders)
        for (auto v: {c.center.x, c.center.y, c.radius, c.top})
            hash = mixIn(hash, v);

    for (const auto& b: level.blocks)
        for (auto v:
             {b.center.x, b.center.y, b.half.x, b.half.y, b.top, b.rise.x, b.rise.y})
            hash = mixIn(hash, v);

    for (auto v:
         {level.hideout.x, level.hideout.y, level.hideout.z, (float) level.hiding})
        hash = mixIn(hash, v);

    for (const auto& list: level.batch.lists)
        for (const auto& i: list)
            for (auto v: {i.model3.x, i.model3.y, i.model3.z, i.model0.x, i.color.x})
                hash = mixIn(hash, v);

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
    for (auto seed = 1u; seed <= 40u; ++seed)
        check(levelHash(generate(meadowFixture(), seed)) == meadowGolden[seed - 1]);
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
