#include "MeshLoader/MeshBlob.h"

#include <array>
#include <bit>
#include <cstring>
#include <optional>

using namespace eacp;
using namespace eacp::Maths;

namespace Cows
{
namespace
{
static_assert(std::endian::native == std::endian::little,
              "mesh blobs are little-endian, as every target is");

constexpr std::array<char, 4> magic {'C', 'M', 'S', 'H'};
constexpr std::uint16_t version = 1;
constexpr auto quanta = 65535.f;
constexpr auto halfQuanta = 32768.f;
constexpr auto normalScale = 127.f;
constexpr std::uint32_t widestShortIndex = 65536;

struct Reader final
{
    template <typename Value>
    std::optional<Value> read()
    {
        if (blob.size() - offset < sizeof(Value))
            return std::nullopt;

        auto value = Value {};
        std::memcpy(&value, blob.data() + offset, sizeof(Value));
        offset += sizeof(Value);
        return value;
    }

    std::optional<Vec3> readFloats()
    {
        auto x = read<float>();
        auto y = read<float>();
        auto z = read<float>();

        if (!z)
            return std::nullopt;

        return Vec3 {*x, *y, *z};
    }

    std::span<const std::byte> blob;
    std::size_t offset = 0;
};

float unquantise(std::int16_t value, float low, float high)
{
    return low + ((float) value + halfQuanta) / quanta * (high - low);
}

std::optional<std::uint32_t> readShort(Reader& reader)
{
    if (auto narrow = reader.read<std::uint16_t>())
        return *narrow;

    return std::nullopt;
}

struct QuantisationBounds final
{
    Vec3 low;
    Vec3 high;
};

std::string readGroupName(Reader& reader)
{
    auto name = std::string {};
    auto length = reader.read<std::uint8_t>();

    for (auto letter = 0; length && letter < *length; ++letter)
        name += reader.read<char>().value_or('?');

    return name;
}

template <typename Component, std::size_t Count>
std::array<Component, Count> readComponents(Reader& reader)
{
    auto components = std::array<Component, Count> {};

    for (auto& axis: components)
        axis = reader.read<Component>().value_or(0);

    return components;
}

Vec3 unquantisePosition(const std::array<std::int16_t, 3>& position,
                        const QuantisationBounds& bounds)
{
    return {unquantise(position[0], bounds.low.x, bounds.high.x),
            unquantise(position[1], bounds.low.y, bounds.high.y),
            unquantise(position[2], bounds.low.z, bounds.high.z)};
}

Vec3 unquantiseNormal(const std::array<std::int8_t, 3>& normal)
{
    return normalize(Vec3 {(float) normal[0] / normalScale,
                           (float) normal[1] / normalScale,
                           (float) normal[2] / normalScale});
}

void readVertex(Reader& reader, const QuantisationBounds& bounds, MeshGroup& group)
{
    auto position = readComponents<std::int16_t, 3>(reader);
    auto normal = readComponents<std::int8_t, 3>(reader);

    group.positions.add(unquantisePosition(position, bounds));
    group.normals.add(unquantiseNormal(normal));
}

std::optional<std::uint32_t> readIndex(Reader& reader, bool wide)
{
    if (wide)
        return reader.read<std::uint32_t>();

    return readShort(reader);
}

bool isValidIndex(const std::optional<std::uint32_t>& index,
                  std::uint32_t vertexCount)
{
    return index && *index < vertexCount;
}

bool readIndices(Reader& reader,
                 std::uint32_t indexCount,
                 std::uint32_t vertexCount,
                 MeshGroup& group)
{
    auto wide = vertexCount >= widestShortIndex;

    for (std::uint32_t index = 0; index < indexCount; ++index)
    {
        auto value = readIndex(reader, wide);

        if (!isValidIndex(value, vertexCount))
            return false;

        group.indices.add(*value);
    }

    return true;
}

std::optional<MeshGroup> readGroup(Reader& reader, const QuantisationBounds& bounds)
{
    auto group = MeshGroup {};
    group.name = readGroupName(reader);

    auto vertices = reader.read<std::uint32_t>();
    auto indices = reader.read<std::uint32_t>();

    if (!indices)
        return std::nullopt;

    for (std::uint32_t vertex = 0; vertex < *vertices; ++vertex)
        readVertex(reader, bounds, group);

    if (!readIndices(reader, *indices, *vertices, group))
        return std::nullopt;

    return group;
}

bool hasExpectedHeader(const std::array<char, 4>& header, std::uint16_t blobVersion)
{
    return header == magic && blobVersion == version;
}
} // namespace

Vector<MeshGroup> readMeshBlob(std::span<const std::byte> blob)
{
    auto reader = Reader {blob};
    auto header = reader.read<std::array<char, 4>>();
    auto blobVersion = reader.read<std::uint16_t>();
    auto count = reader.read<std::uint16_t>();
    auto low = reader.readFloats();
    auto high = reader.readFloats();

    if (!high || !hasExpectedHeader(*header, *blobVersion))
        return {};

    auto groups = Vector<MeshGroup> {};

    for (auto group = 0; group < *count; ++group)
    {
        auto read = readGroup(reader, QuantisationBounds {*low, *high});

        if (!read)
            return {};

        groups.add(std::move(*read));
    }

    return groups;
}

MeshGroup findGroup(const Vector<MeshGroup>& groups, std::string_view name)
{
    for (const auto& group: groups)
        if (group.name == name)
            return group;

    return {};
}
} // namespace Cows
