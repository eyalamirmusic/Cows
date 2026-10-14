#!/usr/bin/env bash
# Usage: tools/deploy-android.sh [track] [status] [confirm]
# The versionCode is the fourth number of project(Cows VERSION) in CMakeLists.txt.
set -euo pipefail
cd "$(dirname "$0")/.."

track="${1:-internal}"
status="${2:-completed}"
confirm="${3:-false}"

tools/release-android.sh
exec fastlane android upload track:"$track" status:"$status" confirm:"$confirm"
