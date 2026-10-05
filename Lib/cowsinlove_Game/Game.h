#pragma once

#include "Level.h"

#include <cstdint>
#include <string>

namespace Cows
{
// Find the other cow: the player walks, jumps and climbs the meadow until the
// two meet up where she hides, and the postcard's kiss plays as the ending.
struct Game final
{
    enum class State
    {
        Menu,
        Searching,
        Found
    };

    // Starts a new search on `makeLevel(newSeed)`, behind the menu when it is
    // showing.
    void reset(std::uint32_t newSeed);

    // Leaves the menu for the game behind it.
    void start();

    // Shows the menu over the game, which waits where it is until start().
    void openMenu();

    // The state the game is in, or waits in behind the menu.
    State playing() const;

    // `ahead` walks forward (1) or back (-1) along the cow's heading; `turn`
    // turns it left (1) or right (-1); `jump` leaps if it is on its feet.
    // Falling below the level's killDepth puts the cow back on its checkpoint:
    // the last firm ground it stood on. In the menu the game waits, but the
    // heartbeat in the eyes runs on and a hop in the air lands.
    void update(float delta, float ahead, float turn, bool jump);

    // Moos, if she has not just mooed.
    void moo();

    float distance() const;

    // 0 far away from her, 1 right beside her.
    float warmth() const;

    // The ending's clock, on the postcard's timeline.
    float endingSeconds() const;

    // Where the ending plays: the pair's midpoint, turned so the player's cow
    // faces +x.
    Maths::Mat4 stage() const;

    // Her answer to the player's moo is on screen.
    bool hintShowing() const;

    // The player fell a moment ago and is back on the checkpoint.
    bool justFell() const;

    LevelMaker makeLevel = [](std::uint32_t) { return Level {}; };

    State state = State::Menu;
    State behindMenu = State::Searching;
    std::uint32_t seed = 0;
    Level level;

    Maths::Vec3 player;
    float playerHeading = 0.f;
    float verticalSpeed = 0.f;
    bool grounded = true;
    float bounce = 0.f;
    float hopClock = 0.f;
    float beatClock = 0.f;

    Maths::Vec3 partner;
    float partnerHeading = 0.f;

    Maths::Vec3 stageCenter;
    float stageHeading = 0.f;
    float sinceFound = 0.f;
    float sinceMoo = 100.f;

    Maths::Vec3 checkpoint;
    float checkpointHeading = 0.f;
    float sinceFell = 100.f;
    float seconds = 0.f;

private:
    void animate(float delta, bool walking);
};

// What Escape, or Android's Back, does: nothing mid-swing, back to the menu
// from the editor, the menu from play, and on the menu itself nothing of the
// game's, so the key goes on to quit or to leave the app.
enum class EscapeAction
{
    Swallow,
    CloseEditor,
    OpenMenu,
    PassOn
};

EscapeAction escapeAction(Game::State state, bool dressing, bool swinging);

// Whose controls the footer names: the keys, the touch controls, or a
// controller by the labels it wears.
enum class Hints
{
    Keys,
    Touch,
    Xbox,
    PlayStation,
    Nintendo,
    Gamepad
};

constexpr bool isGamepad(Hints hints)
{
    return hints != Hints::Keys && hints != Hints::Touch;
}

// The footer: how to start in the menu, the controls while searching, `hint` while her answer shows, a
// word after a fall.
std::string footerText(const Game& game, const std::string& hint, Hints hints);

// The footer while the cow is being dressed.
std::string editorText(Hints hints);
} // namespace Cows
