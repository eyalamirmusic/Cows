#!/usr/bin/env bash
# Usage: tools/android.sh sim [shot.png]   build, run on the "cows" emulator
#        tools/android.sh device           build, run on the phone adb sees
#
# Configures with the android preset (Android Studio's SDK, the NDK eacp pins),
# then eacp's Cows-run installs and launches the app: on a phone over USB, or on
# the emulator it boots (EACP_AVD, default cows). EACP=<path> (default
# ~/projects/eacp-android-integration) builds against that eacp checkout, else
# CPM fetches the default in CMake/Findeacp.cmake. COWS_CONFIG=Release|Debug
# (default Release; Debug is 5 fps while a finger moves) picks the build type,
# each in its own build dir. COWS_ENV="COWS_PROFILE=1 COWS_SEED=3" launches the
# app with that environment (eacp's EACP_RUN_ENV).
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

adb="${ANDROID_HOME:-$HOME/Library/Android/sdk}/platform-tools/adb"
eacp="${EACP:-$HOME/projects/eacp-android-integration}"
config="${COWS_CONFIG:-Release}"
build="build-android-$(echo "$config" | tr '[:upper:]' '[:lower:]')"
export EACP_AVD="${EACP_AVD:-cows}"

case "${1:-}" in
    sim | device) ;;
    *) sed -n '2,13p' "$0"; exit 1 ;;
esac

if [[ ! -f $build/CMakeCache.txt ]]; then
    eacp_args=()
    [[ -d $eacp ]] && eacp_args=(-DCPM_eacp_SOURCE="$eacp")
    cmake --preset android -B "$build" -DCMAKE_BUILD_TYPE="$config" \
        ${eacp_args[@]+"${eacp_args[@]}"}
fi

EACP_RUN_ENV="${COWS_ENV:-}" cmake --build "$build" --target Cows-run

if [[ -n "${2:-}" ]]; then
    mkdir -p "$(dirname "$2")"
    sleep "${COWS_SHOT_DELAY:-8}"
    "$adb" exec-out screencap -p > "$2"
    echo "$2"
fi
