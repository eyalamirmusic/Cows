#!/usr/bin/env bash
# Usage: tools/screenshots.sh
# Renders the store screenshots through the game's own views with the CowsArt
# tool (tools/Art): Steam and Microsoft Store 1920x1080 into
# Deploy/Steam/Screenshots, Mac App Store 2880x1800 into
# Deploy/Apple-macOS/Screenshots, and App Store iPhone 6.9" (1320x2868) and
# 6.5" (1284x2778) into Deploy/Apple-iOS/Screenshots.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

cmake --build build --target CowsArt >&2

shots() {
    mkdir -p "$2"
    build/tools/Art/CowsArt "$1" "$2"
}

shots steam Deploy/Steam/Screenshots
shots mac Deploy/Apple-macOS/Screenshots
shots ios-6.9 Deploy/Apple-iOS/Screenshots/iPhone-6.9
shots ios-6.5 Deploy/Apple-iOS/Screenshots/iPhone-6.5

# The App Store rejects screenshots with an alpha channel.
magick mogrify -background white -alpha remove -alpha off -strip \
    Deploy/Steam/Screenshots/*.png Deploy/Apple-macOS/Screenshots/*.png \
    Deploy/Apple-iOS/Screenshots/*/*.png
