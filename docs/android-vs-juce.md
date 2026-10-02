# Android: eacp vs JUCE

JUCE at `be29c81` (9.0.3, `~/projects/juce-ref`); eacp at `jp/android-vulkan-1-1` `fc7ac190` (`~/projects/eacp-android-integration`); Cows `android` at `dd39a12`. eacp stays zero-Java, so JUCE is read here for DX and lifecycle lessons only. Paths are relative to each repo.

## Build system

JUCE's CMake API does not do Android: `docs/CMake API.md:6` says "Android targets are not currently supported". `juce_add_gui_app` only swaps in `add_library(SHARED)` (`extras/Build/CMake/JUCEUtils.cmake:2261`). The real path is the Projucer exporter (`extras/Projucer/Source/ProjectSaving/jucer_ProjectExport_Android.h`, 2134 lines). It writes a Gradle project (settings/build/app `build.gradle`, `local.properties`, `gradle.properties`, the wrapper embedded in the Projucer binary, the manifest, strings/icons) plus a generated `app/CMakeLists.txt` with every module source flattened into it (`writeCmakeFile`, :397), so your CMake never builds the Android app.

eacp's `CMake/AndroidStudio.cmake` does the right version of this: the Gradle project is a by-product of the configure, the way `-G Xcode` is. It has one module per `eacp_add_app`, and `externalNativeBuild` points back at the real top-level `CMakeLists.txt`, with CPM sources passed through so Gradle fetches nothing.

Pinning: JUCE hard-codes the NDK in the generator (`ndkVersionString = "28.1.13356709"`, :708) and Java 8 (:706). Gradle 8.14.5 and AGP 8.13.2 are per-project fields with defaults (:160, :163), so old `.jucer` files keep stale versions. eacp keeps everything in `CMake/AndroidVersions.cmake`, read by both the build and `Scripts/android-setup.cmake`, and fetches the wrapper by SHA-256.

SDK location: JUCE removed per-exporter SDK/NDK paths in favour of Projucer global paths (`BREAKING_CHANGES.md:4350`), and `local.properties` gets `sdk.dir` from there (:1086). eacp's `CMake/AndroidToolchain.cmake` checks `$ANDROID_HOME`, then `~/.eacp/android/sdk`, and never Studio's default SDK. That gap is why Cows carries its own `CMake/AndroidToolchain.cmake`, which repeats the NDK version (`30.0.16248370`) and adds `~/Library/Android/sdk` and `%LOCALAPPDATA%/Android/Sdk`. `tools/android.sh` and `tools/release-android.sh` repeat the version again.

ABIs: JUCE's debug default is still `armeabi-v7a x86 arm64-v8a x86_64`, and release defaults to empty, meaning everything (:295). eacp's Studio project builds `arm64-v8a;x86_64` (`AndroidStudio.cmake`), and a plain build is one ABI per build dir.

Debug/release: JUCE turns each Projucer config into a productFlavor and then disables unwanted variants with an `androidComponents` filter (:738, :862), which is clumsy. eacp maps `release` to `-DCMAKE_BUILD_TYPE=Release` (`CMake/AndroidStudio/app.build.gradle.kts.in`).

Signing: JUCE writes the keystore path and both passwords from the `.jucer` into `build.gradle` in plain text (`getAndroidSigningConfig`, :780). The default is `~/.android/debug.keystore` (:156), so a release is debug-signed unless you change it. eacp's Studio release is also debug-signed (`app.build.gradle.kts.in`). eacp's Ninja path signs with the debug keystore or `$EACP_ANDROID_KEYSTORE` (`Scripts/android-apk.cmake:84`). Play upload signing exists only in Cows: `tools/release-android.sh` (132 lines) builds the per-ABI loop, aapt2 proto link, bundletool, jarsigner from env vars, the debug-symbols metadata and `--local-testing`.

What you need installed: JUCE needs the Projucer plus Android Studio (or the SDK and a JDK for `gradlew`). It launches Studio itself through the `androidStudioExePath` setting (:165, :207). eacp needs CMake, Ninja and Git. `cmake -P Scripts/android-setup.cmake` installs the SDK, NDK, JDK and emulator. `<target>-run` (`Scripts/android-run.cmake`) boots an AVD, wakes the phone, handles a debug-key mismatch and tails logcat. JUCE has nothing like it.

On "require Android Studio like we require Xcode": Xcode is required because Apple's compilers and signing exist nowhere else. Android's toolchain installs from the command line, and eacp already proves it. My recommendation is to require *an SDK*, accept Studio's as a first-class one, and keep Studio optional, as the README's section 6 already treats it.

## Entry point, lifecycle, event loop

JUCE's library is loaded from Java. `JuceApp.onCreate` (`modules/juce_core/native/javacore/app/com/rmsl/juce/JuceApp.java`) calls `Java.initialiseJUCE`, which `JNI_OnLoad` registered (`juce_core/native/juce_Threads_android.cpp:77`). The app object is then built on the Android UI thread from `ActivityLifecycleCallbacks` (`juce_events/native/juce_Messaging_android.cpp:177`): `onActivityPaused` calls `suspended()`, `onActivityResumed` calls `resumed()`, and destroying the main activity calls `appWillTerminateByForce` then `System.exit`. The message loop is an `android.os.Handler` posting Runnables (:63). JUCE also had to learn that modal loops are impossible on Android (`BREAKING_CHANGES.md:3318`) and that back must call `finish()` (:3999).

eacp uses NativeActivity plus the glue. `android_main` calls the app's ordinary `main()` and then `exit()`s, so a recreated activity never re-enters static state (`Lib/eacp/Graphics/Window/AndroidMain-Android.c`). The eacp loop runs on the glue thread and waits in `ALooper_pollOnce`, with the loop's epoll fd added to the looper (`Lib/eacp/Core/Threads/EventLoop-Android.cpp`). The manifest's `configChanges` list matches JUCE's (`CMake/AndroidManifest.xml.in` vs exporter :1931). Surface loss is handled correctly: `APP_CMD_TERM_WINDOW` drops the swapchain before the glue releases the window (`Window-Android.cpp:457`).

The gaps are elsewhere. Pause/resume goes only to `Graphics::Android::setLifecycleHandler` in an Android-only header (`Window/Android.h`). Nothing in eacp or Cows calls it, so Cows' MakeASound device probably keeps playing in the background (I haven't checked this on a device). NativeActivity also has no `onNewIntent`. With `launchMode="singleTask"`, intent extras arrive only on a cold start, which `android-run.cmake:217` hides by force-stopping first. JUCE's `JuceActivity.java` forwards `onNewIntent`/`onResume`.

## Windowing, touch, insets, IME

JUCE draws into a Java `ComponentPeerView`, and touches arrive as JNI callbacks with a pointer index into `MouseInputSource` (`juce_gui_basics/native/juce_Windowing_android.cpp:1714`). eacp reads `AMotionEvent` directly with pointer id + 1 and full multi-touch (`Window-Android.cpp:340`), and gets frames from `AChoreographer` (`Graphics/View/AndroidViewSurface-Android.cpp`, `Helpers/DisplayLink-Android.cpp`). The Vulkan surface is 25 lines (`GPU/Vulkan/VulkanSurface-Android.cpp`).

Insets: JUCE subscribes via `OnApplyWindowInsetsListener` (:1403). That needs `CreateJavaInterface`, which means bytecode. It computes both the safe area and **keyboard insets** with the `ime()` mask (:1089–1170), and turns on edge-to-edge in Java (`JuceActivity.initEdgeToEdge`). eacp reads `getRootWindowInsets()` over JNI with `systemBars | displayCutout` on surface, config and content-rect changes (`Window-Android.cpp:110`). There is no IME mask.

IME: JUCE implements `onCreateInputConnection` in `ComponentPeerView.java`, which can't be done without Java. eacp has none: `Graphics/Graphics/Keyboard-Android.cpp` is all stubs, and only BACK is mapped (to Escape). The zero-Java ceiling is `ANativeActivity_showSoftInput` plus key events turned into Unicode through `KeyEvent.getUnicodeChar` over JNI. That gives typing, but no composition or autocorrect.

## JNI

JUCE's `JNIClassBase` and `DECLARE_JNI_CLASS*` (`juce_core/native/juce_JNIHelpers_android.h:212`, :297) declare each class once as an X-macro of `METHOD`/`STATICMETHOD`/`FIELD`/`CALLBACK`. IDs are resolved at startup with a min-SDK gate (`DECLARE_JNI_CLASS_WITH_MIN_SDK`, which leaves the IDs null below that SDK; the insets code checks for null at :1099), and there are `LocalRef`/`GlobalRef` RAII wrappers. Separately, `CALLBACK` plus the gzipped dex blobs loaded through `InMemoryDexClassLoader` (`juce_JNIHelpers_android.cpp:287–371`) are how JUCE gets Java to call C++. That part is excluded here.

eacp has written its JNI helpers out by hand three times: `failed`/`LocalFrame`/`ThreadAttachment` in `Text/GlyphRasterizer-Android.cpp:36–100`, `androidJavaFailed`/`androidCallObject` in `Window-Android.cpp:63`, and `javaFailed`/`callObject` in `AndroidEnvironment-Android.cpp`. The `AndroidGraphics::load` table (GlyphRasterizer :101–215) is already JUCE's pattern without the macro. A helper, not the macro, is what eacp needs.

## Text, files, audio

Text: JUCE's Android `Typeface` is FreeType + HarfBuzz, with `AFontMatcher` for fallback (`juce_graphics/native/juce_Fonts_android.cpp:226`, :309) and no JNI. eacp rasterizes through `android.graphics.Paint/Canvas` over JNI (712 lines) and shapes "a code point at a time" (`README.md:103`). eacp already has FreeType/HarfBuzz for Linux (`Text/GlyphRasterizer-Linux.cpp`, `CMake/FindLinuxText.cmake`), and `AFontMatcher` is API 29, below eacp's floor of 33.

Files: JUCE resolves dirs through `Context` over JNI (`juce_core/native/juce_Files_android.cpp`). eacp takes `internalDataPath` from the activity (`Window-Android.cpp:633`, `Core/Utils/FilePath-Android.cpp`) and doesn't touch the APK assets at all. ResEmbed compiles resources in, which is enough for now.

Audio: JUCE's Oboe path (`juce_audio_devices/native/juce_Oboe_android.cpp`) gets the burst size from `AudioManager` `OUTPUT_FRAMES_PER_BUFFER` and checks the `low_latency`/`pro` features (`juce_HighPerformanceAudioHelpers_android.h:50–71`). It tries Exclusive and falls back to Shared (:392), and reopens the stream on `ErrorDisconnected` (:959). MakeASound (miniaudio AAudio/OpenSL) turns on `ma_performance_profile_low_latency` only when `minimizeLatency` is set (`MiniAudio/MiniAudioDeviceManager.cpp:80`). Cows also needs a fake `ALSA::ALSA` (`CMake/Android/FindALSA.cmake`) because MakeASound's RtMidi lookup has no Android branch.

## New-app DX

JUCE needs a `.jucer`, `Main.cpp` and `MainComponent`, and the Projucer generates about 15 files per exporter (:219–243). With CMake you write the Gradle project yourself. eacp needs three files (`Apps/Android/README.md` §5): a 3-line `CMakeLists.txt`, about 20 lines of `main()`, and one `add_subdirectory`. Both have zero platform C++ in the app.

A real app is a different story. Cows' Android-only files are an `if(ANDROID)` block in `Apps/CowsInLove/CMakeLists.txt`, because `eacp_add_app` takes no package, label or icon (`CMake/TargetSetup.cmake:7` hard-codes `com.eacp.<name>`). On top of that it has a 47-line full copy of the manifest, because the only hook is replacing the whole template, `res/` icons, its own toolchain file, the ALSA shim, and two shell scripts (227 lines).

Extension points: JUCE turns on feature-specific Java per flag. `addOptJavaFolderToSourceSetsForModule` adds `javaopt/` for billing, Firebase and MIDI (:1010–1019), plus Gradle dependencies (:940–949) and manifest services. That route is closed to eacp. Play Billing and FCM have no NDK API, so in-app purchases and push notifications would mean either giving up zero-Java for those features or leaving them out. Runtime permissions can be done with zero Java: call `requestPermissions` over JNI, then poll `checkSelfPermission` on `APP_CMD_RESUME`.

CI: JUCE's Android CI is private (`.github/workflows/juce_private_build.yml`). eacp says Android "is not in CI yet" (`README.md`).

## Patterns worth borrowing (all zero Java)

1. **Manifest fragments instead of replacing the template.** This is the lesson in JUCE's `BREAKING_CHANGES.md:4719`: it parses the user's XML and adds only missing defaults (`createManifestElement`, :1808). In eacp, `eacp_add_android_apk` would gain `MANIFEST_ELEMENTS`, `APPLICATION_ATTRIBUTES` and `ACTIVITY_ATTRIBUTES` slots in `CMake/AndroidManifest.xml.in`. Effort S. Cows' 47-line copy becomes about 8 lines.
2. **One app declaration.** `eacp_add_app` would forward `BUNDLE_ID/LABEL/ICON/VERSION_CODE/ORIENTATION` the way `juce_add_gui_app` takes keyword args, with one bundle id for every platform. `CMake/AndroidToolchain.cmake` would also look in Studio's default SDK paths. That's in `CMake/TargetSetup.cmake` and `CMake/AndroidToolchain.cmake`, effort XS–S, and Cows deletes its toolchain file and `if(ANDROID)` block.
3. **A native JNI helper.** `Lib/eacp/Core/Android/Jni.h` would hold `currentEnv()` with a thread-local detach, `LocalFrame`, `LocalRef`/`GlobalRef`, `failed()`, and a "resolve this class table once, log what's missing" helper with optional members gated by min SDK. The three hand-written copies move onto it. Effort S.
4. **App-level `suspended()/resumed()`**, the way JUCE's `JUCEApplicationBase` has them, shared with iOS. It replaces the Android-only `setLifecycleHandler`, and the audio device gets paused. Effort S.
5. **An `<target>-aab` target.** Move Cows' `release-android.sh` into `Scripts/android-bundle.cmake`, with an ABI loop, bundletool pinned by hash in `AndroidVersions.cmake`, upload key from env vars, and a symbols file. Also give the Studio release type an env-driven `signingConfig` rather than JUCE's plaintext passwords. Effort M.
6. **FreeType + HarfBuzz + `AFontMatcher` text**, reusing `GlyphRasterizer-Linux.cpp` with fontconfig swapped out. You get real shaping and the 712-line JNI file goes. Effort M.
7. **IME and keyboard insets.** Add the `ime()` mask to the insets read and expose it as keyboard insets, as JUCE does (:1155), plus `ANativeActivity_showSoftInput` and `getUnicodeChar` for basic typing. Effort S–M.
8. **Audio hardening in MakeASound.** Use AAudio low latency by default on Android, honour the native burst size, and reopen on disconnect. Effort S.

## Things eacp does better

- A CMake-native Android build, with an optional Studio project generated from the real `CMakeLists.txt`. JUCE CMake has neither.
- A single pin file shared by setup and build, and a hash-checked wrapper.
- No IDE needed: setup script, `-run` that boots the emulator, wakes the phone, handles key mismatches and tails logcat.
- Sane ABI defaults (64-bit only) and release meaning CMake Release.
- The NativeActivity loop on its own thread: no `Handler`/Runnable round trips, no class-loader or dex tricks, no ProGuard, no Java 8 toolchain floor.
- Direct `AMotionEvent` multi-touch and `AChoreographer` frame pacing.
- Environment injection (`--es` extras, `debug.<package>.env`), which JUCE has no counterpart for.
- Android 13+ with Vulkan 1.1 as the floor, which deletes JUCE's API 24–30 branches (e.g. the three `WindowInsets` class declarations at :942–982).
