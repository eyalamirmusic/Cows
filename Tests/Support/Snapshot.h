#pragma once

#include "Camera/OrbitCamera.h"
#include "Render/Instances.h"
#include "Render/Lighting.h"
#include "Render/Mesh.h"
#include "Render/Shaders.h"
#include "Render/ShadowMap.h"

#include <eacp/Graphics/Image/Image.h>

#include <functional>
#include <string>

namespace Cows::Testing
{
constexpr Graphics::Color clearColor {1.f, 0.f, 1.f};

bool isClearColor(const Graphics::Color& color);
bool hasDevice();

// Draws `batch`, then `translucent` blended over it, then the additive `glows`,
// under the default Lighting with nothing casting shadows, seen by `camera`, on
// clearColor. `heart` is the mesh for Shape::Heart, which the Engine has not.
// `drawOpaque` draws anything else solid after `batch`, for shaders the
// support library does not know; give them the scene with setSceneUniforms.
struct SnapshotView final : GPUView
{
    explicit SnapshotView(const MeshData& heart = makeSphere(16, 24));

    void render(Frame& frame) override;

    void setSceneUniforms(SceneUniforms& uniforms,
                          const Maths::Mat4& viewProjection);

    Lighting lighting;
    OrbitCamera camera;
    SurfaceBatch batch;
    SurfaceBatch translucent;
    Vector<GlowInstance> glows;
    float time = 0.f;
    std::function<void(RenderPass&, const Maths::Mat4&)> drawOpaque =
        [](RenderPass&, const Maths::Mat4&) {};

private:
    void clearShadows(Frame& frame);
    void draw(RenderPass& pass, SurfaceShader& shader, SurfaceBatch& shapes);
    void drawGlows(RenderPass& pass, const Maths::Mat4& viewProjection);
    const Mesh& meshFor(Shape shape) const;

    ShadowMap shadowMap;
    Mesh sphere;
    Mesh capsule;
    Mesh horn;
    Mesh heart;
    Mesh barrel;
    Mesh box;
    Mesh wedge;
    SurfaceShader surfaceShader;
    SurfaceShader translucentShader;
    GlowShader glowShader;
};

// Renders `view` width by height and saves it to docs/shots/tests/<name>.png.
// The image is invalid when there is no GPU to draw with.
Graphics::Image
    snapshot(SnapshotView& view, float width, float height, const std::string& name);
} // namespace Cows::Testing
