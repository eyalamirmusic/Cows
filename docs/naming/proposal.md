# Function names: proposal

Every function this repo declares in `Lib/`, `Apps/`, `Tests/` and `tools/`
(not `_deps`), listed through libclang by `tools/rename-functions.py
inventory build`, is in `functions.csv` with a decision, a new name and a
reason. This page is that file grouped by library. Phase 2 runs
`tools/rename-functions.py rename build docs/naming/functions.csv --write`.

## Counts

| | rows |
|---|---|
| functions, methods, constructors, operators, callback members, named lambdas | 979 |
| keep | 449 |
| rename | 530 (486 distinct old → new pairs; overloads and same-named test helpers share one) |

Renames by kind: 266 file-local functions, 117 methods, 102 free functions,
32 named lambdas, 6 callback members, 7 static or template.

Dry run (`rename build docs/naming/functions.csv`): 2904 code edits and 106
comment and doc edits, 3010 in 173 files; every rename has at least one code
edit; no cursor points at text that does not spell the old name. A trial
`--write` on this branch, reverted afterwards, built warning-free and passed
all 309 tests; clang-format then had 549 lines in 81 files to rewrap, which
`--write` now does itself.

## What was kept, and why

- **Named by their type** (40): constructors, destructors, conversion
  operators. **Operators** (8). **`main`** (2).
- **eacp's names** (52): overrides of `GPUView::update` / `render`,
  `View::mouse*`, `key*`, `touch*`, `resized`, `safeAreaInsetsChanged`,
  `ShaderProgram::define` / `reflectMembers`. Not ours to rename.
- **Miro** (5): `reflect` (found by name) and `toJSON` (pairs with Miro's
  `fromJSON`).
- **Our own virtual** `Segment::build` and its 2 overrides: already a verb.
- **Callbacks that say what happened** (7): `onStateChanged`,
  `onQualityChanged`, `onControl` (twice), `onKeyDown`, `onKeyUp`, `onEscape`.
  Function members that are hooks rather than events keep their verb:
  `drawHud`, `drawOpaque`, `drawOverlay`, `makeLevel`, `setUp`,
  `LayoutShape::build`.
- **Pure data types**: `Region` (`arena`, `holds`, `inset`, `center`,
  `size`, `area`, `contains`, `isEmpty`) and `Flex::Size` (`fit`, `grow`,
  `fixed`, `share`, named constructors), eacp style, like `Rect`.
- **Standard verbs** that read aloud: `easeOut`, `easeInOut`, `smoothstep`,
  `mix`, `absolute`, `placeTargetOnScreen`, `FrameProfile::shared` (eacp's
  singleton spelling, as `Device::shared`), `std`-style `size`/`begin`/`end`.
- **Already a clear verb phrase** (312): `add…`, `make…`, `draw…`, `read…`,
  `check…`, `choose…`, `open…`, `set…`, `render…`, `select…`, `advance`,
  `reset`, `openEditor`, `returnKeyFocus` and the rest.
- **`CowsApp::save`**: `saveSettings` would hide the free `saveSettings` it
  calls.
- `TouchControls::pointerDown` / `pointerMoved` / `pointerUp` are event
  handlers named as eacp's `mouseDown` is, so they stay.

## Conflicts and things to know

- **Same new name in one scope**: none. `projectOntoGround` replaces `flat`
  (CowsView.cpp), `ground` (Game.cpp) and `ground` in three tests, each in
  its own file's anonymous namespace.
- **Overloads renamed together**: `Cow::placement` (2), `settingsFile` (2),
  and in tests `near`, `same`, `finite`, `touchesPath`.
- **New overload of an eacp name**: `transformed` in `Render/Shaders.cpp`
  and `Title/TitleShader.cpp` becomes `transformPoint(Float4x4, Float3)`,
  beside `Maths::transformPoint(Mat4, Vec3)`. Neither file calls the Maths
  one, so it builds; `transformShaderPoint` if two of them is one too many.
- **New names eacp also spells** (no base class has them, so nothing is
  hidden): `getText`, `measureText`, `getAscent`, `getShape`, `isPressed`,
  `findTargetAt`, `findTargetIn`, `shouldLoop`, `takeSnapshot`.
- **Fields that share a renamed method's spelling**, left alone on purpose:
  `lightViewProjection` (`EACP_SHADER` in `Lighting.h`, `Shaders.h`) and
  `placement` (`EACP_GPU_FIELDS` in `TitleShader.h`). They are the only code
  tokens with an old spelling no cursor claims.
- **Old names several rows rename differently** (`at`, `distance`,
  `inBarn`, `inputOwner`, `inside`, `pose`, `render`, `right`, `spot`,
  `squeezed`, `stepped`, `wrapped`): code is renamed by USR; their mentions
  in comments and docs are left for a hand edit.
- **Comments that read oddly after a mechanical rename**, e.g. "where the
  old cowArea was" becomes "the old getCowArea". Phase 2 reads the comment
  diff once.
- **Flagged names not on develop**: `frameRateRow`, `frameRateLabel`,
  `frameRateNamed`, `settingsRows`, `withKnownValue`, `isWide`,
  `renderMenus`, `nextSeed` (presumably on the branch about to merge;
  re-run `inventory` after it lands). `stepUp` is named in CLAUDE.md but
  nothing declares it.

## Least sure

1. `Layout::unit` / `between` / `spot` / `pick` / `index` and `randomUnit` /
   `randomBetween` / `randomPick` → `draw…`. CLAUDE.md calls them "seeded
   draws", but `draw` means render everywhere else here.
   `chooseRandomSpot` and friends would avoid that.
2. `Flex::Tree::resolve` → `calculateLayout`. `layOut` would match
   `Menu::layOut` and `Editor::layOut`.
3. `CowsView::callOut` → `mooAndPlayAnswer`. Two verbs; `callToPartner` is
   shorter but vaguer.
4. `CowsView::control` → `handleControlEvent`; or `applyControl`.
5. `ItemClass::choice` / `choose` (function members) → `getChoice` /
   `setChoice`. They are accessors held as functions, not events, so not
   `on…`.
6. `Level::pushedOut` → `pushOutOfScenery`. It returns the pushed point and
   changes nothing; `push` suggests it moves something.
   `findPointPushedOut` is more exact.
7. `Level::floorAt` / `ceilingAt` → `find…At`, against `Block::heightAt` →
   `getHeightAt`. `find` because it searches every collider.
8. `Level::isFree` → `isClearOfScenery`.
9. Camera getters: `eye` → `getEyePosition`, `view` → `getViewMatrix`,
   `projection` → `getProjectionMatrix`, `forward`/`right`/`up` → `get…`. A
   lot of churn for a type whose bare nouns already read.
10. `matte` → `makeMatteMaterial` (21 call sites, longer lines), along with
    `cloth`, `shiny` and `plain`.
11. `Palette::linear` / `display` → `makeLinearColor` / `makeDisplayColor`,
    and `toLinear` → `convertToLinear`.
12. `Game::stage` → `getStagePlacement`, `Game::playing` →
    `getPlayingState`.
13. `Stages::level` → `getLevelMaker`. It returns the maker, not a level.
14. `CowsView::framedCow` → `calculatePoseFramingCow`: clunky.
    `calculateEditorPoseForCow`?
15. `bentOver` / `waved` (vertex transforms) → `bendConeTipOver` /
    `waveBrimVertex`.
16. `near` → `isNear` in tests (160 edits). `check(near(a, b))` already reads.
17. `Mesh::detailed` → `scaleCountByDetail`.
18. `KissHearts` `random(kiss, heart, salt)` → `getSeededRandom`; it is a
    hash, so `hashToUnit` is also true.
19. `FitToSize::onGrid` → `snapToGrid` and `onFontGrid` → `snapToFontGrid`.
    They round down to the step, so `roundDownToGrid` would be more exact.
20. `Layout::share(count, region)` → `calculateShareOf`: what it returns is
    the region's part of `count` by area, so `countForRegion` may be
    plainer.

## Renames

### cowsinlove_Engine (127)

| old | new | why | edits | at |
|---|---|---|---|---|
| `blend` | `blendPoses` | blend what | 6 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:22 |
| `distanceToFit` | `calculateDistanceToFit` | maths | 6 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:41 |
| `driftAt` | `calculateDriftAt` | maths | 5 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:51 |
| `OrbitCamera::pose` | `getPose` | camera is an object with behaviour | 6 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:68 |
| `OrbitCamera::eye` | `getEyePosition` | derived value | 16 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:71 |
| `OrbitCamera::forward` | `getForward` | derived value | 11 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:72 |
| `OrbitCamera::right` | `getRight` | derived value | 11 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:73 |
| `OrbitCamera::up` | `getUp` | derived value | 8 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:74 |
| `OrbitCamera::view` | `getViewMatrix` | derived value | 8 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:76 |
| `OrbitCamera::projection` | `getProjectionMatrix` | derived value | 8 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:77 |
| `OrbitCamera::verticalFieldOfView` | `getVerticalFieldOfView` | derived value | 11 | Lib/cowsinlove_Engine/Camera/OrbitCamera.h:81 |
| `millisecondsBetween` | `getMillisecondsBetween` | derived value | 5 | Lib/cowsinlove_Engine/Render/FrameProfile.cpp:13 |
| `fixed` | `formatFixed` | formats a number | 11 | Lib/cowsinlove_Engine/Render/FrameProfile.cpp:19 |
| `FrameProfile::report::perFrame` | `averagePerFrame` | averages | 6 | Lib/cowsinlove_Engine/Render/FrameProfile.cpp:138 |
| `FrameProfile::report::drawn` | `formatDrawCounts` | formats | 3 | Lib/cowsinlove_Engine/Render/FrameProfile.cpp:140 |
| `FrameProfile::frameStarted` | `markFrameStarted` | a command, not an event | 3 | Lib/cowsinlove_Engine/Render/FrameProfile.h:49 |
| `FrameProfile::add` | `addPartTime` | add what | 3 | Lib/cowsinlove_Engine/Render/FrameProfile.h:50 |
| `FrameProfile::drew` | `recordDraw` | records a pass's draws | 5 | Lib/cowsinlove_Engine/Render/FrameProfile.h:51 |
| `FrameProfile::passesText` | `makePassesText` | builds a string | 3 | Lib/cowsinlove_Engine/Render/FrameProfile.h:54 |
| `boxInView::allOutside` | `areAllOutside` | bool | 6 | Lib/cowsinlove_Engine/Render/Frustum.cpp:23 |
| `boxInView` | `isBoxInView` | bool | 10 | Lib/cowsinlove_Engine/Render/Frustum.h:11 |
| `sphereInView` | `isSphereInView` | bool; Jamie's example | 7 | Lib/cowsinlove_Engine/Render/Frustum.h:15 |
| `instanceInView` | `isInstanceInView` | bool | 5 | Lib/cowsinlove_Engine/Render/Frustum.h:22 |
| `signedPower` | `calculateSignedPower` | maths | 4 | Lib/cowsinlove_Engine/Render/Mesh.cpp:91 |
| `barrelProfile` | `getBarrelProfile` | derived value | 4 | Lib/cowsinlove_Engine/Render/Mesh.cpp:96 |
| `barrelPoint` | `makeBarrelPoint` | builds a vertex | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:106 |
| `barrelLatitudeAt` | `getBarrelLatitudeAt` | derived value | 3 | Lib/cowsinlove_Engine/Render/Mesh.cpp:119 |
| `directionOr` | `normaliseOrFallBack` | says what it does | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:125 |
| `longitudesAcross` | `findLongitudesAcross` | finds | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:133 |
| `longitudesAcross::at` | `getLongitudeAt` | lambda; "at" alone | 3 | Lib/cowsinlove_Engine/Render/Mesh.cpp:138 |
| `cutHole::inside` | `isInsideHole` | bool | 4 | Lib/cowsinlove_Engine/Render/Mesh.cpp:174 |
| `keyOf` | `makePointKey` | builds a key | 3 | Lib/cowsinlove_Engine/Render/Mesh.cpp:190 |
| `keyOf::snap` | `snapToKeyGrid` | snaps to what | 4 | Lib/cowsinlove_Engine/Render/Mesh.cpp:192 |
| `boundaryOf` | `findBoundaryEdges` | finds | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:203 |
| `boundaryOf::keyed` | `makeEdgeKey` | builds a key | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:207 |
| `roundHole` | `roundOffHole` | rounds the hole's edge | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:234 |
| `profileNormal` | `calculateProfileNormal` | maths | 2 | Lib/cowsinlove_Engine/Render/Mesh.cpp:269 |
| `detailed` | `scaleCountByDetail` | Jamie's detailed; says what it does | 18 | Lib/cowsinlove_Engine/Render/Mesh.h:73 |
| `normalMatrixFor` | `makeNormalMatrix` | builds a matrix | 4 | Lib/cowsinlove_Engine/Render/Mesh.h:88 |
| `boundingRadius` | `calculateBoundingRadius` | maths | 5 | Lib/cowsinlove_Engine/Render/Mesh.h:103 |
| `Palette::display` | `makeDisplayColor` | builds a colour from hex | 2 | Lib/cowsinlove_Engine/Render/Palette.h:44 |
| `Palette::toLinear` | `convertToLinear` | to... is for the receiver | 5 | Lib/cowsinlove_Engine/Render/Palette.h:51 |
| `Palette::linear` | `makeLinearColor` | builds a linear colour from hex | 22 | Lib/cowsinlove_Engine/Render/Palette.h:56 |
| `Palette::linear::channel` | `convertChannel` | lambda | 4 | Lib/cowsinlove_Engine/Render/Palette.h:58 |
| `QualityGovernor::decided` | `hasDecided` | bool | 6 | Lib/cowsinlove_Engine/Render/QualityGovernor.h:43 |
| `transformed` | `transformPoint` | maths | 5 | Lib/cowsinlove_Engine/Render/Shaders.cpp:12 |
| `Shading::hashOf` | `hashPosition` | hash what | 2 | Lib/cowsinlove_Engine/Render/Shading.cpp:46 |
| `Shading::quickHashOf` | `quickHashPosition` | hash what | 2 | Lib/cowsinlove_Engine/Render/Shading.cpp:51 |
| `Shading::valueNoiseHashed` | `calculateValueNoiseWithHash` | maths | 3 | Lib/cowsinlove_Engine/Render/Shading.cpp:59 |
| `Shading::valueNoiseHashed::corner` | `hashCorner` | lambda | 9 | Lib/cowsinlove_Engine/Render/Shading.cpp:69 |
| `Shading::shadowAt::litAt` | `sampleLitAt` | samples the shadow map | 3 | Lib/cowsinlove_Engine/Render/Shading.cpp:131 |
| `Shading::shadowAt::averageOf` | `averageSamplesAround` | averages PCF taps | 3 | Lib/cowsinlove_Engine/Render/Shading.cpp:138 |
| `latticeValue` | `calculateLatticeValue` | maths | 2 | Lib/cowsinlove_Engine/Render/Shading.cpp:212 |
| `Shading::ambient` | `calculateAmbient` | maths | 3 | Lib/cowsinlove_Engine/Render/Shading.h:22 |
| `Shading::shadowAt` | `calculateShadowAt` | maths | 5 | Lib/cowsinlove_Engine/Render/Shading.h:29 |
| `Shading::hazeAt` | `calculateHazeAt` | maths | 5 | Lib/cowsinlove_Engine/Render/Shading.h:33 |
| `Shading::withHaze` | `applyHaze` | applies | 5 | Lib/cowsinlove_Engine/Render/Shading.h:37 |
| `Shading::toDisplay` | `convertToDisplay` | to... is for the receiver | 7 | Lib/cowsinlove_Engine/Render/Shading.h:41 |
| `Shading::valueNoise` | `calculateValueNoise` | maths | 7 | Lib/cowsinlove_Engine/Render/Shading.h:43 |
| `Shading::quickValueNoise` | `calculateQuickValueNoise` | maths | 3 | Lib/cowsinlove_Engine/Render/Shading.h:47 |
| `Shading::latticeNoise` | `sampleLatticeNoise` | samples a texture | 4 | Lib/cowsinlove_Engine/Render/Shading.h:53 |
| `ShadowMap::lightViewProjection` | `calculateLightViewProjection` | maths | 5 | Lib/cowsinlove_Engine/Render/ShadowMap.h:16 |
| `ShadowMap::pipeline` | `makePipelineDescriptor` | builds a descriptor | 3 | Lib/cowsinlove_Engine/Render/ShadowMap.h:19 |
| `makeSolid::steps` | `countSteps` | lambda | 8 | Lib/cowsinlove_Engine/Render/ShapeMeshes.cpp:11 |
| `wheelZoom` | `calculateWheelZoom` | maths | 4 | Lib/cowsinlove_Engine/UI/ControlEvent.h:26 |
| `aspectWide` (2 overloads) | `isAspectWide` | bool | 6 | Lib/cowsinlove_Engine/UI/Editor.cpp:64 |
| `cowIn` (2 overloads) | `calculateCowArea` | maths of the layout | 5 | Lib/cowsinlove_Engine/UI/Editor.cpp:70 |
| `panelIn` | `calculatePanelArea` | maths of the layout | 5 | Lib/cowsinlove_Engine/UI/Editor.cpp:78 |
| `squeezed` (2 overloads) | `squeezeShape` | transforms a shape | 6 | Lib/cowsinlove_Engine/UI/Editor.cpp:88 |
| `label` | `makeLabel` | builds a node | 3 | Lib/cowsinlove_Engine/UI/Editor.cpp:95 |
| `button` | `makeButton` | builds a node | 5 | Lib/cowsinlove_Engine/UI/Editor.cpp:105 |
| `linesFor` | `countLinesFor` | counts | 2 | Lib/cowsinlove_Engine/UI/Editor.cpp:147 |
| `panelTree` | `makePanelTree` | builds a tree | 3 | Lib/cowsinlove_Engine/UI/Editor.cpp:155 |
| `Editor::layOut::targetFor` | `findTargetFor` | lambda | 3 | Lib/cowsinlove_Engine/UI/Editor.cpp:296 |
| `Editor::layOut::baselineIn` | `findBaselineIn` | lambda | 4 | Lib/cowsinlove_Engine/UI/Editor.cpp:368 |
| `Editor::cowArea` | `getCowArea` | Jamie's example | 8 | Lib/cowsinlove_Engine/UI/Editor.h:58 |
| `Editor::panelArea` | `getPanelArea` | derived value | 3 | Lib/cowsinlove_Engine/UI/Editor.h:60 |
| `Editor::wide` | `isWide` | Jamie's wide(); bool | 7 | Lib/cowsinlove_Engine/UI/Editor.h:62 |
| `Editor::targetAt` | `findTargetAt` | hit test | 12 | Lib/cowsinlove_Engine/UI/Editor.h:74 |
| `Editor::shape` | `getShape` | derived value | 5 | Lib/cowsinlove_Engine/UI/Editor.h:112 |
| `Editor::doneIndex` | `getDoneIndex` | derived value | 8 | Lib/cowsinlove_Engine/UI/Editor.h:125 |
| `Editor::act` | `activateTarget` | act on what | 6 | Lib/cowsinlove_Engine/UI/Editor.h:126 |
| `Editor::onStep` | `onRowValueStepped` | Jamie's example | 7 | Lib/cowsinlove_Engine/UI/Editor.h:129 |
| `Editor::onDone` | `onDonePressed` | says what happened | 5 | Lib/cowsinlove_Engine/UI/Editor.h:130 |
| `onFontGrid` | `snapToFontGrid` | Jamie's onFontGrid; reads as a callback | 8 | Lib/cowsinlove_Engine/UI/FontCache.h:18 |
| `FontCache::fontFor` | `getFontFor` | returns cached state | 11 | Lib/cowsinlove_Engine/UI/FontCache.h:30 |
| `uiFont` | `getUIFont` | Jamie's uiFont | 5 | Lib/cowsinlove_Engine/UI/FontCache.h:55 |
| `Hud::measure` | `measureText` | measure what | 9 | Lib/cowsinlove_Engine/UI/Hud.h:39 |
| `Hud::ascent` | `getAscent` | derived value | 6 | Lib/cowsinlove_Engine/UI/Hud.h:40 |
| `titleIn` | `calculateTitleArea` | maths of the layout | 2 | Lib/cowsinlove_Engine/UI/Menu.cpp:52 |
| `bodyIn` | `calculateBodyArea` | maths of the layout | 3 | Lib/cowsinlove_Engine/UI/Menu.cpp:57 |
| `itemsIn` | `calculateItemsArea` | maths of the layout | 3 | Lib/cowsinlove_Engine/UI/Menu.cpp:80 |
| `smallestScale` | `findSmallestScale` | finds | 2 | Lib/cowsinlove_Engine/UI/Menu.cpp:91 |
| `noteSizeFor` | `calculateNoteSize` | maths | 3 | Lib/cowsinlove_Engine/UI/Menu.cpp:117 |
| `itemTree` | `makeItemTree` | builds a tree | 3 | Lib/cowsinlove_Engine/UI/Menu.cpp:125 |
| `Menu::layOut::targetIn` | `findTargetIn` | lambda | 3 | Lib/cowsinlove_Engine/UI/Menu.cpp:242 |
| `Menu::titleArea` | `getTitleArea` | Jamie's example | 5 | Lib/cowsinlove_Engine/UI/Menu.h:40 |
| `Menu::cowArea` | `getCowArea` | derived value | 7 | Lib/cowsinlove_Engine/UI/Menu.h:43 |
| `Menu::wide` | `isWide` | bool | 6 | Lib/cowsinlove_Engine/UI/Menu.h:45 |
| `Menu::itemAt` | `findItemAt` | hit test | 8 | Lib/cowsinlove_Engine/UI/Menu.h:54 |
| `Menu::shape` | `getShape` | derived value | 6 | Lib/cowsinlove_Engine/UI/Menu.h:93 |
| `Menu::onChoose` | `onItemChosen` | says what happened | 6 | Lib/cowsinlove_Engine/UI/Menu.h:98 |
| `menuFontFamily` | `getMenuFontFamily` | returns a value | 3 | Lib/cowsinlove_Engine/UI/Menu.h:124 |
| `sameRect` | `areRectsEqual` | bool | 2 | Lib/cowsinlove_Engine/UI/MenuStyle.cpp:33 |
| `sameInsets` | `areInsetsEqual` | bool | 2 | Lib/cowsinlove_Engine/UI/MenuStyle.cpp:38 |
| `pink` | `makePink` | builds a colour | 8 | Lib/cowsinlove_Engine/UI/MenuStyle.h:25 |
| `faded` | `fadeColor` | transforms a colour | 10 | Lib/cowsinlove_Engine/UI/MenuStyle.h:26 |
| `menuFont` | `getMenuFont` | Jamie's menuFont | 8 | Lib/cowsinlove_Engine/UI/MenuStyle.h:29 |
| `menuMeasure` | `makeMenuMeasure` | builds a measure function | 4 | Lib/cowsinlove_Engine/UI/MenuStyle.h:31 |
| `ScreenFrame::contentArea` | `getContentArea` | derived value | 10 | Lib/cowsinlove_Engine/UI/MenuStyle.h:39 |
| `frameOf` | `makeScreenFrame` | builds a ScreenFrame from a view | 12 | Lib/cowsinlove_Engine/UI/MenuStyle.h:46 |
| `sameFrame` | `areFramesEqual` | Jamie's sameFrame; bool | 6 | Lib/cowsinlove_Engine/UI/MenuStyle.h:48 |
| `tapTarget` | `calculateTapTarget` | maths | 4 | Lib/cowsinlove_Engine/UI/MenuStyle.h:52 |
| `smallestReadableSize` | `calculateSmallestReadableSize` | maths | 7 | Lib/cowsinlove_Engine/UI/MenuStyle.h:57 |
| `largestShape` | `chooseLargestShape` | chooses among shapes | 4 | Lib/cowsinlove_Engine/UI/MenuStyle.h:80 |
| `Footer::lines` | `splitIntoLines` | says what it does | 3 | Lib/cowsinlove_Engine/UI/Overlay.h:17 |
| `Footer::reserveFor` | `calculateReservedHeight` | maths | 3 | Lib/cowsinlove_Engine/UI/Overlay.h:28 |
| `Footer::text` | `getText` | function member that supplies the text | 7 | Lib/cowsinlove_Engine/UI/Overlay.h:30 |
| `RootView::backKey` | `handleBackKey` | handles a key | 4 | Lib/cowsinlove_Engine/UI/Overlay.h:54 |
| `shaped` | `applyStickCurve` | shapes the stick's response | 3 | Lib/cowsinlove_Engine/UI/TouchControls.cpp:35 |
| `TouchControls::roleAt` | `findRoleAt` | hit test | 12 | Lib/cowsinlove_Engine/UI/TouchControls.h:56 |
| `TouchControls::find` | `findPointer` | find what | 4 | Lib/cowsinlove_Engine/UI/TouchControls.h:57 |
| `TouchControls::otherLooking` | `findOtherLookingPointer` | finds | 3 | Lib/cowsinlove_Engine/UI/TouchControls.h:58 |
| `TouchControls::pressed` | `isPressed` | bool | 14 | Lib/cowsinlove_Engine/UI/TouchControls.h:59 |

### cowsinlove_Layout (20)

| old | new | why | edits | at |
|---|---|---|---|---|
| `Flex::fitsAt` | `doesFitAt` | bool | 6 | Lib/cowsinlove_Layout/Layout/FitToSize.cpp:13 |
| `Flex::fitSizeAt` | `calculateFitSizeAt` | maths | 3 | Lib/cowsinlove_Layout/Layout/FitToSize.cpp:23 |
| `Flex::sizeWhereNeedReaches` | `findSizeWhereNeedReaches` | interpolates | 3 | Lib/cowsinlove_Layout/Layout/FitToSize.cpp:36 |
| `Flex::onGrid` | `snapToGrid` | Jamie's onGrid | 5 | Lib/cowsinlove_Layout/Layout/FitToSize.h:23 |
| `Flex::largestPointSize` | `findLargestPointSize` | Jamie's largestPointSize | 7 | Lib/cowsinlove_Layout/Layout/FitToSize.h:39 |
| `Flex::snapped` | `snapToPixels` | Jamie's snapped | 3 | Lib/cowsinlove_Layout/Layout/Tree.cpp:20 |
| `Flex::along` | `getMainAxisComponent` | derived value | 3 | Lib/cowsinlove_Layout/Layout/Tree.cpp:30 |
| `Flex::across` | `getCrossAxisComponent` | derived value | 3 | Lib/cowsinlove_Layout/Layout/Tree.cpp:35 |
| `Flex::mainSize` | `getMainSize` | derived value | 4 | Lib/cowsinlove_Layout/Layout/Tree.cpp:40 |
| `Flex::crossSize` | `getCrossSize` | derived value | 3 | Lib/cowsinlove_Layout/Layout/Tree.cpp:45 |
| `Flex::basis` | `calculateBasis` | maths | 8 | Lib/cowsinlove_Layout/Layout/Tree.cpp:53 |
| `Flex::crossBasis` | `calculateCrossBasis` | maths | 2 | Lib/cowsinlove_Layout/Layout/Tree.cpp:72 |
| `Flex::offsetFor` | `calculateOffsetFor` | maths | 3 | Lib/cowsinlove_Layout/Layout/Tree.cpp:80 |
| `Flex::Tree::resolve` | `calculateLayout` | Jamie's resolve; says what is resolved | 21 | Lib/cowsinlove_Layout/Layout/Tree.h:114 |
| `Flex::Tree::node` | `getNode` | returns state | 4 | Lib/cowsinlove_Layout/Layout/Tree.h:116 |
| `Flex::Tree::rect` | `getRect` | returns state | 40 | Lib/cowsinlove_Layout/Layout/Tree.h:117 |
| `Flex::Tree::overflow` | `isOverflowing` | bool | 7 | Lib/cowsinlove_Layout/Layout/Tree.h:118 |
| `Flex::Tree::overflows` | `hasOverflow` | bool | 8 | Lib/cowsinlove_Layout/Layout/Tree.h:120 |
| `Flex::Tree::fitSize` | `getFitSize` | Jamie's fitSize | 3 | Lib/cowsinlove_Layout/Layout/Tree.h:122 |
| `Flex::Tree::fitOf` | `calculateFitOf` | maths | 4 | Lib/cowsinlove_Layout/Layout/Tree.h:142 |

### cowsinlove_AudioEngine (4)

| old | new | why | edits | at |
|---|---|---|---|---|
| `sampleAt` | `interpolateSampleAt` | interpolates between samples | 2 | Lib/cowsinlove_AudioEngine/SamplePlayer.cpp:22 |
| `mixFrames` | `calculateMixFrameCount` | maths | 2 | Lib/cowsinlove_AudioEngine/SamplePlayer.cpp:33 |
| `playbackSession` | `makePlaybackSession` | builds a config | 2 | Lib/cowsinlove_AudioEngine/SamplePlayer.cpp:38 |
| `SamplePlayer::render` | `renderAudioBlock` | render what: one device callback's block | 5 | Lib/cowsinlove_AudioEngine/SamplePlayer.h:51 |

### cowsinlove_MeshLoader (0)

| old | new | why | edits | at |
|---|---|---|---|---|

### cowsinlove_Actors (86)

| old | new | why | edits | at |
|---|---|---|---|---|
| `Choreography::phaseOf` | `calculatePhase` | maths | 4 | Lib/cowsinlove_Actors/Animation/Choreography.cpp:12 |
| `Choreography::hopPhase` | `calculateHopPhase` | maths | 3 | Lib/cowsinlove_Actors/Animation/Choreography.cpp:18 |
| `Choreography::closeness` | `calculateCloseness` | maths of the sway | 6 | Lib/cowsinlove_Actors/Animation/Choreography.h:18 |
| `Choreography::kissTime` | `getKissTime` | returns a time | 10 | Lib/cowsinlove_Actors/Animation/Choreography.h:21 |
| `Choreography::latestKiss` | `findLatestKiss` | finds an index | 5 | Lib/cowsinlove_Actors/Animation/Choreography.h:22 |
| `Choreography::hopHeight` | `calculateHopHeight` | maths | 9 | Lib/cowsinlove_Actors/Animation/Choreography.h:26 |
| `Choreography::landingSquash` | `calculateLandingSquash` | maths | 5 | Lib/cowsinlove_Actors/Animation/Choreography.h:29 |
| `Choreography::heartbeat` | `calculateHeartbeat` | maths | 6 | Lib/cowsinlove_Actors/Animation/Choreography.h:32 |
| `Choreography::sunPulse` | `calculateSunPulse` | maths | 5 | Lib/cowsinlove_Actors/Animation/Choreography.h:35 |
| `hideMaterial` | `makeHideMaterial` | builds a value | 6 | Lib/cowsinlove_Actors/Cow/Cow.cpp:21 |
| `plain` | `makePlainMaterial` | builds a material | 9 | Lib/cowsinlove_Actors/Cow/Cow.cpp:29 |
| `eyeMaterial` | `makeEyeMaterial` | builds a value | 2 | Lib/cowsinlove_Actors/Cow/Cow.cpp:36 |
| `place` | `makePlacement` | builds a matrix | 9 | Lib/cowsinlove_Actors/Cow/Cow.cpp:44 |
| `ball` | `makeBallPart` | builds a CowPart | 13 | Lib/cowsinlove_Actors/Cow/Cow.cpp:49 |
| `about` | `rotateAbout` | rotation about a pivot | 3 | Lib/cowsinlove_Actors/Cow/Cow.cpp:58 |
| `hopTime` | `calculateHopTime` | maths | 5 | Lib/cowsinlove_Actors/Cow/Cow.cpp:63 |
| `kissLean` | `calculateKissLean` | maths | 3 | Lib/cowsinlove_Actors/Cow/Cow.cpp:68 |
| `legsAt` | `getLegPairOffset` | offset along the body of a leg pair | 2 | Lib/cowsinlove_Actors/Cow/Cow.cpp:73 |
| `headPose` | `calculateHeadPose` | maths | 2 | Lib/cowsinlove_Actors/Cow/Cow.cpp:180 |
| `tailPose` | `calculateTailPose` | maths | 2 | Lib/cowsinlove_Actors/Cow/Cow.cpp:188 |
| `eyePose` | `calculateEyePose` | maths | 2 | Lib/cowsinlove_Actors/Cow/Cow.cpp:195 |
| `squashed` | `makeSquashMatrix` | builds the squash | 2 | Lib/cowsinlove_Actors/Cow/Cow.cpp:200 |
| `torsoPlacement` | `getTorsoPlacement` | returns a value | 5 | Lib/cowsinlove_Actors/Cow/Cow.h:38 |
| `legPlacements` | `getLegPlacements` | returns a value | 4 | Lib/cowsinlove_Actors/Cow/Cow.h:39 |
| `tailRoot` | `getTailRoot` | returns a value | 3 | Lib/cowsinlove_Actors/Cow/Cow.h:42 |
| `Cow::placement` (2 overloads) | `calculatePlacement` | maths (both overloads) | 6 | Lib/cowsinlove_Actors/Cow/Cow.h:62 |
| `Cow::pose` | `calculatePose` | maths | 5 | Lib/cowsinlove_Actors/Cow/Cow.h:72 |
| `Cow::freePose` | `calculateFreePose` | maths | 9 | Lib/cowsinlove_Actors/Cow/Cow.h:74 |
| `Cow::contact` | `getContactPoint` | the point the pair touch | 3 | Lib/cowsinlove_Actors/Cow/Cow.h:89 |
| `cowReach` | `calculateCowReach` | maths | 3 | Lib/cowsinlove_Actors/Cow/Cow.h:97 |
| `itemClass` | `makeItemClass` | builds a value | 3 | Lib/cowsinlove_Actors/Cow/CowSkin.cpp:13 |
| `ItemClass::choice` | `getChoice` | function member that reads the class's choice from a skin | 9 | Lib/cowsinlove_Actors/Cow/CowSkin.h:54 |
| `ItemClass::choose` | `setChoice` | function member that writes it | 4 | Lib/cowsinlove_Actors/Cow/CowSkin.h:55 |
| `itemClasses` | `getItemClasses` | returns state | 10 | Lib/cowsinlove_Actors/Cow/CowSkin.h:59 |
| `stepped` | `stepItemChoice` | returns the skin with one class's choice stepped | 12 | Lib/cowsinlove_Actors/Cow/CowSkin.h:62 |
| `spelledOut` | `spellOutEnumeratorName` | says what it spells | 6 | Lib/cowsinlove_Actors/Cow/CowSkin.h:65 |
| `cowSkinFromJSON` | `parseCowSkinFromJSON` | parses | 8 | Lib/cowsinlove_Actors/Cow/CowSkin.h:71 |
| `withNamedChoices` | `resetUnnamedChoicesToDefault` | Jamie's example; says what happens | 4 | Lib/cowsinlove_Actors/Cow/CowSkin.h:74 |
| `outline` | `calculateHeartOutline` | maths | 2 | Lib/cowsinlove_Actors/Cow/HeartMesh.cpp:16 |
| `random` | `getSeededRandom` | deterministic per kiss and heart | 4 | Lib/cowsinlove_Actors/Cow/KissHearts.cpp:27 |
| `popIn` | `calculatePopIn` | maths | 2 | Lib/cowsinlove_Actors/Cow/KissHearts.cpp:46 |
| `addHeart::r` | `getHeartRandom` | one letter | 10 | Lib/cowsinlove_Actors/Cow/KissHearts.cpp:73 |
| `MooVoice::call` | `playMooWithAnswer` | plays the recorded moo and her answer | 3 | Lib/cowsinlove_Actors/Cow/Moo.h:22 |
| `cloth` | `makeClothMaterial` | builds a material | 28 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:54 |
| `shiny` | `makeShinyMaterial` | builds a material | 5 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:61 |
| `seatAt` | `makeHatSeat` | builds the hat's seat on the crown | 10 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:90 |
| `wizardRadiusAt` | `calculateWizardRadiusAt` | maths | 3 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:279 |
| `aroundTorso` | `scaleAroundTorso` | scales about the torso's middle | 3 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:314 |
| `onTorso` | `transformIntoTorsoSpace` | maths | 3 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:321 |
| `hips` | `getHipHeight` | returns a height | 4 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:326 |
| `denim` | `makeDenimPatch` | builds a patch | 4 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:331 |
| `belly` | `makeBellyPatch` | builds a patch | 3 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:340 |
| `bellyTrim` | `makeBellyTrimPatch` | builds a patch | 2 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:347 |
| `seat` | `makeSeatPatch` | builds a patch | 2 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:354 |
| `belt` | `makeBeltPatch` | builds a patch | 2 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:364 |
| `makePantsMesh::cut` | `makeCutPatch` | builds a patch from the cut | 5 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:461 |
| `bentOver` | `bendConeTipOver` | transforms a vertex | 2 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:531 |
| `waved` | `waveBrimVertex` | transforms a vertex | 2 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:571 |
| `makeCowMeshAt` | `makeCowMeshForShape` | "At" reads as a position | 2 | Lib/cowsinlove_Actors/Cow/Wardrobe.cpp:644 |
| `cowMeshes` | `makeCowMeshes` | builds the mesh function | 8 | Lib/cowsinlove_Actors/Cow/Wardrobe.h:23 |
| `baleParts` | `makeBaleParts` | builds a value | 8 | Lib/cowsinlove_Actors/Props/Bale.h:18 |
| `turned` | `rotateByQuarterTurns` | Jamie's turned; maths | 8 | Lib/cowsinlove_Actors/Props/Barn.cpp:101 |
| `roofHeightAt` | `getRoofHeightAt` | derived value | 5 | Lib/cowsinlove_Actors/Props/Barn.cpp:110 |
| `standing` | `makeStandingPiece` | builds a piece | 10 | Lib/cowsinlove_Actors/Props/Barn.cpp:162 |
| `floating` | `makeFloatingPiece` | builds a piece | 8 | Lib/cowsinlove_Actors/Props/Barn.cpp:167 |
| `ramp` | `makeRampPiece` | builds a piece | 3 | Lib/cowsinlove_Actors/Props/Barn.cpp:172 |
| `pitch` | `makePitchPiece` | builds a piece | 3 | Lib/cowsinlove_Actors/Props/Barn.cpp:177 |
| `facing` | `calculateFacingHeading` | heading after quarter turns | 2 | Lib/cowsinlove_Actors/Props/Barn.cpp:361 |
| `barnSpots::spot` | `makeBarnSpot` | builds a spot | 5 | Lib/cowsinlove_Actors/Props/Barn.cpp:370 |
| `barnSpots` | `getBarnSpots` | derived value | 12 | Lib/cowsinlove_Actors/Props/Barn.h:26 |
| `barnReach` | `getBarnReach` | returns a value | 6 | Lib/cowsinlove_Actors/Props/Barn.h:30 |
| `Block::heightAt` | `getHeightAt` | derived value on an object with behaviour | 21 | Lib/cowsinlove_Actors/Props/Collision.h:21 |
| `Block::closestTo` | `findClosestPoint` | finds | 9 | Lib/cowsinlove_Actors/Props/Collision.h:22 |
| `contentMeshes` | `makeContentMeshes` | builds the mesh function | 4 | Lib/cowsinlove_Actors/Props/Meshes.h:16 |
| `travelled` | `calculateDistanceTravelled` | maths | 3 | Lib/cowsinlove_Actors/Props/Mover.cpp:11 |
| `lyingDrum` | `makeLyingDrumModel` | builds a matrix | 8 | Lib/cowsinlove_Actors/Props/Mover.h:11 |
| `Mover::positionAt` | `getPositionAt` | derived value | 12 | Lib/cowsinlove_Actors/Props/Mover.h:29 |
| `Mover::modelAt` | `getModelAt` | derived value | 4 | Lib/cowsinlove_Actors/Props/Mover.h:30 |
| `groupsOf` | `getCachedGroups` | loads once, returns the cache | 2 | Lib/cowsinlove_Actors/Props/PropMeshes.cpp:51 |
| `toMesh` | `makeMeshFromGroup` | to... is for the receiver; this converts its argument | 2 | Lib/cowsinlove_Actors/Props/PropMeshes.cpp:67 |
| `randomUnit` | `drawRandomUnit` | a seeded draw | 7 | Lib/cowsinlove_Actors/Props/Props.h:12 |
| `randomBetween` | `drawRandomBetween` | a seeded draw | 12 | Lib/cowsinlove_Actors/Props/Props.h:13 |
| `randomPick` | `drawRandomPick` | a seeded draw | 5 | Lib/cowsinlove_Actors/Props/Props.h:16 |
| `matte` | `makeMatteMaterial` | Jamie's matte; builds a material | 21 | Lib/cowsinlove_Actors/Props/Props.h:22 |
| `SkyDecor::wrapped` | `wrapAcrossSky` | wraps x across the sky's width | 2 | Lib/cowsinlove_Actors/Sky/SkyDecor.cpp:52 |

### cowsinlove_World (46)

| old | new | why | edits | at |
|---|---|---|---|---|
| `pushedOutOf` | `pushOutOfCollider` | Jamie's pushedOut family | 3 | Lib/cowsinlove_World/Level.cpp:14 |
| `standsOn` | `isStandingOn` | bool | 4 | Lib/cowsinlove_World/Level.cpp:30 |
| `floats` | `isFloating` | bool | 3 | Lib/cowsinlove_World/Level.cpp:36 |
| `passesUnder` | `canPassUnder` | bool | 4 | Lib/cowsinlove_World/Level.cpp:42 |
| `sidesOf` | `getSidesOf` | derived value | 2 | Lib/cowsinlove_World/Level.cpp:59 |
| `entryInto` | `findEntryInto` | finds | 2 | Lib/cowsinlove_World/Level.cpp:89 |
| `headingToward` | `calculateHeadingToward` | maths | 3 | Lib/cowsinlove_World/Level.h:18 |
| `Level::pushedOut` | `pushOutOfScenery` | Jamie's pushedOut; returns the point pushed clear | 13 | Lib/cowsinlove_World/Level.h:47 |
| `Level::keptInGap` | `keepInsideGap` | same family | 3 | Lib/cowsinlove_World/Level.h:50 |
| `Level::floorAt` | `findFloorAt` | Jamie's floorAt | 34 | Lib/cowsinlove_World/Level.h:53 |
| `Level::ceilingAt` | `findCeilingAt` | same family | 3 | Lib/cowsinlove_World/Level.h:58 |
| `Level::isFree` | `isClearOfScenery` | Jamie's isFree; free of what | 13 | Lib/cowsinlove_World/Level.h:62 |
| `Level::clearReach` | `findClearReach` | finds | 13 | Lib/cowsinlove_World/Level.h:67 |
| `Level::groundAt` | `getGroundHeightAt` | derived value | 3 | Lib/cowsinlove_World/Level.h:70 |
| `Level::overGap` | `isOverGap` | Jamie's overGap; bool | 9 | Lib/cowsinlove_World/Level.h:71 |
| `Level::onMover` | `isOnMover` | bool | 3 | Lib/cowsinlove_World/Level.h:74 |
| `hideoutSpot` | `findHideoutSpot` | finds | 2 | Lib/cowsinlove_World/Levels/Biome.cpp:130 |
| `meadowBiome` | `makeMeadowBiome` | builds a value | 7 | Lib/cowsinlove_World/Levels/Biome.h:21 |
| `populate` | `populateRegion` | Jamie's populate; populate what | 6 | Lib/cowsinlove_World/Levels/Biome.h:26 |
| `hasRoom::blocked` | `isBlocked` | lambda bool | 2 | Lib/cowsinlove_World/Levels/Layout.cpp:95 |
| `hasRoom::walled` | `isWalled` | lambda bool | 2 | Lib/cowsinlove_World/Levels/Layout.cpp:101 |
| `amongTrees::near` | `isNear` | lambda bool | 2 | Lib/cowsinlove_World/Levels/Layout.cpp:110 |
| `chooseHideout::unfit` | `isUnfit` | lambda bool | 2 | Lib/cowsinlove_World/Levels/Layout.cpp:130 |
| `Layout::unit` | `drawUnit` | a seeded draw | 11 | Lib/cowsinlove_World/Levels/Layout.h:37 |
| `Layout::between` | `drawBetween` | a seeded draw | 18 | Lib/cowsinlove_World/Levels/Layout.h:38 |
| `Layout::spot` | `drawSpot` | a seeded draw | 9 | Lib/cowsinlove_World/Levels/Layout.h:42 |
| `Layout::pick` | `drawPick` | a seeded draw | 2 | Lib/cowsinlove_World/Levels/Layout.h:45 |
| `Layout::index` | `drawIndex` | a seeded draw | 4 | Lib/cowsinlove_World/Levels/Layout.h:50 |
| `Layout::share` | `calculateShareOf` | count for a region by its area | 11 | Lib/cowsinlove_World/Levels/Layout.h:53 |
| `Layout::inClearing` | `isInClearing` | bool | 5 | Lib/cowsinlove_World/Levels/Layout.h:59 |
| `Layout::onPath` | `isOnPath` | bool | 9 | Lib/cowsinlove_World/Levels/Layout.h:62 |
| `amongTrees` | `isAmongTrees` | bool | 3 | Lib/cowsinlove_World/Levels/Layout.h:73 |
| `criticalPath` | `makeCriticalPath` | builds a region | 2 | Lib/cowsinlove_World/Levels/LevelGenerator.cpp:9 |
| `segmentRegions` | `makeSegmentRegions` | builds a value | 5 | Lib/cowsinlove_World/Levels/LevelGenerator.h:13 |
| `generate` | `generateLevel` | Jamie's generate; generate what | 11 | Lib/cowsinlove_World/Levels/LevelGenerator.h:20 |
| `RavineSegment::baleSpawners` | `makeBaleSpawners` | builds a value | 3 | Lib/cowsinlove_World/Levels/Segments/RavineSegment.h:32 |
| `crossesGap` | `doesTileCrossGap` | bool | 2 | Lib/cowsinlove_World/Terrain/Grass.cpp:19 |
| `distanceToTile` | `calculateDistanceToTile` | maths | 2 | Lib/cowsinlove_World/Terrain/Grass.cpp:34 |
| `shareAt` | `calculateShareAt` | maths | 2 | Lib/cowsinlove_World/Terrain/Grass.cpp:41 |
| `nearGap` | `isNearGap` | bool | 2 | Lib/cowsinlove_World/Terrain/Grass.cpp:53 |
| `makeBlade::at` | `getPointAt` | lambda | 7 | Lib/cowsinlove_World/Terrain/Grass.cpp:81 |
| `bladesToDraw` | `countBladesToDraw` | maths | 9 | Lib/cowsinlove_World/Terrain/Grass.h:83 |
| `GrassField::tileAt` | `getTileAt` | returns state | 8 | Lib/cowsinlove_World/Terrain/Grass.h:92 |
| `cutsAlong` | `findCutsAlong` | finds | 3 | Lib/cowsinlove_World/Terrain/Ground.cpp:17 |
| `GroundShader::define::contactOf` | `calculateContact` | lambda maths | 3 | Lib/cowsinlove_World/Terrain/TerrainShaders.cpp:36 |
| `GrassShader::define::noiseOf` | `sampleNoise` | lambda | 6 | Lib/cowsinlove_World/Terrain/TerrainShaders.cpp:63 |

### cowsinlove_Game (41)

| old | new | why | edits | at |
|---|---|---|---|---|
| `Ending::stagingFor` | `getStagingFor` | derived value | 6 | Lib/cowsinlove_Game/Ending.h:53 |
| `Ending::titleRise` | `calculateTitleRise` | maths | 6 | Lib/cowsinlove_Game/Ending.h:56 |
| `Ending::titlePlacement` | `calculateTitlePlacement` | maths | 4 | Lib/cowsinlove_Game/Ending.h:58 |
| `Ending::foundStart` | `getFoundStart` | derived value | 5 | Lib/cowsinlove_Game/Ending.h:62 |
| `Ending::loops` | `shouldLoop` | bool | 7 | Lib/cowsinlove_Game/Ending.h:65 |
| `padLabels` | `makePadLabels` | builds a value | 5 | Lib/cowsinlove_Game/Game.cpp:54 |
| `searchingPadText` | `makeSearchingPadText` | builds a string | 2 | Lib/cowsinlove_Game/Game.cpp:69 |
| `menuText` | `makeMenuText` | builds a string | 2 | Lib/cowsinlove_Game/Game.cpp:75 |
| `foundPadText` | `makeFoundPadText` | builds a string | 2 | Lib/cowsinlove_Game/Game.cpp:83 |
| `searchingText` | `makeSearchingText` | builds a string | 2 | Lib/cowsinlove_Game/Game.cpp:88 |
| `foundText` | `makeFoundText` | builds a string | 2 | Lib/cowsinlove_Game/Game.cpp:96 |
| `ground` | `projectOntoGround` | drops y | 10 | Lib/cowsinlove_Game/Game.cpp:104 |
| `Game::playing` | `getPlayingState` | the state behind the menu | 9 | Lib/cowsinlove_Game/Game.h:32 |
| `Game::distance` | `getDistanceToPartner` | distance to what | 6 | Lib/cowsinlove_Game/Game.h:44 |
| `Game::warmth` | `getWarmth` | derived value | 8 | Lib/cowsinlove_Game/Game.h:47 |
| `Game::endingSeconds` | `getEndingSeconds` | derived value | 5 | Lib/cowsinlove_Game/Game.h:50 |
| `Game::stage` | `getStagePlacement` | the ending's stage matrix | 4 | Lib/cowsinlove_Game/Game.h:54 |
| `Game::hintShowing` | `isHintShowing` | bool | 7 | Lib/cowsinlove_Game/Game.h:57 |
| `Game::justFell` | `hasJustFallen` | bool | 8 | Lib/cowsinlove_Game/Game.h:60 |
| `escapeAction` | `chooseEscapeAction` | chooses | 8 | Lib/cowsinlove_Game/Game.h:105 |
| `inputOwner` | `chooseInputOwner` | chooses | 11 | Lib/cowsinlove_Game/Game.h:118 |
| `footerText` | `makeFooterText` | Jamie's footerText; builds a string | 13 | Lib/cowsinlove_Game/Game.h:139 |
| `editorText` | `makeEditorText` | builds a string | 4 | Lib/cowsinlove_Game/Game.h:142 |
| `Input::setHeld` | `setKeyHeld` | held what | 18 | Lib/cowsinlove_Game/Input.h:14 |
| `Input::walkAhead` | `getWalkAhead` | derived value | 19 | Lib/cowsinlove_Game/Input.h:15 |
| `Input::walkTurn` | `getWalkTurn` | derived value | 15 | Lib/cowsinlove_Game/Input.h:16 |
| `rescaled` | `rescalePastDeadZone` | transforms | 3 | Lib/cowsinlove_Game/Pad.cpp:19 |
| `outsideDeadZone` | `applyRadialDeadZone` | transforms | 3 | Lib/cowsinlove_Game/Pad.cpp:24 |
| `curved` | `applyResponseCurve` | transforms | 4 | Lib/cowsinlove_Game/Pad.cpp:35 |
| `digital` | `readDigitalAxis` | reads | 3 | Lib/cowsinlove_Game/Pad.cpp:40 |
| `anyButton` | `isAnyButtonDown` | bool | 2 | Lib/cowsinlove_Game/Pad.cpp:47 |
| `trigger` | `readTrigger` | reads | 3 | Lib/cowsinlove_Game/Pad.cpp:60 |
| `readPad` | `readPadControls` | Jamie's readPad; already a verb, now says what it reads | 4 | Lib/cowsinlove_Game/Pad.h:30 |
| `padHints` | `chooseHintsForPad` | Jamie's padHints | 3 | Lib/cowsinlove_Game/Pad.h:32 |
| `MenuTitle::placement` | `calculatePlacement` | maths | 3 | Lib/cowsinlove_Game/Title/MenuTitle.h:22 |
| `arc` | `makeArc` | builds a stroke | 5 | Lib/cowsinlove_Game/Title/TitleFont.cpp:49 |
| `line` | `makeLine` | builds a stroke | 9 | Lib/cowsinlove_Game/Title/TitleFont.cpp:56 |
| `archThenLeg` | `makeArchThenLeg` | builds a stroke | 4 | Lib/cowsinlove_Game/Title/TitleFont.cpp:64 |
| `glyphFor` | `getGlyphFor` | returns a value | 2 | Lib/cowsinlove_Game/Title/TitleFont.cpp:71 |
| `tangentAt` | `calculateTangentAt` | maths | 2 | Lib/cowsinlove_Game/Title/TitleFont.cpp:165 |
| `transformed` | `transformPoint` | maths | 5 | Lib/cowsinlove_Game/Title/TitleShader.cpp:12 |

### Apps/CowsInLove (75)

| old | new | why | edits | at |
|---|---|---|---|---|
| `firstWindowSize` | `chooseFirstWindowSize` | picks the size for the screen; a noun read as a value | 2 | Apps/CowsInLove/Source/CowsApp.cpp:24 |
| `windowOptions` | `makeWindowOptions` | builds a value | 2 | Apps/CowsInLove/Source/CowsApp.cpp:43 |
| `opensOnMenu` | `shouldOpenOnMenu` | bool read from COWS_MENU; says it is a question | 4 | Apps/CowsInLove/Source/CowsApp.cpp:59 |
| `opensDressing` | `shouldOpenCowEditor` | bool from COWS_DRESS; "dressing" is jargon for the cow editor | 2 | Apps/CowsInLove/Source/CowsApp.cpp:64 |
| `opensSettings` | `shouldOpenSettings` | bool from COWS_SETTINGS | 2 | Apps/CowsInLove/Source/CowsApp.cpp:69 |
| `firstStage` | `readFirstStage` | reads COWS_STAGE | 2 | Apps/CowsInLove/Source/CowsApp.cpp:74 |
| `firstSeed` | `readFirstSeed` | reads COWS_SEED, else the clock | 2 | Apps/CowsInLove/Source/CowsApp.cpp:82 |
| `titlebarClearance` | `getTitleBarClearance` | derived value; English spelling of title bar | 2 | Apps/CowsInLove/Source/CowsApp.cpp:90 |
| `CowsApp::dress` | `dressCow` | dress whom: the player's cow (wears, shows, saves) | 3 | Apps/CowsInLove/Source/CowsApp.h:17 |
| `CowsApp::showSkin` | `showSkinInEditor` | copies the skin's choices into the editor rows; "show skin" alone reads as drawing it | 3 | Apps/CowsInLove/Source/CowsApp.h:18 |
| `CowsApp::showQuality` | `showQualitySettings` | fills the Settings panel's rows | 4 | Apps/CowsInLove/Source/CowsApp.h:21 |
| `mainMenuItems` | `makeMainMenuItems` | builds a value | 3 | Apps/CowsInLove/Source/MainMenu.h:26 |
| `versionText` | `makeVersionText` | builds a string | 2 | Apps/CowsInLove/Source/MainMenu.h:36 |
| `aboutItems` | `makeAboutItems` | builds a value | 3 | Apps/CowsInLove/Source/MainMenu.h:42 |
| `dressRows` | `makeCowEditorRows` | builds the Dress Your Cow rows | 3 | Apps/CowsInLove/Source/MainMenu.h:50 |
| `qualityRow` | `makeQualityRow` | builds a value (Jamie's qualityRow) | 3 | Apps/CowsInLove/Source/MainMenu.h:62 |
| `skyPipeline` | `makeSkyPipeline` | builds a descriptor | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:91 |
| `solidPipeline` | `makeSolidPipeline` | builds a descriptor | 8 | Apps/CowsInLove/Source/Scene/CowsView.cpp:101 |
| `translucentPipeline` | `makeTranslucentPipeline` | builds a descriptor | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:110 |
| `glowPipeline` | `makeGlowPipeline` | builds a descriptor | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:118 |
| `mooShape` | `calculateMooEnvelope` | sine envelope of the moo over its length | 3 | Apps/CowsInLove/Source/Scene/CowsView.cpp:143 |
| `flat` | `projectOntoGround` | drops y: Vec3 to the xz plane | 7 | Apps/CowsInLove/Source/Scene/CowsView.cpp:151 |
| `distanceWord` | `describeDistance` | returns the words for a distance | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:158 |
| `directionWord` | `describeDirection` | returns the words for a direction | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:169 |
| `startAfterSetting` | `readStartAfterSetting` | reads COWS_START | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:180 |
| `startTime` | `readStartTime` | reads COWS_TIME | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:188 |
| `ProfileSettings::skips` | `isPartSkipped` | bool; says what is asked | 8 | Apps/CowsInLove/Source/Scene/CowsView.cpp:203 |
| `profileSettings` | `getProfileSettings` | returns cached state | 13 | Apps/CowsInLove/Source/Scene/CowsView.cpp:216 |
| `displayColor` | `convertLinearToDisplayColor` | converts its argument | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:244 |
| `displayColor::encode` | `encodeChannel` | encodes one channel | 4 | Apps/CowsInLove/Source/Scene/CowsView.cpp:246 |
| `centreOnScreen` | `getCentreOnScreen` | derived value | 2 | Apps/CowsInLove/Source/Scene/CowsView.cpp:251 |
| `CowsView::control` | `handleControlEvent` | handles a ControlEvent; "control" alone says nothing | 8 | Apps/CowsInLove/Source/Scene/CowsView.h:77 |
| `CowsView::inputOwner` | `getInputOwner` | returns derived state | 4 | Apps/CowsInLove/Source/Scene/CowsView.h:80 |
| `CowsView::swingTo` | `startCameraSwing` | starts the swing; the goal is the argument | 6 | Apps/CowsInLove/Source/Scene/CowsView.h:82 |
| `CowsView::wear` | `wearSkin` | wear what | 4 | Apps/CowsInLove/Source/Scene/CowsView.h:84 |
| `CowsView::escape` | `handleEscape` | handles Esc, returns whether it did | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:85 |
| `CowsView::swingCamera` | `advanceCameraSwing` | one frame of the swing | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:86 |
| `CowsView::menuOpacity` | `getMenuOpacity` | derived value | 5 | Apps/CowsInLove/Source/Scene/CowsView.h:87 |
| `CowsView::editorOpacity` | `getEditorOpacity` | derived value | 5 | Apps/CowsInLove/Source/Scene/CowsView.h:88 |
| `CowsView::menuPose` | `getMenuPose` | derived value (Jamie's menuPose) | 5 | Apps/CowsInLove/Source/Scene/CowsView.h:89 |
| `CowsView::editorPose` | `getEditorPose` | derived value | 5 | Apps/CowsInLove/Source/Scene/CowsView.h:90 |
| `CowsView::editorFramingChanged` | `hasEditorFramingChanged` | bool | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:91 |
| `CowsView::frameEditor` | `frameCowForEditor` | frames what: the cow | 4 | Apps/CowsInLove/Source/Scene/CowsView.h:92 |
| `CowsView::framedCow` | `calculatePoseFramingCow` | maths: a pose that frames the cow in an area | 4 | Apps/CowsInLove/Source/Scene/CowsView.h:93 |
| `CowsView::playPose` | `getPlayPose` | derived value | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:97 |
| `CowsView::menuKey` | `handleMenuKey` | handles a key | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:98 |
| `CowsView::editorKey` | `handleEditorKey` | handles a key | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:99 |
| `CowsView::useMenuPad` | `applyPadToMenu` | "use" is vague | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:100 |
| `CowsView::useEditorPad` | `applyPadToEditor` | "use" is vague | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:101 |
| `CowsView::usePad` | `applyPad` | dispatches the pad to menu, editor or play | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:103 |
| `CowsView::useHints` | `setHints` | sets hints and notifies | 9 | Apps/CowsInLove/Source/Scene/CowsView.h:104 |
| `CowsView::callOut` | `mooAndPlayAnswer` | the player moos and her answer plays | 5 | Apps/CowsInLove/Source/Scene/CowsView.h:110 |
| `CowsView::answerFrom` | `calculateMooAnswer` | pan, muffle and volume of her answer from the camera | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:111 |
| `CowsView::keepEyeClear` | `keepCameraClearOfScenery` | whose eye: the camera's; Jamie's example | 4 | Apps/CowsInLove/Source/Scene/CowsView.h:116 |
| `CowsView::framePortrait` | `adjustCameraForPortrait` | switches pitch and distance once for a tall window | 3 | Apps/CowsInLove/Source/Scene/CowsView.h:117 |
| `CowsView::useQuality` | `applyQualityTier` | applies a tier's settings | 4 | Apps/CowsInLove/Source/Scene/CowsView.h:121 |
| `CowsView::groundFocus` | `getGroundFocus` | derived value | 6 | Apps/CowsInLove/Source/Scene/CowsView.h:126 |
| `squeezed` | `removeSpacesAndLowercase` | says what it does to the name | 2 | Apps/CowsInLove/Source/Scene/Quality.cpp:11 |
| `settingsFor` | `makeQualitySettings` | builds the tier's knobs | 8 | Apps/CowsInLove/Source/Scene/Quality.h:46 |
| `qualityName` | `getQualityName` | returns a name | 5 | Apps/CowsInLove/Source/Scene/Quality.h:48 |
| `qualityNamed` | `findQualityNamed` | optional lookup by name | 8 | Apps/CowsInLove/Source/Scene/Quality.h:49 |
| `choiceLabel` | `getChoiceLabel` | returns a label | 5 | Apps/CowsInLove/Source/Scene/Quality.h:63 |
| `chosenQuality` | `getChosenQuality` | optional tier of a choice | 7 | Apps/CowsInLove/Source/Scene/Quality.h:64 |
| `qualityToUse` | `chooseQualityToUse` | picks forced, chosen or measured | 7 | Apps/CowsInLove/Source/Scene/Quality.h:68 |
| `qualityOverride` | `readQualityOverride` | reads COWS_QUALITY | 3 | Apps/CowsInLove/Source/Scene/Quality.h:73 |
| `withKnownValues` | `resetUnknownValuesToDefault` | Jamie's example; says what happens to unknown values | 3 | Apps/CowsInLove/Source/Scene/Quality.h:89 |
| `settingsFromJSON` | `parseSettingsFromJSON` | parses | 5 | Apps/CowsInLove/Source/Settings.h:29 |
| `settingsFile` (2 overloads) | `getSettingsFile` | returns a path (both overloads) | 9 | Apps/CowsInLove/Source/Settings.h:32 |
| `wrapped` | `wrapStageIndex` | wraps a stage index into range | 2 | Apps/CowsInLove/Source/Stages.cpp:11 |
| `Stages::level` | `getLevelMaker` | returns a LevelMaker, not a level | 6 | Apps/CowsInLove/Source/Stages.h:18 |
| `clockSeed` | `makeSeedFromClock` | says where the seed comes from | 4 | Apps/CowsInLove/Source/Stages.h:25 |
| `jumpLine` | `makeJumpLineSegment` | builds a segment | 10 | Apps/CowsInLove/Source/Templates.cpp:17 |
| `meadowTemplate` | `makeMeadowTemplate` | builds a value | 4 | Apps/CowsInLove/Source/Templates.h:8 |
| `meadowRavineTemplate` | `makeMeadowRavineTemplate` | builds a value | 3 | Apps/CowsInLove/Source/Templates.h:13 |

### tools/Art (15)

| old | new | why | edits | at |
|---|---|---|---|---|
| `outputPath` | `makeOutputPath` | builds a path | 3 | tools/Art/Main.cpp:51 |
| `save` | `saveImage` | save what | 4 | tools/Art/Main.cpp:56 |
| `standBarnAt::far` | `isFar` | lambda bool | 3 | tools/Art/Main.cpp:231 |
| `insideBarn` | `isInsideBarn` | bool | 2 | tools/Art/Main.cpp:249 |
| `stepped` | `findStandingPointAhead` | the point reach ahead of a spot at feet height | 4 | tools/Art/Main.cpp:258 |
| `renderEndings::render` | `renderEndingAt` | lambda; render what | 3 | tools/Art/Main.cpp:311 |
| `frame` | `aimCamera` | frame reads as a noun | 3 | tools/Art/Main.cpp:434 |
| `saveTo` | `saveSnapshotTo` | save what | 3 | tools/Art/Main.cpp:446 |
| `run` | `runMode` | run what | 2 | tools/Art/Main.cpp:539 |
| `settle` | `runFramesToSettle` | Jamie's settle | 6 | tools/Art/Screenshots.cpp:35 |
| `standAt` | `standPlayerAt` | Jamie's standAt; who stands | 5 | tools/Art/Screenshots.cpp:45 |
| `headingTo` | `calculateHeadingTo` | Jamie's headingTo; maths | 2 | tools/Art/Screenshots.cpp:55 |
| `shots` | `makeShots` | Jamie's shots; builds the list | 2 | tools/Art/Screenshots.cpp:60 |
| `render` | `renderShot` | render what | 2 | tools/Art/Screenshots.cpp:144 |
| `openMeadowSeed` | `findOpenMeadowSeed` | finds the first such seed | 5 | tools/Art/Screenshots.h:43 |

### Tests (116)

| old | new | why | edits | at |
|---|---|---|---|---|
| `near` (12 overloads) | `isNear` | bool test helper | 160 | Tests/Actors/ChoreographyTests.cpp:12 |
| `everyStep` | `holdsAtEveryStep` | bool over a span | 5 | Tests/Actors/ChoreographyTests.cpp:18 |
| `wearing` (2 overloads) | `makeSkinWearing` | builds a skin | 17 | Tests/Actors/CowSkinTests.cpp:13 |
| `savedKeys` | `readSavedKeys` | reads | 2 | Tests/Actors/CowSkinTests.cpp:20 |
| `finite` (4 overloads) | `isFinite` | bool | 12 | Tests/Actors/CowTests.cpp:17 |
| `instanceCount` (2 overloads) | `countInstances` | counts | 11 | Tests/Actors/CowTests.cpp:41 |
| `posedCount` | `countPosedParts` | counts | 3 | Tests/Actors/CowTests.cpp:51 |
| `eyePixels` | `countEyePixels` | counts | 3 | Tests/Actors/CowTests.cpp:163 |
| `cowSeenFrom` | `renderCowSeenFrom` | renders | 3 | Tests/Actors/CowTests.cpp:175 |
| `Burst::hearts` | `countHearts` | counts | 4 | Tests/Actors/KissHeartsTests.cpp:18 |
| `looking` | `makeShotLooking` | builds a shot | 8 | Tests/Actors/PropsTests.cpp:51 |
| `ground` (3 overloads) | `projectOntoGround` | drops y | 21 | Tests/Actors/PropsTests.cpp:116 |
| `ahead` (2 overloads) | `getDirectionAhead` | derived value | 4 | Tests/Actors/PropsTests.cpp:121 |
| `floats` | `isFloating` | bool | 4 | Tests/Actors/PropsTests.cpp:126 |
| `ramps` | `isRamp` | bool | 6 | Tests/Actors/PropsTests.cpp:131 |
| `blocksAt` | `findBlocksAt` | finds | 8 | Tests/Actors/PropsTests.cpp:137 |
| `highestTop` | `findHighestTop` | finds | 2 | Tests/Actors/PropsTests.cpp:148 |
| `climbTop` (2 overloads) | `findClimbTop` | finds | 4 | Tests/Actors/PropsTests.cpp:167 |
| `roofEdgeAhead` | `findRoofEdgeAhead` | finds | 2 | Tests/Actors/PropsTests.cpp:190 |
| `barnScenery` | `makeBarnScenery` | builds a value | 7 | Tests/Actors/PropsTests.cpp:206 |
| `shot` | `takeShot` | lambda | 11 | Tests/Actors/PropsTests.cpp:342 |
| `hatParts` | `makeHatParts` | builds a value | 3 | Tests/Actors/WardrobeTests.cpp:32 |
| `dressed` | `makeDressedSkin` | builds a skin | 3 | Tests/Actors/WardrobeTests.cpp:68 |
| `shoot` | `takeSnapshot` | says what it does | 3 | Tests/Actors/WardrobeTests.cpp:85 |
| `constant` | `makeConstantSignal` | builds a signal | 11 | Tests/AudioEngine/SamplePlayerTests.cpp:17 |
| `mix` | `mixVoice` | mix what | 7 | Tests/AudioEngine/SamplePlayerTests.cpp:27 |
| `left` | `getLeftSample` | derived value | 12 | Tests/AudioEngine/SamplePlayerTests.cpp:36 |
| `right` | `getRightSample` | derived value | 4 | Tests/AudioEngine/SamplePlayerTests.cpp:41 |
| `scratchDirectory` | `makeScratchDirectory` | creates it | 3 | Tests/CowsInLove/SettingsTests.cpp:14 |
| `dressedAndChosen` | `makeDressedAndChosenSettings` | builds a value | 5 | Tests/CowsInLove/SettingsTests.cpp:27 |
| `levelOf` | `makeLevelFrom` | builds a level | 5 | Tests/CowsInLove/StagesTests.cpp:14 |
| `inside` (2 overloads) | `isInside` | bool | 9 | Tests/Engine/EditorSnapshotTests.cpp:21 |
| `editorShot` | `snapshotEditor` | renders | 4 | Tests/Engine/EditorSnapshotTests.cpp:28 |
| `within` | `isWithin` | bool | 5 | Tests/Engine/EditorSnapshotTests.cpp:44 |
| `lookingDownMinusZ` | `makeViewLookingDownMinusZ` | builds a matrix | 11 | Tests/Engine/FrustumTests.cpp:11 |
| `anyDrawn` | `isAnythingDrawn` | bool | 6 | Tests/Engine/HudSnapshotTests.cpp:14 |
| `anyDrawnIn` | `isAnythingDrawnIn` | bool | 6 | Tests/Engine/MenuSnapshotTests.cpp:22 |
| `menuShot` | `snapshotMenu` | renders | 4 | Tests/Engine/MenuSnapshotTests.cpp:39 |
| `indicesInRange` | `areIndicesInRange` | bool | 9 | Tests/Engine/MeshTests.cpp:13 |
| `normalsUnitLength` | `areNormalsUnitLength` | bool | 8 | Tests/Engine/MeshTests.cpp:22 |
| `facesOutward` | `doFacesPointOutward` | bool | 5 | Tests/Engine/MeshTests.cpp:31 |
| `cone` | `makeConeProfile` | builds a profile | 2 | Tests/Engine/MeshTests.cpp:48 |
| `settings` | `makeGovernorSettings` | builds a value | 12 | Tests/Engine/QualityGovernorTests.cpp:10 |
| `feed` | `feedFrames` | feed what | 20 | Tests/Engine/QualityGovernorTests.cpp:27 |
| `back` | `makeBackKeyEvent` | builds an event | 7 | Tests/Engine/RootKeyTests.cpp:12 |
| `contentOf` | `getContentOf` | derived value | 2 | Tests/Engine/ScreenWalkTests.cpp:35 |
| `smooth` | `isStepSmooth` | bool | 5 | Tests/Engine/ScreenWalkTests.cpp:40 |
| `same` (6 overloads) | `areSame` | bool | 26 | Tests/Engine/TouchControlsTests.cpp:63 |
| `inBarn` | `makeGameInBarn` | builds a game | 5 | Tests/Game/BarnJumpTests.cpp:38 |
| `foundGame` | `makeFoundGame` | builds a game | 6 | Tests/Game/EndingTests.cpp:14 |
| `openField` | `makeOpenFieldGame` | builds a game | 15 | Tests/Game/GameTests.cpp:22 |
| `Pads::read` | `readControls` | read what | 23 | Tests/Game/PadTests.cpp:25 |
| `Phone::steered` | `hasSteered` | bool | 4 | Tests/Game/PlayTouchTests.cpp:48 |
| `levelWithAGap` | `makeLevelWithAGap` | builds a level | 4 | Tests/Game/RespawnTests.cpp:19 |
| `gameOn` | `makeGameOn` | builds a game | 7 | Tests/Game/RespawnTests.cpp:28 |
| `middleIsClear` | `isMiddleClear` | bool | 2 | Tests/Game/TitleFontTests.cpp:24 |
| `onTheGrid` | `isOnTheGrid` | bool | 2 | Tests/Layout/FitToSizeTests.cpp:21 |
| `labels` | `makeLabels` | builds a tree | 4 | Tests/Layout/FitToSizeTests.cpp:28 |
| `fits` | `doesFit` | bool | 5 | Tests/Layout/FitToSizeTests.cpp:49 |
| `fixedBox` | `makeFixedBox` | builds a node | 10 | Tests/Layout/TreeTests.cpp:33 |
| `textLeaf` | `makeTextLeaf` | builds a node | 6 | Tests/Layout/TreeTests.cpp:41 |
| `check3` | `checkAlignment` | a digit for a name | 4 | Tests/Layout/TreeTests.cpp:113 |
| `BlobWriter::blob` | `getBlob` | returns state | 5 | Tests/MeshLoader/MeshBlobTests.cpp:54 |
| `twoGroups` | `makeTwoGroups` | builds a value | 5 | Tests/MeshLoader/MeshBlobTests.cpp:61 |
| `fileBytes` | `readFileBytes` | reads | 2 | Tests/MeshLoader/MeshBlobTests.cpp:103 |
| `jumpLine` | `makeJumpLineSegment` | builds a segment | 10 | Tests/Support/Fixtures.cpp:17 |
| `meadowFixture` | `makeMeadowFixture` | builds a value | 5 | Tests/Support/Fixtures.h:9 |
| `meadowRavineFixture` | `makeMeadowRavineFixture` | builds a value | 5 | Tests/Support/Fixtures.h:10 |
| `Testing::solidPipeline` | `makeSolidPipeline` | builds a descriptor | 3 | Tests/Support/Snapshot.cpp:21 |
| `Testing::translucentPipeline` | `makeTranslucentPipeline` | builds a descriptor | 2 | Tests/Support/Snapshot.cpp:30 |
| `Testing::glowPipeline` | `makeGlowPipeline` | builds a descriptor | 2 | Tests/Support/Snapshot.cpp:38 |
| `Testing::isClearColor::near` | `isNear` | bool test helper | 4 | Tests/Support/Snapshot.cpp:58 |
| `Testing::snapshot` | `takeSnapshot` | takes it | 16 | Tests/Support/Snapshot.h:66 |
| `southHalf` | `makeSouthHalf` | builds a region | 5 | Tests/World/BiomeTests.cpp:11 |
| `loft` | `makeLoft` | builds a block | 5 | Tests/World/FloatingBlockTests.cpp:30 |
| `underARoof` | `makeLevelUnderARoof` | builds a level | 2 | Tests/World/FloatingBlockTests.cpp:37 |
| `middleHasGrass` | `doesMiddleHaveGrass` | bool | 2 | Tests/World/GrassTests.cpp:23 |
| `chaseView` | `makeChaseView` | builds a matrix | 2 | Tests/World/GrassTests.cpp:117 |
| `seesEverything` | `makeViewSeeingEverything` | builds a matrix | 4 | Tests/World/GrassTests.cpp:123 |
| `distance` | `measureDistance` | lambda | 3 | Tests/World/GrassTests.cpp:155 |
| `levelWithGap` | `makeLevelWithGap` | builds a level | 3 | Tests/World/GroundTests.cpp:12 |
| `mixIn` | `mixIntoHash` | mix into what | 5 | Tests/World/LevelGeneratorTests.cpp:81 |
| `levelHash` | `hashLevel` | hashes | 2 | Tests/World/LevelGeneratorTests.cpp:86 |
| `touchesPath` (2 overloads) | `doesTouchPath` | bool | 4 | Tests/World/LevelGeneratorTests.cpp:124 |
| `levelWithBlock` | `makeLevelWithBlock` | builds a level | 4 | Tests/World/LevelTests.cpp:21 |
| `withBarn` | `makeLevelWithBarn` | builds a level | 4 | Tests/World/LevelTests.cpp:42 |
| `inBarn` | `isInBarn` | bool | 3 | Tests/World/MeadowTests.cpp:19 |
| `ravine` | `findRavine` | finds | 7 | Tests/World/RavineSegmentTests.cpp:31 |
| `bridge` | `findBridge` | finds | 5 | Tests/World/RavineSegmentTests.cpp:36 |
