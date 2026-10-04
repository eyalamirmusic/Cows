#!/usr/bin/env bash
# Usage: COWS_STEAM_APP_ID=<app> COWS_STEAM_DEPOT_WINDOWS=<depot>
#        COWS_STEAM_DEPOT_MACOS=<depot> COWS_STEAM_USER=<steam login>
#        [COWS_STEAM_BRANCH=<beta branch>] tools/steam-upload.sh
# Uploads Deploy/Steam/content/{windows,macos} (from tools/release-windows.sh
# and tools/release-macos.sh) as one Steam build, with steamcmd
# (brew install steamcmd, or the Steamworks SDK's tools/ContentBuilder).
# Without COWS_STEAM_BRANCH the build is uploaded but not set live; setting the
# default branch live is done in Steamworks.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

app="${COWS_STEAM_APP_ID:?set COWS_STEAM_APP_ID to the Steam app id}"
windows="${COWS_STEAM_DEPOT_WINDOWS:?set COWS_STEAM_DEPOT_WINDOWS to the Windows depot id}"
macos="${COWS_STEAM_DEPOT_MACOS:?set COWS_STEAM_DEPOT_MACOS to the macOS depot id}"
user="${COWS_STEAM_USER:?set COWS_STEAM_USER to the Steam build account login}"
branch="${COWS_STEAM_BRANCH:-}"

content="$root/Deploy/Steam/content"
output="$root/Deploy/Steam/output"

for depot in windows/Cows.exe "macos/Cows In Love.app"; do
    [[ -e "$content/$depot" ]] || { echo "missing $content/$depot" >&2; exit 1; }
done

codesign --verify --strict "$content/macos/Cows In Love.app"
xcrun stapler validate "$content/macos/Cows In Love.app" \
    || { echo "the macOS app is not notarized: run tools/release-macos.sh" >&2; exit 1; }

version="$(sed -n 's/^project(Cows VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"
scripts="$output/scripts"
mkdir -p "$scripts"

for vdf in Deploy/Steam/scripts/*.vdf; do
    sed -e "s|@APP_ID@|$app|g" -e "s|@DEPOT_WINDOWS@|$windows|g" \
        -e "s|@DEPOT_MACOS@|$macos|g" -e "s|@VERSION@|$version|g" \
        -e "s|@CONTENT@|$content|g" -e "s|@OUTPUT@|$output|g" \
        -e "s|@BRANCH@|$branch|g" "$vdf" > "$scripts/$(basename "$vdf")"
done

[[ -n "$branch" ]] || sed -i '' '/"SetLive"/d' "$scripts/app_build.vdf"

steamcmd +login "$user" +run_app_build "$scripts/app_build.vdf" +quit
