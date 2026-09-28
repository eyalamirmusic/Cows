#include "LogoView.h"
#include "Render/Palette.h"

using namespace Maths;

namespace Cows
{
LogoView::LogoView(std::string_view text)
    : title(makeTitle(text))
{
    setDepth(true);

    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = sampleCount();
    descriptor.depth = true;
    descriptor.cullMode = CullMode::Back;

    titleShader.setVertices(title.vertices.data(), title.vertices.size());
    titleShader.setIndices(title.indices.data(), title.indices.size());
    titleShader.prepare(descriptor);
}

void LogoView::render(Frame& frame)
{
    auto shadows = RenderPassDescriptor {};
    shadows.clearColor = Graphics::Color {1.f, 1.f, 1.f};
    frame.beginPass(shadowMap.texture, shadows);

    auto pass = frame.beginPass({background});
    auto aspect = (float) pass.targetWidth() / (float) pass.targetHeight();
    auto viewProjection = camera.projection(aspect) * camera.view();

    titleShader.setLighting(lighting);
    titleShader.viewProjection = viewProjection;
    titleShader.lightViewProjection =
        shadowMap.lightViewProjection(lighting.keyDirection, camera.target);
    titleShader.eyePosition = camera.eye();
    titleShader.time = time;
    titleShader.shadowMap = shadowMap.texture;
    titleShader.placement = Mat4 {};
    titleShader.titleColor = Palette::linear(Palette::title);

    pass.draw(titleShader);
}
} // namespace Cows
