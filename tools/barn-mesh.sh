#!/bin/bash
# Regenerates Lib/cowsinlove_Actors/Resources/barn.obj from tools/blender/barn.py.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
blender="${BLENDER:-/Applications/Blender.app/Contents/MacOS/Blender}"
out="$root/Lib/cowsinlove_Actors/Resources/barn.obj"

"$blender" -b --factory-startup --python-exit-code 1 \
    --python "$root/tools/blender/barn.py" -- "$out"

rm -f "${out%.obj}.mtl"
grep -c '^f ' "$out" | xargs -I{} echo "barn.obj: {} triangles"
