#include "CowsView.h"
#include "CowsViewTuning.h"
#include "Cow/Wardrobe.h"
#include "Ending.h"
#include "Props/Meshes.h"
#include "Render/FrameProfile.h"

#include <algorithm>
#include <array>
#include <utility>

using namespace Maths;

namespace Cows
{
using namespace CowsViewTuning;

namespace
{
constexpr auto fullScreenTriangle = std::to_array<CornerVertex>({
    {{-1.f, -1.f}},
    {{3.f, -1.f}},
    {{-1.f, 3.f}},
});

constexpr auto glowQuad = std::to_array<CornerVertex>({
    {{-1.f, -1.f}},
    {{1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, -1.f}},
    {{1.f, 1.f}},
    {{-1.f, 1.f}},
});

Vector<Cow> makeCouple()
{
    auto cows = Vector<Cow> {};
    cows.add(Cow {-1.f, {0.f, 0.f, 0.f}});
    cows.add(Cow {1.f, {7.3f, 2.1f, 4.6f}});
    return cows;
}

Vec2 flat(Vec3 vector)
{
    auto ground = Vec2 {vector.x, vector.z};
    auto size = length(ground);
    return size > 0.f ? ground / size : Vec2 {};
}

std::string distanceWord(float distance)
{
    if (distance < 20.f)
        return "close by";

    if (distance < 50.f)
        return "not far";

    return "far off";
}

std::string directionWord(float ahead, float across)
{
    if (ahead > 0.7f)
        return "ahead of you";

    if (ahead < -0.7f)
        return "behind you";

    return across > 0.f ? "to your right" : "to your left";
}

std::optional<float> startAfterSetting()
{
    if (auto after = getEnv("COWS_START"))
        return std::stof(*after);

    return std::nullopt;
}

float startTime()
{
    if (auto start = getEnv("COWS_TIME"))
        return std::stof(*start);

    return 0.f;
}

std::string_view getInputRecipientName(InputRecipient recipient)
{
    switch (recipient)
    {
        case InputRecipient::Menu:
            return "menu";
        case InputRecipient::Editor:
            return "editor";
        case InputRecipient::Play:
            return "play";
    }

    return "unknown";
}
} // namespace

CowsView::CowsView(const QualityPreference& preference,
                   Stages stagesToPlay,
                   std::uint32_t firstSeed)
    : stages(std::move(stagesToPlay))
    , qualityPreference(preference)
    , shapes(makeContentMesh)
    , ground(makePlane(groundSize))
    , cowParts(makeCowParts())
    , playerParts(cowParts)
    , cows(makeCouple())
    , elapsed(startTime())
    , frozen(!getEnvValue("COWS_FREEZE").empty())
{
    startAfter = startAfterSetting();

    setDepth(true);
    skyShader.setVertices(fullScreenTriangle);
    glowShader.setVertices(glowQuad);
    setTitle(Ending::titleText);
    menuTitleWide = MenuTitle::makeWide();
    menuTitleTall = MenuTitle::makeTall();

    forcedQuality = readQualityFromEnvironment();
    forcedFrameRate = readFrameRateFromEnvironment();
    applyQualityChoice();

    game.makeLevel = stages.level();
    game.reset(firstSeed);
    game.start();
    layTerrain();

    camera.yaw = startYaw + game.playerHeading;
    camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};

    if (!getEnvValue("COWS_FOUND").empty())
        game.player = Ending::foundStart(game);

    input.walkingForward = getEnvValue("COWS_WALK") == "1";

    setHandlesMouseEvents(true);
    setContinuous(true);
}

void CowsView::update(Threads::FrameTime time)
{
    auto& profile = FrameProfile::shared();
    profile.frameStarted();
    auto timed = FrameProfile::Scope {profile, FrameProfile::Part::Update};
    measureQuality();

    auto delta = frozen ? 0.f : (float) time.delta;
    auto wallDelta = swingClock.step(time);
    advanceClocks(delta, wallDelta);
    readGameInput(delta);
    pressStartWhenDue((float) time.delta);
    stepGame(delta);
    steerCamera(delta, wallDelta);
    applyFrameRateCap();

    if (profile.enabled)
        logInputRecipientWhenItChanges();
}

void CowsView::advanceClocks(float delta, float wallDelta)
{
    elapsed += delta;
    lookHold = std::max(0.f, lookHold - delta);
    auto dressDirection = dressing ? 1.f : -1.f;
    dressAmount =
        std::clamp(dressAmount + dressDirection * wallDelta / dressLength, 0.f, 1.f);
}

void CowsView::pressStartWhenDue(float wallDelta)
{
    if (startAfter.has_value() && (*startAfter -= wallDelta) <= 0.f)
    {
        startAfter.reset();
        startGame();
    }
}

bool CowsView::isJumpHeld() const
{
    return input.jumping || input.padJumping || input.jumpPending;
}

void CowsView::stepGame(float delta)
{
    auto wasSearching = game.state == Game::State::Searching;
    auto wasGrounded = game.grounded;
    auto fellBefore = game.sinceFell;
    game.update(delta, input.walkAhead(), input.walkTurn(), isJumpHeld());
    if (wasGrounded)
        input.jumpPending = false;

    if (wasSearching && game.state == Game::State::Found)
        onStateChanged();

    if (game.sinceFell < fellBefore)
        camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};

    if (Ending::hasEndingPlayedOut(game))
        advanceStage();

    notifyWhenHintOrFallChanges();
}

void CowsView::notifyWhenHintOrFallChanges()
{
    auto hintChanged = game.isPartnersAnswerShowing() != showedHint;
    auto fallChanged = game.justFell() != showedFall;

    if (!hintChanged && !fallChanged)
        return;

    showedHint = game.isPartnersAnswerShowing();
    showedFall = game.justFell();
    onStateChanged();
}

void CowsView::logInputRecipientWhenItChanges()
{
    auto recipient = getInputRecipient();

    if (recipient == lastLoggedInputRecipient)
        return;

    lastLoggedInputRecipient = recipient;
    LOG("Cows: input to ", getInputRecipientName(recipient));
}

void CowsView::applyFrameRateCap()
{
    auto menusHaveInput = getInputRecipient() != InputRecipient::Play;
    auto cameraIsStill = !swinging;
    auto rateToUse = forcedFrameRate.value_or(chosenFrameRate);
    auto cap = getFrameRateCap(rateToUse, menusHaveInput && cameraIsStill);

    if (maxFps() != cap)
        setMaxFps(cap);
}

void CowsView::wear(const CowSkin& skin)
{
    playerParts = makeCowParts(skin);
}

void CowsView::useHints(Hints used)
{
    if (used == hints)
        return;

    hints = used;
    onStateChanged();
}

void CowsView::callOut()
{
    auto before = game.sinceMoo;
    game.moo();

    if (game.sinceMoo == before)
        return;

    auto toHer = flat(game.partner - camera.eye());
    auto ahead = dot(toHer, flat(camera.forward()));
    auto across = dot(toHer, flat(camera.right()));

    hint = "she moos back, " + distanceWord(game.distance()) + ", "
           + directionWord(ahead, across);
    mooVoice.call(answerFrom());
}

MooAnswer CowsView::answerFrom() const
{
    auto toHer = flat(game.partner - camera.eye());

    auto answer = MooAnswer {};
    answer.pan = dot(toHer, flat(camera.right()));
    answer.muffle = std::clamp(-dot(toHer, flat(camera.forward())), 0.f, 1.f);
    answer.volume = std::clamp(1.f - game.distance() / loudReach, quietest, 1.f);
    return answer;
}

void CowsView::restart()
{
    game.reset(clockSeed());
    layTerrain();
    camera.yaw = startYaw + game.playerHeading;
    camera.target = game.player + Vec3 {0.f, searchHeight, 0.f};
    onStateChanged();
}

void CowsView::setTitle(std::string_view text)
{
    title = makeTitle(text);
    titleShader.setVertices(title.vertices.data(), title.vertices.size());
    titleShader.setIndices(title.indices.data(), title.indices.size());
}

void CowsView::advanceStage()
{
    stages.advance();
    game.makeLevel = stages.level();
    restart();
}

void CowsView::layTerrain()
{
    ground = Mesh {makeGround(game.level, groundSize)};
    chasms = makeChasms(game.level);
    grass.layOver(game.level);

    if (uploadedBlades != &grass.tile)
        uploadedBlades = nullptr;
}
} // namespace Cows
