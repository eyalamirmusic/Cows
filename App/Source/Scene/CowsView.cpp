#include "CowsView.h"
#include "HeartMesh.h"
#include "KissHearts.h"
#include "Palette.h"
#include "SkyDecor.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto msaaSamples = 4;
constexpr auto groundSize = 600.f;
constexpr auto orbitSpeed = 0.006f;
constexpr auto lineZoom = 0.1f;
constexpr auto preciseZoom = 0.01f;

constexpr auto titleText = "cowsinlove.com";
constexpr Vec3 titleCenter {0.f, 9.6f, -16.f};
constexpr auto titleScale = 1.12f;
constexpr Vec3 kissPoint {0.f, 1.75f, 0.f};
constexpr auto titleDrop = 5.f;
constexpr auto titleDelay = 0.6f;
constexpr auto titleRiseTime = 2.4f;

constexpr auto searchHeight = 2.2f;
constexpr auto startYaw = -halfPi;
constexpr auto chaseRate = 4.f;
constexpr auto contactReach = 0.4f;
constexpr auto hintTime = 4.f;
constexpr auto flareTime = 2.6f;
constexpr auto flareHeight = 22.f;
constexpr auto flareHearts = 6;
constexpr auto quietest = 0.15f;
constexpr auto loudReach = 120.f;

constexpr auto searchingText =
    "wasd / arrows to walk  -  space to jump  -  m to moo  -  drag to look  -  q "
    "to quit";
constexpr auto foundText = "you found her  -  r to play again  -  q to quit";
constexpr auto endingHeight = 3.1f;
constexpr auto endingPitch = 0.08f;
constexpr auto endingDistance = 11.f;
constexpr auto settleTime = 3.f;
constexpr auto settleRate = 2.f;
constexpr auto grassTiles = 2;

constexpr CornerVertex fullScreenTriangle[3] = {
    {{-1.f, -1.f}},
    {{3.f, -1.f}},
    {{-1.f, 3.f}},
};

constexpr CornerVertex glowQuad[6] = {
    {{-1.f, -1.f}},
    {{1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, 1.f}},
};

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

float easeInOut(float amount)
{
    auto t = std::clamp(amount, 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
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

float startTime()
{
    if (auto* start = std::getenv("COWS_TIME"))
        return std::strtof(start, nullptr);

    return 0.f;
}

Graphics::Color displayColor(const Vec3& linear)
{
    auto encode = [](float channel) { return std::pow(channel, 1.f / 2.2f); };
    return {encode(linear.x), encode(linear.y), encode(linear.z)};
}
} // namespace

CowsView::CowsView()
    : sphere(makeSphere(32, 48))
    , capsule(makeCapsule(0.14f, 32))
    , horn(makeHorn(24))
    , heart(makeHeart())
    , barrel(makeBarrel(48))
    , box(makeBox())
    , wedge(makeWedge())
    , ground(makePlane(groundSize))
    , title(makeTitle(titleText))
    , cowParts(makeCowParts())
    , cows(makeCouple())
    , meadow(makeMeadow())
    , elapsed(startTime())
    , frozen(std::getenv("COWS_FREEZE") != nullptr)
{
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
    grassShader.setInstances(1, meadow.data(), meadow.size());
    grassShader.prepare(solidPipeline(samples, CullMode::None));

    titleShader.setVertices(title.vertices.data(), title.vertices.size());
    titleShader.setIndices(title.indices.data(), title.indices.size());
    titleShader.prepare(solidPipeline(samples));

    glowShader.setVertices(glowQuad);
    glowShader.prepare(glowPipeline(samples));

    camera.yaw = startYaw + game.playerHeading;
    camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};

    setHandlesMouseEvents(true);
    setContinuous(true);
}

void CowsView::update(Threads::FrameTime time)
{
    auto delta = frozen ? 0.f : (float) time.delta;
    elapsed += delta;

    auto wasSearching = game.state == Game::State::Searching;
    game.update(delta, walkAhead(), walkTurn(), jumping);

    if (wasSearching && game.state == Game::State::Found)
        onStateChanged();

    if (hintShowing() != showedHint)
    {
        showedHint = hintShowing();
        onStateChanged();
    }

    steerCamera(delta);
}

void CowsView::steerCamera(float delta)
{
    if (game.state == Game::State::Searching)
    {
        camera.follow(game.player + Vec3 {0.f, searchHeight, 0.f}, delta);
        camera.swayYaw = 0.f;
        camera.swayPitch = 0.f;

        if (walkAhead() != 0.f || walkTurn() != 0.f)
            camera.turnToward(game.playerHeading + startYaw,
                              std::min(1.f, delta * chaseRate));

        return;
    }

    camera.follow(game.stageCenter + Vec3 {0.f, endingHeight, 0.f}, delta);
    camera.drift(game.sinceFound);

    if (game.sinceFound > settleTime)
        return;

    auto amount = std::min(1.f, delta * settleRate);
    camera.turnToward(game.stageHeading, amount);
    camera.pitch += (endingPitch - camera.pitch) * amount;
    camera.distance += (endingDistance - camera.distance) * amount;
}

void CowsView::keyDown(const Graphics::KeyEvent& event)
{
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

    setHeld(event.keyCode, true);
}

void CowsView::keyUp(const Graphics::KeyEvent& event)
{
    setHeld(event.keyCode, false);
}

void CowsView::setHeld(std::uint16_t keyCode, bool down)
{
    using namespace Graphics::KeyCode;

    switch (keyCode)
    {
        case W:
        case UpArrow:
            walkingForward = down;
            break;
        case S:
        case DownArrow:
            walkingBack = down;
            break;
        case A:
        case LeftArrow:
            walkingLeft = down;
            break;
        case D:
        case RightArrow:
            walkingRight = down;
            break;
        case Space:
            jumping = down;
            break;
        default:
            break;
    }
}

float CowsView::walkAhead() const
{
    return (walkingForward ? 1.f : 0.f) - (walkingBack ? 1.f : 0.f);
}

float CowsView::walkTurn() const
{
    return (walkingLeft ? 1.f : 0.f) - (walkingRight ? 1.f : 0.f);
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

bool CowsView::hintShowing() const
{
    return game.state == Game::State::Searching && game.sinceMoo >= mooAnswerDelay
           && game.sinceMoo < mooAnswerDelay + hintTime;
}

std::string CowsView::footerText() const
{
    if (game.state == Game::State::Found)
        return foundText;

    return hintShowing() ? hint : searchingText;
}

void CowsView::addAnswerFlare()
{
    auto since = game.sinceMoo - mooAnswerDelay;

    if (since < 0.f || since > flareTime)
        return;

    auto fade = 1.f - since / flareTime;
    auto color = Palette::linear(Palette::heart) * (0.9f * fade);

    for (auto heart = 0; heart < flareHearts; ++heart)
    {
        auto lift =
            flareHeight * (since / flareTime) * (1.f - 0.12f * (float) heart);
        auto at = game.partner + Vec3 {0.f, 2.5f + lift, 0.f};
        glows.add(makeGlow(at, 2.4f - 0.2f * (float) heart, color));
    }
}

void CowsView::restart()
{
    game.reset(clockSeed());
    camera.yaw = startYaw;
    camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};
    onStateChanged();
}

void CowsView::render(Frame& frame)
{
    gatherInstances(elapsed);
    lightViewProjection =
        shadowMap.lightViewProjection(lighting.keyDirection, groundFocus());

    drawShadows(frame);

    auto pass = frame.beginPass({displayColor(lighting.horizonColor)});

    auto width = (float) pass.targetWidth();
    auto height = (float) pass.targetHeight();

    if (width <= 0.f || height <= 0.f)
        return;

    auto aspect = width / height;
    auto viewProjection = camera.projection(aspect) * camera.view();

    setSceneUniforms(surfaceShader, viewProjection);
    setSceneUniforms(translucentShader, viewProjection);
    setSceneUniforms(groundShader, viewProjection);
    setSceneUniforms(grassShader, viewProjection);
    setSceneUniforms(titleShader, viewProjection);

    drawSky(pass, aspect);
    drawGround(pass);
    drawBatch(pass, surfaceShader, backdropBatch);
    drawBatch(pass, surfaceShader, game.obstacles.batch);
    drawBatch(pass, surfaceShader, cowBatch);
    drawGrass(pass);
    drawTitle(pass);
    drawBatch(pass, translucentShader, heartBatch);
    drawGlows(pass, viewProjection);
}

void CowsView::mouseDown(const Graphics::MouseEvent&)
{
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
    camera.orbit(event.delta.x * orbitSpeed, event.delta.y * orbitSpeed);
}

void CowsView::mouseWheel(const Graphics::MouseEvent& event)
{
    auto scale = event.preciseScrolling ? preciseZoom : lineZoom;
    camera.zoom(event.delta.y * scale);
}

void CowsView::gatherInstances(float seconds)
{
    cowBatch.clear();
    backdropBatch.clear();
    heartBatch.clear();
    glows.clear();

    CowPose poses[2];

    if (game.state == Game::State::Searching)
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

        addKissHearts(heartBatch, glows, ending, transformPoint(stage, kissPoint));
    }

    for (auto index = 0; index < 2; ++index)
    {
        cows[index].addTo(cowBatch, glows, cowParts, poses[index]);
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
    drawBatch(pass, shadowCaster, game.obstacles.batch);
}

void CowsView::drawSky(RenderPass& pass, float aspect)
{
    auto halfHeight = std::tan(camera.fieldOfView * 0.5f);

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

    for (auto x = -grassTiles; x < grassTiles; ++x)
        for (auto z = -grassTiles; z < grassTiles; ++z)
        {
            grassShader.patchOffset =
                corner + Vec2 {(float) x * meadowTile, (float) z * meadowTile};
            pass.drawInstanced(grassShader, meadow.size());
        }
}

void CowsView::drawTitle(RenderPass& pass)
{
    if (game.state != Game::State::Found)
        return;

    titleShader.placement = titlePlacement();
    titleShader.titleColor = Palette::linear(Palette::title);

    pass.draw(titleShader);
}

Mat4 CowsView::titlePlacement() const
{
    auto rise =
        std::max(easeInOut((game.sinceFound - titleDelay) / titleRiseTime), 0.01f);
    auto lift = Vec3 {0.f, -titleDrop * (1.f - rise), 0.f};

    return game.stage() * Mat4::translation(titleCenter + lift)
           * Mat4::rotationX(-0.08f) * Mat4::scale(titleScale * rise);
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

        const auto& mesh = meshFor((Shape) index);

        shader.setInstances(1, list.data(), list.size());
        pass.bind(shader, mesh.vertices);
        shader.bindInstances(pass);
        pass.drawIndexedInstanced(mesh.indices, mesh.indexCount, list.size());
    }
}

const Mesh& CowsView::meshFor(Shape shape) const
{
    switch (shape)
    {
        case Shape::Capsule:
            return capsule;
        case Shape::Horn:
            return horn;
        case Shape::Heart:
            return heart;
        case Shape::Barrel:
            return barrel;
        case Shape::Box:
            return box;
        case Shape::Wedge:
            return wedge;
        case Shape::Sphere:
            break;
    }

    return sphere;
}
} // namespace Cows
