set shell := ["bash", "-euo", "pipefail", "-c"]

device := "D731347C-AC33-53DD-814A-1B1E4ED47AC7"

# Build and open the macOS app. EACP=$HOME/Code/eacp builds against a local eacp.
macos:
    [[ -f build/CMakeCache.txt ]] || cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build
    open build/Apps/CowsInLove/Cows.app

# Build and run on the "Cows iPhone" simulator, optionally saving a screenshot.
sim-ios shot="":
    tools/ios.sh sim {{shot}}

# Build (Debug; COWS_CONFIG=Release for Release) and run on the "cows" Android
# emulator, optionally saving a screenshot.
sim-android shot="":
    tools/android.sh sim {{shot}}

# Write the Android Studio project (build-android/AndroidStudio, a Cows module)
# and open it.
studio-android:
    cmake --preset android ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    open -a "Android Studio" build-android/AndroidStudio

# Build, sign and run on a phone (devicectl UDID; defaults to pond).
ios udid=device:
    COWS_DEVICE={{udid}} tools/ios.sh device

# Screenshot the macOS app (COWS_TIME / COWS_FREEZE / COWS_SEED pass through).
shot out="":
    tools/shot.sh {{out}}

# List phones devicectl can see.
devices:
    xcrun devicectl list devices

# Build and run every library's test suite.
test:
    cmake --build build
    ctest --test-dir build --output-on-failure

# Render the icons and store art (Deploy/Art, Steam, MSIX, Google Play, app icons).
store-art:
    tools/store-art.sh

# Render the Steam, App Store and Google Play screenshots.
screenshots:
    tools/screenshots.sh

# Universal macOS build, Developer ID signed and notarized, staged for Steam.
release-macos:
    tools/release-macos.sh

# App Store Connect credentials, from the "Cows In Love" vault of the personal
# 1Password account through Deploy/asc.env (tools/asc-run.sh).
asc := "tools/asc-run.sh"

# App Store archive and export (--upload sends it to App Store Connect), signed
# and uploaded with the API key from 1Password.
release-ios *args:
    {{asc}} tools/release-ios.sh {{args}}

# The App Store Connect listing: check, info, version, screenshots, build, submit
# or all, with --platform ios|macos and --write (tools/asc-listing.py).
asc *args:
    {{asc}} tools/asc-listing.py {{args}}

# Google Play .aab, Release arm64-v8a + x86_64, through Gradle's bundleRelease
# (args go to the configure: -DCOWS_BUILD_NUMBER=<n>).
# Signed with the debug key until eacp has a slot for the upload key.
release-android *args:
    cmake --preset android ${EACP:+-DCPM_eacp_SOURCE=$EACP} {{args}}
    cd build-android/AndroidStudio && JAVA_HOME="${JAVA_HOME:-/Applications/Android Studio.app/Contents/jbr/Contents/Home}" ./gradlew :Cows:bundleRelease

# Windows x64 Release build on the Windows box, staged for Steam.
release-windows:
    tools/release-windows.sh

# Upload the staged Windows and macOS depots to Steam with steamcmd.
steam-upload:
    tools/steam-upload.sh

# Mac App Store archive and export (--upload sends it to App Store Connect), signed
# and uploaded with the API key from 1Password.
release-mas *args:
    {{asc}} tools/release-mas.sh {{args}}
