#pragma once

#include "Cow/Cow.h"
#include "Game.h"
#include "Stages.h"
#include "Input.h"
#include "Terrain/Grass.h"
#include "Render/Lighting.h"
#include "Render/Mesh.h"
#include "Title/TitleShader.h"
#include "Terrain/TerrainShaders.h"
#include "Cow/Moo.h"
#include "Camera/OrbitCamera.h"
#include "Render/ShadowMap.h"
#include "Render/Shaders.h"

#include <functional>
#include <string>

namespace Cows
{
struct CowsView final : GPUView
{
    CowsView();

    void update(Threads::FrameTime time) override;
    void render(Frame& frame) override;

    void mouseDown(const Graphics::MouseEvent& event) override;
    void mouseUp(const Graphics::MouseEvent& event) override;
    void mouseDragged(const Graphics::MouseEvent& event) override;
    void mouseWheel(const Graphics::MouseEvent& event) override;
    void keyDown(const Graphics::KeyEvent& event) override;
    void keyUp(const Graphics::KeyEvent& event) override;

    void look(float horizontal, float vertical);
    void returnKeyFocus();
    void restart();
    void callOut();
    MooAnswer answerFrom() const;
    void addAnswerFlare();
    void steerCamera(float delta);
    void framePortrait(float aspect);

    void gatherInstances(float seconds);
    Maths::Vec3 groundFocus() const;
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

    Stages stages;
    Game game;
    Input input;
    MooVoice mooVoice;
    std::string hint;
    bool showedHint = false;
    Lighting lighting;
    OrbitCamera camera;
    ShadowMap shadowMap;
    Maths::Mat4 lightViewProjection;

    Mesh sphere;
    Mesh capsule;
    Mesh horn;
    Mesh heart;
    Mesh barrel;
    Mesh box;
    Mesh wedge;
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
    Maths::Vec3 contacts[2];

    std::function<void()> onStateChanged = [] {};

    float elapsed = 0.f;
    bool frozen = false;
    bool touchHints = false;
    bool framedPortrait = false;
};
} // namespace Cows
