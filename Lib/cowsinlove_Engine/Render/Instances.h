#pragma once

#include "Render/Common.h"

#include <array>

namespace Cows
{
struct Material final
{
    Maths::Vec3 color;
    float alpha = 1.f;

    // 0 for plain parts.
    float spots = 0.f;
    float emission = 0.f;
    // Wraps light round the terminator, for soft things like clouds.
    float softness = 0.f;
    float gloss = 0.3f;
    // 0 for the sky's own things, which sit behind the mist.
    float mist = 1.f;
};

// Matrices go as columns since a vertex attribute cannot be a matrix.
struct SurfaceInstance final
{
    Maths::Vec4 model0;
    Maths::Vec4 model1;
    Maths::Vec4 model2;
    Maths::Vec4 model3;
    Maths::Vec4 normal0;
    Maths::Vec4 normal1;
    Maths::Vec4 normal2;
    // The rest pose, so spots stay painted on while the cow hops and squashes.
    Maths::Vec4 pattern0;
    Maths::Vec4 pattern1;
    Maths::Vec4 pattern2;
    Maths::Vec4 pattern3;
    Maths::Vec4 color;
    Maths::Vec4 material;
    float mist = 1.f;
};

SurfaceInstance makeInstance(const Maths::Mat4& model,
                             const Material& material,
                             const Maths::Mat4& pattern = {});

// Additive; color.w is Material::mist.
struct GlowInstance final
{
    Maths::Vec4 centerAndSize;
    Maths::Vec4 color;
};

inline GlowInstance makeGlow(const Maths::Vec3& center,
                             float size,
                             const Maths::Vec3& color,
                             float mist = 1.f)
{
    return {{center.x, center.y, center.z, size}, {color.x, color.y, color.z, mist}};
}

enum class Shape
{
    Sphere,
    Capsule,
    Horn,
    Heart,
    Barrel,
    Box,
    Wedge,
    Cylinder,
    Cone,
    Belly,
    BellyBand,
    Seat,
    SeatBand,
    BucketCrown,
    BucketBrim,
    WizardCone,
    WizardBrim,
    Star,
    BarnWall,
    BarnTrim,
    BarnRoof,
    BarnWood,
    BarnHay,
    BarnMetal,
    BarnTwine,
    BaleStraw,
    BaleTwine
};

constexpr auto shapeCount = 27;

struct SurfaceBatch final
{
    void add(Shape shape, const SurfaceInstance& instance)
    {
        lists[(int) shape].add(instance);
    }

    void clear()
    {
        for (auto& list: lists)
            list.clear();
    }

    std::array<Vector<SurfaceInstance>, shapeCount> lists;
};
} // namespace Cows
