#include "Render/Frustum.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
Mat4 lookingDownMinusZ()
{
    auto projection = Mat4::perspective(1.f, 1.f, 0.1f, 200.f);
    return projection
           * Mat4::lookAt({0.f, 2.f, 0.f}, {0.f, 2.f, -1.f}, {0.f, 1.f, 0.f});
}
} // namespace

auto tAhead = test("Frustum/aBoxAheadIsInView") = []
{ check(boxInView(lookingDownMinusZ(), {-1.f, 0.f, -11.f}, {1.f, 1.f, -9.f})); };

auto tBehind = test("Frustum/aBoxBehindTheEyeIsNot") = []
{ check(!boxInView(lookingDownMinusZ(), {-1.f, 0.f, 9.f}, {1.f, 1.f, 11.f})); };

auto tSide = test("Frustum/aBoxFarToTheSideIsNot") = []
{
    check(!boxInView(lookingDownMinusZ(), {40.f, 0.f, -11.f}, {42.f, 1.f, -9.f}));
    check(!boxInView(lookingDownMinusZ(), {-42.f, 0.f, -11.f}, {-40.f, 1.f, -9.f}));
};

auto tStraddles = test("Frustum/aBoxAroundTheEyeIsInView") = []
{ check(boxInView(lookingDownMinusZ(), {-5.f, 0.f, -5.f}, {5.f, 1.f, 5.f})); };

auto tAcrossEdge = test("Frustum/aBoxAcrossTheEdgeOfViewIsInView") = []
{ check(boxInView(lookingDownMinusZ(), {5.f, 0.f, -11.f}, {30.f, 1.f, -9.f})); };

auto tSphere = test("Frustum/aSphereReachingIntoViewIsInView") = []
{
    check(sphereInView(lookingDownMinusZ(), {9.f, 2.f, -10.f}, 4.f));
    check(!sphereInView(lookingDownMinusZ(), {9.f, 2.f, -10.f}, 1.f));
};

auto tInstanceScale = test("Frustum/anInstanceGrowsItsSphereByItsScale") = []
{
    auto at = Mat4::translation({9.f, 2.f, -10.f});
    auto small = makeInstance(at, Material {});
    auto large = makeInstance(at * Mat4::scale(4.f), Material {});

    check(!instanceInView(lookingDownMinusZ(), small, 1.f));
    check(instanceInView(lookingDownMinusZ(), large, 1.f));
};

auto tShadowBox = test("Frustum/anOrthographicBoxKeepsOnlyWhatIsInsideIt") = []
{
    auto box = Mat4::orthographic(-14.f, 14.f, -14.f, 14.f, 1.f, 60.f)
               * Mat4::lookAt({0.f, 30.f, 0.f}, {}, {0.f, 0.f, -1.f});

    check(sphereInView(box, {10.f, 0.f, 10.f}, 1.f));
    check(!sphereInView(box, {40.f, 0.f, 0.f}, 1.f));
};
