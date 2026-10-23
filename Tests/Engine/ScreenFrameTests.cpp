#include "UI/MenuStyle.h"

#include <NanoTest/NanoTest.h>

using namespace nano;
using namespace Cows;

namespace
{
constexpr auto screenWidth = 466.f;
constexpr auto screenHeight = 678.f;
constexpr auto cameraSideInset = 84.f;
constexpr auto homeIndicatorInset = 34.f;
} // namespace

auto tCentred = test("ScreenFrame/contentAreaStaysCentredWithOneSideInset") = []
{
    auto frame = ScreenFrame {};
    frame.bounds = {0.f, 0.f, screenWidth, screenHeight};
    frame.safe = {.bottom = homeIndicatorInset, .right = cameraSideInset};

    auto content = frame.contentArea();
    auto contentCentre = content.x + content.w / 2.f;

    check(content.x == cameraSideInset, "left inset matches the right one");
    check(content.x + content.w == screenWidth - cameraSideInset,
          "right edge clears the right inset");
    check(contentCentre == screenWidth / 2.f, "centred on the screen");
};
