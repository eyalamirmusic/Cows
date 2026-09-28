#pragma once

#include "Camera/OrbitCamera.h"
#include "Render/Lighting.h"
#include "Render/ShadowMap.h"
#include "Title/TitleShader.h"

#include <string_view>

namespace Cows
{
// The tube-font title alone, on a flat colour, for cutting out as a logo.
struct LogoView final : GPUView
{
    explicit LogoView(std::string_view text);

    void render(Frame& frame) override;

    Graphics::Color background {0.f, 0.f, 0.f};
    Lighting lighting;
    OrbitCamera camera;
    float time = 0.f;
    TitleMesh title;

private:
    ShadowMap shadowMap;
    TitleShader titleShader;
};
} // namespace Cows
