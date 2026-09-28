#include "Props/Collision.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
bool near(float a, float b)
{
    return std::abs(a - b) < 1e-4f;
}

bool near(Vec2 a, Vec2 b)
{
    return near(a.x, b.x) && near(a.y, b.y);
}
} // namespace

auto tFlatHeight = test("Collision/flatBlockHeight") = []
{
    auto block = Cows::Block {{2.f, 3.f}, {1.f, 2.f}, 1.5f, {}};

    check(near(block.heightAt({2.f, 3.f}), 1.5f));
    check(near(block.heightAt({2.9f, 4.9f}), 1.5f));
};

auto tRampHeight = test("Collision/rampRisesAlongItsRise") = []
{
    auto ramp = Cows::Block {{0.f, 0.f}, {3.5f, 1.5f}, 2.f, {1.f, 0.f}};

    check(near(ramp.heightAt({-3.5f, 0.f}), 0.f));
    check(near(ramp.heightAt({0.f, 0.f}), 1.f));
    check(near(ramp.heightAt({3.5f, 0.f}), 2.f));
    check(near(ramp.heightAt({0.f, 1.f}), 1.f));
    check(near(ramp.heightAt({-10.f, 0.f}), 0.f));
    check(near(ramp.heightAt({10.f, 0.f}), 2.f));

    auto turned = Cows::Block {{0.f, 0.f}, {1.5f, 3.5f}, 2.f, {0.f, -1.f}};
    check(near(turned.heightAt({0.f, 3.5f}), 0.f));
    check(near(turned.heightAt({0.f, -3.5f}), 2.f));
};

auto tContains = test("Collision/containsWithMargin") = []
{
    auto block = Cows::Block {{0.f, 0.f}, {1.f, 1.f}, 1.f, {}};

    check(block.contains({0.f, 0.f}));
    check(block.contains({1.f, -1.f}));
    check(!block.contains({1.2f, 0.f}));
    check(block.contains({1.2f, 0.f}, 0.3f));
    check(!block.contains({0.f, -1.5f}, 0.3f));
};

auto tClosest = test("Collision/closestPointOnEachSide") = []
{
    auto block = Cows::Block {{1.f, 1.f}, {2.f, 1.f}, 1.f, {}};

    check(near(block.closestTo({10.f, 1.5f}), {3.f, 1.5f}));
    check(near(block.closestTo({-10.f, 0.5f}), {-1.f, 0.5f}));
    check(near(block.closestTo({0.f, 10.f}), {0.f, 2.f}));
    check(near(block.closestTo({2.f, -10.f}), {2.f, 0.f}));
    check(near(block.closestTo({10.f, 10.f}), {3.f, 2.f}));
    check(near(block.closestTo({1.5f, 1.2f}), {1.5f, 1.2f}));
};
