set shell := ["bash", "-euo", "pipefail", "-c"]
# Windows runs the same recipes in Git for Windows' bash (PowerShell has no bash on PATH).
set windows-shell := ["C:/Program Files/Git/bin/bash.exe", "-euo", "pipefail", "-c"]

device := "D731347C-AC33-53DD-814A-1B1E4ED47AC7"

# Debug build in build/. EACP=$HOME/Code/eacp builds against a local eacp.
[unix]
build:
    [[ -f build/CMakeCache.txt ]] || cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug ${EACP:+-DCPM_eacp_SOURCE=$EACP}
    cmake --build build

# Debug build in build/ with the newest Visual Studio, native to this machine (arm64 or x64; COWS_ARCH overrides). EACP=<path> builds against a local eacp.
[windows]
build:
    COWS_EACP="${EACP:+$(cygpath -m "$EACP")}" cmd //c "tools\\build-windows.bat Debug"

# Build and open the macOS app.
[macos]
macos: build
    open build/Apps/CowsInLove/Cows.app

# Build and launch the Windows exe (COWS_TIME / COWS_FREEZE / COWS_SEED pass through).
[windows]
windows: build
    build/Apps/CowsInLove/Cows.exe >/dev/null 2>&1 &

# Build and run on the "Cows iPhone" simulator, optionally saving a screenshot.
[macos]
sim-ios shot="":
    tools/ios.sh sim {{shot}}

# Build, sign and run on a phone (devicectl UDID; defaults to pond).
[macos]
ios udid=device:
    COWS_DEVICE={{udid}} tools/ios.sh device

# Screenshot the macOS app (COWS_TIME / COWS_FREEZE / COWS_SEED pass through).
[macos]
shot out="":
    tools/shot.sh {{out}}

# List phones devicectl can see.
[macos]
devices:
    xcrun devicectl list devices

# Build and run every library's test suite.
test: build
    ctest --test-dir build --output-on-failure

# Render the icons and store art (Deploy/Art, Deploy/Steam/Store, app icons).
[macos]
store-art:
    tools/store-art.sh

# Render the Steam and App Store screenshots.
[macos]
screenshots:
    tools/screenshots.sh

# Universal macOS build, Developer ID signed and notarized, staged for Steam.
[macos]
release-macos:
    tools/release-macos.sh

# App Store archive and export (COWS_TEAM; --upload sends it to App Store Connect).
[macos]
release-ios *args:
    tools/release-ios.sh {{args}}

# Windows x64 Release build, here on Windows or over ssh on the Windows box, staged for Steam.
release-windows:
    tools/release-windows.sh

# Upload the staged Windows and macOS depots to Steam with steamcmd.
[macos]
steam-upload:
    tools/steam-upload.sh

# Mac App Store archive and export (COWS_TEAM; --upload sends it to App Store Connect).
[macos]
release-mas *args:
    tools/release-mas.sh {{args}}
