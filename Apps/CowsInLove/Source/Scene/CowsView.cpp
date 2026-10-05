#include "CowsView.h"
#include "Cow/KissHearts.h"
#include "Cow/Wardrobe.h"
#include "Ending.h"
#include "Render/FrameProfile.h"
#include "Render/Palette.h"
#include "Sky/SkyDecor.h"

#include <eacp/Core/Utils/Environment.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto msaaSamples = 4;
constexpr auto orbitSpeed = 0.006f;
constexpr auto padYawRate = 2.6f;
constexpr auto padPitchRate = 1.5f;
constexpr auto padZoomRate = 1.5f;
constexpr auto lookHoldTime = 0.5f;

constexpr auto searchHeight = 2.2f;
constexpr auto startYaw = -halfPi;
constexpr auto chaseRate = 4.f;
constexpr auto contactReach = 0.4f;
constexpr auto flareTime = 2.6f;
constexpr auto flareHeight = 22.f;
constexpr auto flareHearts = 6;
constexpr auto quietest = 0.15f;
constexpr auto loudReach = 120.f;

constexpr auto grassTiles = 2;
constexpr auto portraitDistance = 6.5f;
constexpr auto portraitPitch = 0.26f;

constexpr auto menuTurn = 0.45f;
constexpr auto menuHeight = 2.3f;
constexpr auto menuPitch = 0.1f;
constexpr auto menuDistance = 9.5f;
constexpr auto tallMenuHeight = 4.2f;
constexpr auto tallMenuPitch = 0.f;
constexpr auto tallMenuDistance = 7.f;
constexpr auto menuStick = 0.5f;
constexpr auto swingLength = 1.1f;
constexpr auto dressLength = 0.6f;
constexpr auto editorHeight = 1.4f;
constexpr auto editorDistance = 7.5f;
constexpr auto editorTurn = 0.3f;
constexpr auto tallEditorHeight = 0.2f;
constexpr auto tallEditorDistance = 8.f;

constexpr auto fullScreenTriangle = std::to_array<CornerVertex>({
    {{-1.f, -1.f}},
    {{3.f, -1.f}},
    {{-1.f, 3.f}},
});

constexpr auto glowQuad = std::to_array<CornerVertex>({
    {{-1.f, -1.f}},
    {{1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, 1.f}},
});

RenderPipelineDescriptor skyPipeline(int samples)
{
    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = samples;
    descriptor.depth = true;
    descriptor.depthCompare = DepthCompare::Always;
    descriptor.depthWrite = false;
    return descriptor;
}

RenderPipelineDescriptor solidPipeline(int samples, CullMode cull = CullMode::Back)
{
    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = samples;
    descriptor.depth = true;
    descriptor.cullMode = cull;
    return descriptor;
}

RenderPipelineDescriptor translucentPipeline(int samples)
{
    auto descriptor = solidPipeline(samples);
    descriptor.blendMode = BlendMode::AlphaBlend;
    descriptor.depthWrite = false;
    return descriptor;
}

RenderPipelineDescriptor glowPipeline(int samples)
{
    auto blend = BlendState {};
    blend.enabled = true;
    blend.sourceColor = BlendFactor::One;
    blend.destinationColor = BlendFactor::One;
    blend.sourceAlpha = BlendFactor::Zero;
    blend.destinationAlpha = BlendFactor::One;

    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = samples;
    descriptor.depth = true;
    descriptor.depthWrite = false;
    descriptor.blend = blend;
    return descriptor;
}

Vector<Cow> makeCouple()
{
    auto cows = Vector<Cow> {};
    cows.add(Cow {-1.f, {0.f, 0.f, 0.f}});
    cows.add(Cow {1.f, {7.3f, 2.1f, 4.6f}});
    return cows;
}

float mooShape(float sinceMoo)
{
    if (sinceMoo < 0.f || sinceMoo > mooLength)
        return 0.f;

    return std::sin(pi * sinceMoo / mooLength);
}

Vec2 flat(Vec3 vector)
{
    auto ground = Vec2 {vector.x, vector.z};
    auto size = length(ground);
    return size > 0.f ? ground / size : Vec2 {};
}

std::string distanceWord(float distance)
{
    if (distance < 20.f)
        return "close by";

    if (distance < 50.f)
        return "not far";

    return "far off";
}

std::string directionWord(float ahead, float across)
{
    if (ahead > 0.7f)
        return "ahead of you";

    if (ahead < -0.7f)
        return "behind you";

    return across > 0.f ? "to your right" : "to your left";
}

std::optional<float> startAfterSetting()
{
    if (auto after = getEnv("COWS_START"))
        return std::stof(*after);

    return std::nullopt;
}

float startTime()
{
    if (auto start = getEnv("COWS_TIME"))
        return std::stof(*start);

    return 0.f;
}

Graphics::Color displayColor(const Vec3& linear)
{
    auto encode = [](float channel) { return std::pow(channel, 1.f / 2.2f); };
    return {encode(linear.x), encode(linear.y), encode(linear.z)};
}
} // namespace

CowsView::CowsView()
    : shapes(makeCowMesh)
    , ground(makePlane(groundSize))
    , cowParts(makeCowParts())
    , playerParts(cowParts)
    , cows(makeCouple())
    , elapsed(startTime())
    , frozen(!getEnvValue("COWS_FREEZE").empty())
{
    startAfter = startAfterSetting();

    setSampleCount(msaaSamples);
    setDepth(true);

    auto samples = sampleCount();

    skyShader.setVertices(fullScreenTriangle);
    skyShader.prepare(skyPipeline(samples));

    surfaceShader.prepare(solidPipeline(samples));
    translucentShader.prepare(translucentPipeline(samples));
    shadowCaster.prepare(shadowMap.pipeline());
    groundShader.prepare(solidPipeline(samples));

    auto blade = makeBlade();
    grassShader.setVertices(blade.vertices.data(), blade.vertices.size());
    grassShader.setIndices(blade.indices.data(), blade.indices.size());
    grassShader.setInstances(1, grass.tile.data(), grass.tile.size());
    uploadedBlades = &grass.tile;
    grassShader.prepare(solidPipeline(samples, CullMode::None));

    setTitle(Ending::titleText);
    titleShader.prepare(solidPipeline(samples));

    menuTitleWide = MenuTitle::makeWide();
    menuTitleTall = MenuTitle::makeTall();
    menuTitleShader.prepare(solidPipeline(samples));

    glowShader.setVertices(glowQuad);
    glowShader.prepare(glowPipeline(samples));

    game.makeLevel = stages.level();
    game.reset(stages.firstSeed());
    game.start();
    layTerrain();

    camera.yaw = startYaw + game.playerHeading;
    camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};

    if (!getEnvValue("COWS_FOUND").empty())
        game.player = game.partner + Vec3 {Ending::foundStartGap, 0.f, 0.f};

    setHandlesMouseEvents(true);
    setContinuous(true);
}

void CowsView::update(Threads::FrameTime time)
{
    auto& profile = FrameProfile::shared();
    profile.frameStarted();
    auto timed = FrameProfile::Scope {profile, FrameProfile::Part::Update};

    auto delta = frozen ? 0.f : (float) time.delta;
    elapsed += delta;
    lookHold = std::max(0.f, lookHold - delta);
    dressAmount = std::clamp(
        dressAmount + (dressing ? 1.f : -1.f) * (float) time.delta / dressLength,
        0.f,
        1.f);
    readGameInput(delta);

    if (startAfter.has_value() && (*startAfter -= (float) time.delta) <= 0.f)
    {
        startAfter.reset();
        startGame();
    }

    auto wasSearching = game.state == Game::State::Searching;
    auto wasGrounded = game.grounded;
    auto fellBefore = game.sinceFell;
    game.update(delta,
                input.walkAhead(),
                input.walkTurn(),
                input.jumping || input.padJumping || input.jumpPending);
    if (wasGrounded)
        input.jumpPending = false;

    if (wasSearching && game.state == Game::State::Found)
        onStateChanged();

    if (game.sinceFell < fellBefore)
        camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};

    if (Ending::loops(game))
        advanceStage();

    if (game.hintShowing() != showedHint || game.justFell() != showedFall)
    {
        showedHint = game.hintShowing();
        showedFall = game.justFell();
        onStateChanged();
    }

    steerCamera(delta);
}

void CowsView::steerCamera(float delta)
{
    if (swinging)
    {
        swingCamera(delta);
        return;
    }

    if (game.state == Game::State::Menu)
    {
        if (!dressing)
            camera.setPose(menuPose());

        return;
    }

    if (game.state == Game::State::Searching)
    {
        camera.follow(game.player + Vec3 {0.f, searchHeight, 0.f}, delta);
        camera.swayYaw = 0.f;
        camera.swayPitch = 0.f;

        auto walking = input.walkAhead() != 0.f || input.walkTurn() != 0.f;

        if (walking && lookHold <= 0.f)
            camera.turnToward(game.playerHeading + startYaw,
                              std::min(1.f, delta * chaseRate));

        return;
    }

    camera.follow(game.stageCenter + Vec3 {0.f, Ending::endingHeight, 0.f}, delta);
    camera.drift(game.sinceFound);

    if (game.sinceFound > Ending::settleTime)
        return;

    auto amount = std::min(1.f, delta * Ending::settleRate);
    camera.turnToward(game.stageHeading, amount);
    camera.pitch += (Ending::endingPitch - camera.pitch) * amount;
    camera.distance += (Ending::endingDistance - camera.distance) * amount;
}

CameraPose CowsView::menuPose() const
{
    auto tall = viewAspect < 1.f;
    auto sway = driftAt(elapsed);

    auto pose = CameraPose {};
    pose.target = game.player + Vec3 {0.f, tall ? tallMenuHeight : menuHeight, 0.f};
    pose.yaw = game.playerHeading + startYaw + pi + menuTurn;
    pose.pitch = tall ? tallMenuPitch : menuPitch;
    pose.distance = tall ? tallMenuDistance : menuDistance;
    pose.swayYaw = sway.x;
    pose.swayPitch = sway.y;
    return pose;
}

CameraPose CowsView::editorPose() const
{
    auto tall = viewAspect < 1.f;

    auto pose = CameraPose {};
    pose.target =
        game.player + Vec3 {0.f, tall ? tallEditorHeight : editorHeight, 0.f};
    pose.yaw = game.playerHeading + startYaw + pi + editorTurn;
    pose.pitch = menuPitch;
    pose.distance = tall ? tallEditorDistance : editorDistance;
    return pose;
}

CameraPose CowsView::playPose() const
{
    auto pose = CameraPose {};

    if (game.playing() == Game::State::Found)
    {
        auto sway = driftAt(game.sinceFound);
        pose.target = game.stageCenter + Vec3 {0.f, Ending::endingHeight, 0.f};
        pose.yaw = game.stageHeading;
        pose.pitch = Ending::endingPitch;
        pose.distance = Ending::endingDistance;
        pose.swayYaw = sway.x;
        pose.swayPitch = sway.y;
        return pose;
    }

    pose.target = game.player + Vec3 {0.f, searchHeight, 0.f};
    pose.yaw = game.playerHeading + startYaw;
    pose.pitch = playPitch;
    pose.distance = playDistance;
    return pose;
}

void CowsView::swingCamera(float delta)
{
    swingTime = std::min(swingTime + delta, swingLength);
    auto amount = easeOut(swingTime / swingLength);
    camera.setPose(blend(swingFrom,
                         swingGoal == SwingGoal::Play    ? playPose()
                         : swingGoal == SwingGoal::Dress ? editorPose()
                                                         : menuPose(),
                         amount));

    if (swingTime < swingLength)
        return;

    swinging = false;

    if (swingGoal == SwingGoal::Play)
    {
        game.start();
        onStateChanged();
    }
}

float CowsView::menuOpacity() const
{
    auto behindEditor = 1.f - editorOpacity();
    auto aroundCow =
        swingGoal == SwingGoal::Dress || swingGoal == SwingGoal::Undress;

    if (!swinging || aroundCow)
        return game.state == Game::State::Menu ? behindEditor : 0.f;

    auto amount = easeOut(swingTime / swingLength);
    return swingGoal == SwingGoal::Play ? 1.f - amount : amount;
}

float CowsView::editorOpacity() const
{
    return game.state == Game::State::Menu ? easeOut(dressAmount) : 0.f;
}

void CowsView::openMenu(bool swing)
{
    if (game.state == Game::State::Menu)
        return;

    playPitch = camera.pitch;
    playDistance = camera.distance;
    game.openMenu();
    input = Input {};

    swingTo(SwingGoal::Menu);
    swinging = swing;

    if (!swing)
        camera.setPose(menuPose());

    onStateChanged();
}

void CowsView::startGame()
{
    if (game.state != Game::State::Menu || swinging || dressing)
        return;

    swingTo(SwingGoal::Play);
}

void CowsView::swingTo(SwingGoal goal)
{
    swingFrom = camera.pose();
    swingTime = 0.f;
    swinging = true;
    swingGoal = goal;
}

void CowsView::openEditor()
{
    if (game.state != Game::State::Menu || swinging || dressing)
        return;

    dressing = true;
    swingTo(SwingGoal::Dress);

    if (editor != nullptr)
        editor->selected = 0;

    onStateChanged();
}

void CowsView::closeEditor()
{
    if (!dressing)
        return;

    dressing = false;
    swingTo(SwingGoal::Undress);
    onStateChanged();
}

void CowsView::wear(const CowSkin& skin)
{
    playerParts = makeCowParts(skin);
}

bool CowsView::escape()
{
    if (swinging)
        return true;

    if (dressing)
    {
        closeEditor();
        return true;
    }

    if (game.state == Game::State::Menu)
        return false;

    openMenu();
    return true;
}

bool CowsView::menuKey(const Graphics::KeyEvent& event)
{
    if (game.state != Game::State::Menu || menu == nullptr)
        return false;

    if (dressing)
        return editorKey(event);

    using namespace Graphics::KeyCode;

    switch (event.keyCode)
    {
        case Return:
        case KeypadEnter:
        case Space:
            if (!event.isRepeat)
            {
                keyFromMenu = event.keyCode;
                menu->chooseSelected();
            }
            break;
        case LeftArrow:
        case UpArrow:
        case A:
        case W:
            menu->selectPrevious();
            break;
        case RightArrow:
        case DownArrow:
        case D:
        case S:
            menu->selectNext();
            break;
        default:
            break;
    }

    return true;
}

bool CowsView::editorKey(const Graphics::KeyEvent& event)
{
    if (editor == nullptr)
        return true;

    using namespace Graphics::KeyCode;

    switch (event.keyCode)
    {
        case Return:
        case KeypadEnter:
        case Space:
            if (!event.isRepeat)
            {
                keyFromMenu = event.keyCode;
                editor->chooseSelected();
            }
            break;
        case LeftArrow:
        case A:
            editor->stepSelected(-1);
            break;
        case RightArrow:
        case D:
            editor->stepSelected(1);
            break;
        case UpArrow:
        case W:
            editor->selectPrevious();
            break;
        case DownArrow:
        case S:
            editor->selectNext();
            break;
        default:
            break;
    }

    return true;
}

void CowsView::keyDown(const Graphics::KeyEvent& event)
{
    useHints(pointerHints);

    if (menuKey(event))
        return;

    if (keyFromMenu == event.keyCode)
        return;

    if (event.keyCode == Graphics::KeyCode::M)
    {
        if (!event.isRepeat)
            callOut();

        return;
    }

    if (event.keyCode == Graphics::KeyCode::R)
    {
        if (!event.isRepeat)
            restart();

        return;
    }

    input.setHeld(event.keyCode, true);
}

void CowsView::keyUp(const Graphics::KeyEvent& event)
{
    if (keyFromMenu == event.keyCode)
        keyFromMenu.reset();

    input.setHeld(event.keyCode, false);
}

void CowsView::control(const ControlEvent& event)
{
    switch (event.kind)
    {
        case ControlEvent::Kind::Steer:
            input.setStick(event.x, event.y);
            break;
        case ControlEvent::Kind::Jump:
            input.jump();
            break;
        case ControlEvent::Kind::Moo:
            callOut();
            break;
        case ControlEvent::Kind::Restart:
            if (game.state == Game::State::Found)
                advanceStage();
            else
                restart();
            break;
        case ControlEvent::Kind::Look:
            camera.orbit(event.x * orbitSpeed, event.y * orbitSpeed);
            break;
        case ControlEvent::Kind::Zoom:
            camera.zoom(event.x);
            break;
    }
}

void CowsView::readGameInput(float delta)
{
    if (gameInput == nullptr)
        return;

    usePad(readPad(gameInput->snapshot()), delta);
}

void CowsView::usePad(const PadControls& pad, float delta)
{
    if (!pad.jumping)
        padRestarted = false;

    if (pad.active)
        useHints(padHints(pad.family));

    if (game.state == Game::State::Menu)
    {
        if (dressing)
            useEditorPad(pad, delta);
        else
            useMenuPad(pad);

        return;
    }

    if (pad.startPressed)
    {
        openMenu();
        return;
    }

    if (pad.jumpPressed)
    {
        if (game.state == Game::State::Found)
        {
            padRestarted = true;
            control({ControlEvent::Kind::Restart});
        }
        else
            control({ControlEvent::Kind::Jump});
    }

    if (pad.mooPressed)
        control({ControlEvent::Kind::Moo});

    if (pad.againPressed)
        control({ControlEvent::Kind::Restart});

    if (pad.recenterPressed && game.state == Game::State::Searching)
        camera.turnToward(game.playerHeading + startYaw, 1.f);

    if (pad.lookX != 0.f || pad.lookY != 0.f)
    {
        camera.orbit(pad.lookX * padYawRate * delta,
                     -pad.lookY * padPitchRate * delta);
        lookHold = lookHoldTime;
    }

    camera.zoom(pad.zoom * padZoomRate * delta);
    input.setPad(pad.ahead, pad.turn, pad.jumping && !padRestarted);
}

void CowsView::useMenuPad(const PadControls& pad)
{
    input.setPad(0.f, 0.f, false);

    if (menu == nullptr)
        return;

    auto back = pad.turn > menuStick || pad.ahead > menuStick;
    auto on = pad.turn < -menuStick || pad.ahead < -menuStick;
    auto step = back ? -1 : on ? 1 : 0;

    if (step != padMenuStep && step < 0)
        menu->selectPrevious();

    if (step != padMenuStep && step > 0)
        menu->selectNext();

    padMenuStep = step;

    if (pad.jumpPressed || pad.startPressed)
    {
        padRestarted = true;
        menu->showSelection = true;
        menu->chooseSelected();
    }
}

void CowsView::useEditorPad(const PadControls& pad, float delta)
{
    input.setPad(0.f, 0.f, false);

    if (pad.lookX != 0.f || pad.lookY != 0.f)
        camera.orbit(pad.lookX * padYawRate * delta,
                     -pad.lookY * padPitchRate * delta);

    camera.zoom(pad.zoom * padZoomRate * delta);

    if (editor == nullptr)
        return;

    auto sideways = std::abs(pad.turn) >= std::abs(pad.ahead);
    auto across = pad.turn > menuStick ? -1 : pad.turn < -menuStick ? 1 : 0;
    auto along = pad.ahead > menuStick ? -1 : pad.ahead < -menuStick ? 1 : 0;

    if (!sideways)
        across = 0;
    else
        along = 0;

    if (across != padAcross && across != 0)
        editor->stepSelected(across);

    if (along != padAlong && along < 0)
        editor->selectPrevious();

    if (along != padAlong && along > 0)
        editor->selectNext();

    padAcross = across;
    padAlong = along;

    if (pad.jumpPressed)
    {
        padRestarted = true;
        editor->showSelection = true;
        editor->chooseSelected();
    }
    else if (pad.backPressed || pad.startPressed)
    {
        closeEditor();
    }
}

void CowsView::useHints(Hints used)
{
    if (used == hints)
        return;

    hints = used;
    onStateChanged();
}

void CowsView::callOut()
{
    auto before = game.sinceMoo;
    game.moo();

    if (game.sinceMoo == before)
        return;

    auto toHer = flat(game.partner - camera.eye());
    auto ahead = dot(toHer, flat(camera.forward()));
    auto across = dot(toHer, flat(camera.right()));

    hint = "she moos back, " + distanceWord(game.distance()) + ", "
           + directionWord(ahead, across);
    mooVoice.call(answerFrom());
}

MooAnswer CowsView::answerFrom() const
{
    auto toHer = flat(game.partner - camera.eye());

    auto answer = MooAnswer {};
    answer.pan = dot(toHer, flat(camera.right()));
    answer.muffle = std::clamp(-dot(toHer, flat(camera.forward())), 0.f, 1.f);
    answer.volume = std::clamp(1.f - game.distance() / loudReach, quietest, 1.f);
    return answer;
}

void CowsView::addAnswerFlare()
{
    auto since = game.sinceMoo - mooAnswerDelay;

    if (since < 0.f || since > flareTime)
        return;

    auto fade = 1.f - since / flareTime;
    auto color = Palette::linear(Palette::heart) * (0.9f * fade);

    for (auto flare = 0; flare < flareHearts; ++flare)
    {
        auto lift =
            flareHeight * (since / flareTime) * (1.f - 0.12f * (float) flare);
        auto at = game.partner + Vec3 {0.f, 2.5f + lift, 0.f};
        glows.add(makeGlow(at, 2.4f - 0.2f * (float) flare, color));
    }
}

void CowsView::restart()
{
    game.reset(stages.nextSeed());
    layTerrain();
    camera.yaw = startYaw + game.playerHeading;
    camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};
    onStateChanged();
}

void CowsView::setTitle(std::string_view text)
{
    title = makeTitle(text);
    titleShader.setVertices(title.vertices.data(), title.vertices.size());
    titleShader.setIndices(title.indices.data(), title.indices.size());
}

void CowsView::advanceStage()
{
    stages.advance();
    game.makeLevel = stages.level();
    restart();
}

void CowsView::layTerrain()
{
    ground = Mesh {makeGround(game.level, groundSize)};
    chasms = makeChasms(game.level);
    grass.layOver(game.level);

    if (uploadedBlades != &grass.tile)
        uploadedBlades = nullptr;
}

void CowsView::render(Frame& frame)
{
    using Part = FrameProfile::Part;
    auto& profile = FrameProfile::shared();

    {
        auto timed = FrameProfile::Scope {profile, Part::Gather};
        gatherInstances(elapsed);
    }

    lightViewProjection =
        shadowMap.lightViewProjection(lighting.keyDirection, groundFocus());

    {
        auto timed = FrameProfile::Scope {profile, Part::Shadows};
        drawShadows(frame);
    }

    auto timedScene = std::optional<FrameProfile::Scope> {};
    timedScene.emplace(profile, Part::Scene);
    auto pass = frame.beginPass({displayColor(lighting.horizonColor)});

    auto width = (float) pass.targetWidth();
    auto height = (float) pass.targetHeight();

    if (width <= 0.f || height <= 0.f)
        return;

    auto aspect = width / height;
    viewAspect = aspect;
    framePortrait(aspect);
    auto viewProjection = camera.projection(aspect) * camera.view();

    setSceneUniforms(surfaceShader, viewProjection);
    setSceneUniforms(translucentShader, viewProjection);
    setSceneUniforms(groundShader, viewProjection);
    setSceneUniforms(grassShader, viewProjection);
    setSceneUniforms(titleShader, viewProjection);
    setSceneUniforms(menuTitleShader, viewProjection);

    drawSky(pass, aspect);
    drawGround(pass);
    drawBatch(pass, surfaceShader, chasms);
    drawBatch(pass, surfaceShader, backdropBatch);
    drawBatch(pass, surfaceShader, game.level.batch);
    drawBatch(pass, surfaceShader, game.level.moving);
    drawBatch(pass, surfaceShader, cowBatch);
    drawGrass(pass);
    drawTitle(pass);
    drawMenuTitle(pass, width, height);
    drawBatch(pass, translucentShader, heartBatch);
    drawGlows(pass, viewProjection);
    timedScene.reset();

    auto timedHud = FrameProfile::Scope {profile, Part::Hud};
    hud.begin(frame, pass, sampleCount());
    drawHud(hud);
    hud.end();
}

void CowsView::framePortrait(float aspect)
{
    if (framedPortrait || aspect >= 1.f)
        return;

    framedPortrait = true;
    playPitch = portraitPitch;
    playDistance = portraitDistance;

    if (game.state == Game::State::Menu)
        return;

    camera.distance = portraitDistance;
    camera.pitch = portraitPitch;
}

void CowsView::mouseDown(const Graphics::MouseEvent&)
{
    useHints(pointerHints);
    returnKeyFocus();
}

void CowsView::mouseUp(const Graphics::MouseEvent&)
{
    returnKeyFocus();
}

void CowsView::returnKeyFocus()
{
    if (auto* root = getParent())
        root->focus();
}

void CowsView::mouseDragged(const Graphics::MouseEvent& event)
{
    useHints(pointerHints);
    camera.orbit(event.delta.x * orbitSpeed, event.delta.y * orbitSpeed);
    lookHold = lookHoldTime;
}

void CowsView::mouseWheel(const Graphics::MouseEvent& event)
{
    useHints(pointerHints);
    camera.zoom(wheelZoom(event));
}

void CowsView::gatherInstances(float seconds)
{
    cowBatch.clear();
    backdropBatch.clear();
    heartBatch.clear();
    glows.clear();

    auto poses = std::array<CowPose, 2> {};

    if (game.playing() == Game::State::Searching)
    {
        auto warmth = game.warmth();
        poses[0] = cows[0].freePose(game.player,
                                    game.playerHeading,
                                    game.hopClock,
                                    game.bounce,
                                    seconds,
                                    game.beatClock,
                                    0.6f + 1.4f * warmth);
        poses[0].moo = mooShape(game.sinceMoo);
        poses[1] = cows[1].freePose(game.partner,
                                    game.partnerHeading,
                                    game.hopClock,
                                    0.f,
                                    seconds,
                                    game.beatClock,
                                    2.f * warmth * warmth);
        poses[1].moo = mooShape(game.sinceMoo - mooAnswerDelay);
        addAnswerFlare();
    }
    else
    {
        auto ending = game.endingSeconds();
        auto stage = game.stage();

        for (auto index = 0; index < 2; ++index)
            poses[index] = cows[index].pose(ending, stage);

        addKissHearts(
            heartBatch, glows, ending, transformPoint(stage, Ending::kissPoint));
    }

    for (auto index = 0; index < 2; ++index)
    {
        cows[index].addTo(
            cowBatch, glows, index == 0 ? playerParts : cowParts, poses[index]);
        contacts[index] = cows[index].contact(poses[index]);

        if (poses[index].world.column(3).y > contactReach)
            contacts[index].z = 0.f;
    }

    auto origin = groundFocus();
    SkyDecor::addSun(backdropBatch, glows, seconds, origin);
    SkyDecor::addClouds(backdropBatch, seconds, origin);
    SkyDecor::addHills(backdropBatch, origin);
}

Vec3 CowsView::groundFocus() const
{
    return {camera.target.x, 0.f, camera.target.z};
}

void CowsView::setSceneUniforms(SceneUniforms& uniforms, const Mat4& viewProjection)
{
    uniforms.setLighting(lighting);
    uniforms.viewProjection = viewProjection;
    uniforms.lightViewProjection = lightViewProjection;
    uniforms.eyePosition = camera.eye();
    uniforms.time = elapsed;
    uniforms.shadowMap = shadowMap.texture;
}

void CowsView::drawShadows(Frame& frame)
{
    auto descriptor = RenderPassDescriptor {};
    descriptor.clearColor = Graphics::Color {1.f, 1.f, 1.f};
    descriptor.label = "shadows";

    auto pass = frame.beginPass(shadowMap.texture, descriptor);
    shadowCaster.lightViewProjection = lightViewProjection;
    drawBatch(pass, shadowCaster, cowBatch);
    drawBatch(pass, shadowCaster, game.level.batch);
    drawBatch(pass, shadowCaster, game.level.moving);
}

void CowsView::drawSky(RenderPass& pass, float aspect)
{
    auto halfHeight = std::tan(camera.verticalFieldOfView(aspect) * 0.5f);

    skyShader.setLighting(lighting);
    skyShader.cameraForward = camera.forward();
    skyShader.cameraRight = camera.right();
    skyShader.cameraUp = camera.up();
    skyShader.lensScale = Vec2 {halfHeight * aspect, halfHeight};
    skyShader.towardSun =
        normalize(groundFocus() + SkyDecor::sunCenter - camera.eye());

    pass.draw(skyShader);
}

void CowsView::drawGround(RenderPass& pass)
{
    groundShader.firstContact = contacts[0];
    groundShader.secondContact = contacts[1];

    pass.bind(groundShader, ground.vertices);
    pass.drawIndexed(ground.indices, ground.indexCount);
}

void CowsView::drawGrass(RenderPass& pass)
{
    auto focus = groundFocus();
    auto corner = Vec2 {std::round(focus.x / meadowTile) * meadowTile,
                        std::round(focus.z / meadowTile) * meadowTile};

    auto cut = Vector<Vec2> {};

    for (auto x = -grassTiles; x < grassTiles; ++x)
        for (auto z = -grassTiles; z < grassTiles; ++z)
        {
            auto at = corner + Vec2 {(float) x * meadowTile, (float) z * meadowTile};
            const auto& blades = grass.tileAt(at);

            if (&blades == &grass.tile)
                drawGrassTile(pass, at, blades);
            else
                cut.add(at);
        }

    for (auto at: cut)
        drawGrassTile(pass, at, grass.tileAt(at));
}

void CowsView::drawGrassTile(RenderPass& pass,
                             Vec2 corner,
                             const Vector<BladeInstance>& blades)
{
    if (blades.empty())
        return;

    if (uploadedBlades != &blades)
    {
        grassShader.setInstances(1, blades.data(), blades.size());
        uploadedBlades = &blades;
    }

    grassShader.patchOffset = corner;
    pass.drawInstanced(grassShader, blades.size());
}

void CowsView::drawTitle(RenderPass& pass)
{
    if (game.playing() != Game::State::Found || title.indices.empty())
        return;

    titleShader.placement = Ending::titlePlacement(game);
    titleShader.titleColor = Palette::linear(Palette::title);

    pass.draw(titleShader);
}

void CowsView::drawMenuTitle(RenderPass& pass, float width, float height)
{
    if (game.state != Game::State::Menu || menu == nullptr
        || game.playing() == Game::State::Found)
        return;

    const auto& shown = width >= height ? menuTitleWide : menuTitleTall;

    if (menuTitleShown != &shown)
    {
        menuTitleShader.setVertices(shown.vertices.data(), shown.vertices.size());
        menuTitleShader.setIndices(shown.indices.data(), shown.indices.size());
        menuTitleShown = &shown;
    }

    auto scale = width / std::max(menu->getLocalBounds().w, 1.f);
    auto viewSize = Graphics::Point {width / scale, height / scale};

    auto area = menu->titleArea();
    area.y -= (area.y + area.h) * (1.f - menuOpacity());

    menuTitleShader.placement = MenuTitle::placement(camera, viewSize, area, shown);
    menuTitleShader.titleColor = Palette::linear(Palette::heart);

    pass.draw(menuTitleShader);
}

void CowsView::drawGlows(RenderPass& pass, const Mat4& viewProjection)
{
    if (glows.empty())
        return;

    glowShader.viewProjection = viewProjection;
    glowShader.cameraRight = camera.right();
    glowShader.cameraUp = camera.up();
    glowShader.setInstances(1, glows.data(), glows.size());

    pass.drawInstanced(glowShader, glows.size());
}

void CowsView::drawBatch(RenderPass& pass,
                         ShaderProgram& shader,
                         SurfaceBatch& batch)
{
    for (auto index = 0; index < shapeCount; ++index)
    {
        const auto& list = batch.lists[index];

        if (list.empty())
            continue;

        const auto& mesh = shapes[(Shape) index];

        shader.setInstances(1, list.data(), list.size());
        pass.bind(shader, mesh.vertices);
        shader.bindInstances(pass);
        pass.drawIndexedInstanced(mesh.indices, mesh.indexCount, list.size());
    }
}

} // namespace Cows
