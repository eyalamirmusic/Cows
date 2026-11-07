#pragma once

#include "Camera/OrbitCamera.h"
#include "Render/Instances.h"
#include "Render/Lighting.h"
#include "Render/Mesh.h"
#include "Render/ShapeMeshes.h"
#include "Render/Shaders.h"
#include "Render/ShadowMap.h"
#include "UI/Hud.h"

#include <eacp/Graphics/Image/Image.h>

#include <functional>
#include <string>

namespace Cows::Testing
{
constexpr Graphics::Color clearColor {1.f, 0.f, 1.f};

bool isClearColor(const Graphics::Color& color);
bool isAnythingDrawnIn(const Graphics::Image& image, const Graphics::Rect& area);
bool hasDevice();
bool isMiddleClear(const Graphics::Image& image);

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

    ShadowMap shadowMap;
    ShapeMeshes shapes;
    SurfaceShader surfaceShader;
    SurfaceShader translucentShader;
    GlowShader glowShader;
};

// Saves docs/shots/tests/<name>.png; the image is invalid without a GPU.
Graphics::Image
    snapshot(SnapshotView& view, float width, float height, const std::string& name);

// A screen is anything laid out by setBounds and drawn through a Hud: Menu, Editor.
template <typename Screen>
Graphics::Image snapshotScreen(Screen& screen,
                               float width,
                               float height,
                               const std::string& name)
{
    screen.setBounds({0.f, 0.f, width, height});

    auto hud = Hud {};
    auto view = SnapshotView {};
    view.drawOverlay = [&](Frame& frame, RenderPass& pass)
    {
        hud.begin(frame, pass, view.sampleCount());
        screen.draw(hud);
        hud.end();
    };

    return snapshot(view, width, height, name);
}
} // namespace Cows::Testing
