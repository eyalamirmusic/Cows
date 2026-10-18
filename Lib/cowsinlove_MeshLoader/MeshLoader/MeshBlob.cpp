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

std::optional<MeshGroup> readGroup(Reader& reader, Vec3 low, Vec3 high)
{
    auto group = MeshGroup {};
    auto length = reader.read<std::uint8_t>();

    for (auto letter = 0; length && letter < *length; ++letter)
        group.name += reader.read<char>().value_or('?');

    auto vertices = reader.read<std::uint32_t>();
    auto indices = reader.read<std::uint32_t>();

    if (!indices)
        return std::nullopt;

    for (std::uint32_t vertex = 0; vertex < *vertices; ++vertex)
    {
        auto position = std::array<std::int16_t, 3> {};
        auto normal = std::array<std::int8_t, 3> {};

        for (auto& axis: position)
            axis = reader.read<std::int16_t>().value_or(0);

        for (auto& axis: normal)
            axis = reader.read<std::int8_t>().value_or(0);

        group.positions.add({unquantise(position[0], low.x, high.x),
                             unquantise(position[1], low.y, high.y),
                             unquantise(position[2], low.z, high.z)});
        group.normals.add(normalize(Vec3 {(float) normal[0] / normalScale,
                                          (float) normal[1] / normalScale,
                                          (float) normal[2] / normalScale}));
    }

    auto wide = *vertices >= widestShortIndex;

    for (std::uint32_t index = 0; index < *indices; ++index)
    {
        auto value = wide ? reader.read<std::uint32_t>() : readShort(reader);

        if (!value || *value >= *vertices)
            return std::nullopt;

        group.indices.add(*value);
    }

    return group;
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

    if (!high || *header != magic || *blobVersion != version)
        return {};

    auto groups = Vector<MeshGroup> {};

    for (auto group = 0; group < *count; ++group)
    {
        auto read = readGroup(reader, *low, *high);

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
