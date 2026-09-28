#!/usr/bin/env bash
# Usage: tools/release-windows.sh
# Builds the Release x64 exe (static C runtime, so no redist to install) and
# stages it as the Steam Windows depot in Deploy/Steam/content/windows/.
#
# On Windows (git-bash) it builds in place. Anywhere else it copies the working
# tree to COWS_WINDOWS (default jamie@tamby-windows) over ssh, runs itself
# there, and copies the depot back.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

depot=Deploy/Steam/content/windows

case "$(uname -s)" in
    MINGW* | MSYS* | CYGWIN*)
        cmd //c "tools\\build-windows.bat Release"
        rm -rf "$depot"
        mkdir -p "$depot"
        cp build-windows/Apps/CowsInLove/Cows.exe "$depot/"
        ls -l "$depot"
        exit 0
        ;;
esac

host="${COWS_WINDOWS:-jamie@tamby-windows}"
remote=projects/Cows-release
eacp_rev="$(git -C build/_deps/eacp-src rev-parse HEAD 2>/dev/null || echo main)"

git ls-files -z --cached --others --exclude-standard \
    | while IFS= read -r -d '' file; do [[ -e "$file" ]] && printf '%s\0' "$file"; done \
    | COPYFILE_DISABLE=1 tar --no-xattrs --null -T - -czf - \
    | ssh "$host" "mkdir -p ~/$remote && tar -xzf - -C ~/$remote"

ssh "$host" bash -s -- "$remote" "$eacp_rev" <<'REMOTE'
set -euo pipefail
cd ~/projects
[[ -d Cows-eacp ]] || git clone -q --filter=blob:none https://github.com/eyalamirmusic/eacp.git Cows-eacp
git -C Cows-eacp fetch -q origin
git -C Cows-eacp checkout -q "$2"
export COWS_EACP="$(cygpath -m ~/projects/Cows-eacp)"
cd ~/"$1"
tools/release-windows.sh > release-windows.log 2>&1 || { tail -40 release-windows.log; exit 1; }
grep -E "warning C|error C" release-windows.log | grep -v "Cows-eacp\|_deps" | sort -u || true
tail -3 release-windows.log
REMOTE

rm -rf "$depot"
mkdir -p "$depot"
scp -q "$host:$remote/$depot/*" "$depot/"
ls -l "$depot"
