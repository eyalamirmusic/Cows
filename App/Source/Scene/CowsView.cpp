#include "CowsView.h"
#include "HeartMesh.h"
#include "KissHearts.h"
#include "Palette.h"
#include "SkyDecor.h"

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

    setHandlesMouseEvents(true);
    setContinuous(true);
}

void CowsView::update(Threads::FrameTime time)
{
    if (!frozen)
        elapsed += (float) time.delta;
}

void CowsView::render(Frame& frame)
{
    gatherInstances(elapsed);
    lightViewProjection = shadowMap.lightViewProjection(lighting.keyDirection);

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
    drawBatch(pass, surfaceShader, cowBatch);
    drawGrass(pass);
    drawTitle(pass);
    drawBatch(pass, translucentShader, heartBatch);
    drawGlows(pass, viewProjection);
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
    camera.drift(seconds);

    cowBatch.clear();
    backdropBatch.clear();
    heartBatch.clear();
    glows.clear();

    for (const auto& cow: cows)
        cow.addTo(cowBatch, glows, cowParts, seconds);

    addKissHearts(heartBatch, glows, seconds, kissPoint);

    SkyDecor::addSun(backdropBatch, glows, seconds);
    SkyDecor::addClouds(backdropBatch, seconds);
    SkyDecor::addHills(backdropBatch);
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
}

void CowsView::drawSky(RenderPass& pass, float aspect)
{
    auto halfHeight = std::tan(camera.fieldOfView * 0.5f);

    skyShader.setLighting(lighting);
    skyShader.cameraForward = camera.forward();
    skyShader.cameraRight = camera.right();
    skyShader.cameraUp = camera.up();
    skyShader.lensScale = Vec2 {halfHeight * aspect, halfHeight};
    skyShader.towardSun = normalize(SkyDecor::sunCenter - camera.eye());

    pass.draw(skyShader);
}

void CowsView::drawGround(RenderPass& pass)
{
    groundShader.firstContact = cows[0].contact(elapsed);
    groundShader.secondContact = cows[1].contact(elapsed);

    pass.bind(groundShader, ground.vertices);
    pass.drawIndexed(ground.indices, ground.indexCount);
}

void CowsView::drawGrass(RenderPass& pass)
{
    pass.drawInstanced(grassShader, meadow.size());
}

void CowsView::drawTitle(RenderPass& pass)
{
    titleShader.placement = Mat4::translation(titleCenter) * Mat4::rotationX(-0.08f)
                            * Mat4::scale(titleScale);
    titleShader.titleColor = Palette::linear(Palette::title);

    pass.draw(titleShader);
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
        case Shape::Sphere:
            break;
    }

    return sphere;
}
} // namespace Cows
