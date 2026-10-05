#include "Cow/Moo.h"
#include "Ending.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;

namespace
{
Game foundGame(float sinceFound)
{
    auto game = Game {};
    game.reset(3);
    game.state = Game::State::Found;
    game.sinceFound = sinceFound;
    return game;
}
} // namespace

auto tRiseEnds = test("Ending/titleRiseEnds") = []
{
    check(Ending::titleRise(0.f) == 0.01f);
    check(Ending::titleRise(Ending::titleDelay + Ending::titleRiseTime) == 1.f);
};

auto tRiseMonotonic = test("Ending/titleRiseNeverFalls") = []
{
    auto last = 0.f;

    for (auto seconds = 0.f; seconds < Ending::loopTime; seconds += 0.013f)
    {
        auto rise = Ending::titleRise(seconds);
        check(rise >= last);
        last = rise;
    }
};

auto tPlacementFinite = test("Ending/titlePlacementIsFinite") = []
{
    for (auto seconds: {0.f, 1.f, 3.f, Ending::loopTime})
    {
        auto placement = Ending::titlePlacement(foundGame(seconds));

        for (auto column = 0; column < 4; ++column)
        {
            auto values = placement.column(column);
            check(std::isfinite(values.x) && std::isfinite(values.y)
                  && std::isfinite(values.z) && std::isfinite(values.w));
        }
    }
};

auto tLoops = test("Ending/loopsAfterLoopTime") = []
{
    check(!Ending::loops(foundGame(0.f)));
    check(!Ending::loops(foundGame(Ending::loopTime)));
    check(Ending::loops(foundGame(Ending::loopTime + 0.001f)));

    auto searching = foundGame(Ending::loopTime + 1.f);
    searching.state = Game::State::Searching;
    check(!Ending::loops(searching));
};

auto tFooter = test("Ending/footerText") = []
{
    struct Expected
    {
        Hints hints;
        std::string searching;
        std::string found;
    };

    auto cases = {
        Expected {Hints::Keys,
                  "wasd / hjkl / arrows to walk  -  space to jump  -  m to "
                  "moo  -  drag to look  -  q to quit",
                  "you found her  -  q to quit"},
        Expected {Hints::Touch,
                  "find her  -  drag to look  -  moo for a hint",
                  "you found her"},
        Expected {Hints::Xbox,
                  "left stick to walk  -  A to jump  -  X to moo  -  right "
                  "stick to look",
                  "you found her  -  A for another meadow"},
        Expected {Hints::PlayStation,
                  "left stick to walk  -  cross to jump  -  square to moo  -  "
                  "right stick to look",
                  "you found her  -  cross for another meadow"},
        Expected {Hints::Nintendo,
                  "left stick to walk  -  B to jump  -  Y to moo  -  right "
                  "stick to look",
                  "you found her  -  B for another meadow"},
        Expected {Hints::Gamepad,
                  "left stick to walk  -  A to jump  -  X to moo  -  right "
                  "stick to look",
                  "you found her  -  A for another meadow"},
    };

    for (const auto& expected: cases)
    {
        auto game = Game {};
        game.reset(3);
        game.start();
        check(footerText(game, "hint", expected.hints) == expected.searching);

        game.sinceMoo = mooAnswerDelay + 1.f;
        check(game.hintShowing());
        check(footerText(game, "hint", expected.hints) == "hint");

        game.state = Game::State::Found;
        check(!game.hintShowing());
        check(footerText(game, "hint", expected.hints) == expected.found);
    }
};

auto tGamepadHints = test("Ending/isGamepad") = []
{
    check(!isGamepad(Hints::Keys));
    check(!isGamepad(Hints::Touch));
    check(isGamepad(Hints::Xbox) && isGamepad(Hints::PlayStation)
          && isGamepad(Hints::Nintendo) && isGamepad(Hints::Gamepad));
};
