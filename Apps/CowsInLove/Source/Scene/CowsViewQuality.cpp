#include "CowsView.h"
#include "CowsViewTuning.h"
#include "Props/Meshes.h"

namespace Cows
{
using namespace CowsViewTuning;

namespace
{
RenderPipelineDescriptor makeDepthTestedPipeline(int samples)
{
    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = samples;
    descriptor.depth = true;
    return descriptor;
}

RenderPipelineDescriptor skyPipeline(RenderPipelineDescriptor descriptor)
{
    descriptor.depthCompare = DepthCompare::LessEqual;
    descriptor.depthWrite = false;
    return descriptor;
}

RenderPipelineDescriptor solidPipeline(RenderPipelineDescriptor descriptor,
                                       CullMode cull = CullMode::Back)
{
    descriptor.cullMode = cull;
    return descriptor;
}

RenderPipelineDescriptor translucentPipeline(const RenderPipelineDescriptor& base)
{
    auto descriptor = solidPipeline(base);
    descriptor.blendMode = BlendMode::AlphaBlend;
    descriptor.depthWrite = false;
    return descriptor;
}

RenderPipelineDescriptor glowPipeline(RenderPipelineDescriptor descriptor)
{
    auto blend = BlendState {};
    blend.enabled = true;
    blend.sourceColor = BlendFactor::One;
    blend.destinationColor = BlendFactor::One;
    blend.sourceAlpha = BlendFactor::Zero;
    blend.destinationAlpha = BlendFactor::One;

    descriptor.depthWrite = false;
    descriptor.blend = blend;
    return descriptor;
}
} // namespace

void CowsView::chooseQuality(QualityChoice choice)
{
    qualityPreference.chosen = choice;
    forcedQuality.reset();
    applyQualityChoice();
    onQualityChanged();
}

void CowsView::applyQualityChoice()
{
    auto use = chooseQualityToUse(
        forcedQuality, qualityPreference.chosen, qualityPreference.measured);
    measuring = !use.has_value();
    governor = QualityGovernor {};
    lastTimedFrame = Device::shared().lastFrameTimings().frameIndex;

    auto tier = use.value_or((Quality) governor.level);

    if (!qualityReady || tier != quality)
        useQuality(tier);
}

void CowsView::useQuality(Quality chosen)
{
    quality = chosen;
    qualityReady = true;
    auto settings = getSettingsForQuality(chosen);

    setSampleCount(profileSettings().samples.value_or(settings.samples));
    setRenderScale(profileSettings().renderScale.value_or(settings.renderScale));
    grassDensity = settings.grass;
    auto detail = profileSettings().meshDetail.value_or(settings.meshDetail);
    shapes = ShapeMeshes {contentMeshes(detail), detail};
    shadowMap.reset();
    shadowMap.emplace(
        profileSettings().shadowResolution.value_or(settings.shadowResolution));

    surfaceShader.reset();
    surfaceShader.emplace(settings.shadowTaps);
    plainShader.reset();
    plainShader.emplace(settings.shadowTaps, false);
    translucentShader.reset();
    translucentShader.emplace(settings.shadowTaps, false);
    groundShader.reset();
    groundShader.emplace(settings.shadowTaps, settings.cheapGroundNoise);
    grassShader.reset();
    grassShader.emplace(settings.shadowTaps, settings.quickGrassNoise);

    auto blade = makeBlade(settings.bladeSegments);
    grassShader->setVertices(blade.vertices.data(), blade.vertices.size());
    grassShader->setIndices(blade.indices.data(), blade.indices.size());
    bladeTriangles = (int) blade.indices.size() / 3;
    grassShader->setInstances(1, grass.tile.data(), grass.tile.size());
    uploadedBlades = &grass.tile;

    preparePipelines();
    LOG("Cows: quality ", getQualityName(chosen));
}

void CowsView::preparePipelines()
{
    auto depthTested = makeDepthTestedPipeline(sampleCount());

    skyShader.prepare(skyPipeline(depthTested));
    surfaceShader->prepare(solidPipeline(depthTested));
    plainShader->prepare(solidPipeline(depthTested));
    translucentShader->prepare(translucentPipeline(depthTested));
    shadowCaster.prepare(shadowMap->pipeline());
    groundShader->prepare(solidPipeline(depthTested));
    grassShader->prepare(solidPipeline(depthTested, CullMode::None));
    titleShader.prepare(solidPipeline(depthTested));
    menuTitleShader.prepare(solidPipeline(depthTested));
    glowShader.prepare(glowPipeline(depthTested));
}

void CowsView::measureQuality()
{
    if (!measuring)
        return;

    const auto& timings = Device::shared().lastFrameTimings();

    if (timings.frameIndex == lastTimedFrame)
        return;

    lastTimedFrame = timings.frameIndex;

    if (governor.addFrame(timings.milliseconds))
        useQuality((Quality) governor.level);

    if (governor.decided())
    {
        measuring = false;
        qualityPreference.measured = quality;
        onQualityChanged();
    }
}

} // namespace Cows
