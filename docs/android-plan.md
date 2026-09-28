# Android plan

Cows In Love on Android, through eacp. eacp work lives on the `android-mvp`
branch (worktree `~/projects/eacp-android`); Cows work on the `android` branch.
"Status" at the end records what has actually been done.

## Shape of the port

Android is "Linux without Wayland": eacp's Linux backend already has most of
what Android needs — a POSIX `Core`, a Vulkan backend with runtime GLSL→SPIR-V
(glslang), and a view layer built around an opaque `ViewSurface` record that a
presenter (`GPUView`) draws into. The Android port replaces the Wayland pieces
with `ANativeWindow`, `ALooper`, `AInputQueue` and `AChoreographer`, and leaves
the rest alone.

CMake detail that drives everything: with the NDK toolchain `CMAKE_SYSTEM_NAME`
is `Android`, so `LINUX` is false but `UNIX` is true. Every `if (LINUX)` gate
stays closed and every `elseif (UNIX)` / `else ()` branch opens. The port is
mostly deciding, file by file, which of those two it wants.

## eacp, module by module

### App entry and event loop (Core)

- **NativeActivity + `android_native_app_glue`**, not GameActivity. No Java
  code, no AAR, no Gradle needed; the APK is a manifest plus
  `lib/<abi>/libX.so`. GameActivity buys better IME/text input and inset
  callbacks, which Cows does not need; revisit if a text field ever matters.
- Entry: the glue calls `android_main(android_app*)` on its own thread. eacp
  supplies `android_main` (a C file, since only C may call `main`): it stores
  the `android_app*`, points `HOME` / `XDG_*` at the activity's internal and
  cache directories (so `FilePath-Linux.cpp` and the Vulkan pipeline cache work
  unchanged), and calls the app's ordinary `main()`. `Apps::run<T>()` then
  works as on every other platform. The app target is `add_library(SHARED)`
  instead of `add_executable`, linked with `-u ANativeActivity_onCreate`.
- Loop: `EventLoop-Linux.cpp` blocks in `poll()`, and nothing Android delivers
  (glue commands, the input queue, choreographer callbacks) is a plain fd. So
  `EventLoop-Android.cpp` is the same loop over `ALooper_pollOnce`: the waker
  pipe and `addLoopSource` fds are `ALooper_addFd`'d, and glue sources
  (`LOOPER_ID_MAIN` / `LOOPER_ID_INPUT`) are handed to a hook the Graphics
  layer installs. Timers and `callAfter` are thread-based and portable.
- Logging: `Logging-Android.cpp` sends each line to logcat
  (`__android_log_write`, tag `eacp`); stdout goes nowhere on Android.
- Process lifetime: Android may destroy and recreate the activity inside one
  process (rotation, config changes); `android:configChanges` in the manifest
  keeps Cows on one activity instance. A second `android_main` in the same
  process is out of scope for the MVP.

### Graphics (Window, View, Display, DisplayLink)

- `Window-Android.cpp`: one window per activity, full screen. It follows the
  glue's commands: `INIT_WINDOW` gives the content view its surface,
  `TERM_WINDOW` takes it away (`onLost` → swapchain destroyed), `WINDOW_RESIZED`
  / `CONFIG_CHANGED` resize, `PAUSE` / `RESUME` / `LOST_FOCUS` pause rendering
  and audio, `DESTROY` quits. Most `Window` methods are iOS-style no-ops.
- `View-Android.cpp`: the portable view tree as on Linux, minus subsurfaces.
  Android gives one surface per window, so the first presenting view (the
  `GPUView`) gets the `ANativeWindow`. More than one presenting view per window
  would need `ASurfaceControl` child surfaces (API 29+) — not needed by Cows.
- Backing scale: `AConfiguration_getDensity / 160`. Points = pixels / scale.
- Safe areas: NativeActivity exposes no insets in C. Options: go immersive
  full-screen (`FLAG_FULLSCREEN` via `ANativeActivity_setWindowFlags`) and use
  a conservative inset, or read `WindowInsets` over JNI (display cutout,
  gesture nav). JNI is ~60 lines and the honest answer for Play.
- `DisplayLink-Android.cpp` and GPU view pacing: `AChoreographer` (vsync) in
  place of the Linux clock thread and `wl_surface.frame`.
- `Display-Android.cpp`: `ANativeWindow` size and density.
- Keyboard, menus, tray, hot keys, image codecs, system appearance: the Linux
  stubs apply unchanged (they compile against no Wayland), except
  `Keyboard-Linux.cpp`, which reads the Wayland seat; Android gets the iOS
  stubs.

### GPU (Vulkan backend)

- The Vulkan backend is already portable C++ over volk; nothing links
  libvulkan, `volkInitialize()` dlopens it — on Android that is
  `libvulkan.so`, which exists on every API 24+ device.
- Linux-specific parts are small: `VK_USE_PLATFORM_WAYLAND_KHR` in
  `CMake/FindVulkanBackend.cmake`, `VK_KHR_wayland_surface` in instance
  creation (`VulkanContext-Linux.cpp`), and `vkCreateWaylandSurfaceKHR` in
  `GPUView-Linux.cpp`. Android swaps in `VK_USE_PLATFORM_ANDROID_KHR`,
  `VK_KHR_android_surface`, `vkCreateAndroidSurfaceKHR`.
- Surface transforms: Android reports `currentTransform` (ROTATE_90 etc.) and
  expects either pre-rotated rendering or `IDENTITY` with the compositor paying
  for the rotation. The Linux code passes `currentTransform` as `preTransform`,
  which on a rotated Android device means the image comes out sideways. MVP:
  lock orientation in the manifest and use IDENTITY; later pre-rotate in the
  projection matrix (a perf win on real devices).
- Swapchain formats: Android surfaces offer `R8G8B8A8_UNORM` first, rarely
  `B8G8R8A8`. eacp pipelines default to BGRA8 and render passes take the
  swapchain format; any pipeline built with the default must match what the
  swapchain picked. Check first.
- Feature floor: eacp requires Vulkan 1.3 with timeline semaphores,
  synchronization2, dynamic rendering, partially bound descriptors and
  format-less storage writes. **This is the biggest device-coverage risk.**
  Android 13+ flagship GPUs (Adreno 7xx, Mali-G7xx, Pixel) report 1.3; many
  2020–2022 phones stop at 1.1. The emulator on this Mac (gfxstream over
  MoltenVK, "Apple M5 Max") reports 1.3 with every one of them. Play's device
  catalogue can filter on `android.hardware.vulkan.version` 1.3
  (`0x403000`) in the manifest, which is the honest first release; a 1.1
  fallback is a large eacp change (render passes instead of dynamic rendering,
  binary semaphores).
- Shaders: compiled at run time from GLSL by glslang (`eacp-spirv`), exactly
  as on Linux; `EACP_BUILD_SPIRV` must default on for Android. Costs ~5 MB of
  .so and the first-frame compile (warmed by `Spirv::warmUp`); the
  `VkPipelineCache` persists under `$XDG_CACHE_HOME/eacp`, which the entry
  point points at the app's cache dir. Build-time SPIR-V is a later
  optimisation, not a requirement.

### Input

- `AInputQueue` motion events → the portable `MouseEvent` path for pointer 0
  (as `View-iOS.mm` does with `[touches anyObject]`), so eacp widgets work.
- eacp has **no multi-touch API**. Cows gets multi-touch on iOS from its own
  `TouchSurface` UIView. On Android the same role is a hook on the Android
  window: eacp exposes the raw `AInputEvent*` (or a small `TouchEvent` with
  pointer id, phase, position in points), and Cows' `Platform/Android`
  `TouchSurface` turns it into `TouchControls::pointerDown/Moved/Up(id, pos)`.
  A portable `View::touch...` API in eacp is the long-term answer.
- Back button: `AKEYCODE_BACK` → Cows' Esc/quit path (or ignored in-game and
  moved to "pause" per Play guidance).

### Text and fonts

- eacp `Text` (glyph atlas) is FreeType + HarfBuzz + fontconfig on Linux, via
  pkg-config. Android has no pkg-config and no fontconfig: FreeType and
  HarfBuzz would be CPM-fetched and built from source, and the fontconfig
  calls (match, list, fallback sort, coverage) replaced by a small resolver
  over `/system/fonts` (`AFontMatcher`, API 29+) and `FT_Get_Char_Index`.
  Sizeable (~2–3 days) but not needed for Cows.
- **What Cows actually needs is the 2D tier**: the footer (`FooterView`) and
  the touch HUD (`TouchControls`) paint through `Graphics::Context`
  (`strokePath`/`fillPath` of circles, `drawText` in Menlo). eacp has no
  `Context` off Apple/Windows (`EACP_HAS_CONTEXT` is off on Linux and would be
  on Android), and Android has one surface, so there is no native layer to
  composite a 2D view onto anyway. Options:
  1. Draw the HUD in the Cows GPU frame on Android (SDF rings/discs + labels).
     Smallest, Cows-only. Labels need glyphs: Cows' tube font, or a small
     baked bitmap font.
  2. A software `Context` on Android rendering views to an `Image` that the
     GPUView composites as a texture. General, large.
  3. eacp `UI` on the GPU (needs `Text`, see above) and port the HUD to it.
  Done: option 2, smaller than feared. eacp's `SoftwareContext` (Android
  only) paints the two views into an `Image` and Cows' `HudLayer` draws it as
  one sprite over the scene; 3 is still the long-term answer.
- **Dependency**: FreeType, upstream `freetype/freetype` tag `VER-2-13-3`,
  fetched by CPM (`CMake/FindAndroidFreeType.cmake` → `eacp-freetype`,
  PRIVATE to `eacp-graphics`, Android only), built with zlib, bzip2, png,
  HarfBuzz and brotli disabled so it cross-compiles for the NDK with no
  system packages. Not vendored. The same library the Linux text path uses,
  so eacp `Text` on Android can reuse it.

### Files and resources

- `res_embed` is compiled into the binary, so it works unchanged; no
  `AAssetManager` needed (moo sample, embedded shaders, AppInfo).
- `FilePath` home/config/cache directories come from `HOME` / `XDG_*`, set by
  the entry point to `activity->internalDataPath` and the cache dir.

### Audio

- MakeASound's backend is miniaudio 0.11.25, which has AAudio and OpenSL ES
  built in. MakeASound itself has no Android branch (it falls into the
  `AudioSession-Default` path, which is right).
- **Blocker to fix**: MakeASound always links RtMidi, whose find module treats
  Android as UNIX and does `find_package(ALSA REQUIRED)`. Needs an Android
  branch (RtMidi's `__AMIDI__` backend or a dummy build) in MakeASound — or,
  on the Cows side, not pulling MIDI on Android.
- Lifecycle: stop the device on `APP_CMD_PAUSE`, start it on `RESUME`; audio
  focus is ignorable for a game with short samples.

### Threading, timers

`Timer-Linux.cpp`, `CallAfter`, `ThreadUtils-Linux.cpp` are pthread-based and
portable. The "main thread" is the glue thread, not Java's UI thread;
`initMainThread()` runs in `EventLoop::run` on it.

### CMake and toolchain

- NDK r27 (`27.3.13750724`), `-DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/
  android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=
  android-33` (Vulkan 1.3 loader). ABIs: arm64-v8a (devices, the emulator on
  Apple Silicon), x86_64 (the emulator on Intel hosts), one build dir each.
- `ANDROID` guards: capability variables (`EACP_HAS_DRAW`/`GPU` on,
  `TEXT`/`CONTEXT` off at first), Core/Graphics/GPU source lists, the Vulkan
  platform define, `EACP_BUILD_SPIRV` default.
- Packaging without Gradle: `aapt2 link` (manifest + `android.jar`) → add
  `lib/arm64-v8a/libcows.so` with `zip` → `zipalign` → `apksigner` with a
  debug keystore. Gradle (AGP + `externalNativeBuild`) is only needed for the
  AAB; `bundletool` can also build an AAB from the same pieces.

### Play packaging

- Google Play needs an **AAB**, targetSdk ≥ 35 (Aug 2025 rule), 64-bit libs,
  16 KB page-size aligned `.so`s (NDK r27+: `-Wl,-z,max-page-size=16384`,
  default in r28), an upload key (Play App Signing holds the app key),
  a privacy policy URL, content rating, data-safety form.
- `Deploy/Google-Play/` in the style of `Deploy/Apple-iOS/`: listing text
  (`Metadata/play-store.md`), screenshots (phone 16:9 / 9:16), feature
  graphic 1024×500, icon 512×512, and the `bundletool`/Gradle recipe.

## Cows-side work

- `Lib/cowsinlove_Engine/Platform/Android/`: `Platform.h/.cpp` with the iOS
  interface (`touch = true`, `attach(root, touchControls, footer)`), a
  `TouchSurface` over the Android input hook feeding
  `TouchControls::pointerDown/Moved/Up`, insets → `touchControls.safeArea` and
  `footer.bottomInset`, pause/resume → audio.
- Engine CMake: `elseif (ANDROID)` picks `Platform/Android`.
- App CMake: `add_library(Cows SHARED ...)` on Android, `-u
  ANativeActivity_onCreate`, no bundle properties; APK via `tools/android.sh`.
- HUD: see Text and fonts — drawn in the GPU frame on Android.
- `CMake/Findeacp.cmake`: `COWS_EACP_TAG` (default `main`) so the branch can
  pin `android-mvp`; `CPM_eacp_SOURCE` for the local worktree.
- `tools/android.sh sim [shot.png]` and `just sim-android`.

## Phases (rough sizes, one engineer)

| Phase | What | Size |
|---|---|---|
| 0 | SDK/NDK/emulator on the Mac | 0.5 h |
| 1 MVP | Core + Graphics + GPU build for arm64; NativeActivity entry; a GPUView clears to a colour through Vulkan; touches in logcat | 1–2 d |
| 2 | Triangle/GPU examples through the eacp GPU API; swapchain format and transform handled | 0.5–1 d |
| 3 | Cows renders: shared lib, audio (MakeASound RtMidi fix), res_embed | 1–2 d |
| 4 | Input + HUD on GPU, safe areas (JNI insets), back button, pause/resume, surface loss | 2–3 d |
| 5 | Play: AAB, signing, 16 KB pages, store listing, device testing on real Adreno/Mali | 2–3 d |
| later | eacp Text on Android (FreeType/HarfBuzz, font resolver), multi-touch in eacp, Vulkan 1.1 fallback | 1–2 w |

## Risks, and what to check first

1. **Vulkan 1.3 floor** (dynamic rendering, sync2, timeline semaphores).
   Checked first on the emulator: `adb shell cmd gpu vkjson` → 1.3.0 with all
   five features. Real devices: needs a Play device-catalogue check.
2. **Swapchain format / transform** mismatches (RGBA vs BGRA, rotation).
3. **2D HUD**: no `Context` on Android (see above).
4. **Shader pipeline**: glslang at runtime is proven on Linux; on Android it
   is size and first-frame time, not correctness.
5. **Emulator GPU**: gfxstream over MoltenVK; driver bugs there are not device
   bugs. Test on a real phone before trusting a visual difference.
6. **MakeASound → RtMidi → ALSA** configure failure.

## Status (2026-09-28)

**Cows In Love runs on the Android emulator**: meadow, cow, grass, shadows,
fog, the touch HUD and footer, multi-touch stick/Moo/Jump, the moo through
AAudio, home-and-back (surface lost and rebuilt), back quits.
Screenshots: `docs/shots/android-mvp.png` (eacp Hello: Vulkan clear following
the finger), `docs/shots/android-cows.png` (the game). `docs/shots` is
gitignored, as for the other shots.

### Toolchain on this Mac

- `brew install openjdk@21` (the `temurin` cask needs sudo) and
  `brew install --cask android-commandlinetools` (sdkmanager 23.0).
- `ANDROID_HOME=~/Library/Android/sdk`,
  `JAVA_HOME=/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home`:
  `sdkmanager --sdk_root=$ANDROID_HOME --licenses`, then
  `sdkmanager "cmdline-tools;latest" platform-tools "platforms;android-35"
  "build-tools;35.0.0" "ndk;27.3.13750724" emulator
  "system-images;android-35;google_apis;arm64-v8a"`.
- Versions: NDK r27d (27.3.13750724), build-tools 35.0.0, platform 35,
  emulator 37.1.11, adb 37.0.1, system image android-35 google_apis arm64 r9.
  Sizes: NDK 2.4 GB, system image 3.8 GB, emulator 1.1 GB, rest ~0.5 GB.
- AVD: `avdmanager create avd -n cows -k
  "system-images;android-35;google_apis;arm64-v8a" -d pixel_7` (1080×2400,
  420 dpi). Emulator GPU: gfxstream over MoltenVK, Vulkan 1.3.0 with every
  feature eacp needs (checked with `adb shell cmd gpu vkjson`).

### How to run

`just sim-android [shot.png]` (= `tools/android.sh sim`): configures
`build-android/` against `EACP=~/projects/eacp-android` (NDK, arm64-v8a,
android-29), builds `Cows-apk`, boots the `cows` AVD if nothing is attached,
installs and launches `com.cowsinlove.cows`. `COWS_EACP_TAG` in
`CMake/Findeacp.cmake` can pin a fetched eacp revision once `android-mvp` is
pushed; until then the local worktree (`CPM_eacp_SOURCE`) is the only way.

### eacp `android-mvp` (worktree `~/projects/eacp-android`, off 50b16ad)

- Core: `EventLoop-Android.cpp` (the Linux loop over `ALooper_pollOnce`,
  with a hook for looper events it does not own), `Logging-Android.cpp`
  (logcat, tag `eacp`), Network left out (no libcurl in the NDK),
  `posix_spawn_file_actions_addchdir_np` guarded below API 34,
  `Platform::isDLL()` false on Android (the app *is* a .so).
- Graphics: `Window-Android.cpp` (glue commands, touches → `Android.h` hook +
  MouseEvents for pointer 0, back → Escape, insets from `WindowInsets` over
  JNI with the content rect as fallback), `View-Android.cpp` (Linux view tree
  without subsurfaces; first presenting view gets the `ANativeWindow`,
  AChoreographer pacing), `DisplayLink-Android.cpp`, `Display-Android.cpp`,
  `Keyboard-Android.cpp`, `AndroidMain-Android.c` (android_main → HOME/XDG →
  `main()`), the NDK glue compiled in. `EACP_HAS_DRAW`/`GPU` on for Android.
- GPU: `VK_KHR_android_surface`; `eacp-spirv` on by default (glslang at run
  time). Nothing else in the Vulkan backend needed changing; the swapchain
  comes out BGRA8, matching eacp's default pipeline format.
- `SoftwareContext` + `Font`/`TextMetrics` on Android: a CPU `Context` into a
  straight-alpha RGBA8 `Image` (non-zero fills with 16× vertical
  supersampling, strokes as quads + round joins), glyphs from `/system/fonts`
  (Menlo/Mono → DroidSansMono with synthetic bold; else Roboto/DroidSans).
  Rasteriser: FreeType 2.13.3 fetched by CPM (`CMake/FindAndroidFreeType.cmake`,
  `eacp-freetype`), light hinting, `FT_GlyphSlot_Embolden` for synthetic bold.
- Packaging: `eacp_add_android_apk()` (`CMake/AndroidApk.cmake`,
  `AndroidManifest.xml.in`, `Scripts/android-apk`): aapt2 → zip →
  `zipalign -P 16` → apksigner (debug keystore), no Gradle. Manifest requires
  Vulkan 1.3, NativeActivity, `configChanges` so rotation never recreates it.
- `Apps/Android/Hello`: clear through Vulkan, log touches, SoftwareContext
  overlay as a sprite.

### Cows `android` branch

- `Platform/Android/`: `Platform` (touch = true, `attach`, `drawOverlay`),
  `TouchSurface` (pointer ids from 1, as on iOS; safe area → controls and
  footer), `HudLayer` (paints `TouchControls` and `FooterView` through
  `SoftwareContext` only when their visible state changes, uploads, draws as a
  sprite at the end of `CowsView`'s pass). `drawOverlay` is a no-op on
  macOS/iOS.
- `CMake/Android/FindALSA.cmake` + `rtmidi` forced to `__RTMIDI_DUMMY__`: the
  MakeASound → RtMidi → ALSA configure failure, worked around in Cows. The
  real fix belongs in MakeASound's `FindRTMidi.cmake`.
- App: shared library on Android, APK via `eacp_add_android_apk`, launcher
  icon from the iOS icon at five densities (`Apps/CowsInLove/Android/res`).

### Google Play (branch `play-store`)

`just release-android` (`tools/release-android.sh`) builds Release
arm64-v8a and x86_64 `.so`s (both build and run; the Release .so is 5.9 MB
stripped), and packages a signed `.aab` without Gradle: `aapt2 link
--proto-format`, the base module zip, `bundletool build-bundle` 1.18.3 with
`Deploy/Google-Play/BundleConfig.json` (uncompressed native libs, 16 KB
aligned; the .so is linked with 16 KB pages through
`ANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES`), symbol tables in `BUNDLE-METADATA`,
`jarsigner` with the upload key. `--install` validates, installs through
`bundletool --local-testing` and launches (`docs/shots/play-aab.png`).

- Manifest: Cows' own template, `Apps/CowsInLove/Android/AndroidManifest.xml.in`
  (eacp's plus `isGame`, `appCategory="game"`, `allowBackup="false"`,
  `hasFragileUserData="false"`, touchscreen and portrait features, every
  screen size). versionName = project version, versionCode =
  `COWS_BUILD_NUMBER`, targetSdk 35.
- **minSdk 29** (Android 10): eacp's frame pacing calls
  `AChoreographer_postFrameCallback64`, API 29; nothing else in the .so needs
  more than 24. The real floor is the Vulkan 1.3 `uses-feature`, which Play
  filters on. `tools/android.sh` builds at 29 too.
- `allowBackup="false"`: the game saves nothing; the only file it writes is
  the Vulkan pipeline cache.
- Icon: adaptive only (minSdk ≥ 26), from the CowsArt icon render framed for
  the 66dp safe zone, sky background, monochrome hearts. The legacy PNGs are
  gone.
- Listing, Data safety, IARC and target audience answers:
  `Deploy/Google-Play/Metadata/store-listing.md`. Tablets supported, with
  tablet screenshots.

### Deviations from the other platforms, on purpose

- HUD text is DroidSansMono (synthetic bold for Menlo-Bold), not Menlo.
- The HUD is a CPU-painted texture composited in the scene's pass, not
  native layers.
- Back quits (same as Esc on desktop). Play prefers back to leave the game to
  the launcher, which is what quitting does here.

### Not done yet, in order

1. Audio pause on `APP_CMD_PAUSE` (the AAudio stream keeps running silently in
   the background) — Cows needs a hook to its `SamplePlayer`.
2. Real devices: Adreno/Mali Vulkan 1.3 behaviour, `preTransform` on a rotated
   device (Cows is portrait-locked, so IDENTITY in practice), 16 KB pages.
3. eacp upstreaming: split `OS::Android` out of `OS::Linux`, a portable
   multi-touch API on `View`, eacp `Text` on Android (FreeType + HarfBuzz +
   an `AFontMatcher` resolver), a second `android_main` in one process.

### Estimate of what is left

| Phase | Left |
|---|---|
| MVP, triangle-equivalent, Cows rendering | done |
| Input/audio/lifecycle polish (audio pause, real-device insets, rotation) | 1–2 d |
| Real-device testing (2–3 phones), perf, Release size | 1–2 d |
| Play packaging (AAB, signing, listing) | done; review is Jamie's upload |
| eacp upstream review/cleanup of `android-mvp` | 2–4 d |
