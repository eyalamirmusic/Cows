#include "Game.h"

#include <NanoTest/NanoTest.h>

#include <cmath>

using namespace nano;
using namespace Cows;
using namespace Maths;

namespace
{
constexpr auto walkSpeed = 7.f;
constexpr auto turnSpeed = 2.4f;
constexpr auto frame = 0.01f;

bool near(float a, float b, float margin = 1e-3f)
{
    return std::abs(a - b) < margin;
}

Game openField()
{
    auto game = Game {};
    game.reset(3);
    game.start();
    game.level = Level {};
    game.partner = {60.f, 0.f, 60.f};
    return game;
}
} // namespace

auto tWalk = test("Game/walkingMovesAlongTheHeading") = []
{
    auto game = openField();
    game.update(frame, 1.f, 0.f, false);

    check(near(game.player.x, walkSpeed * frame));
    check(near(game.player.z, 0.f));

    game.playerHeading = halfPi;
    auto before = game.player;
    game.update(frame, 1.f, 0.f, false);

    check(near(game.player.x, before.x));
    check(near(game.player.z, before.z - walkSpeed * frame));
};

auto tTurn = test("Game/turningChangesTheHeading") = []
{
    auto game = openField();
    game.update(frame, 0.f, 1.f, false);
    check(near(game.playerHeading, turnSpeed * frame, 1e-5f));

    game.update(frame, 0.f, -1.f, false);
    check(near(game.playerHeading, 0.f, 1e-5f));
};

auto tJump = test("Game/jumpRisesThenLands") = []
{
    auto game = openField();
    game.update(frame, 0.f, 0.f, true);

    check(!game.grounded);
    check(game.player.y > 0.f);

    auto peak = game.player.y;

    for (auto count = 0; count < 300 && !game.grounded; ++count)
    {
        game.update(frame, 0.f, 0.f, false);
        peak = std::max(peak, game.player.y);
    }

    check(game.grounded);
    check(peak > 1.f);
    check(near(game.player.y, 0.f));
};

auto tFound = test("Game/foundBesideHer") = []
{
    auto game = openField();
    game.partner = {0.f, 0.f, -4.f};
    game.update(frame, 0.f, 0.f, false);

    check(game.state == Game::State::Found);
    check(near(game.stageHeading, halfPi));
    check(near(game.stageCenter.z, -4.f));
    check(game.sinceFound == 0.f);
};

auto tNotFoundFar = test("Game/searchingWhenTooFar") = []
{
    auto game = openField();
    game.partner = {4.6f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    check(game.state == Game::State::Searching);
};

auto tNotFoundAbove = test("Game/searchingWhenSheIsTooHigh") = []
{
    auto game = openField();
    game.partner = {3.f, 1.6f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    check(game.state == Game::State::Searching);
};

auto tWarmth = test("Game/warmthRisesAsSheNears") = []
{
    auto game = openField();
    game.partner = {85.f, 0.f, 0.f};
    check(game.warmth() == 0.f);

    game.partner = {1.f, 0.f, 0.f};
    check(game.warmth() == 1.f);

    auto last = 2.f;

    for (auto x = 0.f; x < 100.f; x += 0.5f)
    {
        game.partner = {x, 0.f, 0.f};
        check(game.warmth() <= last);
        last = game.warmth();
    }
};

auto tMooCooldown = test("Game/mooWaitsForTheCooldown") = []
{
    auto game = openField();
    game.moo();
    check(game.sinceMoo == 0.f);

    game.update(1.f, 0.f, 0.f, false);
    game.moo();
    check(near(game.sinceMoo, 1.f));

    game.update(2.3f, 0.f, 0.f, false);
    game.moo();
    check(game.sinceMoo == 0.f);
};

auto tReset = test("Game/resetStartsANewSearch") = []
{
    auto game = openField();
    game.partner = {3.f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    game.update(1.f, 0.f, 0.f, false);
    check(game.state == Game::State::Found);
    check(game.sinceFound > 0.f);

    game.reset(5);
    check(game.state == Game::State::Searching);
    check(game.sinceFound == 0.f);
    check(game.seed == 5u);
};

auto tEndingClock = test("Game/endingSecondsFollowSinceFound") = []
{
    auto game = openField();
    game.partner = {3.f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);

    auto start = game.endingSeconds();
    game.update(1.25f, 0.f, 0.f, false);

    check(near(game.sinceFound, 1.25f));
    check(near(game.endingSeconds() - start, 1.25f));
};

auto tMenuFirst = test("Game/startsInTheMenuAndStartMovesOn") = []
{
    auto game = Game {};
    game.reset(3);
    check(game.state == Game::State::Menu);
    check(game.playing() == Game::State::Searching);

    auto standing = game.player;
    game.update(1.f, 1.f, 0.f, true);
    check(game.player.x == standing.x && game.player.z == standing.z);
    check(game.seconds == 0.f);

    game.start();
    check(game.state == Game::State::Searching);
    check(footerText(game, "", Hints::Keys) != "enter to start  -  q to quit");
};

auto tMenuPauses = test("Game/theMenuPausesAndReturnsToTheSameState") = []
{
    auto game = openField();
    game.update(frame, 1.f, 0.f, false);
    auto walked = game.player;

    game.openMenu();
    check(game.state == Game::State::Menu);
    check(footerText(game, "", Hints::Keys) == "enter to start  -  q to quit");
    check(footerText(game, "", Hints::Xbox) == "A to start");
    check(footerText(game, "", Hints::Touch).empty());

    game.update(1.f, 1.f, 0.f, false);
    check(near(game.player.x, walked.x));

    game.start();
    check(game.state == Game::State::Searching);

    game.partner = {game.player.x + 3.f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    check(game.state == Game::State::Found);

    game.openMenu();
    check(game.playing() == Game::State::Found);
    game.start();
    check(game.state == Game::State::Found);
};

auto tAgainSkipsMenu = test("Game/anotherMeadowAfterTheEndingSkipsTheMenu") = []
{
    auto game = openField();
    game.partner = {3.f, 0.f, 0.f};
    game.update(frame, 0.f, 0.f, false);
    check(game.state == Game::State::Found);

    game.reset(4);
    check(game.state == Game::State::Searching);

    game.openMenu();
    game.reset(5);
    check(game.state == Game::State::Menu);
    check(game.playing() == Game::State::Searching);
};

auto tMenuAnimates = test("Game/theMenuPausesTheGameButNotTheHeartbeat") = []
{
    auto game = openField();
    game.update(0.5f, 1.f, 0.f, false);
    check(game.bounce > 0.5f);

    game.openMenu();
    auto player = game.player;
    auto seconds = game.seconds;
    auto beat = game.beatClock;
    auto hop = game.hopClock;

    for (auto step = 0; step < 100; ++step)
        game.update(frame, 1.f, 1.f, true);

    check(game.player.x == player.x && game.player.z == player.z);
    check(game.seconds == seconds);
    check(game.beatClock > beat);
    check(near(game.hopClock - hop, 1.f));
    check(game.bounce < 0.01f);
};

auto tEscapeInGameOpensMenu = test("Game/escapeInGameOpensMenu") = []
{
    check(escapeAction(Game::State::Searching, false, false)
          == EscapeAction::OpenMenu);
    check(escapeAction(Game::State::Found, false, false) == EscapeAction::OpenMenu);
};

auto tEscapeInEditorClosesIt = test("Game/escapeInEditorGoesBackToMenu") = []
{
    check(escapeAction(Game::State::Menu, true, false) == EscapeAction::CloseEditor);
};

auto tEscapeOnMenuPassesOn = test("Game/escapeOnMenuPassesOn") = []
{ check(escapeAction(Game::State::Menu, false, false) == EscapeAction::PassOn); };

auto tEscapeMidSwingIsSwallowed = test("Game/escapeMidSwingIsSwallowed") = []
{
    for (auto state: {Game::State::Menu, Game::State::Searching})
        for (auto dressing: {false, true})
            check(escapeAction(state, dressing, true) == EscapeAction::Swallow);
};
