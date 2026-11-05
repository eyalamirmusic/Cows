#include "ObjLoader/ObjFile.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace eacp;
using namespace eacp::Maths;

namespace
{
constexpr auto twoGroups = R"(# a quad and a triangle
mtllib barn.mtl
v 0 0 0
v 1 0 0
v 1 0 1
v 0 0 1
v 0 1 0
vn 0 1 0
vn 0 0 2
usemtl wood
f 1//1 2//1 3//1 4//1
usemtl hay
f 1//2 2//2 5//2
usemtl wood
f 1//1 3//1 4//1
)";

constexpr auto tolerance = 1e-5f;

bool near(Vec3 a, Vec3 b)
{
    return length(a - b) < tolerance;
}
} // namespace

auto tGroups = test("ObjFile/groupsByMaterial") = []
{
    auto groups = parseObj(twoGroups);

    check(groups.size() == 2);
    check(groups[0].name == "wood");
    check(groups[1].name == "hay");
};

auto tQuad = test("ObjFile/fansQuadsAndJoinsGroupsAgain") = []
{
    auto wood = findGroup(parseObj(twoGroups), "wood");

    check(wood.indices.size() == 9);
    check(wood.positions.size() == 4);
    check(wood.indices[3] == 0 && wood.indices[4] == 2 && wood.indices[5] == 3);
};

auto tNormals = test("ObjFile/keepsTheFilesNormals") = []
{
    auto hay = findGroup(parseObj(twoGroups), "hay");

    check(hay.positions.size() == 3);
    check(near(hay.positions[2], {0.f, 1.f, 0.f}));

    for (const auto& normal: hay.normals)
        check(near(normal, {0.f, 0.f, 1.f}));
};

auto tSharedCorner = test("ObjFile/splitsACornerWithTwoNormals") = []
{
    auto text = "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nvn 0 0 -1\n"
                "f 1//1 2//1 3//1\nf 1//2 3//2 2//2\n";
    auto groups = parseObj(text);

    check(groups.size() == 1);
    check(groups[0].positions.size() == 6);
};

auto tMissing = test("ObjFile/missingGroupIsEmpty") = []
{ check(findGroup(parseObj(twoGroups), "roof").indices.empty()); };
