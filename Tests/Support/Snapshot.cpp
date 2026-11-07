#include "Snapshot.h"

#include <eacp/Core/Utils/Files.h>

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

RenderPipelineDescriptor makeTranslucent(RenderPipelineDescriptor descriptor)
{
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

bool isPixelDrawn(const Graphics::Image& image, int x, int y)
{
    auto isInsideImage = x >= 0 && y >= 0 && x < image.width() && y < image.height();
    return isInsideImage && !isClearColor(image.at(x, y));
}
} // namespace

bool isAnythingDrawnIn(const Graphics::Image& image, const Graphics::Rect& area)
{
    auto left = (int) area.x;
    auto top = (int) area.y;
    auto right = (int) (area.x + area.w);
    auto bottom = (int) (area.y + area.h);

    for (auto y = top; y < bottom; ++y)
        for (auto x = left; x < right; ++x)
            if (isPixelDrawn(image, x, y))
                return true;

    return false;
}

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

SnapshotView::SnapshotView(const std::function<MeshData(Shape)>& content)
    : shapes(content)
{
    setDepth(true);

    auto samples = sampleCount();
    surfaceShader.prepare(solidPipeline(samples));
    translucentShader.prepare(makeTranslucent(solidPipeline(samples)));
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
    glowShader.drawGlows(pass, glows, viewProjection, camera);
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
                        SurfaceBatch& instances)
{
    for (auto index = 0; index < shapeCount; ++index)
    {
        const auto& list = instances.lists[index];

        if (list.empty())
            continue;

        const auto& mesh = shapes[(Shape) index];

        shader.setInstances(1, list.data(), list.size());
        pass.bind(shader, mesh.vertices);
        shader.bindInstances(pass);
        pass.drawIndexedInstanced(mesh.indices, mesh.indexCount, list.size());
    }
}

bool isMiddleClear(const Graphics::Image& image)
{
    auto centerX = image.width() / 2;
    auto centerY = image.height() / 2;

    constexpr auto halfHeight = 10;
    constexpr auto halfWidth = 20;

    for (auto y = centerY - halfHeight; y < centerY + halfHeight; ++y)
        for (auto x = centerX - halfWidth; x < centerX + halfWidth; ++x)
            if (!isClearColor(image.at(x, y)))
                return false;

    return true;
}

Graphics::Image
    snapshot(SnapshotView& view, float width, float height, const std::string& name)
{
    view.setBounds({0.f, 0.f, width, height});

    auto image = view.renderToImage(1.f);

    if (image.isValid())
    {
        auto file = FilePath {std::string(COWS_SHOTS_DIR) + "/" + name + ".png"};
        Files::createDirectories(file.parentDirectory());
        image.save(file);
    }

    return image;
}
} // namespace Cows::Testing
