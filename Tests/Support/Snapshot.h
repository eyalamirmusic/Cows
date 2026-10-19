#pragma once

#include "Camera/OrbitCamera.h"
#include "Render/Instances.h"
#include "Render/Lighting.h"
#include "Render/Mesh.h"
#include "Render/ShapeMeshes.h"
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

struct SnapshotView final : GPUView
{
    explicit SnapshotView(const std::function<MeshData(Shape)>& content = [](Shape)
                          { return makeSphere(16, 24); });

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
    std::function<void(Frame&, RenderPass&)> drawOverlay = [](Frame&,
                                                              RenderPass&) {};

private:
    void clearShadows(Frame& frame);
    void draw(RenderPass& pass, SurfaceShader& shader, SurfaceBatch& instances);
    void drawGlows(RenderPass& pass, const Maths::Mat4& viewProjection);

    ShadowMap shadowMap;
    ShapeMeshes shapes;
    SurfaceShader surfaceShader;
    SurfaceShader translucentShader;
    GlowShader glowShader;
};

// Saves docs/shots/tests/<name>.png; the image is invalid without a GPU.
Graphics::Image
    snapshot(SnapshotView& view, float width, float height, const std::string& name);
} // namespace Cows::Testing
