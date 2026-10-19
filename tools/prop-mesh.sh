#!/bin/bash
# Usage: [BLENDER=path] tools/prop-mesh.sh barn|bale
set -euo pipefail
export PYTHONDONTWRITEBYTECODE=1

name="${1:?usage: tools/prop-mesh.sh <name>}"
root="$(cd "$(dirname "$0")/.." && pwd)"
blender="${BLENDER:-/Applications/Blender.app/Contents/MacOS/Blender}"
out="$root/Lib/cowsinlove_Actors/Resources/$name.mesh"

"$blender" -b --factory-startup --python-exit-code 1 \
    --python "$root/tools/blender/$name.py" -- "$out" | grep "^$name: "
ls -l "$out" | awk '{print $5 " bytes"}'
