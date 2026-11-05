#include "ObjLoader/ObjFile.h"

#include <cstdlib>
#include <map>
#include <sstream>
#include <utility>

using namespace eacp;
using namespace eacp::Maths;

namespace Cows
{
namespace
{
// A face corner's position and normal, counted from 0.
using Corner = std::pair<int, int>;

struct GroupBuilder final
{
    ObjGroup group;
    std::map<Corner, std::uint32_t> corners;
};

Corner readCorner(const std::string& word)
{
    auto position = std::atoi(word.c_str());
    auto normal = std::atoi(word.substr(word.rfind('/') + 1).c_str());
    return {position - 1, normal - 1};
}

Vec3 readVec3(std::istringstream& line)
{
    auto x = 0.f;
    auto y = 0.f;
    auto z = 0.f;
    line >> x >> y >> z;
    return {x, y, z};
}

struct Reader final
{
    GroupBuilder& current()
    {
        for (auto& builder: builders)
            if (builder.group.name == name)
                return builder;

        builders.add(GroupBuilder {ObjGroup {name, {}, {}, {}}, {}});
        return builders.back();
    }

    std::uint32_t vertexFor(GroupBuilder& builder, Corner corner)
    {
        auto found = builder.corners.find(corner);

        if (found != builder.corners.end())
            return found->second;

        auto& group = builder.group;
        auto index = (std::uint32_t) group.positions.size();
        group.positions.add(positions[corner.first]);
        group.normals.add(normals[corner.second]);
        builder.corners.emplace(corner, index);
        return index;
    }

    void readFace(std::istringstream& line)
    {
        auto& builder = current();
        auto indices = Vector<std::uint32_t> {};
        auto word = std::string {};

        while (line >> word)
            indices.add(vertexFor(builder, readCorner(word)));

        for (auto corner = 1; corner + 1 < (int) indices.size(); ++corner)
            builder.group.indices.add(
                {indices[0], indices[corner], indices[corner + 1]});
    }

    void readLine(const std::string& text)
    {
        auto line = std::istringstream {text};
        auto keyword = std::string {};
        line >> keyword;

        if (keyword == "v")
            positions.add(readVec3(line));
        else if (keyword == "vn")
            normals.add(normalize(readVec3(line)));
        else if (keyword == "f")
            readFace(line);
        else if (keyword == "usemtl")
            line >> name;
    }

    Vector<Vec3> positions;
    Vector<Vec3> normals;
    Vector<GroupBuilder> builders;
    std::string name;
};
} // namespace

Vector<ObjGroup> parseObj(std::string_view text)
{
    auto reader = Reader {};
    auto stream = std::istringstream {std::string {text}};
    auto line = std::string {};

    while (std::getline(stream, line))
        reader.readLine(line);

    auto groups = Vector<ObjGroup> {};

    for (auto& builder: reader.builders)
        groups.add(std::move(builder.group));

    return groups;
}

ObjGroup findGroup(const Vector<ObjGroup>& groups, std::string_view name)
{
    for (const auto& group: groups)
        if (group.name == name)
            return group;

    return {};
}
} // namespace Cows
