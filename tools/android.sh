#!/usr/bin/env bash
# Usage: tools/android.sh sim [shot.png]   build, run on the "cows" emulator
#        tools/android.sh device           build, install and run on the phone
#                                          adb sees (USB debugging on)
#
# Needs the Android SDK at $ANDROID_HOME (default ~/Library/Android/sdk) with
# NDK r30 (eacp builds with the current stable NDK), build-tools 35 and
# platform 35, and eacp's jp/android-vulkan-1-1 branch: EACP=<path>
# (default ~/projects/eacp-android-integration) builds against that
# checkout, or, when it is absent, fetches
# eyalamirmusic/eacp@jp/android-vulkan-1-1 through CPM.
# COWS_CONFIG=Release|Debug (default Release; Debug is 5 fps while a finger
# moves, the HUD repaint alone ~400 ms) picks the build type, each in its
# own build dir. COWS_ENV="COWS_PROFILE=1 COWS_SEED=3" is set as the
# debug.cows.env property the app reads its COWS_* settings from.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

sdk="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
ndk="${ANDROID_NDK:-$sdk/ndk/30.0.16248370}"
eacp="${EACP:-$HOME/projects/eacp-android-integration}"
avd="${COWS_AVD:-cows}"
package="${COWS_BUNDLE_ID:-com.cowsinlove.cows}"
adb="$sdk/platform-tools/adb"
config="${COWS_CONFIG:-Release}"
build=build-android
[[ $config == Debug ]] || build="build-android-$(echo "$config" | tr '[:upper:]' '[:lower:]')"

build() {
    local eacp_args=(-DCPM_eacp_SOURCE="$eacp")
    [[ -d $eacp ]] || eacp_args=(-DCOWS_EACP_REPOSITORY=eyalamirmusic/eacp
                                 -DCOWS_EACP_TAG=jp/android-vulkan-1-1)

    if [[ ! -f $build/CMakeCache.txt ]]; then
        cmake -G Ninja -B $build -DCMAKE_BUILD_TYPE="$config" \
            -DCMAKE_TOOLCHAIN_FILE="$ndk/build/cmake/android.toolchain.cmake" \
            -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-33 \
            "${eacp_args[@]}" -DCOWS_BUILD_TESTS=OFF
    fi
    cmake --build $build --target Cows-apk
}

boot_emulator() {
    if "$adb" get-state >/dev/null 2>&1; then
        return
    fi

    nohup "$sdk/emulator/emulator" -avd "$avd" -no-snapshot-save -no-audio \
        -gpu host >/dev/null 2>&1 &
    "$adb" wait-for-device

    until [[ "$("$adb" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" == 1 ]]; do
        sleep 2
    done
}

run() {
    local shot="${1:-}"

    "$adb" install -r "$build/Apps/CowsInLove/Cows.apk" >&2
    "$adb" shell setprop debug.cows.env "'${COWS_ENV:-}'"
    "$adb" shell am force-stop "$package"
    "$adb" shell am start -n "$package/android.app.NativeActivity" >&2

    if [[ -n "$shot" ]]; then
        mkdir -p "$(dirname "$shot")"
        sleep "${COWS_SHOT_DELAY:-8}"
        "$adb" exec-out screencap -p > "$shot"
        echo "$shot"
    fi
}

case "${1:-}" in
    sim)
        build
        boot_emulator
        run "${2:-}"
        ;;
    device)
        build
        run
        ;;
    *)
        sed -n '2,13p' "$0"
        exit 1
        ;;
esac
