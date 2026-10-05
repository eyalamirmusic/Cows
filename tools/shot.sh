#!/usr/bin/env bash
# Usage: [COWS_TIME=s] [COWS_FREEZE=1] [COWS_SEED=n] [COWS_STAGE=n] [COWS_FOUND=1] [COWS_MENU=0] [COWS_START=s] [COWS_DRESS=1] [SHOT_NOBUILD=1] tools/shot.sh [out.png]
# Builds, launches Cows, screenshots its window, quits. Prints the output path.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

out="${1:-docs/shots/$(date +%Y%m%d-%H%M%S).png}"
mkdir -p "$(dirname "$out")"

if [[ -z "${SHOT_NOBUILD:-}" ]]; then
    cmake --build build >&2 || { echo "build failed" >&2; exit 1; }
fi

bin="build/Apps/CowsInLove/Cows In Love.app/Contents/MacOS/Cows In Love"
[[ -x "$bin" ]] || { echo "no binary at $bin" >&2; exit 1; }

env_args=()
for v in COWS_TIME COWS_FREEZE COWS_SEED COWS_STAGE COWS_FOUND COWS_MENU COWS_START COWS_DRESS; do
    [[ -n "${!v+x}" ]] && env_args+=("$v=${!v}")
done

env ${env_args[@]+"${env_args[@]}"} "$bin" >/dev/null 2>&1 &
pid=$!
trap 'kill "$pid" 2>/dev/null || true' EXIT

sleep 3

wid="$(swift - "$pid" 2>/dev/null <<'SWIFT' || true
import CoreGraphics
let pid = Int(CommandLine.arguments[1]) ?? -1
let list = CGWindowListCopyWindowInfo([.optionOnScreenOnly, .excludeDesktopElements], kCGNullWindowID) as? [[String: Any]] ?? []
let wins = list.filter {
    ($0[kCGWindowLayer as String] as? Int) == 0 &&
    (($0[kCGWindowOwnerPID as String] as? Int) == pid ||
     ($0[kCGWindowOwnerName as String] as? String) == "Cows In Love")
}
if let w = wins.first, let n = w[kCGWindowNumber as String] as? Int { print(n) }
SWIFT
)"

if [[ -z "$wid" ]] || ! screencapture -x -o -l "$wid" "$out"; then
    echo "Cows window capture failed; capturing full screen" >&2
    screencapture -x "$out" || {
        echo "screencapture failed: grant Screen Recording to this terminal" \
             "(System Settings > Privacy & Security)" >&2
        exit 1
    }
fi

echo "$out"
