# Controller support

Game controllers for Cows In Love, macOS first. Following the house rule
("eacp fix first, workaround second"), every gap below is an eacp issue that
names the API eacp should take. No step adds a Cows-side workaround.

## Where things stand

### eacp `GameInput` (#88, `8b511bec`)

`Lib/eacp/Graphics/Input/GameInput.{h,cpp}`, `GameInputQueue.{h,cpp}`,
`GameInputBackend.h`, `GameInput-Apple.mm`, `GameInput-Default.cpp`.

What it has:
- A lock-free queue (`GameInputQueue`) any thread can push into. Each frame
  `snapshot()` gives `isDown`, `wasPressed`, `wasReleased`, reconciled against
  each key's real held state so nothing sticks after an overflow.
- Focus gating: input counts only while the window is key; losing focus calls
  `releaseAll`.
- The Apple backend feeds `GCKeyboard.coalescedKeyboard` and every `GCMouse`
  from one process-wide hub on its own high-priority queue.
- `GameInput-Default.cpp` returns no backend on Windows, Linux and Android;
  those use the window's own key and mouse events.
- 19 queue and window tests in `Tests/Graphics/GameInputTests.cpp`, and
  `Apps/GPU/Maze`, which keeps a `GameInput*` and polls it in `update`.

What it lacks:
- **Controllers of any kind.** No sticks, buttons, triggers, D-pad, connect or
  disconnect, player index, controller family or "last used device".
  `InputEventType` has only `KeyDown`, `KeyUp`, `MouseDown`, `MouseUp`,
  `MouseMove`. The framework is already linked on Apple
  (`-framework GameController`), so the Apple work is all code.
- **Haptics.**
- **Polling.** Backends only push from their own threads. XInput has to be
  polled, so `GameInputBackend` needs a `poll(time)` hook that `snapshot()`
  calls.
- **Focus reporting on Android and iOS.** `Window-Android.cpp` never calls
  `events.input.activationChanged`; once an Android backend exists,
  `snapshot()` sees `active == false` and releases everything every frame.
  `Window-iOS.mm` reports focus once, at launch, never losing it.
- **Android key mapping.** `androidKeyCode` maps only `BACK`, `DEL`, `ENTER`,
  and `AKEYCODE_BACK` becomes `Escape`. An unhandled controller
  `AKEYCODE_BUTTON_B` becomes `BACK`, so on Android a controller's B button
  quits Cows through `RootView`.

### Cows

- `Lib/cowsinlove_Game/Input` adds the held keys (±1 each) to `stickAhead` and
  `stickTurn` from the touch stick, then clamps to [-1, 1]. `jumping` follows
  Space held; `jumpPending` is a one-shot from the touch Jump button.
- `Game::update` uses the size of the input, not its sign: `speed = walkSpeed
  * ahead`, `heading += turn * turnSpeed`. An analog stick needs no change in
  `Game`.
- `UI/TouchControls` reports `ControlEvent`s: `Steer` (0.12 deadzone), `Jump`,
  `Moo`, `Restart` ("Again" at the ending), `Look` (pixel drag × `orbitSpeed`
  0.006), `Zoom`. Added only when `touchScreen` is true.
- `CowsView::control()` is where those land. `Restart` moves to the next stage
  when she has been found, retries otherwise. Keyboard `r` always calls
  `restart()`, even at the ending: an existing difference from touch "Again"
  that the controller should not copy.
- `CowsView::steerCamera` swings the camera back behind the cow at `chaseRate`
  while walking; any look input fights it, including a mouse drag today.
- `footerText(game, hint, touchHints)` chooses between four fixed strings.
  `RootView` quits on q or Esc.
- `Deploy/Steam/README.md` and `controller_config.vdf`: the Deck runs the
  Windows build through Proton; Steam Input copies the keyboard (left stick
  and D-pad as WASD, A and B as Space, X as M, Y as R, right trackpad as a
  mouse, Start as Esc, which quits).
- `Deploy/Microsoft-Store/Metadata/store-listing.md` lists "keyboard and
  mouse".

## 1. Mapping

| Control | Action | Path |
|---|---|---|
| Left stick | walk ahead (y) and turn (x, left positive) | `Input::setPad`, summed |
| D-pad | the same, digital (±1) | added into the pad terms |
| Right stick | orbit the camera | `camera.orbit(x * yawRate * dt, -y * pitchRate * dt)` |
| Right stick click | camera back behind the cow | `camera.turnToward(heading + startYaw, 1)` |
| LT / RT | zoom out / in | `camera.zoom((rt - lt) * zoomRate * dt)` |
| A / Cross (south) | jump, held like Space; **Again** at the ending | `input.padJumping`; `control(Restart)` when Found |
| X / Square (west) | moo | `control(Moo)` (cooldown already in `Game::moo`) |
| B / Circle (east) | moo | the same, so neither layout surprises |
| Y / Triangle (north) | new meadow; at the ending, the next stage | `control(Restart)` |
| Menu / Start, View / Options | nothing | no pause screen; quitting by accident is worse than not quitting |
| Home / PS | left to the system | |

### Deadzone and response curve

Radial, on the stick's overall deflection:
- Below 0.15 reads zero; above 0.95 reads full; in between rescaled linearly
  to 0..1.
- Walk (ahead) stays linear: `Game` treats it as a fraction of full speed.
- Turn goes through `sign(t) * |t|^1.6`: precise small turns, full rate at
  full push.
- Look uses the same exponent, about 2.6 rad/s yaw and 1.5 rad/s pitch at
  full deflection.

The constants live in `Pad.cpp` and get tuned on a real controller. eacp
delivers raw -1..1, y up, as GameController and XInput report; the deadzone
is a game decision.

### Analog steering, not held keys

The stick steers like the touch stick, full analog range, no snapping to ±1.
Movement keeps the tank scheme (ahead/back, turn, camera chasing), matching
the keys, the touch stick and `Game::update(ahead, turn)`. Camera-relative
movement is a later experiment needing `Game` to take a wanted heading.

### Summing

`Input` gains `padAhead`, `padTurn`, `padJumping`, kept separate from the
touch stick so a controller at rest cannot cancel a finger and the reverse.
`walkAhead()` returns `clamp(keys + stickAhead + padAhead, -1, 1)`;
`walkTurn()` the same. `CowsView::update` reads `input.jumping ||
input.padJumping || input.jumpPending`.

### Camera chase vs. right stick

While the right stick is pushed, and for 0.5 s after release (`lookHold`),
`steerCamera` skips the `turnToward` chase. A mouse drag sets the same timer
in `mouseDragged`, a small improvement that comes for free.

## 2. Where the controller feeds in

**Poll `GameInput` in `CowsView::update` and send the buttons through
`CowsView::control()`. No new `ControlEvent` source.**

- A controller is state (stick positions every frame); `ControlEvent`
  describes pointer events. Turning positions into `Steer`/`Look` events each
  frame is state to events and back.
- eacp designed `GameInput` to be polled once per frame in `update()` (Maze),
  with focus gating and release-on-unfocus for free.
- Button edges (`wasPressed`) map one-to-one onto `Jump`, `Moo`, `Restart`
  through `control()`, keeping one meaning per action across touch and
  controller, including Restart at the ending.

### Shape

- **`Lib/cowsinlove_Game/Pad.{h,cpp}` (new).** `struct PadControls { float
  ahead, turn, lookX, lookY, zoom; bool jumping, jumpPressed, mooPressed,
  againPressed, recenterPressed, active; }` and `PadControls readPad(const
  Graphics::GameInputFrame&)`: a pure function, testable. With several
  controllers it takes the stick with the largest deflection and ORs the
  buttons; Cows is single player. `active` means a button or trigger pressed
  or a stick past the deadzone this frame.
- **`CowsApp` owns the `GameInput`**, declared after `window`, constructed as
  `Graphics::GameInput gameInput {window};`, and sets
  `scene.gameInput = &gameInput`, as Maze does.
- **`CowsView` holds `Graphics::GameInput* gameInput = nullptr`.** At the top
  of `update`: snapshot, `readPad`, `input.setPad(...)`, edges into
  `control(...)`, `camera.orbit` / `camera.zoom` with `delta`, and if
  `active`, `lastInput = InputKind::Pad`.

### Footer hints follow the last-used input

- `enum class Hints { Keys, Touch, Xbox, PlayStation, Gamepad }` in `Game.h`
  replaces `footerText`'s `bool touchHints`.
- `CowsView` tracks `lastInput`: `keyDown`, `mouseDown` and touch `control()`
  set Keys or Touch; `readPad(...).active` sets Pad. On change,
  `onStateChanged()` redraws the footer. The family comes from eacp's
  `GamepadState::family`.
- **The glyphs are words**, drawn in the footer's Menlo font through `Hud`,
  so the Rendering Rule holds with no new artwork. Xbox, searching:
  `"left stick to walk  -  A to jump  -  X to moo  -  right stick to look"`;
  found: `"you found her  -  A for another meadow"`. PlayStation says cross,
  square, triangle. Real glyph discs through `Hud::fillDisc` are a later
  option, still on the GPU path.

### iOS touch controls

In `CowsApp`'s `drawHud`, draw the touch controls only when `touchScreen &&
scene.lastInput != InputKind::Pad`. The `TouchControls` view stays in the
tree, so a touch flips `lastInput` back and the controls return. Hiding on
last use, not on connection, so an idle controller in a bag hides nothing.

## 3. Platforms and eacp issues to file

### eacp issue A: gamepads in `GameInput`, a `GCController` feed on Apple

Blocks macOS; gives iOS the same feed. Proposed API in `GameInputQueue.h`:

```cpp
enum class GamepadButton : uint8_t
{
    South, East, West, North, LeftShoulder, RightShoulder,
    LeftStick, RightStick, Start, Back, Home,
    DpadUp, DpadDown, DpadLeft, DpadRight, Count
};

enum class GamepadAxis : uint8_t
{
    LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger, Count
};

enum class GamepadFamily : uint8_t { Generic, Xbox, PlayStation, Nintendo };

struct GamepadState
{
    int id = 0;            // stable while connected
    int playerIndex = -1;  // what the backend assigned / lit
    GamepadFamily family = GamepadFamily::Generic;
    bool isDown(GamepadButton) const;
    bool wasPressed(GamepadButton) const;
    bool wasReleased(GamepadButton) const;
    float axis(GamepadAxis) const;  // sticks -1..1, y up; triggers 0..1; raw
    Point leftStick() const;
    Point rightStick() const;
};

// GameInputFrame
const Vector<GamepadState>& gamepads() const;  // connected ones
bool gamepadsChanged() const;                   // a connect or disconnect this frame

// GameInputQueue producers, any thread
void gamepadConnected(int id, GamepadFamily, double time);
void gamepadDisconnected(int id, double time);  // releases buttons, zeroes axes
void gamepadButtonChanged(int id, GamepadButton, bool down, double time);
void gamepadAxisChanged(int id, GamepadAxis, float value, double time);
```

Queue rules: axes are state, not edges (latest value per slot, a fixed array
of `std::atomic<float>`, copied into the frame at snapshot); buttons follow
the key rules; `releaseAll` also zeroes the axes so losing focus stops the
cow.

Apple: observe `GCControllerDidConnectNotification` / `DidDisconnect` in the
hub; `controller.handlerQueue` = the hub's queue;
`extendedGamepad.valueChangedHandler`; `buttonA/B/X/Y` → South/East/West/
North, `buttonMenu` → Start, `buttonOptions` → Back, `buttonHome` → Home;
family from `productCategory` or the `GCXboxGamepad` /
`GCDualSenseGamepad` / `GCDualShockGamepad` subclasses; set `playerIndex` on
connect (lights the LED); leave `shouldMonitorBackgroundEvents` at `NO` so
only the frontmost app gets input, matching key-window gating.

Tests: connect, button edges, axis is the latest value, disconnect
releases, `releaseAll` zeroes, overflow keeps state. Maze gets right-stick
look and left-stick move. A sentence in eacp's CLAUDE.md.

### eacp issue B: haptics

```cpp
// Main thread. strength 0..1; replaces whatever is playing on that pad.
void GameInput::rumble(int gamepadId, float strength, double seconds);
bool GameInput::canRumble(int gamepadId) const;
```

Apple: `GCController.haptics`, `createEngineWithLocality:
GCHapticsLocalityDefault`, a lazy `CHHapticEngine` per controller, a
continuous event; false where there is no `haptics`. Windows:
`XInputSetState` with the stop time checked in `poll`. Android:
`InputDevice.getVibratorManager()` (API 31+) over JNI. Linux: `EVIOCSFF`
with `FF_RUMBLE`.

### eacp issue C: a Windows backend (`GameInput-Windows.cpp`)

**XInput** (`xinput1_4.dll`, Windows 8+, `xinput.lib`): simplest, no
redistributable, works in MSIX, and under Steam and Proton every controller,
the Deck included, appears as an XInput pad, so one backend covers the Steam
build on Windows and on the Deck. `GameInputBackend` grows `virtual void
poll(double time) {}`, called from `GameInput::snapshot()` before the queue
drains; the backend polls `XInputGetState` for slots 0..3, compares
`dwPacketNumber`, pushes changes, reports a missing slot as a disconnect.
Family is always Xbox. Not chosen: Windows.Gaming.Input (WinRT, fiddly focus
in plain Win32) and Microsoft's GameInput (needs a redistributable; the
route for native DualSense later). Without Steam, a PlayStation controller
will not appear through XInput; the Store listing then says "Xbox
controller".

### eacp issue D: an Android backend

`Window-Android.cpp` already receives `AInputEvent`s. Buttons: for sources
with `AINPUT_SOURCE_GAMEPAD` or `JOYSTICK`, map `AKEYCODE_BUTTON_A/B/X/Y/L1/
R1/THUMBL/THUMBR/START/SELECT/MODE` and `AKEYCODE_DPAD_*` to
`gamepadButtonChanged` with `AInputEvent_getDeviceId` as the id, and
**consume them so B never becomes Back**. Axes: `AMotionEvent_getAxisValue`
for `AXIS_X`, `Y`, `Z`, `RZ`, `LTRIGGER`/`BRAKE`, `RTRIGGER`/`GAS`, `HAT_X`,
`HAT_Y` (hats become D-pad buttons). Hot-plug: `InputManager.
InputDeviceListener` over JNI, or connect on first event. Not chosen: AGDK
Paddleboat (a new dependency). Also needs `activationChanged` from
`APP_CMD_GAINED_FOCUS` / `LOST_FOCUS`, else the `active` flag kills any
backend.

### eacp issue E: a Linux backend (evdev)

Only for a native Linux or Deck build ("not attempted for 1.0"). Scan
`/dev/input/event*` for `BTN_GAMEPAD`, inotify for hot-plug, no SDL or
libudev; `BTN_SOUTH/EAST/WEST/NORTH/TL/TR/THUMBL/THUMBR/START/SELECT/MODE`,
`ABS_X/Y/RX/RY/Z/RZ/HAT0X/Y`, normalised with `EVIOCGABS`. On a Deck, Steam
Input exposes "Steam Virtual Gamepad", an Xbox 360 layout through uinput.

### eacp issue F: iOS reporting losing focus

`Window-iOS.mm` should send `activationChanged(false/true)` from the scene's
resign-active / become-active notifications, else a held stick is not
released when the app goes to the background.

### Steam Deck until issue C lands

The shipped keyboard emulation covers walk, jump, moo, new meadow. Gaps:
right-stick look is not possible (the right trackpad drag stands in; walking
turns the camera anyway), and Start sends Esc, which quits instantly: change
it to nothing now, a one-line `vdf` edit. Once issue C ships, replace the
keyboard config with a gamepad one (Steam's "Gamepad" template or a new
`controller_config.vdf`) and mark full controller support in Steamworks;
the two configs must not ship together, or Steam's virtual controller emits
only the mapped keys and the native path never sees the controller.

## 4. Feel

- **Haptics (after issue B)**, in `CowsView`: 0.35 for 0.12 s at each kiss
  during the ending (track `Choreography::latestKiss(sinceFound)`); on
  landing in proportion to the downward speed when `grounded` goes false →
  true (`Game` exposes `landingSpeed`, with a threshold so ordinary hops stay
  silent); 0.5 for 0.2 s on respawn after a fall; optionally a faint heartbeat
  on `beatClock` when `warmth() > 0.8`. Only the last-active controller, only
  when `lastInput == Pad`.
- **Hot-plug mid-game.** eacp's disconnect releases buttons and zeroes axes;
  `readPad` sees zero and the cow stops; `lastInput` drops to Keys or Touch,
  the footer flips back, the touch controls reappear on iOS. Connecting
  changes nothing until the controller is used.
- **Window not key.** eacp's `releaseAll` on losing focus, extended to the
  axes, makes `readPad` neutral; GameController delivers nothing to a
  background app. Cows adds nothing.

## 5. Tests

- **`Tests/Game/PadTests.cpp` (new):** drive a `GameInputQueue` with
  synthetic controller state, `snapshot(t)`, check `readPad`:
  `deadZoneReadsZero`, `fullStickWalksFull`, `turnCurveIsGentleNearCenter`,
  `dpadIsDigital`, `southIsHeldJumpAndAnEdge`, `westAndEastMoo`,
  `northRestarts`, `largestStickOfTwoPadsWins`, `disconnectStopsWalking`,
  `releaseAllStopsWalking`.
- **`Tests/Game/InputTests.cpp`:** `padAddsToKeysAndStickThenClamps`,
  `padDoesNotClearTouchStick`, `padJumpingHolds`.
- **`Tests/Game/EndingTests.cpp`:** `footerText` for each `Hints` value,
  searching and found.
- **`Tests/Engine/HudSnapshotTests.cpp`:** `footerWithPadHints`, the Xbox
  searching string at 1280×800 saved as `docs/shots/tests/engine-hud-pad.png`,
  wrapping within 640×400; skipped without a GPU.
- Not unit-tested: `CowsView`'s routing (needs a window and a GPU); verified
  on hardware below.

## 6. Steps

eacp steps are their own PRs; Cows builds against them with
`-DCPM_eacp_SOURCE=$HOME/projects/eacp-develop` until they merge.

| # | Unit | Files | Hours |
|---|---|---|---|
| E1 | eacp issue A: controller state in the queue and frame, the `GCController` feed, tests, Maze | eacp `Graphics/Input/GameInputQueue.{h,cpp}`, `GameInput.{h,cpp}`, `GameInput-Apple.mm`, `Tests/Graphics/GameInputTests.cpp`, `Apps/GPU/Maze/Main.cpp`, `CLAUDE.md` | 10–14 |
| E2 | eacp issue B: rumble on Apple | `GameInput.{h,cpp}`, `GameInputBackend.h`, `GameInput-Apple.mm` (+ CoreHaptics) | 3–4 |
| E3 | eacp issue C: XInput backend and `poll` hook | `GameInputBackend.h`, `GameInput.cpp`, new `GameInput-Windows.cpp`, `Graphics/CMakeLists.txt` | 5–7 |
| E4 | eacp issues D and F: Android backend and focus; iOS focus | `Window-Android.cpp`, new `GameInput-Android.cpp`, `Window-iOS.mm`, CMake | 8–11 |
| E5 | eacp issue E: Linux evdev backend (later) | new `GameInput-Linux.cpp`, CMake | 6–8 |
| C0 | Steam config: Start does nothing (can land now) | `Deploy/Steam/controller_config.vdf`, `Deploy/Steam/README.md` | 0.5 |
| C1 | `Pad` and controller terms in `Input`, with tests (after E1) | new `Lib/cowsinlove_Game/Pad.{h,cpp}`, `Input.{h,cpp}`, `Lib/cowsinlove_Game/CMakeLists.txt`, new `Tests/Game/PadTests.cpp`, `Tests/Game/InputTests.cpp`, `Tests/Game/CMakeLists.txt` | 3 |
| C2 | Wire `GameInput`: poll, `control()` routing, right-stick orbit, trigger zoom, recentre, chase hold | `Apps/CowsInLove/Source/CowsApp.{h,cpp}`, `Scene/CowsView.{h,cpp}` | 3 |
| C3 | Last-used input and footer hints | `Lib/cowsinlove_Game/Game.{h,cpp}`, `CowsView.{h,cpp}`, `CowsApp.cpp`, `Tests/Game/EndingTests.cpp`, `Tests/Engine/HudSnapshotTests.cpp` | 2–3 |
| C4 | iOS: hide touch controls when the controller was last used; controller plist keys | `CowsApp.cpp`, `Apps/CowsInLove/CMakeLists.txt` (`GCSupportedGameControllers` = ExtendedGamepad, `GCSupportsControllerUserInteraction`) | 1–2 |
| C5 | Haptics: kiss, landing, respawn (after E2) | `Game.{h,cpp}` (`landingSpeed`), `CowsView.{h,cpp}`, `Tests/Game/GameTests.cpp` | 2 |
| C6 | Steam and Store (after E3): gamepad Steam config, store copy | `Deploy/Steam/controller_config.vdf`, `Deploy/Steam/README.md`, `Deploy/Microsoft-Store/Metadata/store-listing.md` | 1.5 |
| C7 | Docs | `CLAUDE.md`, `docs/structure.md` | 0.5 |

macOS play needs E1 plus C1 to C3, about 18 to 22 hours. The whole list is
about 31 to 44 hours of eacp work and about 13 hours in Cows.

### Verify on this Mac (an Xbox and a PlayStation controller over Bluetooth)

After C2 and C3:
1. Pair, launch, touch the stick: the footer switches to the Xbox (or
   PlayStation) words. Press a key: it switches back.
2. Left stick: a slight push creeps, a full push walks at keyboard speed, a
   small push turns finely, back-pedalling works. Stick and keys together
   never exceed the keys alone.
3. Right stick orbits, keeps orbiting while walking, the chase returns about
   0.5 s after release. Up looks up. Clicking recentres. Triggers zoom.
4. A jumps; holding A hops as holding Space does. X and B moo; the cooldown
   holds when mashed. Y gives a new meadow. At the ending, A and Y both go to
   the next stage.
5. Turn the controller off mid-walk: the cow stops, the footer returns to
   keys. On again: nothing changes until it is used.
6. Hold the stick and Cmd-Tab away: the cow stops; buttons in another app do
   nothing in Cows. Cmd-Tab back: input resumes with no phantom jump.
7. Home or PS: note what macOS does; Cows must not react.
8. After C5: rumble on the kiss and a hard landing, nothing on small hops, on
   both Xbox and DualSense.
9. `just test`; the new snapshot at `docs/shots/tests/engine-hud-pad.png`.

### Verify in the Steam build (after E3 and C6)

1. Windows with an Xbox controller, Steam Input off and with the gamepad
   config: inputs arrive once, not twice (a leftover keyboard config shows as
   digital walking and key hints).
2. Deck through Proton with the official gamepad config: steps 2 to 6 above;
   suspend and resume must release the stick.
3. Before C6, on the shipped keyboard config: Start no longer quits (after C0).
