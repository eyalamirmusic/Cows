#pragma once

#include "Render/Common.h"

#include <optional>

namespace Cows
{
struct Material final
{
    Maths::Vec3 color;
    float alpha = 1.f;

    // How much of the cow's patchwork shows (0 for plain parts).
    float spots = 0.f;
    // Light of its own, independent of the scene's lighting.
    float emission = 0.f;
    // Wraps the light around the terminator, for soft things like clouds.
    float softness = 0.f;
    float gloss = 0.3f;
};

// One placed copy of a mesh. The model and normal matrices go as columns since
// a vertex attribute cannot be a matrix. `pattern` is the part's place in its
// cow's rest pose, which is where the spots are drawn so that they stay painted
// on while the cow hops and squashes.
struct SurfaceInstance final
{
    Maths::Vec4 model0;
    Maths::Vec4 model1;
    Maths::Vec4 model2;
    Maths::Vec4 model3;
    Maths::Vec4 normal0;
    Maths::Vec4 normal1;
    Maths::Vec4 normal2;
    Maths::Vec4 pattern0;
    Maths::Vec4 pattern1;
    Maths::Vec4 pattern2;
    Maths::Vec4 pattern3;
    Maths::Vec4 color;
    Maths::Vec4 material;
};

SurfaceInstance makeInstance(const Maths::Mat4& model,
                             const Material& material,
                             const Maths::Mat4& pattern = {});

// A camera-facing soft glow, drawn additively.
struct GlowInstance final
{
    Maths::Vec4 centerAndSize;
    Maths::Vec4 color;
};

inline GlowInstance
    makeGlow(const Maths::Vec3& center, float size, const Maths::Vec3& color)
{
    return {{center.x, center.y, center.z, size}, {color.x, color.y, color.z, 1.f}};
}

enum class Shape
{
    Sphere,
    Capsule,
    Horn,
    Heart,
    Barrel,
    Box,
    Wedge
};

constexpr auto shapeCount = 7;

// A frame's worth of instances, one list per mesh.
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

    Vector<SurfaceInstance> lists[shapeCount];
};

// A batch that stays put for a whole level, uploaded once rather than every
// frame: one GPU buffer per mesh that has instances.
struct StaticBatch final
{
    StaticBatch() = default;
    explicit StaticBatch(const SurfaceBatch& batch);

    std::optional<Buffer> lists[shapeCount];
    int counts[shapeCount] = {};
};
} // namespace Cows
