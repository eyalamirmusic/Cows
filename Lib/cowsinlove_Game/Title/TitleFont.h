#pragma once

#include "Render/Mesh.h"

#include <string_view>

namespace Cows
{
struct TitleVertex final
{
    Maths::Vec3 position;
    Maths::Vec3 normal;
    float letter = 0.f;
};

struct TitleMesh final
{
    Vector<TitleVertex> vertices;
    Vector<std::uint32_t> indices;
    int letterCount = 0;
    float width = 0.f;
    float bottom = 0.f;
    float top = 0.f;
};

// x-height 1, centred on x = 0, baseline at y = 0; unknown letters are spaces.
TitleMesh makeTitle(std::string_view text);

TitleMesh stackTitles(const TitleMesh& upper, const TitleMesh& lower, float drop);
} // namespace Cows
