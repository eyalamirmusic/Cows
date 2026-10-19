#!/usr/bin/env bash
# Usage: COWS_SIGN_IDENTITY="Developer ID Application: <Name> (<TEAM>)" COWS_NOTARY_PROFILE=<profile>|skip [COWS_SIGN_KEYCHAIN=path] [COWS_BUNDLE_ID=id] tools/release-macos.sh
# One-time: xcrun notarytool store-credentials <profile> --apple-id <id> --team-id <TEAM> --password <app-specific password>
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

identity="${COWS_SIGN_IDENTITY:?set COWS_SIGN_IDENTITY to the Developer ID Application identity (security find-identity -v -p codesigning)}"
notary="${COWS_NOTARY_PROFILE:?set COWS_NOTARY_PROFILE to a notarytool keychain profile (or skip)}"
bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.play}"
build=build-macos-release
depot=Deploy/Steam/content/macos
app="$build/Apps/CowsInLove/Cows In Love.app"

cmake -G Ninja -B "$build" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCOWS_BUILD_TESTS=OFF \
    -DCOWS_BUNDLE_ID="$bundle_id" \
    ${EACP:+-DCPM_eacp_SOURCE=$EACP}
cmake --build "$build" --target Cows

lipo -info "$app/Contents/MacOS/Cows In Love" >&2

codesign --force --timestamp --options runtime \
    ${COWS_SIGN_KEYCHAIN:+--keychain "$COWS_SIGN_KEYCHAIN"} \
    --sign "$identity" "$app"
codesign --verify --strict --verbose=2 "$app"

if [[ "$notary" != skip ]]; then
    zip="$(mktemp -d)/Cows.zip"
    ditto -c -k --keepParent "$app" "$zip"
    xcrun notarytool submit "$zip" --keychain-profile "$notary" --wait
    xcrun stapler staple "$app"
    spctl --assess --type execute --verbose "$app"
else
    echo "NOT NOTARIZED (COWS_NOTARY_PROFILE=skip): do not upload this build" >&2
fi

rm -rf "$depot"
mkdir -p "$depot"
ditto "$app" "$depot/Cows In Love.app"
echo "$depot/Cows In Love.app" >&2
