#!/bin/bash
# Renders the cow pitch's mockups into docs/design/cow: each refinement before
# and after (refine-<name>.png), the chosen ones together against today
# (together-<view>.png), the spot seed sheet, and the rejected directions.
set -euo pipefail

root="$(cd "$(dirname "$0")/../../.." && pwd)"
out="$root/docs/design/cow"
font=/System/Library/Fonts/Supplemental/Arial.ttf
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

cmake --build "$root/build" --target CowPitch
mkdir -p "$out"
rm -f "$out"/*.png
"$root/build/tools/Art/CowPitch" "$work"
"$root/build/tools/Art/CowPitch" "$work" seeds

png=(-background white -depth 8 -define png:compression-level=9)
labelled() { montage -font "$font" -pointsize 18 "$@"; }

for before in "$work"/before-*.png; do
    name="$(basename "$before" .png)"
    name="${name#before-}"
    labelled -label "today" "$before" -label "$name" "$work/after-$name.png" \
        -tile 2x1 -geometry 596x447+2+2 "${png[@]}" "$out/refine-$name.png"
done

for view in three-quarter kiss wardrobe wardrobe-back far; do
    labelled -label "today" "$work/current-$view.png" \
        -label "refined" "$work/refined-$view.png" \
        -tile 2x1 -geometry 596x447+2+2 -filter point "${png[@]}" \
        "$out/together-$view.png"
done

labelled -label "today" "$work/current-three-quarter.png" \
    -label "A  plush" "$work/plush-three-quarter.png" \
    -label "B  storybook" "$work/storybook-three-quarter.png" \
    -label "C  terminal" "$work/terminal-three-quarter.png" \
    -tile 2x2 -geometry 596x447+2+2 "${png[@]}" "$out/rejected-directions.png"

montage -font "$font" "$work"/seed-plush-*.png "$work"/seed-storybook-*.png \
    -tile 4x4 -geometry 298x223+1+1 "${png[@]}" "$out/spot-seeds.png"

ls -la "$out"
