#!/usr/bin/env bash
# Usage: [EACP=dir] tools/dead-code.sh [--check]
# --check fails on anything tools/dead-code-allow.txt does not name. Needs: brew install llvm cppcheck cmake ninja
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

if [[ ${1:-} == --check ]]; then
    report="$(mktemp)"
    "$0" | tee "$report"
    python3 tools/dead-code-check.py tools/dead-code-allow.txt <"$report"
    exit
fi

llvm="${LLVM_PREFIX:-$(brew --prefix llvm 2>/dev/null || echo /opt/homebrew/opt/llvm)}"
missing=()
for tool in cmake ninja python3 nm c++filt cppcheck; do
    command -v "$tool" >/dev/null || missing+=("$tool")
done
for tool in clang clang-tidy run-clang-tidy; do
    [[ -x "$llvm/bin/$tool" ]] || missing+=("$llvm/bin/$tool (brew install llvm)")
done
[[ -f "$llvm/lib/libclang.dylib" ]] || missing+=("$llvm/lib/libclang.dylib (brew install llvm)")
if ((${#missing[@]})); then
    printf 'missing: %s\n' "${missing[@]}" >&2
    exit 1
fi
export LLVM_PREFIX="$llvm"

build=build-dead-code
ours="^$root/(Lib|Apps|Tests|tools)/"
flags="-ffunction-sections -fdata-sections -Wunused -Wunused-member-function"
flags+=" -Wunused-template -Wunused-private-field -Wunused-const-variable"
flags+=" -Wunused-macros -Wunreachable-code -Wmissing-prototypes"

if [[ ! -f $build/CMakeCache.txt ]]; then
    CPM_SOURCE_CACHE="${CPM_SOURCE_CACHE:-$root/$build/cpm}" \
        cmake -G Ninja -B $build -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        "-DCMAKE_C_FLAGS=-ffunction-sections -fdata-sections" \
        "-DCMAKE_CXX_FLAGS=$flags" "-DCMAKE_OBJCXX_FLAGS=$flags" \
        "-DCMAKE_CXX_LINKER_LAUNCHER=$root/tools/link-map.sh" \
        "-DCMAKE_OBJCXX_LINKER_LAUNCHER=$root/tools/link-map.sh" \
        ${EACP:+-DCPM_eacp_SOURCE=$EACP} >/dev/null
fi
# Warnings only print for objects that recompile.
find $build/Lib $build/Apps $build/Tests $build/tools -name '*.o' \
    -not -path '*/_deps/*' -delete 2>/dev/null || true
cmake --build $build >$build/build.log 2>&1 || { tail -30 $build/build.log; exit 1; }

echo "## warnings"
grep -E 'warning:' $build/build.log | grep -E "$ours" \
    | sed "s|$root/||" | sort -u || true
echo

echo "## tidy"
"$llvm/bin/run-clang-tidy" -p $build -quiet -clang-tidy-binary "$llvm/bin/clang-tidy" \
    -extra-arg=-isysroot"$(xcrun --show-sdk-path)" \
    -checks='-*,misc-unused-parameters,misc-unused-using-decls,misc-unused-alias-decls,misc-definitions-in-headers,readability-redundant-declaration,misc-include-cleaner' \
    -header-filter="$root/(Lib|Apps|Tests|tools)/.*" "$root/(Lib|Apps|Tests|tools)/.*\.(cpp|mm)$" 2>/dev/null \
    | grep -E 'warning:' | grep -v /_deps/ \
    | grep -vE 'no header providing|is directly included' \
    | grep -vE 'included header (Common|Core)\.h is not used directly' \
    | sed "s|$root/||" | sort -u || true
echo

echo "## cppcheck"
python3 - "$build" "$root" >$build/ours.json <<'PY'
import json, re, sys
entries = json.load(open(f"{sys.argv[1]}/compile_commands.json"))
print(json.dumps([e for e in entries
                  if re.match(re.escape(sys.argv[2]) + r"/(Lib|Apps|Tests|tools)/", e["file"])]))
PY
mkdir -p $build/cppcheck
cppcheck --project=$build/ours.json --enable=unusedFunction --std=c++20 \
    -D__APPLE__ -D__MACH__ -D__clang__ --quiet -j "$(sysctl -n hw.ncpu)" \
    --cppcheck-build-dir=$build/cppcheck \
    --template='{file}:{line}: {message}' 2>&1 >/dev/null \
    | grep 'is never used' | grep -E "^($root/)?(Lib|Apps|Tests|tools)/" \
    | grep -vE "'(safeAreaInsetsChanged)'" \
    | sed "s|$root/||" | sort -u || true
echo

echo "## refs"
python3 tools/dead-code-refs.py $build
echo "## link"
python3 tools/dead-code-link.py $build
echo

echo "## files"
python3 - "$build" <<'PY'
import glob, json, os, re, subprocess, sys
compiled = {e["file"] for e in json.load(open(f"{sys.argv[1]}/compile_commands.json"))}
tracked = subprocess.run(["git", "ls-files"], capture_output=True, text=True).stdout.split()
code = [f for f in tracked if re.match(r"(Lib|Apps|Tests|tools)/.*\.(cpp|mm|c|h)$", f)]
for f in code:
    if not f.endswith(".h") and os.path.abspath(f) not in compiled:
        print(f"not compiled: {f}")
includes = [(f, m) for f in code
            for m in re.findall(r'#\s*include\s*"([^"]+)"', open(f, errors="replace").read())]
for h in (f for f in code if f.endswith(".h")):
    if not any(h.endswith("/" + i) and ("/" in i or os.path.dirname(f) == os.path.dirname(h))
               for f, i in includes):
        print(f"never included: {h}")
cmake = "".join(open(f).read() for f in tracked if f.endswith("CMakeLists.txt"))
for f in tracked:
    if re.match(r"Lib/[^/]+/Resources/", f) and f.endswith((".ttf", ".f32", ".png", ".wav")) \
            and os.path.basename(f) not in cmake:
        print(f"resource not embedded: {f}")
callers = [f for f in tracked if re.match(r"(justfile|CMakeLists\.txt|AGENTS\.md|Deploy/|\.github/|tools/|Apps/.*CMakeLists\.txt)", f)]
text = {f: open(f, errors="replace").read() for f in callers if os.path.isfile(f)}
for f in tracked:
    if re.match(r"tools/[^/]+\.(sh|py|ps1|bat)$", f) and f != "tools/dead-code.sh":
        if not any(os.path.basename(f) in t for c, t in text.items() if c != f):
            print(f"script nothing calls: {f}")
docs = "".join(open(f).read() for f in tracked if f.endswith(".md"))
for f in tracked:
    if f.startswith("docs/shots/") and os.path.basename(f) not in docs:
        print(f"image no doc shows: {f}")
PY
