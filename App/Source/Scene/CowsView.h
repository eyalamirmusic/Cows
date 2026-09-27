#pragma once

#include "Cow.h"
#include "Game.h"
#include "Grass.h"
#include "Lighting.h"
#include "Mesh.h"
#include "Moo.h"
#include "OrbitCamera.h"
#include "ShadowMap.h"
#include "Shaders.h"

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

    void setHeld(std::uint16_t keyCode, bool down);
    float walkAhead() const;
    float walkTurn() const;
    void setStick(float ahead, float turn);
    void jump();
    void look(float horizontal, float vertical);
    void returnKeyFocus();
    void restart();
    void callOut();
    MooAnswer answerFrom() const;
    std::string footerText() const;
    bool hintShowing() const;
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
    Maths::Mat4 titlePlacement() const;
    void drawGlows(RenderPass& pass, const Maths::Mat4& viewProjection);
    void drawBatch(RenderPass& pass, ShaderProgram& shader, SurfaceBatch& batch);

    const Mesh& meshFor(Shape shape) const;

    Game game;
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
    bool walkingForward = false;
    bool walkingBack = false;
    bool walkingLeft = false;
    bool walkingRight = false;
    bool jumping = false;
    bool jumpPending = false;
    float stickAhead = 0.f;
    float stickTurn = 0.f;
    bool touchHints = false;
    bool framedPortrait = false;
};
} // namespace Cows
