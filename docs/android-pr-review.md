# eacp Android branch: PR review

Branch `android-mvp` in `~/projects/eacp-android`, 12 commits on `50b16ad`
(#57), reviewed as committed at `367d3561` plus the uncommitted working tree as
of 2026-09-28. The working tree deletes SoftwareContext, Font/TextMetrics-Android,
FindAndroidFreeType and the Hello overlay. That change is staged but not
committed.

Merge bar (Jamie): one PR is fine. There must be at least one Android demo app
in the repo that runs, and a fresh clone with the documented toolchain must
build and run it with one command.

## Verdict

**Update (2026-09-28, after the rebase): items 4 and 8 are done.**
`android-mvp` is rebased onto `upstream/develop` (19 commits, head
`61335b26`, pushed). The conflicts (Core/CMakeLists, View-Linux.h,
FindVulkanBackend, GPUView-Linux, VulkanContext-Linux, README, CLAUDE.md,
top-level CMakeLists) were resolved to develop's structure. Android now
presents through `NativeSurfaceHandle::Kind::Android` and a
`ViewSurfaceBackend` under the shared `View-Linux.cpp`, so `View-Android.cpp`
is gone. `EventLoop-Android.cpp` is only the ALooper wait under the shared
epoll loop. The Vulkan surface code is split into `VulkanSurface-Linux.cpp`
and `VulkanSurface-Android.cpp`. `OS::Android`/`isAndroid()` exist.
`FilePath-Android.cpp` replaces the `$HOME`/XDG trick. Below API 34,
`Process` fails the launch rather than ignoring `workingDirectory`. No
`__ANDROID__` is left outside `Platform.h` and `-Android` files. The macOS
ctest failures match plain develop (`DynamicLibrary/unloadWithNoLoop`
fails on both). HelloGPU and Cows run on the emulator. The original review
follows.

**Mergeable after fixes, as one PR.** The code is in good shape. It is
clang-format clean, builds with zero warnings, has fewer comments than the
existing Linux files, and uses the platform text engine (android.graphics over
JNI), not FreeType. `AndroidHello` builds and runs on the `cows` emulator
(checked: cold start 1.5 s, Vulkan clear, text from android.graphics). Three
things stop it meeting the bar, and one more thing Eyal will push back on:

1. **A fresh clone does not build.** Tests are on by default at top level
   (`EACP_ENABLE_TESTS` = `PROJECT_IS_TOP_LEVEL`). A plain
   `cmake -B b -DCMAKE_TOOLCHAIN_FILE=… && cmake --build b` fails in 5 targets:
   `Tests/Network` (`eacp/Network/Network.h` not found, since Network is not
   built on Android), `UITests` and `ScriptHostTests` (`-leacp-network` not
   found), and `CoreTests` (`FilesTests-Posix.cpp:127`: no `std::jthread` in
   NDK r27 libc++). The APK only builds because everyone passes
   `-DEACP_ENABLE_TESTS=OFF`, and that flag is written down nowhere.
2. **There is no one-command build+run, and no docs.** eacp has no README
   section, no toolchain list and no install/launch step. `README.md:204` still
   says "Android is not supported", the platform table has no Android column,
   and `CLAUDE.md`'s capability paragraph says `APPLE OR WIN32 OR LINUX`. The
   whole working recipe (sdkmanager packages, AVD, `adb install`,
   `am start`) lives in Cows (`docs/android-plan.md:255-276`,
   `tools/android.sh`).
3. **The demo is not a proper eacp example yet.** Its text is Cows HUD copy
   ("Moo Jump Again", "wasd / hjkl to walk - q to quit", `Main.cpp:84-86`).
   `Apps/CMakeLists.txt` also returns early on Android, so none of the 28
   `Apps/GPU` examples build there. iOS gets every example as a bundle for
   free through `set_default_target_setting` and the Info.plist template.
4. **The base is stale (what Eyal will object to).** `upstream/develop` (the
   default branch; `origin` is Jamie's fork, where `main` is `50b16ad`) has 5
   merged PRs and +61k lines since the base. A merge conflicts in 5 files:
   `FindVulkanBackend.cmake`, `Core/CMakeLists.txt`, `GPUView-Linux.cpp`,
   `VulkanContext-Linux.cpp` and `View-Linux.h`. Develop replaced
   `ViewSurface{wl_display*, wl_surface*}` with an opaque
   `NativeSurfaceHandle{Kind::None/Wayland/X11}` and added a
   `ViewSurfaceBackend` seam. The branch's `#if defined(__ANDROID__)` inside
   `ViewSurface` has to become `Kind::Android`. Eyal wrote that seam so a third
   window system could plug in without ifdefs.

Eyal's review record: none of the last 40 merged PRs has a review comment or a
review, and that includes Jamie's (#40-#50). He merges his own large ports whole
(#48 Linux: 24,941 lines, 179 files, 23 commits; #59 X11: 16,943 lines, 85 files,
100 commits), with a long summary PR body, and the README/CLAUDE.md updated in
the same PR. PRs are squash-merged ("… (#57)"), so the branch's add-then-delete
history (SoftwareContext, FreeType) disappears. What he will look for instead:
docs updated, CI green, platform logic behind the existing abstractions, and no
app-specific leakage.

## Size

| | + | - | files |
| --- | ---: | ---: | ---: |
| android-mvp HEAD (committed) | 4,057 | 21 | 43 |
| android-mvp working tree (after the staged deletions) | 3,021 | 21 | 37 |
| #57 NativeChildSurface | 641 | 1 | 7 |
| #56 GPU gaps | 4,098 | 227 | 54 |
| #59 Linux X11 | 16,943 | 1,752 | 85 |
| #48 Linux Wayland/Vulkan/text | 24,941 | 1,359 | 179 |

About 3k lines is mid-sized for eacp and well under the two Linux ports. One PR
is fine.

Commit messages already follow Eyal's `Area: sentence` form ("Core: build for
Android", "GPU: the Vulkan backend presents to an ANativeWindow"), with bodies
that explain why.

## The demo and the run path (the merge bar)

What exists:
- `Apps/Android/Hello`: a `SHARED` library, `GPUView` clear through Vulkan,
  touches logged, colour follows the finger, 5 lines of text through
  `Text::TextRenderer`. `eacp_add_android_apk(AndroidHello PACKAGE
  com.eacp.hello LABEL "eacp Hello")` gives it `AndroidHello-apk` (in ALL), a
  17.8 MB debug APK with no Gradle (`Scripts/android-apk`: aapt2, zip,
  zipalign -P 16, apksigner with a generated debug keystore).
- Verified here: configure plus `cmake --build` produces the APK with 0
  warnings. `adb install` plus `am start -n com.eacp.hello/android.app.NativeActivity`
  renders correctly on the `cows` AVD (API 35, arm64, gfxstream).

What a reviewer has to do today, from a fresh clone:
1. Guess the toolchain: SDK, NDK 27.3.13750724, build-tools 35.0.0,
   platforms;android-35, emulator, an arm64 system image, a JDK (for
   keytool/apksigner). None of this is in eacp.
2. Guess `-DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake
   -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-33 -DEACP_ENABLE_TESTS=OFF`.
   Without the last flag the build fails. Without `ANDROID_PLATFORM` the NDK
   defaults to API 21, and nothing stops a build below the APIs the code uses
   (see minSdk below).
3. Create an AVD, boot it, then `adb install` and `am start` by hand. No target
   or script does this.
4. Know the logcat tag is `eacp`.

Compared with the other examples:
- Every desktop example is `add_executable` plus `set_default_target_setting`,
  and on iOS that same call applies `CMake/iOSBundleInfo.plist.in`. Nothing is
  per-app. Android instead has a separate `Apps/Android` tree, and
  `Apps/CMakeLists.txt:3-8` `return()`s before GPU/UI.
- No example has a README. They are documented in `CLAUDE.md`/`README.md`
  prose, so Android needs the same treatment.
- `Platform::current()` returns `OS::Linux` on Android (`Platform.h:33`), so
  shader picks like `Triangle`'s `isLinux() ? "Triangle.glsl"` already work.
  The blocker to running `Apps/GPU/Triangle` is only `add_executable` vs
  `SHARED`.

What would meet the bar:
- `Scripts/android-run <target> [--emulator <avd>]`, or a CMake
  `<target>-run` custom target added by `eacp_add_android_apk`: `adb install
  -r`, `am start -n <pkg>/android.app.NativeActivity`, then
  `adb logcat -s eacp`. Boot the AVD if nothing is attached (lift
  `boot_emulator` from Cows `tools/android.sh`).
- A README "Android" section:
  1. The `sdkmanager` line and the `avdmanager create avd` line (both in Cows'
     `android-plan.md`).
  2. The configure line.
  3. `cmake --build build-android --target AndroidHello-run`.
  4. Expected output.
  5. Vulkan 1.3 as the device floor.
- A CMake preset, or a documented cache default, so tests are off or pass on
  Android.
- Hello rewritten as neutral eacp copy ("Hello from eacp on Android", the
  device name, touch count). Better: make `Triangle` and `GlyphAtlas` build as
  APKs through an `eacp_add_app()` helper (executable elsewhere, SHARED + APK on
  Android), so Android shows up the way iOS does.

## Style findings

**clang-format:** `clang-format` 21.1.8 over all 13 Android sources and the 9
touched shared C++ files gives 0 diffs. clang-tidy was not run (no
`clang-tidy` on this box). The `.clang-tidy` is the repo's bugprone set, so it
can run against `compile_commands.json` from an Android build dir.

**Comment density** (comment lines / code lines, non-blank). The branch sits at
or below the baseline, so comments are not a problem.

| Baseline file | ratio | Android file | ratio |
| --- | ---: | --- | ---: |
| Window-Linux.cpp | 0.06 | Window-Android.cpp | 0.03 |
| View-Linux.cpp | 0.06 | View-Android.cpp | 0.02 |
| EventLoop-Linux.cpp | 0.03 | EventLoop-Android.cpp | 0.02 |
| GlyphRasterizer-Linux.cpp | 0.05 | GlyphRasterizer-Android.cpp | 0.08 |
| GlyphRasterizer-Windows.cpp | 0.15 | DisplayLink-Android.cpp | 0.02 |
| Window-iOS.mm | 0.07 | Keyboard-Android.cpp | 0.03 |

The only file above baseline is `GlyphRasterizer-Android.cpp`. Its 15-line
header (`:17-32`) explains the per-code-point shaping limit and the colour-emoji
limit. That is the kind of "why" Eyal keeps; he trimmed Linux comments in #48
("Trimmed comments across the Linux graphics work"), but kept these. No
comment just restates the code. Comments that are wrong or stale:
- `README.md:204` "Android is not supported".
- `Lib/eacp/Text/CMakeLists.txt:3-6`: the reflowed line runs past 85 columns,
  and "the other two text engines reorder inside themselves" should now say
  three.
- `View-Linux.h` (branch): `void* display` is "kept so the record reads the
  same", a placeholder field. It goes away with `NativeSurfaceHandle`.

**Platform branching.** `Platform.h` says `current()` is "the single
compile-time platform check in the library … Everything else queries it
instead". The base has 5 platform-macro `#if`s in all of `Lib`. The branch adds
`__ANDROID__` blocks to 5 shared files:
- `Core/Platform/Platform.cpp:12-20`: `isDLL()` forced false.
- `Core/Process/Process-Posix.cpp:216-233`: below API 34,
  `workingDirectory` is silently ignored. That is a behaviour change with no
  error; fall back to `fork`+`chdir`, or fail the launch.
- `GPU/View/GPUView-Linux.cpp:224-247`.
- `GPU/Vulkan/VulkanContext-Linux.cpp:711-715`.
- `Graphics/View/View-Linux.h:5-10, 26-33`.

Fix: add `OS::Android` / `isAndroid()` (keep `isLinux()` or `isPosix()` true as
the codebase needs). On develop, add `NativeSurfaceHandle::Kind::Android` so
GPUView switches on the kind at runtime the way it does for Wayland/X11.

**Naming and structure.** These match the existing platform code:
- Anonymous namespaces are used everywhere.
- File-local names carry an `android` prefix. That is the unity-build
  convention (`WebSocket` does the same).
- `auto` is used throughout, there is no `auto` return type, and data members
  sit at the bottom.
- No struct is `final`, but that matches the Linux files: 0 of 21 structs in
  EventLoop/View/Window/GlyphRasterizer-Linux are `final`.
- `std::function` members have non-null defaults, and the setters restore the
  no-op on null (`Window-Android.cpp:40-43, 546-564`).
- `eacp::` is spelled only in the two `extern "C"` functions outside the
  namespace (`Window-Android.cpp:678, 684`), which is necessary. Line 670 uses
  `using namespace eacp::Graphics` but 678 spells `eacp::Threads`; pick one.

**Smaller items:**
- `Window-Android.cpp:71-161` `androidSystemInsets`: 90 lines, nested 5 deep.
  Split into `callObject(env, obj, name, sig)` helpers, or share
  `failed`/`LocalFrame` with the rasterizer through a small internal
  `JniHelpers-Android.h`. Today the two files each carry their own
  `failed`/attach logic.
- **Bug**, `Window-Android.cpp:121`: `FindClass("android/view/WindowInsets$Type")`
  is not checked. On API < 30 it returns null with a pending exception, and
  `GetStaticMethodID(nullptr, …)` follows. Cows ships `ANDROID_PLATFORM=android-29`.
- `Window-Android.cpp:54-66` and `Display-Android.cpp:28-32` compute the
  density scale twice, with different edge cases (`DENSITY_NONE` is handled in
  one only). Keep one function.
- `Window-Android.cpp:166-181` sets `$HOME`/`XDG_*` so the Linux `FilePath`
  code finds app storage. Eyal will want a `FilePath-Android.cpp` instead of
  process-environment side effects.
- `Window-Android.cpp:181-182`: missing blank line before `struct AndroidWindow`.
- `CMake/TargetSetup.cmake:294-297`: `if (ANDROID) include(AndroidApk)`
  sits against `endfunction()` with no blank line, and is at the bottom while
  `AppleSetup` is included at line 1. Include it at the top, alongside
  AppleSetup.
- `EventLoop-Android.cpp:25-67`: `AndroidPipeWaker` is a copy of
  `EventLoop-Linux.cpp`'s `PipeWaker`. On develop the Linux loop is epoll plus a
  pump, so revisit this during the rebase: sharing the waker, or driving the
  epoll fd from ALooper, may remove most of the file.
- `View-Android.cpp` (410 lines) re-implements `View-Linux.cpp`'s tree
  handling. On develop, check whether it can be a `ViewSurfaceBackend` under
  the shared `View-Linux.cpp`.
- Dead or throwaway code at HEAD, removed only in the uncommitted tree:
  `SoftwareContext.{h,cpp}` (642 lines), `Font-Android.*`,
  `TextMetrics-Android.cpp`, `FindAndroidFreeType.cmake`, the Hello sprite
  overlay. Commit that. No TODOs, no `#if 0`.
- `Scripts/android-apk:20-27` finds a JDK only under `/opt/homebrew`. The SDK
  path comes from `${ANDROID_NDK}/../..` (`AndroidApk.cmake:65`), which only
  works for an NDK installed inside the SDK. Prefer `ANDROID_HOME` or
  `ANDROID_SDK_ROOT`.

**Shared changes that are fine:**
- The capability vars get `OR ANDROID` (`CMakeLists.txt:69-81`).
- Spirv is on by default for Android.
- `FindVulkanBackend` picks the platform define.
- `Text::TextRenderer/GlyphRenderer::setSampleCount` (`58bafcb0`) is platform
  neutral. It is not on develop yet; call it out in the PR body as a general
  fix.

## Tests

- **eacp's norm:** every module has a `Tests/<Module>` NanoTest suite, and PRs
  add tests for what they add (#57 added "Tests: where NativeChildSurface puts
  foreign content"; #47 added a >2 GB mmap test). CI runs them on every lane.
- **The branch:** no `Tests/` changes and no CI lane. Worse, with tests on, the
  Android configure builds suites that fail (see Verdict 1).
- **Minimum:**
  - Gate `Tests/Network` and the network-linked parts of `UI`/`WebView` tests on
    Network being built.
  - Guard or replace `std::jthread` in `FilesTests-Posix.cpp`.
  - Then `CoreTests`, `SimdTests`, `SpirvTests` and `GPUCodegenTests` are plain
    executables that can run on the emulator. Set
    `CMAKE_CROSSCOMPILING_EMULATOR` to a `Scripts/android-adb-exec` that
    `adb push`es the binary and runs it under `adb shell`, and `ctest` just
    works.
- **Android-specific pieces worth a unit test:**
  - `EventLoop-Android` (`call`/`quit`/`runFor`/`addLoopSource`, runnable
    headless in a plain executable because it only needs `ALooper_prepare`).
  - The rasterizer (needs a JavaVM, so emulator-only; could run inside
    `AndroidHello` behind an intent extra).
- **CI:** an `ubuntu-latest` job with `nttld/setup-ndk` (or the preinstalled
  NDK) that configures and builds `AndroidHello-apk`. That is cheap and catches
  the breakage above. An emulator smoke run (install, start, grep logcat for
  "Vulkan device up") is optional; x86_64 emulators on GH runners have no
  Vulkan 1.3, so gate it on a self-hosted runner or skip it.

## Developer experience

| | iOS in eacp | Android branch |
| --- | --- | --- |
| App opts in | `set_default_target_setting` (plist automatic) | separate `add_library SHARED` + `eacp_add_android_apk(... PACKAGE ...)` |
| Manifest/plist | `CMake/iOSBundleInfo.plist.in` | `CMake/AndroidManifest.xml.in` (label, package, orientation, version, icon, `RES_DIR`) |
| Existing examples | all build | none; only `Apps/Android/Hello` |
| README build section | "built for the simulator" (thin, but configure is standard) | none; README says unsupported |
| Toolchain list | Xcode | none in eacp (lives in Cows docs) |
| Build + run | Xcode | build yes, install/run manual |
| Logging | Console/NSLog | logcat tag `eacp` (`Logging-Android.cpp:21`), undocumented |
| Debugging | Xcode | not possible: manifest has no `android:debuggable`, aapt2 has no `--debug-mode`, so no `run-as`/lldb attach. Unstripped `.so` is in the build dir for `ndk-stack`, undocumented |
| minSdk | `CMAKE_OSX_DEPLOYMENT_TARGET` 14.0 forced | whatever `ANDROID_PLATFORM` is (NDK default 21) |
| ABIs | n/a | one per build dir, undocumented |

**minSdk.** The code needs:
- API 29: `AChoreographer_postFrameCallback64`.
- API 30: `WindowInsets$Type`.
- API 28: `Typeface.create(Typeface,int,boolean)`.
- Vulkan 1.3 in practice (manifest `uses-feature 0x403000`).

Set `ANDROID_PLATFORM` to at least 29 with a `FATAL_ERROR` below that, fix the
API-30 JNI path to check the class and fall back to `contentRect` (the fallback
already exists), and document it.

**Cows-specific leakage into eacp:** the Hello strings, the Menlo/Consolas
alias list in `GlyphRasterizer-Android.cpp:263-274` (fine, and generic), and
nothing else. The Play Store `.aab` path and the `COWS_*` settings stayed in
Cows, which is correct.

## Design questions Eyal will ask

- **NativeActivity vs GameActivity.** NativeActivity needs no Java/Gradle and
  no AAR, which is why a CMake-only APK is possible. GameActivity would bring
  better IME/text input and insets, but needs Gradle or a prebuilt AAR plus
  dex. State this choice in the PR body. Text input (IME) is the known gap:
  `Keyboard-Android.cpp` is all stubs, and only Back maps to Escape.
- **Why a C `android_main`** (`AndroidMain-Android.c`): only C may call
  `main`. It keeps apps' ordinary `int main()` / `runWindowedApp`, the same
  entry point as every platform. `exit(result)` after `main` returns stops a
  second `android_main` from reusing static state. Needs a line in the PR body.
- **Loop integration.** The eacp loop runs on the glue's thread's ALooper, and
  glue commands and input arrive through `setLooperEventHandler`. The rebase
  question: develop's Linux loop is one epoll fd with `pumpEventLoop`, which
  may map onto ALooper more simply (register the epoll fd with ALooper).
- **JNI helper placement.** JNI helpers are duplicated in Window and Text. Add
  one internal header and one `getJavaVM()` in Graphics.
- **`ANDROID` vs `UNIX` gating.** CMake puts `elseif (ANDROID)` before
  `elseif (UNIX)`, which is right, since `UNIX` and `LINUX` are both true under
  the NDK toolchain in CMake ≥ 3.25. `Core/CMakeLists.txt` instead nests
  `if (ANDROID)` inside the `UNIX AND NOT APPLE` branch and reuses the Linux
  file list. That is fine, but it is the file that conflicts with develop.
  In C++, add `OS::Android` rather than raw `__ANDROID__`.
- **Network off.** "libcurl is not in the NDK" is true, but Eyal will ask for a
  plan: HttpURLConnection over JNI, or a CPM'd curl. Declare
  `EACP_HAS_NETWORK` (or equivalent) so Tests/Apps read it instead of
  restating `NOT ANDROID`.
- **Text engine.** It follows the rule: the platform engine, android.graphics
  over JNI, no FreeType (the FreeType attempt was removed). The known limits
  are documented in the file: per-code-point shaping (no kerning, ligatures or
  complex scripts), colour emoji as masks, and `registerMemoryFont` returning
  nullopt. He may ask for `TextRunShaper` (API 31) before calling Text
  "supported"; mark it † in the table like Linux.
- **The capability table.** DRAW/GPU/TEXT on, CONTEXT/CAPTURE/WEBVIEW off. This
  is the Linux shape; document it the same way.

## Top 10 fixes (priority order)

1. **Make a fresh configure+build succeed.** Gate `Tests/Network`, UI/WebView
   test links on Network existing, fix `std::jthread` in
   `FilesTests-Posix.cpp`, or default `EACP_ENABLE_TESTS` off on Android with a
   message. Effort: 0.5 d.
2. **One-command run.** Add a `<target>-run` target or `Scripts/android-run`
   (boot AVD if none, install, `am start`, `logcat -s eacp`). Effort: 0.5 d.
3. **README Android section and CLAUDE.md update.** Toolchain (sdkmanager
   packages, NDK 27.3, build-tools 35, JDK), AVD creation, configure line, run
   command, logcat/ndk-stack, minSdk, Vulkan 1.3 floor. Add an Android column
   to the table and delete "Android is not supported". Effort: 0.5 d.
4. **Rebase onto `upstream/develop`.** Resolve 5 conflicts, adopt
   `NativeSurfaceHandle::Kind::Android`, and revisit EventLoop-Android against
   the epoll pump and View-Android against `ViewSurfaceBackend`. Effort: 1–2 d.
5. **Make the demo a real eacp example.** Neutral Hello text; ideally an
   `eacp_add_app` helper so `Apps/GPU/Triangle` and `GlyphAtlas` also build as
   APKs, as iOS gets all examples. Effort: 0.5 d for Hello, +1 d for the helper
   and examples.
6. **Commit the in-flight cleanup.** Remove SoftwareContext, FreeType,
   Font/TextMetrics-Android and the overlay, so HEAD matches what is reviewed.
   Effort: 15 min.
7. **Enforce minSdk ≥ 29 in CMake and fix the API-30 JNI crash.**
   `Window-Android.cpp:121` needs a null/exception check with the
   `contentRect` fallback. Effort: 2 h.
8. **Replace shared-file `__ANDROID__` blocks with `OS::Android`/`isAndroid()`
   and the surface kind.** Make `Process-Posix` fail or fall back instead of
   silently ignoring `workingDirectory`. Move the `$HOME`/XDG hack into
   `FilePath-Android.cpp`. Effort: 0.5–1 d.
9. **Debuggable Debug APKs and portable packaging.** `android:debuggable` /
   `aapt2 --debug-mode` for Debug, SDK from `ANDROID_HOME`, JDK lookup not
   Homebrew-only, document ABIs. Effort: 0.5 d.
10. **CI lane.** An NDK job that builds `AndroidHello-apk` (and the headless
    test executables); optionally run `CoreTests` via an adb-exec emulator
    wrapper. Share JNI helpers between Window and Text while in there.
    Effort: 0.5–1 d.

Total to meet the bar (1–3, 5-Hello, 6, 7): about 2.5 days. With the rebase
and the rest: about 5–6 days.
