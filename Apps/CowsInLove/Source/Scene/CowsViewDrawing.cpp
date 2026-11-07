#include "CowsView.h"
#include "CowsViewTuning.h"
#include "Cow/KissHearts.h"
#include "Ending.h"
#include "Render/FrameProfile.h"
#include "Render/Frustum.h"
#include "Render/Palette.h"
#include "Sky/SkyDecor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

using namespace Maths;

namespace Cows
{
using namespace CowsViewTuning;

namespace
{
float mooShape(float sinceMoo)
{
    if (sinceMoo < 0.f || sinceMoo > mooLength)
        return 0.f;

    return std::sin(pi * sinceMoo / mooLength);
}

Graphics::Color displayColor(const Vec3& linear)
{
    auto encode = [](float channel) { return std::pow(channel, 1.f / 2.2f); };
    return {encode(linear.x), encode(linear.y), encode(linear.z)};
}
} // namespace

void CowsView::render(Frame& frame)
{
    using Part = FrameProfile::Part;
    auto& profile = FrameProfile::shared();

    {
        auto timed = FrameProfile::Scope {profile, Part::Gather};
        gatherInstances(elapsed);
    }

    lightViewProjection =
        shadowMap->lightViewProjection(lighting.keyDirection, groundFocus());

    {
        auto timed = FrameProfile::Scope {profile, Part::Shadows};
        drawShadows(frame);
    }

    auto timedScene = std::optional<FrameProfile::Scope> {};
    timedScene.emplace(profile, Part::Scene);
    auto sceneDescriptor = RenderPassDescriptor {};
    sceneDescriptor.clearColor = displayColor(lighting.horizonColor);
    if (profile.enabled)
        sceneDescriptor.label = "scene";
    auto pass = frame.beginPass(sceneDescriptor);

    auto width = (float) pass.targetWidth();
    auto height = (float) pass.targetHeight();

    if (width <= 0.f || height <= 0.f)
        return;

    auto aspect = width / height;
    viewAspect = aspect;
    framePortrait(aspect);
    auto viewProjection = camera.projection(aspect) * camera.view();

    setAllSceneUniforms(viewProjection);
    drawScene(pass, viewProjection, {width, height});
    timedScene.reset();

    auto timedHud = FrameProfile::Scope {profile, Part::Hud};
    if (profileSettings().skips("hud"))
        return;
    hud.begin(frame, pass, sampleCount());
    drawHud(hud);
    hud.end();
}

void CowsView::setAllSceneUniforms(const Mat4& viewProjection)
{
    setSceneUniforms(*surfaceShader, viewProjection);
    setSceneUniforms(*plainShader, viewProjection);
    setSceneUniforms(*translucentShader, viewProjection);
    setSceneUniforms(*groundShader, viewProjection);
    setSceneUniforms(*grassShader, viewProjection);
    setSceneUniforms(titleShader, viewProjection);
    setSceneUniforms(menuTitleShader, viewProjection);
}

void CowsView::drawScene(RenderPass& pass,
                         const Mat4& viewProjection,
                         Graphics::Point targetSize)
{
    auto sceneTarget = BatchTarget {pass, viewProjection};

    if (!profileSettings().skips("ground"))
        drawGround(pass);
    if (!profileSettings().skips("objects"))
        drawObjects(sceneTarget);
    if (!profileSettings().skips("grass"))
        drawGrass(pass, viewProjection);
    drawTitle(pass);
    drawMenuTitle(pass, targetSize.x, targetSize.y);
    if (!profileSettings().skips("sky"))
        drawSky(pass, targetSize.x / targetSize.y);
    drawBatch(sceneTarget, *translucentShader, heartBatch);
    if (!profileSettings().skips("glow"))
        drawGlows(pass, viewProjection);
}

void CowsView::drawObjects(const BatchTarget& target)
{
    drawBatch(target, *plainShader, chasms);
    drawBatch(target, *plainShader, backdropBatch);
    drawBatch(target, *plainShader, game.level.batch);
    drawBatch(target, *plainShader, game.level.moving);
    drawBatch(target, *surfaceShader, cowBatch);
}

void CowsView::gatherInstances(float seconds)
{
    cowBatch.clear();
    backdropBatch.clear();
    heartBatch.clear();
    glows.clear();

    auto poses = std::array<CowPose, 2> {};

    if (game.getStateBehindMenu() == Game::State::Searching)
        poseCowsSearching(poses, seconds);
    else
        poseCowsInEnding(poses);

    addCowsToBatches(poses);

    auto origin = groundFocus();
    SkyDecor::addSun(backdropBatch, glows, seconds, origin);
    SkyDecor::addClouds(backdropBatch, seconds, origin);
    SkyDecor::addHills(backdropBatch, origin);
}

void CowsView::poseCowsSearching(std::array<CowPose, 2>& poses, float seconds)
{
    auto warmth = game.warmth();
    poses[0] = cows[0].freePose({game.player,
                                 game.playerHeading,
                                 game.hopClock,
                                 game.bounce,
                                 seconds,
                                 game.beatClock,
                                 0.6f + 1.4f * warmth});
    poses[0].moo = mooShape(game.sinceMoo);
    poses[1] = cows[1].freePose({game.partner,
                                 game.partnerHeading,
                                 game.hopClock,
                                 0.f,
                                 seconds,
                                 game.beatClock,
                                 2.f * warmth * warmth});
    poses[1].moo = mooShape(game.sinceMoo - mooAnswerDelay);
    addAnswerFlare();
}

void CowsView::poseCowsInEnding(std::array<CowPose, 2>& poses)
{
    auto ending = game.endingSeconds();
    auto stage = game.stage();

    for (auto index = 0; index < 2; ++index)
        poses[index] = cows[index].pose(ending, stage);

    addKissHearts(
        heartBatch, glows, ending, transformPoint(stage, Ending::kissPoint));
}

void CowsView::addCowsToBatches(const std::array<CowPose, 2>& poses)
{
    for (auto index = 0; index < 2; ++index)
    {
        cows[index].addTo(
            cowBatch, glows, index == 0 ? playerParts : cowParts, poses[index]);
        contacts[index] = cows[index].contact(poses[index]);

        if (poses[index].world.column(3).y > contactReach)
            contacts[index].z = 0.f;
    }
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
        glows.add(makeGlow(at, 2.4f - 0.2f * (float) flare, color, 0.f));
    }
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
    uniforms.shadowMap = shadowMap->texture;
}

void CowsView::drawShadows(Frame& frame)
{
    auto descriptor = RenderPassDescriptor {};
    descriptor.clearColor = Graphics::Color {1.f, 1.f, 1.f};
    descriptor.label = "shadows";

    if (profileSettings().skips("shadows"))
        return;

    auto pass = frame.beginPass(shadowMap->texture, descriptor);
    shadowCaster.lightViewProjection = lightViewProjection;
    auto shadowTarget =
        BatchTarget {pass, lightViewProjection, FrameProfile::Pass::Shadows};
    drawBatch(shadowTarget, shadowCaster, cowBatch);
    drawBatch(shadowTarget, shadowCaster, game.level.batch);
    drawBatch(shadowTarget, shadowCaster, game.level.moving);
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
    groundShader->noise = noiseLattice.texture;
    groundShader->firstContact = contacts[0];
    groundShader->secondContact = contacts[1];

    pass.bind(*groundShader, ground.vertices);
    pass.drawIndexed(ground.indices, ground.indexCount);
    FrameProfile::shared().drew(FrameProfile::Pass::Scene, 1, ground.indexCount / 3);
}

void CowsView::drawGrass(RenderPass& pass, const Mat4& viewProjection)
{
    auto focus = groundFocus();
    auto plan =
        planGrass({focus.x, focus.z}, viewProjection, camera.eye(), grassDensity);
    auto cut = Vector<GrassDraw> {};

    for (const auto& draw: plan)
    {
        const auto& blades = grass.tileAt(draw.corner);

        if (&blades == &grass.tile)
            drawGrassTile(pass, draw, blades);
        else
            cut.add(draw);
    }

    for (const auto& draw: cut)
        drawGrassTile(pass, draw, grass.tileAt(draw.corner));
}

void CowsView::drawGrassTile(RenderPass& pass,
                             const GrassDraw& draw,
                             const Vector<BladeInstance>& blades)
{
    auto count = std::min(bladesToDraw((int) blades.size(), draw.share),
                          profileSettings().blades);

    if (count == 0)
        return;

    if (uploadedBlades != &blades)
    {
        grassShader->setInstances(1, blades.data(), blades.size());
        uploadedBlades = &blades;
    }

    grassShader->patchOffset = draw.corner;
    pass.drawInstanced(*grassShader, count);
    FrameProfile::shared().drew(FrameProfile::Pass::Scene, count, bladeTriangles);
}

void CowsView::drawTitle(RenderPass& pass)
{
    if (game.getStateBehindMenu() != Game::State::Found || title.indices.empty())
        return;

    titleShader.placement = Ending::titlePlacement(game);
    titleShader.titleColor = Palette::linear(Palette::title);

    pass.draw(titleShader);
}

bool CowsView::isMenuTitleShowing() const
{
    return game.state == Game::State::Menu && menu != nullptr
           && game.getStateBehindMenu() != Game::State::Found;
}

void CowsView::drawMenuTitle(RenderPass& pass, float width, float height)
{
    if (!isMenuTitleShowing())
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
    glowShader.eyePosition = camera.eye();
    glowShader.setInstances(1, glows.data(), glows.size());

    pass.drawInstanced(glowShader, glows.size());
}

void CowsView::drawBatch(const BatchTarget& target,
                         ShaderProgram& shader,
                         const SurfaceBatch& batch)
{
    for (auto index = 0; index < shapeCount; ++index)
    {
        const auto& mesh = shapes[(Shape) index];

        visible.clear();

        for (const auto& instance: batch.lists[index])
            if (instanceInView(target.cullWith, instance, mesh.radius))
                visible.add(instance);

        if (visible.empty())
            continue;

        shader.setInstances(1, visible.data(), visible.size());
        target.pass.bind(shader, mesh.vertices);
        shader.bindInstances(target.pass);
        target.pass.drawIndexedInstanced(
            mesh.indices, mesh.indexCount, visible.size());
        FrameProfile::shared().drew(
            target.counted, (int) visible.size(), mesh.indexCount / 3);
    }
}

} // namespace Cows
