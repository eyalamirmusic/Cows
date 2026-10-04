#!/usr/bin/env bash
# Usage: COWS_TEAM=<team id> [COWS_BUILD_NUMBER=n] [COWS_BUNDLE_ID=id]
#        tools/release-ios.sh [--upload]
# Builds a Release archive for the App Store and exports the .ipa into
# Deploy/Apple-iOS/out/. --upload sends it straight to App Store Connect.
# Signing is automatic: Xcode creates the distribution certificate and profile
# when it needs to, through the account Xcode is signed into, or without one
# when COWS_ASC_KEY_ID, COWS_ASC_ISSUER_ID and COWS_ASC_KEY (the .p8 path) name
# an App Store Connect API key (`just release-ios` takes them from 1Password).
# COWS_TEAM=none builds an unsigned archive, to check the build only.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

team="${COWS_TEAM:?set COWS_TEAM to the Apple Developer team id (or none)}"
build_number="${COWS_BUILD_NUMBER:-1}"
bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.play}"
out=Deploy/Apple-iOS/out
build=build-ios-appstore
archive="$out/CowsInLove.xcarchive"

signing=(-DCOWS_IOS_TEAM="$team" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic)
xcode_signing=(-allowProvisioningUpdates)
if [[ -n "${COWS_ASC_KEY_ID:-}" ]]; then
    xcode_signing+=(-authenticationKeyPath "$COWS_ASC_KEY"
        -authenticationKeyID "$COWS_ASC_KEY_ID"
        -authenticationKeyIssuerID "$COWS_ASC_ISSUER_ID")
fi
if [[ "$team" == none ]]; then
    signing=(-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO)
    xcode_signing=()
fi

cmake -G Xcode -B "$build" -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_XCODE_GENERATE_SCHEME=ON \
    -DCMAKE_XCODE_ATTRIBUTE_SKIP_INSTALL=YES \
    -DCOWS_BUILD_TESTS=OFF \
    -DCOWS_BUILD_NUMBER="$build_number" -DCOWS_BUNDLE_ID="$bundle_id" \
    "${signing[@]}"

rm -rf "$archive"
xcodebuild -project "$build/Cows.xcodeproj" -scheme Cows -configuration Release \
    -destination generic/platform=iOS -archivePath "$archive" \
    "${xcode_signing[@]}" archive

app="$archive/Products/Applications/Cows.app"
[[ -d "$app" ]] || { echo "archive has no Cows.app" >&2; exit 1; }
echo "archived $(plutil -extract CFBundleShortVersionString raw "$app/Info.plist")" \
    "($(plutil -extract CFBundleVersion raw "$app/Info.plist"))" >&2

[[ "$team" == none ]] && exit 0

options="$(mktemp -d)/ExportOptions.plist"
sed -e "s/TEAM_ID/$team/" \
    -e "s/DESTINATION/$([[ "${1:-}" == --upload ]] && echo upload || echo export)/" \
    Deploy/Apple-iOS/ExportOptions.plist > "$options"

xcodebuild -exportArchive -archivePath "$archive" -exportPath "$out" \
    -exportOptionsPlist "$options" "${xcode_signing[@]}"
ls "$out"
