#pragma once

#include "Cow.h"
#include "Grass.h"
#include "Lighting.h"
#include "Mesh.h"
#include "OrbitCamera.h"
#include "ShadowMap.h"
#include "Shaders.h"

namespace Cows
{
struct CowsView final : GPUView
{
    CowsView();

    void update(Threads::FrameTime time) override;
    void render(Frame& frame) override;

    void mouseDragged(const Graphics::MouseEvent& event) override;
    void mouseWheel(const Graphics::MouseEvent& event) override;

    void gatherInstances(float seconds);
    void setSceneUniforms(SceneUniforms& uniforms,
                          const Maths::Mat4& viewProjection);

    void drawShadows(Frame& frame);
    void drawSky(RenderPass& pass, float aspect);
    void drawGround(RenderPass& pass);
    void drawGrass(RenderPass& pass);
    void drawTitle(RenderPass& pass);
    void drawGlows(RenderPass& pass, const Maths::Mat4& viewProjection);
    void drawBatch(RenderPass& pass, ShaderProgram& shader, SurfaceBatch& batch);

    const Mesh& meshFor(Shape shape) const;

    Lighting lighting;
    OrbitCamera camera;
    ShadowMap shadowMap;
    Maths::Mat4 lightViewProjection;

    Mesh sphere;
    Mesh capsule;
    Mesh horn;
    Mesh heart;
    Mesh barrel;
    Mesh ground;
    TitleMesh title;

    SkyShader skyShader;
    SurfaceShader surfaceShader;
    SurfaceShader translucentShader;
    ShadowCasterShader shadowCaster;
    GroundShader groundShader;
    GrassShader grassShader;
    TitleShader titleShader;
    GlowShader glowShader;

    Vector<CowPart> cowParts;
    Vector<Cow> cows;
    Vector<BladeInstance> meadow;

    SurfaceBatch cowBatch;
    SurfaceBatch backdropBatch;
    SurfaceBatch heartBatch;
    Vector<GlowInstance> glows;

    float elapsed = 0.f;
    bool frozen = false;
};
} // namespace Cows
