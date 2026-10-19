set shell := ["bash", "-euo", "pipefail", "-c"]

device := "D731347C-AC33-53DD-814A-1B1E4ED47AC7"

# EACP=$HOME/Code/eacp builds against a local eacp.
macos:
    [[ -f build/CMakeCache.txt ]] || cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build
    open "build/Apps/CowsInLove/Cows In Love.app"

sim-ios shot="":
    tools/ios.sh sim {{shot}}

# COWS_CONFIG=Release for a Release build.
sim-android shot="":
    tools/android.sh sim {{shot}}

studio-android:
    cmake --preset android ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    open -a "Android Studio" build-android/AndroidStudio

# udid is a devicectl UDID; defaults to pond.
ios udid=device:
    COWS_DEVICE={{udid}} tools/ios.sh device

shot out="":
    tools/shot.sh {{out}}

devices:
    xcrun devicectl list devices

test:
    cmake --build build
    ctest --test-dir build --output-on-failure

store-art:
    tools/store-art.sh

screenshots:
    tools/screenshots.sh

release-macos:
    tools/release-macos.sh

asc := "tools/asc-run.sh"

# --upload sends it to App Store Connect.
release-ios *args:
    {{asc}} tools/release-ios.sh {{args}}

asc *args:
    {{asc}} tools/asc-listing.py {{args}}

play := "tools/play-run.sh"

# One-time.
play-setup-key:
    tools/setup-play-upload-key.sh

# One-time.
play-setup-api:
    tools/setup-play-service-account.sh

# Args go to CMake.
release-android *args:
    {{play}} tools/release-android.sh {{args}}

play-validate:
    {{play}} fastlane android validate_credentials

play-check:
    {{play}} fastlane android check_access

# The first-ever AAB must be uploaded in the Play Console.
play-upload track="internal" status="completed" confirm="false":
    {{play}} fastlane android upload track:{{track}} status:{{status}} confirm:{{confirm}}

# status=completed requires confirm=true.
play-promote from="internal" to="production" status="draft" confirm="false":
    {{play}} fastlane android promote from:{{from}} to:{{to}} status:{{status}} confirm:{{confirm}}

# Raise the build in CMakeLists.txt first.
deploy-android track="internal" status="completed" confirm="false":
    {{play}} tools/deploy-android.sh {{track}} {{status}} {{confirm}}

release-windows:
    tools/release-windows.sh

release-microsoft:
    tools/release-microsoft.sh

steam-upload:
    tools/steam-upload.sh

# --upload sends it to App Store Connect.
release-mas *args:
    {{asc}} tools/release-mas.sh {{args}}

# scenario: S1 menu, S2 editor, S3 play, S4 walking, S5 ending.
perf scenario label runs="10":
    [[ -f build-release/CMakeCache.txt ]] || cmake -G Ninja -B build-release -DCMAKE_BUILD_TYPE=Release ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build-release
    tools/perf.sh {{scenario}} {{label}} {{runs}}

perf-compare a b:
    #!/usr/bin/env bash
    set -euo pipefail
    dir="${COWS_PERF_DIR:-/tmp/cows-perf}"
    for csv in "$dir/{{a}}"/S*.csv; do
        other="$dir/{{b}}/$(basename "$csv")"
        [[ -f "$other" ]] && tools/perf-stats.py "$csv" "$other"
    done

dead-code:
    tools/dead-code.sh
