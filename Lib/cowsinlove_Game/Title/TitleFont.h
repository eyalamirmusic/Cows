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

// A lowercase text in a rounded tube font: every stroke a pink balloon tube with
// ball ends, the x-height one unit, centred on x = 0 with its baseline at y = 0.
// Only the letters "cowsinlove.com" needs are drawn; anything else is a space.
TitleMesh makeTitle(std::string_view text);

// `lower` under `upper`, its baseline `drop` below, the wave running on through
// it.
TitleMesh stackTitles(const TitleMesh& upper, const TitleMesh& lower, float drop);
} // namespace Cows
