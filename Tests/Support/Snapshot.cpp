#include "Snapshot.h"

#include <array>
#include <cmath>

using namespace Maths;

namespace Cows::Testing
{
namespace
{
constexpr auto glowQuad = std::to_array<CornerVertex>({
    {{-1.f, -1.f}},
    {{1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, 1.f}},
});

RenderPipelineDescriptor solidPipeline(int samples)
{
    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = samples;
    descriptor.depth = true;
    descriptor.cullMode = CullMode::Back;
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
} // namespace

bool isClearColor(const Graphics::Color& color)
{
    auto near = [](float channel, float target)
    { return std::abs(channel - target) < 2.f / 255.f; };

    return near(color.r, clearColor.r) && near(color.g, clearColor.g)
           && near(color.b, clearColor.b);
}

bool hasDevice()
{
    return Device::shared().isValid();
}

SnapshotView::SnapshotView(const MeshData& heartMesh)
    : sphere(makeSphere(32, 48))
    , capsule(makeCapsule(0.14f, 32))
    , horn(makeHorn(24))
    , heart(heartMesh)
    , barrel(makeBarrel(48))
    , box(makeBox())
    , wedge(makeWedge())
{
    setDepth(true);

    auto samples = sampleCount();
    surfaceShader.prepare(solidPipeline(samples));
    translucentShader.prepare(translucentPipeline(samples));
    glowShader.setVertices(glowQuad);
    glowShader.prepare(glowPipeline(samples));
}

void SnapshotView::render(Frame& frame)
{
    clearShadows(frame);

    auto pass = frame.beginPass({clearColor});
    auto aspect = (float) pass.targetWidth() / (float) pass.targetHeight();
    auto viewProjection = camera.projection(aspect) * camera.view();

    setSceneUniforms(surfaceShader, viewProjection);
    setSceneUniforms(translucentShader, viewProjection);

    draw(pass, surfaceShader, batch);
    drawOpaque(pass, viewProjection);
    draw(pass, translucentShader, translucent);
    drawGlows(pass, viewProjection);
    drawOverlay(frame, pass);
}

void SnapshotView::clearShadows(Frame& frame)
{
    auto descriptor = RenderPassDescriptor {};
    descriptor.clearColor = Graphics::Color {1.f, 1.f, 1.f};
    auto pass = frame.beginPass(shadowMap.texture, descriptor);
}

void SnapshotView::setSceneUniforms(SceneUniforms& uniforms,
                                    const Mat4& viewProjection)
{
    uniforms.setLighting(lighting);
    uniforms.viewProjection = viewProjection;
    uniforms.lightViewProjection =
        shadowMap.lightViewProjection(lighting.keyDirection, camera.target);
    uniforms.eyePosition = camera.eye();
    uniforms.time = time;
    uniforms.shadowMap = shadowMap.texture;
}

void SnapshotView::draw(RenderPass& pass,
                        SurfaceShader& shader,
                        SurfaceBatch& shapes)
{
    for (auto index = 0; index < shapeCount; ++index)
    {
        const auto& list = shapes.lists[index];

        if (list.empty())
            continue;

        const auto& mesh = meshFor((Shape) index);

        shader.setInstances(1, list.data(), list.size());
        pass.bind(shader, mesh.vertices);
        shader.bindInstances(pass);
        pass.drawIndexedInstanced(mesh.indices, mesh.indexCount, list.size());
    }
}

void SnapshotView::drawGlows(RenderPass& pass, const Mat4& viewProjection)
{
    if (glows.empty())
        return;

    glowShader.viewProjection = viewProjection;
    glowShader.cameraRight = camera.right();
    glowShader.cameraUp = camera.up();
    glowShader.setInstances(1, glows.data(), glows.size());

    pass.drawInstanced(glowShader, glows.size());
}

const Mesh& SnapshotView::meshFor(Shape shape) const
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

Graphics::Image
    snapshot(SnapshotView& view, float width, float height, const std::string& name)
{
    view.setBounds({0.f, 0.f, width, height});

    auto image = view.renderToImage(1.f);

    if (image.isValid())
        image.save(FilePath {std::string(COWS_SHOTS_DIR) + "/" + name + ".png"});

    return image;
}
} // namespace Cows::Testing
