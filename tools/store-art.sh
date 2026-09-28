#!/usr/bin/env bash
# Usage: tools/store-art.sh [--no-render]
# Renders the game's key art with the CowsArt tool (tools/Art) into
# Deploy/Art/Renders, then cuts every icon and store image from it with
# ImageMagick: the app icons in Apps/CowsInLove/Resources, the Steam store and
# library art in Deploy/Steam/Store, the MSIX tiles in
# Deploy/Microsoft-Store/Assets, the Android adaptive icon in
# Apps/CowsInLove/Android/res and the Google Play icon and feature graphic in
# Deploy/Google-Play/Store.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

renders=Deploy/Art/Renders
resources=Apps/CowsInLove/Resources
store=Deploy/Steam/Store

if [[ "${1:-}" != "--no-render" ]]; then
    cmake --build build --target CowsArt >&2
    build/tools/Art/CowsArt >&2
fi

mkdir -p "$store" "$resources/Assets.xcassets/AppIcon.appiconset"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

# The logo, trimmed, with a soft white halo so it reads over sky and grass.
magick "$renders/logo.png" -trim +repage -bordercolor none -border 60 \
    \( +clone -alpha extract -blur 0x18 -level 0,40% \
       -background white -alpha shape \) \
    +swap -compose over -composite "$work/logo.png"

# cover <backdrop> <width> <height> <gravity> <out>
cover() {
    magick "$1" -resize "${2}x${3}^" -gravity "$4" -extent "${2}x${3}" "$5"
}

# capsule <backdrop> <width> <height> <gravity> <logo width %> <logo top %> <out>
capsule() {
    local width=$2 height=$3
    local logo_width=$((width * $5 / 100))
    local top=$((height * $6 / 100))
    cover "$1" "$width" "$height" "$4" "$work/base.png"
    magick "$work/base.png" \( "$work/logo.png" -resize "${logo_width}x" \) \
        -gravity north -geometry "+0+${top}" -compose over -composite \
        -strip "$7"
}

wide="$renders/backdrop-wide.png"
tall="$renders/backdrop-tall.png"

capsule "$wide" 920 430 center 84 4 "$store/header_capsule.png"
capsule "$wide" 920 430 center 84 4 "$store/library_header.png"
capsule "$wide" 462 174 center 92 2 "$store/small_capsule.png"
capsule "$wide" 1232 706 center 80 6 "$store/main_capsule.png"
capsule "$tall" 748 896 south 88 8 "$store/vertical_capsule.png"
capsule "$tall" 600 900 south 90 10 "$store/library_capsule.png"
cover "$renders/hero.png" 3840 1240 center "$store/library_hero.png"
magick "$work/logo.png" -resize 1280x720 -gravity center -background none \
    -extent 1280x720 -strip "$store/library_logo.png"
cover "$wide" 1438 810 center "$work/page.png"
magick "$work/page.png" -blur 0x6 -modulate 105,80 -strip \
    "$store/page_background.png"

# iOS: one full-bleed 1024 icon with no alpha; the system rounds the corners.
magick "$renders/icon.png" -resize 1024x1024 -background white -alpha remove \
    -alpha off -strip "$resources/Assets.xcassets/AppIcon.appiconset/AppIcon.png"

# macOS and Windows: the same art on Apple's icon grid, an 824 rounded square
# with a soft drop shadow inside a 1024 canvas.
magick "$renders/icon.png" -resize 824x824 \
    \( -size 824x824 xc:none -fill white \
       -draw "roundrectangle 0,0 823,823 185,185" \) \
    -alpha off -compose copy-opacity -composite "$work/rounded.png"
magick -size 1024x1024 xc:none \
    \( "$work/rounded.png" -background black -shadow 50x14+0+10 \) \
    -gravity center -compose over -composite \
    "$work/rounded.png" -gravity center -compose over -composite \
    -strip "$resources/AppIcon-Desktop.png"

magick "$resources/AppIcon-Desktop.png" -resize 184x184 -background white \
    -alpha remove -quality 92 "$store/community_icon.jpg"
magick "$resources/AppIcon-Desktop.png" -define icon:auto-resize=256,64,48,32,16 \
    "$store/client_icon.ico"

# Microsoft Store (MSIX): square tiles are the full-bleed icon; the wide tile
# and splash screen are the icon on the sky colour. scale-100 and scale-200.
msix=Deploy/Microsoft-Store/Assets
sky="#8BD5F6"
mkdir -p "$msix"
full="$resources/Assets.xcassets/AppIcon.appiconset/AppIcon.png"

square() {
    for scale in 100 200; do
        local size=$(($2 * scale / 100))
        magick "$full" -resize "${size}x${size}" -strip "$msix/$1.scale-$scale.png"
    done
}

banner() {
    for scale in 100 200; do
        local width=$(($2 * scale / 100)) height=$(($3 * scale / 100))
        local icon=$((height * 80 / 100))
        magick -size "${width}x${height}" "xc:$sky" \
            \( "$resources/AppIcon-Desktop.png" -resize "${icon}x${icon}" \) \
            -gravity center -compose over -composite -strip \
            "$msix/$1.scale-$scale.png"
    done
}

square Square44x44Logo 44
square Square71x71Logo 71
square Square150x150Logo 150
square Square310x310Logo 310
square StoreLogo 50
banner Wide310x150Logo 310 150
banner SplashScreen 620 300

# Microsoft Store listing art: the 2:3 poster and the 1:1 box art.
mkdir -p Deploy/Microsoft-Store/Store
capsule "$tall" 1440 2160 south 88 8 Deploy/Microsoft-Store/Store/poster_1440x2160.png
capsule "$wide" 2160 2160 south 86 12 Deploy/Microsoft-Store/Store/boxart_2160x2160.png

for size in 16 24 32 48 256; do
    magick "$resources/AppIcon-Desktop.png" -resize "${size}x${size}" -strip \
        "$msix/Square44x44Logo.targetsize-${size}_altform-unplated.png"
done

# Android adaptive icon (API 26+, and minSdk is 33, so no legacy PNGs): 108dp
# layers at five densities. The foreground is the icon framed wider, so the two
# cows and the hearts sit inside the 66dp safe circle; the background is the
# sky colour; the monochrome layer (themed icons) is the rising hearts, keyed
# out of the icon render by colour.
android=Apps/CowsInLove/Android/res
magick "$renders/icon.png" -alpha off -crop 1024x540+0+0 +repage \
    -fx '(r>0.6 && g<0.45 && r-g>0.35) ? 1 : 0' -colorspace gray \
    -morphology Open Disk:3 -morphology Close Disk:6 -trim +repage \
    -resize 520x520 -background black -gravity center -extent 1024x1024 \
    "$work/hearts-mask.png"
magick -size 1024x1024 xc:white "$work/hearts-mask.png" -alpha off \
    -compose copy-opacity -composite "$work/monochrome.png"

for density in mdpi:108 hdpi:162 xhdpi:216 xxhdpi:324 xxxhdpi:432; do
    dir="$android/drawable-${density%%:*}"
    size="${density##*:}"
    mkdir -p "$dir"
    magick "$renders/icon-adaptive.png" -resize "${size}x${size}" -alpha off \
        -strip "PNG24:$dir/ic_launcher_foreground.png"
    magick "$work/monochrome.png" -resize "${size}x${size}" -strip \
        "PNG32:$dir/ic_launcher_monochrome.png"
done

# Google Play listing: the 512 hi-res icon (32-bit PNG, opaque; Play rounds
# the corners) and the 1024x500 feature graphic (no alpha).
play=Deploy/Google-Play/Store
mkdir -p "$play"
magick "$renders/icon.png" -resize 512x512 -background white -alpha remove \
    -strip "PNG32:$play/icon_512.png"
capsule "$wide" 1024 500 center 72 6 "$work/feature.png"
magick "$work/feature.png" -background white -alpha remove -alpha off \
    "PNG24:$play/feature_graphic_1024x500.png"

ls -1 "$store" "$resources" "$msix" "$android" "$play" >&2
