#!/usr/bin/env bash
# Usage: COWS_SIGN_IDENTITY="Developer ID Application: <Name> (<TEAM>)"
#        COWS_NOTARY_PROFILE=<profile> [COWS_SIGN_KEYCHAIN=<path>]
#        [COWS_BUNDLE_ID=<id>] tools/release-macos.sh
# Builds the universal (arm64 + x86_64) Release app, signs it with the
# Developer ID (hardened runtime, secure timestamp), notarizes and staples it,
# and stages it as the Steam macOS depot in Deploy/Steam/content/macos/.
# COWS_NOTARY_PROFILE=skip signs without notarizing, for a local check only.
#
# One-time notarization setup (app-specific password, or --key for an API key):
#   xcrun notarytool store-credentials <profile> --apple-id <apple id> \
#       --team-id <TEAM> --password <app-specific password>
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

identity="${COWS_SIGN_IDENTITY:?set COWS_SIGN_IDENTITY to the Developer ID Application identity (security find-identity -v -p codesigning)}"
notary="${COWS_NOTARY_PROFILE:?set COWS_NOTARY_PROFILE to a notarytool keychain profile (or skip)}"
bundle_id="${COWS_BUNDLE_ID:-com.cowsinlove.cows}"
build=build-macos-release
depot=Deploy/Steam/content/macos
app="$build/Apps/CowsInLove/Cows.app"

cmake -G Ninja -B "$build" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCOWS_BUILD_TESTS=OFF \
    -DCOWS_BUNDLE_ID="$bundle_id" \
    ${EACP:+-DCPM_eacp_SOURCE=$EACP}
cmake --build "$build" --target Cows

lipo -info "$app/Contents/MacOS/Cows" >&2

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
ditto "$app" "$depot/Cows.app"
echo "$depot/Cows.app" >&2
