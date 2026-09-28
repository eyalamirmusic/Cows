#include "Ending.h"
#include "Render/Palette.h"
#include "Snapshot.h"
#include "Title/TitleFont.h"
#include "Title/TitleShader.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Cows::Testing;
using namespace Maths;

namespace
{
bool finite(Vec3 vector)
{
    return std::isfinite(vector.x) && std::isfinite(vector.y)
           && std::isfinite(vector.z);
}

bool middleIsClear(const Graphics::Image& image)
{
    auto centerX = image.width() / 2;
    auto centerY = image.height() / 2;

    for (auto y = centerY - 10; y < centerY + 10; ++y)
        for (auto x = centerX - 20; x < centerX + 20; ++x)
            if (!isClearColor(image.at(x, y)))
                return false;

    return true;
}
} // namespace

auto tGlyphs = test("TitleFont/everyGlyphHasGeometry") = []
{
    auto text = std::string_view {Ending::titleText};
    auto title = makeTitle(text);

    check(title.letterCount == (int) text.size());
    check(title.width > 0.f);

    for (auto letter = 0; letter < title.letterCount; ++letter)
    {
        auto found = false;

        for (const auto& vertex: title.vertices)
            found = found || vertex.letter == (float) letter;

        check(found);
    }
};

auto tMeshValid = test("TitleFont/indicesInRangeAndVerticesFinite") = []
{
    auto title = makeTitle(Ending::titleText);

    check(!title.indices.empty());
    check(title.indices.size() % 3 == 0);

    for (auto index: title.indices)
        check(index < (std::uint32_t) title.vertices.size());

    for (const auto& vertex: title.vertices)
        check(finite(vertex.position) && finite(vertex.normal)
              && std::isfinite(vertex.letter));
};

auto tTitleSnapshot = test("TitleFont/snapshot") = []
{
    if (!hasDevice())
        return;

    auto title = makeTitle(Ending::titleText);

    auto view = SnapshotView {};
    view.camera.target = {0.f, 0.5f, 0.f};
    view.camera.yaw = 0.f;
    view.camera.pitch = 0.05f;
    view.camera.distance = 13.f;
    view.time = 0.3f;

    auto titleShader = TitleShader {};
    titleShader.setVertices(title.vertices.data(), title.vertices.size());
    titleShader.setIndices(title.indices.data(), title.indices.size());

    auto descriptor = RenderPipelineDescriptor {};
    descriptor.sampleCount = view.sampleCount();
    descriptor.depth = true;
    descriptor.cullMode = CullMode::Back;
    titleShader.prepare(descriptor);

    view.drawOpaque = [&](RenderPass& pass, const Mat4& viewProjection)
    {
        view.setSceneUniforms(titleShader, viewProjection);
        titleShader.placement = Mat4::scale(1.f);
        titleShader.titleColor = Palette::linear(Palette::title);
        pass.draw(titleShader);
    };

    auto image = snapshot(view, 400.f, 160.f, "game-title");
    check(image.isValid());

    if (!image.isValid())
        return;

    check(!middleIsClear(image));
};
