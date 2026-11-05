#!/bin/bash
# Regenerates Lib/cowsinlove_Actors/Resources/<name>.obj from
# tools/blender/<name>.py: tools/prop-mesh.sh barn, tools/prop-mesh.sh bale.
set -euo pipefail

name="${1:?usage: tools/prop-mesh.sh <name>}"
root="$(cd "$(dirname "$0")/.." && pwd)"
blender="${BLENDER:-/Applications/Blender.app/Contents/MacOS/Blender}"
out="$root/Lib/cowsinlove_Actors/Resources/$name.obj"

"$blender" -b --factory-startup --python-exit-code 1 \
    --python "$root/tools/blender/$name.py" -- "$out"

rm -f "${out%.obj}.mtl"
grep -c '^f ' "$out" | xargs -I{} echo "$name.obj: {} triangles"
