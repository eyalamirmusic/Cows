#include "MeshLoader/MeshBlob.h"

#include <NanoTest/NanoTest.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>

using namespace nano;
using namespace Cows;
using namespace eacp;
using namespace eacp::Maths;

namespace
{
// A position's int16 for the quantised value at `share` of the bounds.
constexpr std::int16_t lowest = -32768;
constexpr std::int16_t highest = 32767;
constexpr std::int8_t unit = 127;
// How near a read position is to the one written: half a quantum of 2 m.
constexpr auto tolerance = 1e-4f;

struct BlobWriter final
{
    template <typename Value>
    void put(Value value)
    {
        auto at = bytes.size();
        bytes.resize(at + sizeof(Value));
        std::memcpy(bytes.data() + at, &value, sizeof(Value));
    }

    void putName(const std::string& name)
    {
        put((std::uint8_t) name.size());

        for (auto letter: name)
            put(letter);
    }

    void putVertex(std::int16_t x, std::int16_t y, std::int16_t z, std::int8_t up)
    {
        put(x);
        put(y);
        put(z);
        put((std::int8_t) 0);
        put(up);
        put((std::int8_t) 0);
    }

    std::span<const std::byte> blob() const { return bytes; }

    std::vector<std::byte> bytes;
};

// Two groups in bounds from -1 to 1: a triangle facing up, and a quad facing
// down made of two triangles sharing an edge.
BlobWriter twoGroups()
{
    auto writer = BlobWriter {};

    for (auto letter: {'C', 'M', 'S', 'H'})
        writer.put(letter);

    writer.put((std::uint16_t) 1);
    writer.put((std::uint16_t) 2);

    for (auto bound: {-1.f, -1.f, -1.f, 1.f, 1.f, 1.f})
        writer.put(bound);

    writer.putName("wood");
    writer.put((std::uint32_t) 3);
    writer.put((std::uint32_t) 3);
    writer.putVertex(lowest, 0, lowest, unit);
    writer.putVertex(highest, 0, lowest, unit);
    writer.putVertex(lowest, 0, highest, unit);

    for (std::uint16_t index: {0, 1, 2})
        writer.put(index);

    writer.putName("hay");
    writer.put((std::uint32_t) 4);
    writer.put((std::uint32_t) 6);
    writer.putVertex(lowest, lowest, lowest, -unit);
    writer.putVertex(highest, lowest, lowest, -unit);
    writer.putVertex(highest, lowest, highest, -unit);
    writer.putVertex(lowest, lowest, highest, -unit);

    for (std::uint16_t index: {0, 2, 1, 0, 3, 2})
        writer.put(index);

    return writer;
}

bool near(Vec3 a, Vec3 b)
{
    return length(a - b) < tolerance;
}

std::vector<std::byte> fileBytes(const std::string& name)
{
    auto file = std::ifstream {std::string(COWS_RESOURCES_DIR) + "/" + name,
                               std::ios::binary};
    auto chars = std::vector<char> {std::istreambuf_iterator<char> {file}, {}};
    auto bytes = std::vector<std::byte>(chars.size());
    std::memcpy(bytes.data(), chars.data(), chars.size());
    return bytes;
}

void checkCommitted(const std::string& name,
                    std::initializer_list<const char*> groups)
{
    auto bytes = fileBytes(name);
    auto read = readMeshBlob(bytes);

    check(read.size() == (int) groups.size());

    for (auto group: groups)
    {
        auto mesh = findGroup(read, group);
        check(!mesh.indices.empty());
        check(mesh.indices.size() % 3 == 0);
        check(mesh.positions.size() == mesh.normals.size());

        for (auto index: mesh.indices)
            check(index < (std::uint32_t) mesh.positions.size());
    }
}
} // namespace

auto tGroups = test("MeshBlob/readsEveryGroupByName") = []
{
    auto writer = twoGroups();
    auto groups = readMeshBlob(writer.blob());

    check(groups.size() == 2);
    check(groups[0].name == "wood");
    check(groups[1].name == "hay");
    check(findGroup(groups, "roof").indices.empty());
};

auto tPositions = test("MeshBlob/positionsSpanTheBounds") = []
{
    auto writer = twoGroups();
    auto wood = findGroup(readMeshBlob(writer.blob()), "wood");

    check(near(wood.positions[0], {-1.f, 0.f, -1.f}));
    check(near(wood.positions[1], {1.f, 0.f, -1.f}));
    check(near(wood.normals[2], {0.f, 1.f, 0.f}));
};

auto tIndices = test("MeshBlob/readsSharedCorners") = []
{
    auto writer = twoGroups();
    auto hay = findGroup(readMeshBlob(writer.blob()), "hay");

    check(hay.positions.size() == 4);
    check(hay.indices.size() == 6);
    check(hay.indices[4] == 3);
    check(near(hay.normals[0], {0.f, -1.f, 0.f}));
};

auto tShort = test("MeshBlob/aCutShortBlobReadsAsNothing") = []
{
    auto writer = twoGroups();
    auto cut = writer.blob().first(writer.bytes.size() - 1);

    check(readMeshBlob(cut).empty());
    check(readMeshBlob({}).empty());
};

auto tCommitted = test("MeshBlob/theCommittedMeshesRead") = []
{
    checkCommitted("barn.mesh",
                   {"wall", "trim", "roof", "wood", "hay", "metal", "twine"});
    checkCommitted("bale.mesh", {"straw", "twine"});
};
