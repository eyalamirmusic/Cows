#pragma once

#include "Cow/Cow.h"
#include "Cow/CowSkin.h"
#include "Game.h"
#include "Stages.h"
#include "Input.h"
#include "Pad.h"
#include "Quality.h"
#include "Terrain/Grass.h"
#include "Terrain/Ground.h"
#include "Render/Lighting.h"
#include "Render/Shading.h"
#include "Render/Mesh.h"
#include "Render/ShapeMeshes.h"
#include "Title/MenuTitle.h"
#include "Title/TitleShader.h"
#include "Terrain/TerrainShaders.h"
#include "Cow/Moo.h"
#include "Camera/OrbitCamera.h"
#include "Render/FrameProfile.h"
#include "Render/QualityGovernor.h"
#include "Render/ShadowMap.h"
#include "Render/Shaders.h"
#include "UI/ControlEvent.h"
#include "UI/Hud.h"
#include "UI/Editor.h"
#include "UI/Menu.h"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace Cows
{
struct CowsView final : GPUView
{
    // Where the camera is swinging to: the menu from play, play from the menu,
    // round to the cow to dress her, and back to the menu from there.
    enum class SwingGoal
    {
        Menu,
        Play,
        Dress,
        Undress
    };

    // Starts at the tier `preference` names, else measures one (see
    // QualityGovernor).
    explicit CowsView(const QualityPreference& preference = {QualityChoice::High,
                                                             {}});

    void update(Threads::FrameTime time) override;
    void render(Frame& frame) override;

    void mouseDown(const Graphics::MouseEvent& event) override;
    void mouseUp(const Graphics::MouseEvent& event) override;
    void mouseDragged(const Graphics::MouseEvent& event) override;
    void mouseWheel(const Graphics::MouseEvent& event) override;
    void keyDown(const Graphics::KeyEvent& event) override;
    void keyUp(const Graphics::KeyEvent& event) override;

    void control(const ControlEvent& event);
    void openMenu(bool swing = true);
    void startGame();
    InputOwner inputOwner() const;
    void openEditor(Editor& which);
    void swingTo(SwingGoal goal);
    void closeEditor();
    void wear(const CowSkin& skin);
    bool escape();
    void swingCamera(float delta);
    float menuOpacity() const;
    float editorOpacity() const;
    CameraPose menuPose() const;
    CameraPose editorPose() const;
    CameraPose playPose() const;
    bool menuKey(const Graphics::KeyEvent& event);
    bool editorKey(const Graphics::KeyEvent& event);
    void useMenuPad(const PadControls& pad);
    void useEditorPad(const PadControls& pad, float delta);
    void readGameInput(float delta);
    void usePad(const PadControls& pad, float delta);
    void useHints(Hints used);
    void returnKeyFocus();
    void restart();
    void setTitle(std::string_view text);
    void advanceStage();
    void layTerrain();
    void callOut();
    MooAnswer answerFrom() const;
    void addAnswerFlare();
    void steerCamera(float delta, float wallDelta);
    void framePortrait(float aspect);

    void chooseQuality(QualityChoice choice);
    void applyQualityChoice();
    void useQuality(Quality chosen);
    void preparePipelines();
    void measureQuality();

    void gatherInstances(float seconds);
    Maths::Vec3 groundFocus() const;
    void setSceneUniforms(SceneUniforms& uniforms,
                          const Maths::Mat4& viewProjection);

    void drawShadows(Frame& frame);
    void drawSky(RenderPass& pass, float aspect);
    void drawGround(RenderPass& pass);
    void drawGrass(RenderPass& pass, const Maths::Mat4& viewProjection);
    void drawGrassTile(RenderPass& pass,
                       const GrassDraw& draw,
                       const Vector<BladeInstance>& blades);
    void drawTitle(RenderPass& pass);
    void drawMenuTitle(RenderPass& pass, float width, float height);
    void drawGlows(RenderPass& pass, const Maths::Mat4& viewProjection);
    void drawBatch(RenderPass& pass,
                   ShaderProgram& shader,
                   const SurfaceBatch& batch,
                   const Maths::Mat4& cullWith,
                   FrameProfile::Pass counted = FrameProfile::Pass::Scene);

    Stages stages;
    Game game;
    Input input;
    MooVoice mooVoice;
    std::string hint;
    bool showedHint = false;
    bool showedFall = false;
    Lighting lighting;
    OrbitCamera camera;
    std::optional<ShadowMap> shadowMap;
    Maths::Mat4 lightViewProjection;

    QualityPreference qualityPreference;
    ShapeMeshes shapes;
    Mesh ground;
    TitleMesh title;
    TitleMesh menuTitleWide;
    TitleMesh menuTitleTall;
    const TitleMesh* menuTitleShown = nullptr;

    SkyShader skyShader;
    std::optional<SurfaceShader> surfaceShader;
    std::optional<SurfaceShader> plainShader;
    std::optional<SurfaceShader> translucentShader;
    ShadowCasterShader shadowCaster;
    std::optional<GroundShader> groundShader;
    NoiseLattice noiseLattice;
    std::optional<GrassShader> grassShader;
    TitleShader titleShader;
    TitleShader menuTitleShader;
    GlowShader glowShader;

    Vector<CowPart> cowParts;
    Vector<CowPart> playerParts;
    Vector<Cow> cows;
    GrassField grass;
    GrassDensity grassDensity;
    Quality quality = Quality::High;
    bool qualityReady = false;
    std::optional<Quality> forcedQuality;
    QualityGovernor governor;
    bool measuring = false;
    std::uint64_t lastTimedFrame = 0;
    const Vector<BladeInstance>* uploadedBlades = nullptr;
    int bladeTriangles = 0;
    Vector<SurfaceInstance> visible;
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
    // The player chose a tier, or Auto finished measuring one.
    std::function<void()> onQualityChanged = [] {};
    Graphics::GameInput* gameInput = nullptr;
    Menu* menu = nullptr;
    // The editor open or last open: the cow's clothes, or the settings.
    Editor* editor = nullptr;
    std::optional<float> startAfter;
    CameraPose swingFrom;
    float swingTime = 0.f;
    SwingClock swingClock;
    bool swinging = false;
    SwingGoal swingGoal = SwingGoal::Menu;
    bool dressing = false;
    float dressAmount = 0.f;
    float playPitch = OrbitCamera {}.pitch;
    float playDistance = OrbitCamera {}.distance;

    float elapsed = 0.f;
    float lookHold = 0.f;
    bool padRestarted = false;
    int padMenuStep = 0;
    int padAcross = 0;
    int padAlong = 0;
    std::optional<std::uint16_t> keyFromMenu;
    float viewAspect = 1.f;
    bool frozen = false;
    Hints pointerHints = Hints::Keys;
    Hints hints = Hints::Keys;
    bool framedPortrait = false;
};
} // namespace Cows
