set shell := ["bash", "-euo", "pipefail", "-c"]

device := "D731347C-AC33-53DD-814A-1B1E4ED47AC7"

# Build and open the macOS app. EACP=$HOME/Code/eacp builds against a local eacp.
macos:
    [[ -f build/CMakeCache.txt ]] || cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build
    open build/Apps/CowsInLove/Cows.app

# Build and run on the "Cows iPhone" simulator, optionally saving a screenshot.
sim-ios shot="":
    tools/ios.sh sim {{shot}}

# Build, sign and run on a phone (devicectl UDID; defaults to pond).
ios udid=device:
    COWS_DEVICE={{udid}} tools/ios.sh device

# Screenshot the macOS app (COWS_TIME / COWS_FREEZE / COWS_SEED pass through).
shot out="":
    tools/shot.sh {{out}}

# List phones devicectl can see.
devices:
    xcrun devicectl list devices

# Build and run every library's test suite.
test:
    cmake --build build
    ctest --test-dir build --output-on-failure

# Render the icons and store art (Deploy/Art, Deploy/Steam/Store, app icons).
store-art:
    tools/store-art.sh

# Render the Steam and App Store screenshots.
screenshots:
    tools/screenshots.sh

# Universal macOS build, Developer ID signed and notarized, staged for Steam.
release-macos:
    tools/release-macos.sh

# App Store archive and export (COWS_TEAM; --upload sends it to App Store Connect).
release-ios *args:
    tools/release-ios.sh {{args}}

# Windows x64 Release build on the Windows box, staged for Steam.
release-windows:
    tools/release-windows.sh

# Upload the staged Windows and macOS depots to Steam with steamcmd.
steam-upload:
    tools/steam-upload.sh

# Mac App Store archive and export (COWS_TEAM; --upload sends it to App Store Connect).
release-mas *args:
    tools/release-mas.sh {{args}}
