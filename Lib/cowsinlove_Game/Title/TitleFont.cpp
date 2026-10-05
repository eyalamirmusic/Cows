#include "Title/TitleFont.h"

#include <algorithm>
#include <cmath>

using namespace Maths;

namespace Cows
{
namespace
{
constexpr auto tubeRadius = 0.135f;
constexpr auto tubeSides = 16;
constexpr auto letterGap = 0.52f;
constexpr auto spaceWidth = 0.6f;
constexpr auto degreesPerStep = 8.f;

struct Stroke final
{
    Vector<Vec2> points;
    bool closed = false;
};

struct Glyph final
{
    Vector<Stroke> strokes;
    Vector<Vec3> dots;
    float width = 0.f;
};

void addArc(Stroke& stroke,
            Vec2 center,
            Vec2 radius,
            float fromDegrees,
            float toDegrees,
            bool skipFirst = false)
{
    auto span = toDegrees - fromDegrees;
    auto steps = std::max(2, (int) std::ceil(std::abs(span) / degreesPerStep));

    for (auto step = skipFirst ? 1 : 0; step <= steps; ++step)
    {
        auto angle = radians(fromDegrees + span * (float) step / (float) steps);
        stroke.points.add(
            center + Vec2 {radius.x * std::cos(angle), radius.y * std::sin(angle)});
    }
}

Stroke arc(Vec2 center, Vec2 radius, float fromDegrees, float toDegrees)
{
    auto stroke = Stroke {};
    addArc(stroke, center, radius, fromDegrees, toDegrees);
    return stroke;
}

Stroke line(Vec2 from, Vec2 to)
{
    auto stroke = Stroke {};
    stroke.points.add(from);
    stroke.points.add(to);
    return stroke;
}

Stroke archThenLeg(Vec2 center, float radius)
{
    auto stroke = arc(center, {radius, radius}, 180.f, 0.f);
    stroke.points.add({center.x + radius, 0.f});
    return stroke;
}

Glyph glyphFor(char letter)
{
    auto glyph = Glyph {};

    switch (letter)
    {
        case 'c':
            glyph.width = 0.95f;
            glyph.strokes.add(arc({0.5f, 0.5f}, {0.47f, 0.5f}, 42.f, 318.f));
            break;

        case 'o':
        {
            glyph.width = 1.f;
            auto ring = arc({0.5f, 0.5f}, {0.5f, 0.5f}, 0.f, 352.f);
            ring.closed = true;
            glyph.strokes.add(ring);
            break;
        }

        case 'w':
        {
            glyph.width = 1.36f;
            Vec2 points[] = {{0.f, 1.f},
                             {0.32f, 0.f},
                             {0.68f, 0.74f},
                             {1.04f, 0.f},
                             {1.36f, 1.f}};

            for (auto i = 0; i < 4; ++i)
                glyph.strokes.add(line(points[i], points[i + 1]));

            break;
        }

        case 's':
        {
            glyph.width = 0.82f;
            auto stroke = Stroke {};
            addArc(stroke, {0.41f, 0.75f}, {0.4f, 0.25f}, 20.f, 270.f);
            addArc(stroke, {0.41f, 0.25f}, {0.4f, 0.25f}, 90.f, -160.f, true);
            glyph.strokes.add(stroke);
            break;
        }

        case 'i':
            glyph.width = 0.f;
            glyph.strokes.add(line({0.f, 0.f}, {0.f, 1.f}));
            glyph.dots.add({0.f, 1.42f, 0.17f});
            break;

        case 'n':
            glyph.width = 0.95f;
            glyph.strokes.add(line({0.f, 0.f}, {0.f, 1.f}));
            glyph.strokes.add(archThenLeg({0.475f, 0.53f}, 0.475f));
            break;

        case 'm':
            glyph.width = 1.5f;
            glyph.strokes.add(line({0.f, 0.f}, {0.f, 1.f}));
            glyph.strokes.add(archThenLeg({0.375f, 0.6f}, 0.375f));
            glyph.strokes.add(archThenLeg({1.125f, 0.6f}, 0.375f));
            break;

        case 'l':
            glyph.width = 0.f;
            glyph.strokes.add(line({0.f, 0.f}, {0.f, 1.62f}));
            break;

        case 'v':
            glyph.width = 1.f;
            glyph.strokes.add(line({0.f, 1.f}, {0.5f, 0.f}));
            glyph.strokes.add(line({0.5f, 0.f}, {1.f, 1.f}));
            break;

        case 'e':
            glyph.width = 1.f;
            glyph.strokes.add(line({0.03f, 0.52f}, {0.98f, 0.52f}));
            glyph.strokes.add(arc({0.5f, 0.5f}, {0.49f, 0.5f}, 2.f, 322.f));
            break;

        case '.':
            glyph.width = 0.f;
            glyph.dots.add({0.f, 0.08f, 0.18f});
            break;

        default:
            glyph.width = spaceWidth - letterGap;
            break;
    }

    return glyph;
}

Vec2 tangentAt(const Stroke& stroke, int index)
{
    auto count = stroke.points.size();

    if (stroke.closed)
        return normalize(stroke.points[(index + 1) % count]
                         - stroke.points[(index + count - 1) % count]);

    auto before = stroke.points[std::max(index - 1, 0)];
    auto after = stroke.points[std::min(index + 1, count - 1)];
    return normalize(after - before);
}

void addTube(MeshData& mesh, const Stroke& stroke)
{
    auto count = stroke.points.size();
    auto rings = stroke.closed ? count + 1 : count;
    auto base = (std::uint32_t) mesh.vertices.size();

    for (auto ring = 0; ring < rings; ++ring)
    {
        auto index = ring % count;
        auto point = stroke.points[index];
        auto tangent = tangentAt(stroke, index);
        auto side = Vec3 {-tangent.y, tangent.x, 0.f};

        for (auto around = 0; around <= tubeSides; ++around)
        {
            auto angle = twoPi * (float) around / (float) tubeSides;
            auto normal = side * std::cos(angle) + Vec3 {0.f, 0.f, std::sin(angle)};
            mesh.vertices.add(
                {Vec3 {point.x, point.y, 0.f} + normal * tubeRadius, normal});
        }
    }

    for (auto ring = 0; ring + 1 < rings; ++ring)
        for (auto around = 0; around < tubeSides; ++around)
        {
            auto a = base + (std::uint32_t) (ring * (tubeSides + 1) + around);
            auto b = a + (std::uint32_t) (tubeSides + 1);
            mesh.indices.add({a, a + 1, b + 1, a, b + 1, b});
        }
}

void addBall(MeshData& mesh, const MeshData& sphere, Vec2 at, float radius)
{
    append(mesh, sphere, Mat4::translation({at.x, at.y, 0.f}) * Mat4::scale(radius));
}

MeshData buildGlyph(const Glyph& glyph, const MeshData& sphere)
{
    auto mesh = MeshData {};

    for (const auto& stroke: glyph.strokes)
    {
        addTube(mesh, stroke);

        if (!stroke.closed)
        {
            addBall(mesh, sphere, stroke.points.front(), tubeRadius);
            addBall(mesh, sphere, stroke.points.back(), tubeRadius);
        }
    }

    for (const auto& dot: glyph.dots)
        addBall(mesh, sphere, {dot.x, dot.y}, dot.z);

    windOutward(mesh);
    return mesh;
}
} // namespace

TitleMesh makeTitle(std::string_view text)
{
    auto sphere = makeSphere(12, 20);
    auto title = TitleMesh {};
    auto pen = 0.f;

    for (auto letter: text)
    {
        auto glyph = glyphFor(letter);
        auto mesh = buildGlyph(glyph, sphere);
        auto base = (std::uint32_t) title.vertices.size();
        auto index = (float) title.letterCount;

        for (const auto& vertex: mesh.vertices)
            title.vertices.add(
                {vertex.position + Vec3 {pen, 0.f, 0.f}, vertex.normal, index});

        for (auto vertexIndex: mesh.indices)
            title.indices.add(base + vertexIndex);

        pen += glyph.width + letterGap;
        ++title.letterCount;
    }

    title.width = pen - letterGap;

    for (auto& vertex: title.vertices)
    {
        vertex.position.x -= title.width * 0.5f;
        title.bottom = std::min(title.bottom, vertex.position.y);
        title.top = std::max(title.top, vertex.position.y);
    }

    return title;
}

TitleMesh stackTitles(const TitleMesh& upper, const TitleMesh& lower, float drop)
{
    auto title = upper;
    auto base = (std::uint32_t) title.vertices.size();
    auto letters = (float) upper.letterCount;

    for (const auto& vertex: lower.vertices)
        title.vertices.add({vertex.position - Vec3 {0.f, drop, 0.f},
                            vertex.normal,
                            vertex.letter + letters});

    for (auto index: lower.indices)
        title.indices.add(base + index);

    title.letterCount += lower.letterCount;
    title.width = std::max(upper.width, lower.width);
    title.bottom = std::min(upper.bottom, lower.bottom - drop);
    return title;
}
} // namespace Cows
