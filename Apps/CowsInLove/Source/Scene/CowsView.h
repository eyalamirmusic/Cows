#pragma once

#include "Cow/Cow.h"
#include "Game.h"
#include "Stages.h"
#include "Input.h"
#include "Terrain/Grass.h"
#include "Terrain/Ground.h"
#include "Render/Lighting.h"
#include "Render/Mesh.h"
#include "Title/TitleShader.h"
#include "Terrain/TerrainShaders.h"
#include "Cow/Moo.h"
#include "Camera/OrbitCamera.h"
#include "Render/ShadowMap.h"
#include "Render/Shaders.h"
#include "UI/ControlEvent.h"
#include "UI/Hud.h"

#include <array>
#include <functional>
#include <string>
#include <string_view>

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

    void control(const ControlEvent& event);
    void returnKeyFocus();
    void restart();
    void setTitle(std::string_view text);
    void advanceStage();
    void layTerrain();
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
    void drawGrassTile(RenderPass& pass,
                       Maths::Vec2 corner,
                       const Vector<BladeInstance>& blades);
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
    bool showedFall = false;
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
    GrassField grass;
    const Vector<BladeInstance>* uploadedBlades = nullptr;
    SurfaceBatch chasms;

    SurfaceBatch cowBatch;
    SurfaceBatch backdropBatch;
    SurfaceBatch heartBatch;
    Vector<GlowInstance> glows;
    std::array<Maths::Vec3, 2> contacts;
    Hud hud;

    // Drawn last in the scene's pass: the footer and the touch controls.
    std::function<void(Hud&)> drawHud = [](Hud&) {};
    std::function<void()> onStateChanged = [] {};

    float elapsed = 0.f;
    bool frozen = false;
    bool touchHints = false;
    bool framedPortrait = false;
};
} // namespace Cows
