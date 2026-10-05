#include "Title/MenuTitle.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows::MenuTitle
{
namespace
{
constexpr auto fill = 0.74f;
} // namespace

TitleMesh makeWide()
{
    return makeTitle(text);
}

TitleMesh makeTall()
{
    return stackTitles(makeTitle(upperLine), makeTitle(lowerLine), lineDrop);
}

Mat4 placement(const OrbitCamera& camera,
               Graphics::Point viewSize,
               const Graphics::Rect& area,
               const TitleMesh& title)
{
    if (viewSize.x <= 0.f || viewSize.y <= 0.f || title.width <= 0.f)
        return Mat4::scale(0.f);

    auto aspect = viewSize.x / viewSize.y;
    auto halfHeight = depth * std::tan(camera.verticalFieldOfView(aspect) * 0.5f);
    auto halfWidth = halfHeight * aspect;
    auto unitsPerPoint = 2.f * halfHeight / viewSize.y;

    auto height = title.top - title.bottom;
    auto scale = std::min(area.w * fill * unitsPerPoint / title.width,
                          area.h * fill * unitsPerPoint / height);

    auto centerX = area.x + area.w * 0.5f;
    auto centerY = area.y + area.h * 0.5f;
    auto across = (2.f * centerX / viewSize.x - 1.f) * halfWidth;
    auto up = (1.f - 2.f * centerY / viewSize.y) * halfHeight;
    auto middle = (title.top + title.bottom) * 0.5f;

    return camera.view().inverted() * Mat4::translation({across, up, -depth})
           * Mat4::scale(scale) * Mat4::translation({0.f, -middle, 0.f});
}
} // namespace Cows::MenuTitle
