#!/usr/bin/env bash
# Usage: COWS_TEAM=<team id> [COWS_BUILD_NUMBER=n] [COWS_BUNDLE_ID=id]
#        tools/release-mas.sh [--upload]
# Builds a sandboxed, universal Release archive for the Mac App Store and
# exports the signed .pkg into Deploy/Apple-macOS/out/. --upload sends it
# straight to App Store Connect. Signing is automatic (Apple Distribution and
# 3rd Party Mac Developer Installer, created by Xcode on first export).
# COWS_TEAM=none builds an unsigned archive, to check the build only.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

team="${COWS_TEAM:?set COWS_TEAM to the Apple Developer team id (or none)}"
build_number="${COWS_BUILD_NUMBER:-1}"
bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.play}"
out=Deploy/Apple-macOS/out
build=build-mas
archive="$out/CowsInLove.xcarchive"

signing=(-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="$team"
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="Apple Development")
xcode_signing=(-allowProvisioningUpdates)
if [[ "$team" == none ]]; then
    signing=(-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO)
    xcode_signing=()
fi

cmake -G Xcode -B "$build" \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_XCODE_GENERATE_SCHEME=ON \
    -DCMAKE_XCODE_ATTRIBUTE_SKIP_INSTALL=YES \
    -DCMAKE_XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME=YES \
    -DCOWS_MAC_ENTITLEMENTS="$root/Deploy/Apple-macOS/Cows.entitlements" \
    -DCOWS_BUILD_TESTS=OFF \
    -DCOWS_BUILD_NUMBER="$build_number" -DCOWS_BUNDLE_ID="$bundle_id" \
    "${signing[@]}"

rm -rf "$archive"
xcodebuild -project "$build/Cows.xcodeproj" -scheme Cows -configuration Release \
    -destination generic/platform=macOS -archivePath "$archive" \
    "${xcode_signing[@]}" archive

app="$archive/Products/Applications/Cows.app"
[[ -d "$app" ]] || { echo "archive has no Cows.app" >&2; exit 1; }
lipo -info "$app/Contents/MacOS/Cows" >&2
echo "archived $(plutil -extract CFBundleShortVersionString raw "$app/Contents/Info.plist")" \
    "($(plutil -extract CFBundleVersion raw "$app/Contents/Info.plist"))" >&2

[[ "$team" == none ]] && exit 0

codesign -d --entitlements - "$app" 2>/dev/null | grep -q app-sandbox \
    || { echo "the archived app is not sandboxed" >&2; exit 1; }

options="$(mktemp -d)/ExportOptions.plist"
sed -e "s/TEAM_ID/$team/" \
    -e "s/DESTINATION/$([[ "${1:-}" == --upload ]] && echo upload || echo export)/" \
    Deploy/Apple-macOS/ExportOptions.plist > "$options"

xcodebuild -exportArchive -archivePath "$archive" -exportPath "$out" \
    -exportOptionsPlist "$options" -allowProvisioningUpdates
ls "$out"
