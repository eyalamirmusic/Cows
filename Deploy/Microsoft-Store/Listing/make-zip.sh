#!/usr/bin/env bash
# Builds out/cows-listing (Partner Center > Import listings > Import folder)
# and out/listing-import.zip of it. listings.csv names images as cows-listing/...
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
store="$(cd "$here/.." && pwd)"
repo="$(cd "$store/../.." && pwd)"
out="$store/out"
dir="$out/cows-listing"

rm -rf "$dir" "$out/listing-import.zip"
mkdir -p "$dir/screenshots"
cp "$here/listings.csv" "$dir/"
cp "$store/Store/poster_1440x2160.png" "$store/Store/boxart_2160x2160.png" "$dir/"
cp "$repo"/Deploy/Steam/Screenshots/{1-meadow,2-moo,3-ravine,4-bridge,5-jump,6-found}.png \
    "$dir/screenshots/"

python3 - "$dir" <<'PY'
import csv, os, sys
d = sys.argv[1]
parent = os.path.dirname(d)
missing = []
for r in csv.reader(open(os.path.join(d, "listings.csv"), encoding="utf-8-sig", newline="")):
    if len(r) > 4 and r[2].startswith("Relative path") and r[4]:
        if not os.path.isfile(os.path.join(parent, r[4])):
            missing.append(r[4])
if missing:
    sys.exit("missing images: " + ", ".join(missing))
PY

(cd "$out" && zip -qrX listing-import.zip cows-listing)
echo "$dir"
echo "$out/listing-import.zip"
