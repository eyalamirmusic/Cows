#!/usr/bin/env bash
# Linker launcher for tools/dead-code.sh: links with dead stripping and writes
# a link map beside the output (<output>.map).
out=""
prev=""
for arg in "$@"; do
    [[ $prev == "-o" ]] && out=$arg
    prev=$arg
done
exec "$@" -Wl,-dead_strip ${out:+"-Wl,-map,$out.map"}
