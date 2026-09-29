# Web plan

Cows In Love in the browser, through eacp. eacp work lives on the `jp/web`
branch (worktree `~/projects/eacp-web`); Cows work on the `web` branch
(worktree `~/projects/Cows-web`, off `android`). "Status" at the end records
what has actually been done.

## Shape of the port

The web is "Linux without a kernel": one thread, no blocking, a loop the
browser owns, a single canvas for a window, and WebGPU for a GPU. Emscripten
compiles the C++ to wasm and supplies libc, a POSIX-ish filesystem in memory
and the JS glue. eacp's view tree, text atlas, `UI` and the shader EDSL are
portable C++ and come along unchanged; what is new is a backend per module
(`-Web.cpp` files beside the `-Linux` / `-Android` ones), a WGSL emitter for the
shader EDSL, and the page.

CMake detail that drives everything: under `emcmake`, `EMSCRIPTEN` is set and
`UNIX` is true (`LINUX`, `APPLE`, `ANDROID` false). As on Android, every
`elseif (UNIX)` branch opens unless an `EMSCRIPTEN` branch comes first; the port
is mostly putting those branches in the right place.

Local builds use the worktree: `CPM_eacp_SOURCE=$HOME/projects/eacp-web`
(`tools/web.sh` does this when the directory exists, and otherwise fetches
`jamierpond/eacp@jp/web`).

## WebGPU, not WebGL2

WebGPU is the target:

- eacp's GPU API is Metal-shaped (command buffers, render pass descriptors,
  pipelines built up front, bind by slot), and WebGPU is the same shape.
  WebGL2 is a state machine and would need an emulation layer under every
  call.
- GPUWidgets and parts of eacp's GPU path use compute; WebGL2 has no compute.
- WebGL2 would need fix-ups everywhere eacp relies on Metal/Vulkan/D3D
  conventions: depth range −1..1 instead of 0..1, flipped Y for render
  targets, no base instance on instanced draws, no storage buffers.
- Reach, September 2026: Chrome and Edge (desktop and Android) since 113,
  Safari 26 (macOS, iOS, iPadOS), Firefox 141+ on Windows, with macOS and
  Linux Firefox following. That is every browser Cows needs; an old browser
  gets eacp's "needs WebGPU" message rather than a slow fallback.

## eacp, module by module

### GPU

- WebGPU through Emscripten's Dawn port (`--use-port=emdawnwebgpu`, which is
  Dawn's `webgpu.h` over the browser's `navigator.gpu`). eacp-gpu carries the
  flag PUBLIC so apps link the JS library.
- Shaders: a WGSL emitter for the shader EDSL (`ShaderEmitter`,
  `WgslBindings.h`, `WgslHelpers.h`), beside the MSL / HLSL / GLSL ones. No
  SPIR-V and no glslang on the web.
- The device is requested before `main()` runs, in the page's `preRun`
  (WebGPU's requests are promises and `main()` cannot wait on one), and handed
  over as `Module.preinitializedWebGPUDevice`.
- The canvas is the swapchain: `GPUView-Web.cpp` configures the canvas context
  with `navigator.gpu.getPreferredCanvasFormat()` (BGRA8 on most, RGBA8 on
  some Android), which eacp's pipelines must match, as on Android.
- MSAA 4× is core WebGPU; depth32float is core. Cows needs nothing optional.

### Core

- The loop is the browser's: `EventLoop-Web.cpp` returns from `main()` into
  the browser and runs posted work from `requestAnimationFrame` / timeouts.
  No ASYNCIFY, so nothing may block the main thread: no `sleep`, no waiting on
  a future, no nested pump.
- `callAfter` and timers are browser timeouts (`CallAfter-Web.cpp`,
  `Timer-Web.cpp`); there are no threads (no `-pthread`), so `std::thread`
  cannot start. `Apps::quit()` tears the app down and the canvas freezes.
- Exceptions are native wasm exceptions (`-fwasm-exceptions`), global:
  every object in the link must be built the same way, which is why Cows
  includes eacp's `WebSetup.cmake` itself (see Cows changes).
- `FilePath-Web.cpp`: MEMFS; nothing persists. Cows saves nothing, so this is
  fine.

### Graphics

- One window per page: `Window-Web.cpp` is the full-window canvas (id
  `canvas`), sized to the viewport times `devicePixelRatio`; `Display-Web.cpp`
  reports the viewport and ratio as the backing scale.
- Views: the Linux view records (`View-Linux.cpp`), with the canvas as the one
  presenting surface. Mouse, wheel, keys and pointer events come from the
  canvas; touch arrives as `View::touchBegan/Moved/Ended`, one `TouchEvent`
  per finger, which is what `TouchControls` already takes.
- Safe area: should come from `env(safe-area-inset-*)` (needs
  `viewport-fit=cover`, which eacp's shell sets); Cows reads it through
  `getSafeAreaInsets`, as on iOS and Android.
- `DisplayLink-Web.cpp`: `requestAnimationFrame`.

### Text

- `GlyphRasterizer-Web.cpp`: Canvas2D on an `OffscreenCanvas` —
  `measureText`, `fillText`, read back with `getImageData`. One glyph per code
  point, like Android: no kerning or ligatures, which Cows' ASCII HUD does not
  need. Font names the browser does not know fall back to `monospace`, so
  "Menlo" becomes the platform's monospace outside Apple.
- The font name is chosen in three places in Cows, not one:
  `UI/Overlay.h` (the footer, Menlo 13) and `UI/TouchControls.h` (Menlo 14 and
  17 bold). A `hudFontName` constant in `UI/Hud.h` would make it one if the web
  wants a named web font.

### Audio

- MakeASound's backend is miniaudio, whose Web Audio backend is built in:
  a ScriptProcessorNode by default, calling the render callback on the main
  thread (so `SamplePlayer`'s mutex never contends); an AudioWorklet with
  `MA_ENABLE_AUDIO_WORKLETS`, `-sAUDIO_WORKLET -sWASM_WORKERS`, which needs
  cross-origin isolation (COOP/COEP) for SharedArrayBuffer — `tools/web.sh
  serve` already sends those headers.
- Autoplay: an AudioContext starts suspended. miniaudio registers `click` /
  `touchend` listeners on `document` (capture phase), not `keydown`, and
  resumes its contexts on the first one. `Settings-Web.cpp` adds a `keydown`
  listener that calls `miniaudio.unlock()` until the context runs, so the
  first tap, click or key unlocks the moo with no start button. A moo made
  while suspended is mixed and plays when the context resumes.
- MakeASound `DeviceManager::start` spawned a recovery `std::thread`, which
  aborts without pthreads. Fixed in the `~/projects/MakeASound` `jp/web`
  working tree (no recovery thread under Emscripten, stereo for Web Audio's
  "any channel count", miniaudio without Threads, the dummy RtMidi API);
  uncommitted there.
- RtMidi: MakeASound always builds it and its find module asks for ALSA on
  every UNIX. Cows answers with its fake `FindALSA` and RtMidi's dummy API, as
  for Android.

## Cows changes

1. **Touch screen at run time.** `touchScreen()` and `canQuit()` in
   `Lib/cowsinlove_Engine/Platform/Device.h`. `Device.cpp` keeps the
   compile-time answer (iOS and Android have touch; everyone can quit);
   `Device-Web.cpp` asks the browser once (`navigator.maxTouchPoints > 0` and
   `(pointer: coarse)`, so a touch-screen laptop keeps mouse and keys) and
   never quits. CMake picks the file; no `__EMSCRIPTEN__` anywhere.
2. **Settings from the URL.** `Settings-Web.cpp` reads the query string and
   sets the environment `getEnvValue` reads:

   | Query | Sets |
   |---|---|
   | `seed=3` | `COWS_SEED=3` |
   | `stage=1` | `COWS_STAGE=1` |
   | `time=12` | `COWS_TIME=12` |
   | `freeze` | `COWS_FREEZE=1` |
   | `found` | `COWS_FOUND=1` |
   | `profile` | `COWS_PROFILE=1` |

   A bare name sets `1`. `COWS_QUERY="seed=3&freeze"` passes it through
   `tools/web.sh serve` and `shot`.
3. **No ALSA.** `CMake/Android/FindALSA.cmake` moved to
   `CMake/NoALSA/FindALSA.cmake`, used for Android and Emscripten, and
   `rtmidi` gets `__RTMIDI_DUMMY__` on both.
4. **Fonts.** Unchanged; see Text.
5. **q / Esc** do nothing on the web (`canQuit()`): on `jp/web`
   `Apps::quit()` destroys the app and leaves a frozen canvas. The footer
   leaves out "q to quit" there (`footerText(..., quitHint)`, from
   `canQuit()`); native text is unchanged.
9. **Audio on keys.** `Settings-Web.cpp` resumes miniaudio's context on the
   first key as well as click / touch (see Audio).
6. **CMake.** The root includes eacp's `WebSetup.cmake` under `EMSCRIPTEN`
   (wasm exceptions for every Cows and MakeASound object, and the variables
   `eacp_web_app` reads, which are otherwise only set in eacp's own
   directory). The app calls `eacp_web_app(Cows)`: `Cows.html` from eacp's
   shell (full-window canvas `canvas`, `touch-action: none`,
   `viewport-fit=cover`, the WebGPU device before `main`), `Cows.js`,
   `Cows.wasm`, plus `Apps/CowsInLove/Web/index.html`, which opens
   `Cows.html` with the query string kept. Tests are not built on the web.
   `CMake/Findeacp.cmake` defaults `COWS_EACP_TAG` to `jp/web` under
   Emscripten and `jp/vulkan-1-1` elsewhere.
7. **Tools.** `tools/web.sh build | serve [port] | shot [png]`, and
   `just web`, `just serve-web`, `just shot-web`. `shot` installs Playwright
   and its Chromium outside the repo (`$TMPDIR/cows-web-tools`), serves on
   8741, opens the page with `--enable-unsafe-webgpu`, waits
   `COWS_SHOT_DELAY` seconds and saves the PNG; with no node or Chromium it
   says so and exits 0.
8. **Static batches uploaded once.** `StaticBatch` (`Render/Instances.h`)
   holds one GPU buffer per mesh; `CowsView` builds one for the level's
   scenery and one for the chasms in `layTerrain` (every new level) and draws
   them through `setInstanceBuffer`, instead of `setInstances` copying the
   whole level into a streaming buffer twice a frame (main and shadow pass).
   CowsArt, which trims the scenery after the level is laid, rebuilds it.
   Pixel-identical: the macOS window at `COWS_SEED=3 COWS_TIME=12
   COWS_FREEZE=1`, stages 0 and 1, differs in 0 pixels before and after.

## Performance risk

- **Grass**: 16 tiles × 14,000 blades = 224k instances of a 16-triangle
  blade, about 3.6M triangles, plus the rest of the scene: roughly 4M
  triangles a frame. Fine on a desktop GPU; a phone browser is where this
  bites. Grass density is not to change; the levers are LOD (fewer segments
  for far tiles), culling tiles behind the camera, and a lower MSAA on phones.
- **Per-frame uploads** that remain: the cows, the sky decor, movers,
  hearts and glows (small), and the grass on levels with gaps. A tile
  crossing a gap is its own blade list, and `drawGrassTile` re-uploads
  whenever the list changes, so every cut tile in view costs a 672 KB upload
  (14,000 × 48 bytes) a frame, and the plain tile another after them. On the
  ravine that is a few MB a frame through `writeBuffer`. The fix is the same
  as for the scenery: one buffer per cut tile, built in `layOver`.
- **Main-thread everything**: simulation, encoding, audio mixing (the
  ScriptProcessor) and the browser's own work share one thread. Profile with
  `?profile` before moving audio to a worklet.
- **wasm size and startup**: Release links with LTO; check the transfer size
  (brotli) once it renders.

## Status (2026-09-29)

Done on `web`, against eacp `jp/web` (eyalamirmusic/eacp#73, stacked on
`jp/android-integration`) and MakeASound `jp/web`
(eyalamirmusic/MakeASound#2), both pushed to the jamierpond forks and the
defaults in `CMake/Findeacp.cmake` and `CMake/FindMakeASound.cmake`:

- Everything under Cows changes above.
- macOS Release: builds warning-free; `ctest` 142/143. The one failure,
  `LevelGenerator/meadowTemplateIsTodaysMeadow`, is Release-only (passes in
  Debug): its golden hashes are exact float sums, which Release's
  optimisation changes. Not a web issue.
- Web: `tools/web.sh build` links `Cows.js` (145 KB) and `Cows.wasm`
  (1.2 MB) warning-free. Headless Chromium (Playwright, Apple Metal adapter),
  no page errors.
- Parity, `seed=3&freeze&time=20`, 2560x1600 drawable both sides (web at
  1280x800 CSS, devicePixelRatio 2; the macOS window's content, converted
  from the display profile to sRGB): above the footer 0.0024% of pixels
  differ by more than 1% (92 px), max 26/255, mean 0.4/255. Colour, haze,
  shadows, MSAA and grass match; both drawables are BGRA8Unorm with the
  tone curve in the shader, so no sRGB mismatch. The footer differs only by
  " -  q to quit". `docs/shots/web-chromium.png`, `mac-for-web.png`,
  `web-diff.png` (×10). A raw `screencapture` is in the display's profile:
  convert it to sRGB before comparing.
- Play: hold `w`, drag, `m`, space: the cow walks, the camera turns, the
  moo hint shows. `?profile`: 60.0 fps (vsync), frame p95 ~16.8 ms, CPU
  update+gather+shadows+scene+hud ~0.3 ms, GPU ~0.35 ms. Headless on a Mac
  is not representative (and Chrome coarsens timestamp queries); profile on
  a phone.
- Audio: the ScriptProcessor callback runs once the context resumes; a
  moo peaks at 0.60. The context resumes on click, and now on the first key.
- Stage 1: `docs/shots/web-ravine.png`.
- Touch: 393x852 at devicePixelRatio 3, `hasTouch`/`isMobile`: the probe
  says touch, stick and Moo / Jump show, footer is the touch line. With CDP
  `Emulation.setSafeAreaInsetsOverride` (59 top, 34 bottom) the controls
  and footer move up by the bottom inset. `docs/shots/web-phone.png`.

Left:

1. The eacp and MakeASound PRs landing upstream, then the find modules go
   back to upstream and `CMake/NoALSA` can go (MakeASound#2 gives rtmidi its
   dummy backend on Android and the web).
2. eacp's shell titles the page "eacp" on a black background; an
   `eacp_web_app` `TITLE` / background option would let Cows say "Cows In
   Love" and paint the sky colour.
3. Miniaudio's ScriptProcessorNode is deprecated; an AudioWorklet build
   needs `-sAUDIO_WORKLET -sWASM_WORKERS`.
4. Real phones (touch, safe area, the coarse-pointer probe on iPad), grass
   uploads per cut tile, profiling on a phone.
