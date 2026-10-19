#!/usr/bin/env python3
# Usage: tools/dead-code-link.py <build dir linked through tools/link-map.sh>

import glob
import os
import subprocess
import sys

build = sys.argv[1]


def defined(objects):
    out = subprocess.run(["nm", "-m", *objects], capture_output=True, text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 4 or "weak" in line or "(__TEXT,__text)" not in line \
                and "(__DATA" not in line and "(__TEXT,__const)" not in line:
            continue
        name = parts[-1]
        if "4Cows" in name and "GLOBAL__N" not in name.split("4Cows")[0]:
            yield name


def kept(link_map):
    names, live = set(), False
    with open(link_map, errors="replace") as f:
        for line in f:
            if line.startswith("# Symbols:"):
                live = True
            elif line.startswith("# Dead Stripped Symbols:"):
                break
            elif live and not line.startswith("#"):
                parts = line.rstrip("\n").split("\t")
                if len(parts) >= 3:
                    names.add(parts[2].split("] ", 1)[-1])
    return names


objects = [o for d in ("Lib", "Apps", "tools")
           for o in glob.glob(os.path.join(build, d, "**", "*.o"), recursive=True)]
ours = set(defined(objects))
maps = glob.glob(os.path.join(build, "**", "*.map"), recursive=True)
maps = [m for m in maps if "/_deps/" not in m]
shipped, tested = set(), set()
for m in maps:
    (tested if "/Tests/" in m else shipped).update(kept(m))


def demangle(names):
    if not names:
        return []
    out = subprocess.run(["c++filt", "-_"], input="\n".join(names),
                         capture_output=True, text=True).stdout
    return sorted({n for n in out.splitlines()
                   if n.startswith("Cows::") and "::$_" not in n})


for title, names in (("not kept by any executable", ours - shipped - tested),
                     ("kept only by tests", (ours - shipped) & tested)):
    rows = demangle(sorted(names))
    print(f"## {title} ({len(rows)})")
    print("\n".join(rows))
    print()
