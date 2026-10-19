#!/usr/bin/env bash
# Usage: tools/screenshots.sh
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
shots play-phone Deploy/Google-Play/Screenshots/phone
shots play-7 Deploy/Google-Play/Screenshots/tablet-7
shots play-10 Deploy/Google-Play/Screenshots/tablet-10

# The App Store and Google Play reject screenshots with an alpha channel.
magick mogrify -background white -alpha remove -alpha off -strip \
    Deploy/Steam/Screenshots/*.png Deploy/Apple-macOS/Screenshots/*.png \
    Deploy/Apple-iOS/Screenshots/*/*.png Deploy/Google-Play/Screenshots/*/*.png
