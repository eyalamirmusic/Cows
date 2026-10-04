#!/usr/bin/env bash
# Usage: tools/ios.sh sim [shot.png]   build, run on the "Cows iPhone" simulator
#        [COWS_TEAM=id] [COWS_DEVICE=udid] tools/ios.sh device
#                                      build, sign, install and run on a phone
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.play}"
sim_name="Cows iPhone"
sim_type="iPhone 17"
sim_runtime="com.apple.CoreSimulator.SimRuntime.iOS-26-5"
device="${COWS_DEVICE:-D731347C-AC33-53DD-814A-1B1E4ED47AC7}"
sign_in="Sign into Xcode > Settings > Accounts with an Apple ID, then rerun"

run_sim() {
    local shot="${1:-}"

    if [[ ! -f build-ios/CMakeCache.txt ]]; then
        cmake -G Xcode -B build-ios -DCMAKE_SYSTEM_NAME=iOS \
            -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=arm64 \
            -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
    fi
    cmake --build build-ios --config Debug --target Cows -- -sdk iphonesimulator

    local udid
    udid="$(xcrun simctl list devices -j | python3 -c '
import json, sys
name = sys.argv[1]
for devices in json.load(sys.stdin)["devices"].values():
    for d in devices:
        if d["name"] == name and d["isAvailable"]:
            print(d["udid"]); raise SystemExit
' "$sim_name")"
    [[ -n "$udid" ]] || udid="$(xcrun simctl create "$sim_name" "$sim_type" "$sim_runtime")"

    xcrun simctl boot "$udid" 2>/dev/null || true
    xcrun simctl bootstatus "$udid" -b >/dev/null
    open -a Simulator

    xcrun simctl install "$udid" build-ios/Apps/CowsInLove/Debug-iphonesimulator/Cows.app
    xcrun simctl terminate "$udid" "$bundle_id" 2>/dev/null || true
    xcrun simctl launch "$udid" "$bundle_id"

    if [[ -n "$shot" ]]; then
        mkdir -p "$(dirname "$shot")"
        sleep 4
        xcrun simctl io "$udid" screenshot "$shot" >&2
        echo "$shot"
    fi
}

# Jamie's free Personal Team; Xcode 26 no longer caches teams in its defaults.
personal_team=DN5WMFL5W5

xcode_team() {
    defaults export com.apple.dt.Xcode - 2>/dev/null | python3 -c '
import plistlib, sys
try:
    teams = plistlib.loads(sys.stdin.buffer.read()).get("IDEProvisioningTeams", {})
except Exception:
    teams = {}
for account in teams.values():
    for team in account:
        if team.get("teamID"):
            print(team["teamID"]); raise SystemExit
'
}

run_device() {
    local team="${COWS_TEAM:-$(xcode_team)}"
    team="${team:-$personal_team}"
    [[ -n "$team" ]] || { echo "$sign_in" >&2; exit 1; }

    cmake -G Xcode -B build-ios-device -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic \
        -DCOWS_IOS_TEAM="$team"

    local log
    log="$(mktemp)"
    if ! cmake --build build-ios-device --config Debug --target Cows -- \
        -allowProvisioningUpdates -allowProvisioningDeviceRegistration \
        2>&1 | tee "$log"; then
        if grep -qiE "sign|provision|account|team" "$log"; then
            echo >&2
            echo "Signing failed for team $team. $sign_in" >&2
        fi
        exit 1
    fi

    local app="build-ios-device/Apps/CowsInLove/Debug-iphoneos/Cows.app"
    xcrun devicectl device install app --device "$device" "$app"
    xcrun devicectl device process launch --device "$device" "$bundle_id"
}

case "${1:-}" in
    sim) run_sim "${2:-}" ;;
    device) run_device ;;
    *) echo "usage: tools/ios.sh sim [shot.png] | tools/ios.sh device" >&2; exit 1 ;;
esac
