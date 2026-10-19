#!/usr/bin/env bash
# Usage: [COWS_CONFIG=Release] [EACP_AVD=avd] [COWS_ENV="COWS_SEED=3"] tools/android.sh sim [shot.png] | device
# Release because Debug is 5 fps while a finger moves; COWS_ENV works in Debug only.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

sdk="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
adb="$sdk/platform-tools/adb"
bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.play}"
config="${COWS_CONFIG:-Debug}"
export JAVA_HOME="${JAVA_HOME:-/Applications/Android Studio.app/Contents/jbr/Contents/Home}"

case "${1:-}" in
    sim | device) ;;
    *) sed -n '2,11p' "$0"; exit 1 ;;
esac

if [[ ! -f build-android/CMakeCache.txt ]]; then
    cmake --preset android ${EACP:+-DCPM_eacp_SOURCE="$EACP"}
fi

if [[ -z "$("$adb" devices | sed -n '2,$p' | grep -w device || true)" ]]; then
    [[ $1 == sim ]] || { echo "No phone attached" >&2; exit 1; }
    "$sdk/emulator/emulator" -avd "${EACP_AVD:-cows}" >/dev/null 2>&1 &
    "$adb" wait-for-device
    until [[ "$("$adb" shell getprop sys.boot_completed | tr -d '\r')" == 1 ]]; do
        sleep 2
    done
fi

(cd build-android/AndroidStudio && ./gradlew ":Cows:install$config")

extras=()
for kv in ${COWS_ENV:-}; do
    extras+=(--es "${kv%%=*}" "${kv#*=}")
done

"$adb" shell am force-stop "$bundle_id"
"$adb" shell am start -n "$bundle_id/android.app.NativeActivity" \
    ${extras[@]+"${extras[@]}"}

if [[ -n "${2:-}" ]]; then
    mkdir -p "$(dirname "$2")"
    sleep "${COWS_SHOT_DELAY:-8}"
    "$adb" exec-out screencap -p > "$2"
    echo "$2"
fi
