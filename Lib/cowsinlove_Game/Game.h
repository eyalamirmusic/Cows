#pragma once

#include "Cow/CowPhysics.h"
#include "Level.h"

#include <cstdint>
#include <string>

namespace Cows
{
struct Game final
{
    enum class State
    {
        Menu,
        Searching,
        Found
    };

    explicit Game(const JumpPhysics& jumpPhysicsToUse);

    void reset(std::uint32_t newSeed);

    void start();

    void openMenu();

    State getStateBehindMenu() const;

    // ahead: 1 forward, -1 back; turn: 1 left, -1 right.
    void update(float delta, float ahead, float turn, bool jump);

    void moo();

    float distance() const;

    // 0 far from her, 1 right beside her.
    float warmth() const;

    float endingSeconds() const;

    void setJumpPhysics(const JumpPhysics& physics);

    // Turned so the player's cow faces +x.
    Maths::Mat4 stage() const;

    bool isPartnersAnswerShowing() const;

    bool justFell() const;

    LevelMaker makeLevel = [](std::uint32_t) { return Level {}; };

    State state = State::Menu;
    State behindMenu = State::Searching;
    std::uint32_t seed = 0;
    Level level;

    Maths::Vec3 player;
    float playerHeading = 0.f;
    float verticalSpeed = 0.f;
    JumpPhysics jumpPhysics;
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

// Not Game::State: that stays Menu while Start swings the camera down into play.
enum class InputRecipient
{
    Menu,
    Editor,
    Play
};

InputRecipient
    getInputRecipient(Game::State state, bool dressing, bool startingPlay);

enum class EscapeAction
{
    Swallow,
    CloseEditor,
    OpenMenu,
    PassOn
};

EscapeAction escapeAction(InputRecipient recipient, bool swinging);

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

std::string footerText(const Game& game, const std::string& hint, Hints hints);

std::string editorText(Hints hints);
} // namespace Cows
