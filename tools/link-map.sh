#!/usr/bin/env bash
# Usage: CMAKE_<LANG>_LINKER_LAUNCHER=tools/link-map.sh (from tools/dead-code.sh)
out=""
prev=""
for arg in "$@"; do
    [[ $prev == "-o" ]] && out=$arg
    prev=$arg
done
exec "$@" -Wl,-dead_strip ${out:+"-Wl,-map,$out.map"}
