#!/usr/bin/env bash
# Usage: tools/deploy-android.sh <versionCode> [track] [status] [confirm]
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ $# -lt 1 || -z "$1" ]]; then
    echo "usage: $0 <versionCode> [track] [status] [confirm]" >&2
    exit 2
fi

build_number="$1"
track="${2:-internal}"
status="${3:-completed}"
confirm="${4:-false}"

tools/release-android.sh "-DCOWS_BUILD_NUMBER=$build_number"
exec fastlane android upload track:"$track" status:"$status" confirm:"$confirm"
