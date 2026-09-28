#include "Props/Props.h"
#include "Render/Palette.h"

#include <cmath>

using namespace Maths;

namespace Cows
{
float randomUnit(std::mt19937& random)
{
    return std::uniform_real_distribution<float> {0.f, 1.f}(random);
}

float randomBetween(std::mt19937& random, float from, float to)
{
    return from + (to - from) * randomUnit(random);
}

Material matte(std::uint32_t hex, float softness, float gloss)
{
    auto material = Material {Palette::linear(hex)};
    material.softness = softness;
    material.gloss = gloss;
    return material;
}

void addBlock(Scenery& scenery, const Block& block, const Material& material)
{
    scenery.blocks.add(block);

    auto ground = Vec3 {block.center.x, 0.f, block.center.y};

    if (lengthSquared(block.rise) == 0.f)
    {
        scenery.batch.add(Shape::Box,
                          makeInstance(Mat4::translation(ground)
                                           * Mat4::scale({block.half.x * 2.f,
                                                          block.top,
                                                          block.half.y * 2.f}),
                                       material));
        return;
    }

    auto along = dot(absolute(block.rise), block.half);
    auto across = block.half.x + block.half.y - along;
    auto heading = std::atan2(-block.rise.y, block.rise.x);

    scenery.batch.add(
        Shape::Wedge,
        makeInstance(Mat4::translation(ground) * Mat4::rotationY(heading)
                         * Mat4::scale({along * 2.f, block.top, across * 2.f}),
                     material));
}
} // namespace Cows
