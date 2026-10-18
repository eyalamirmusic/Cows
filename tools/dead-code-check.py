#!/usr/bin/env python3
"""The CI gate over tools/dead-code.sh's report (read from stdin): every finding
is an error, printed as a GitHub annotation, except docs/shots images no doc
shows and the symbols the allowlist names. An allowlist line nothing matches is
an error too, so the list only holds what is still there.

usage: dead-code.sh | dead-code-check.py <allowlist>
"""

import re
import sys

LOCATED = re.compile(r"^(?P<file>[^:\s]+):(?P<line>\d+):(?:\d+:)? (?P<message>.*)$")
SYMBOL = re.compile(r"^(?P<file>[^:\s]+):(?P<line>\d+): (?P<kind>\w+) (?P<name>\S+)$")
FILE = re.compile(r"^(?P<what>[a-z ]+): (?P<file>\S+)$")
INFORMATIONAL = {"image no doc shows"}

allowed = {}
for number, line in enumerate(open(sys.argv[1]), 1):
    if line.strip() and not line.startswith("#"):
        allowed[line.split()[0]] = [number, 0]


def is_allowed(name):
    for key in (name, name.split("(")[0]):
        if key in allowed:
            allowed[key][1] += 1
            return True
    return False


errors = []


def error(section, message, file=None, line=None):
    where = ",".join(f"{k}={v}" for k, v in (("file", file), ("line", line)) if v)
    errors.append(f"::error {where + ',' if where else ''}title=dead code: {section}::{message}")


section = None
for row in sys.stdin.read().splitlines():
    if row.startswith("## "):
        section = re.sub(r" \(\d+\)$", "", row[3:])
        continue
    if not row.strip() or section in ("refs", "link"):
        continue
    if section in ("unused", "test-only") and (m := SYMBOL.match(row)):
        if not is_allowed(m["name"]):
            use = "is never used" if section == "unused" else "is only used by tests"
            error(section, f"{m['kind']} {m['name']} {use}", m["file"], m["line"])
    elif section in ("not kept by any executable", "kept only by tests"):
        if not is_allowed(row):
            error(section, row)
    elif section == "files" and (m := FILE.match(row)):
        if m["what"] not in INFORMATIONAL:
            error(m["what"], m["file"], m["file"])
    elif m := LOCATED.match(row):
        error(section, m["message"], m["file"], m["line"])
    else:
        error(section, row)

for name, (number, uses) in allowed.items():
    if not uses:
        error("allowlist", f"{name} is allowed but no longer reported",
              sys.argv[1], number)

print("\n".join(errors))
print(f"dead code: {len(errors)} finding(s)" if errors else "dead code: none")
sys.exit(1 if errors else 0)
