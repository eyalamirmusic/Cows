set shell := ["bash", "-euo", "pipefail", "-c"]

device := "D731347C-AC33-53DD-814A-1B1E4ED47AC7"

# Build and open the macOS app. EACP=$HOME/Code/eacp builds against a local eacp.
macos:
    [[ -f build/CMakeCache.txt ]] || cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build
    open "build/Apps/CowsInLove/Cows In Love.app"

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

# Google Play credentials and upload key come from the Cows In Love vault.
play := "tools/play-run.sh"

# One-time: generate the Play upload key and keep its only durable copy in 1Password.
play-setup-key:
    tools/setup-play-upload-key.sh

# One-time: create and store the dedicated Play API service-account credential.
play-setup-api:
    tools/setup-play-service-account.sh

# Build a Play-signed .aab. Args go to CMake, e.g. -DCPM_eacp_SOURCE=<dir>.
release-android *args:
    {{play}} tools/release-android.sh {{args}}

# Check the Google Play service-account credential without uploading.
play-validate:
    {{play}} fastlane android validate_credentials

# Read-only check that the service account can access Cows In Love in Play.
play-check:
    {{play}} fastlane android check_access

# Upload an already-built AAB. The first-ever AAB must be uploaded in the UI.
play-upload track="internal" status="completed" confirm="false":
    {{play}} fastlane android upload track:{{track}} status:{{status}} confirm:{{confirm}}

# Promote a tested release. Production defaults to draft; completed requires confirm=true.
play-promote from="internal" to="production" status="draft" confirm="false":
    {{play}} fastlane android promote from:{{from}} to:{{to}} status:{{status}} confirm:{{confirm}}

# Subsequent releases: raise the build in CMakeLists.txt, then build and upload (internal by default).
deploy-android track="internal" status="completed" confirm="false":
    {{play}} tools/deploy-android.sh {{track}} {{status}} {{confirm}}

# Windows x64 Release build on the Windows box, staged for Steam.
release-windows:
    tools/release-windows.sh

# Microsoft Store x64 + arm64 MSIX packages, built on the Windows box and
# copied back to Deploy/Microsoft-Store/out/.
release-microsoft:
    tools/release-microsoft.sh

# Upload the staged Windows and macOS depots to Steam with steamcmd.
steam-upload:
    tools/steam-upload.sh

# Mac App Store archive and export (--upload sends it to App Store Connect), signed
# and uploaded with the API key from 1Password.
release-mas *args:
    {{asc}} tools/release-mas.sh {{args}}

# Measure one scenario (S1 menu, S2 editor, S3 play, S4 walking, S5 ending) n
# times in Release into $COWS_PERF_DIR/<label> (tools/perf.sh).
perf scenario label runs="10":
    [[ -f build-release/CMakeCache.txt ]] || cmake -G Ninja -B build-release -DCMAKE_BUILD_TYPE=Release ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build-release
    tools/perf.sh {{scenario}} {{label}} {{runs}}

# Compare two labels' runs, scenario by scenario (tools/perf-stats.py).
perf-compare a b:
    #!/usr/bin/env bash
    set -euo pipefail
    dir="${COWS_PERF_DIR:-/tmp/cows-perf}"
    for csv in "$dir/{{a}}"/S*.csv; do
        other="$dir/{{b}}/$(basename "$csv")"
        [[ -f "$other" ]] && tools/perf-stats.py "$csv" "$other"
    done
