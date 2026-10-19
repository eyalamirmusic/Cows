#!/usr/bin/env bash
# Usage: [COWS_WINDOWS=user@host] tools/release-microsoft.sh
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

out=Deploy/Microsoft-Store/out
version="$(sed -n 's/^project(Cows VERSION \([0-9]*\.[0-9]*\.[0-9]*\).*/\1/p' CMakeLists.txt)"
artifact="CowsInLove-$version.0"

case "$(uname -s)" in
    MINGW* | MSYS* | CYGWIN*)
        # Public package metadata, not credentials.
        export COWS_MSIX_IDENTITY="${COWS_MSIX_IDENTITY:-BarnSoftware.CowsInLove}"
        export COWS_MSIX_PUBLISHER="${COWS_MSIX_PUBLISHER:-CN=AB75CE1F-98E3-40BA-869D-0645B2F844EA}"
        export COWS_MSIX_PUBLISHER_NAME="${COWS_MSIX_PUBLISHER_NAME:-Barn Software}"

        build() {
            local log="$1"
            if cmd //c "tools\\build-windows.bat Release" >"$log" 2>&1; then
                tail -3 "$log"
                return 0
            fi
            return 1
        }

        unset COWS_ARCH
        if ! build release-microsoft-x64.log; then
            tail -60 release-microsoft-x64.log
            exit 1
        fi

        # Cross-building, ResEmbed and eacp build arm64 tools they then run; swap in x64 ones.
        host_tools=(
            _deps/resembed-build/Generator/_host/ResourceGenerator.exe
            _deps/eacp-build/Lib/eacp/Graphics/IconTool/eacp-icon-tool.exe
        )
        export COWS_ARCH=arm64
        attempt=1
        until build release-microsoft-arm64.log; do
            if ((attempt == 3)) ||
                ! grep -q 'is not compatible with the version of Windows' \
                    release-microsoft-arm64.log; then
                tail -60 release-microsoft-arm64.log
                exit 1
            fi
            for tool in "${host_tools[@]}"; do
                if [[ -f "build-windows-arm64/$tool" ]]; then
                    cp "build-windows/$tool" "build-windows-arm64/$tool"
                fi
            done
            attempt=$((attempt + 1))
        done
        unset COWS_ARCH

        powershell.exe -NoProfile -ExecutionPolicy Bypass \
            -File tools/release-msix.ps1 -Arch x64
        powershell.exe -NoProfile -ExecutionPolicy Bypass \
            -File tools/release-msix.ps1 -Arch arm64
        ls -lh "$out/$artifact-x64.msix" "$out/$artifact-arm64.msix"
        exit 0
        ;;
esac

host="${COWS_WINDOWS:-jamie@tamby-windows}"
remote=projects/Cows-release
eacp_ref="${COWS_EACP_REF:-$(sed -n 's/^set(COWS_EACP_TAG "\([^"]*\)".*/\1/p' CMake/Findeacp.cmake)}"
: "${eacp_ref:=develop}"

# shellcheck disable=SC2029
git ls-files -z --cached --others --exclude-standard \
    | while IFS= read -r -d '' file; do [[ -e "$file" ]] && printf '%s\0' "$file"; done \
    | COPYFILE_DISABLE=1 tar --no-xattrs --null -T - -czf - \
    | ssh "$host" "mkdir -p ~/$remote && tar -xzf - -C ~/$remote"

ssh "$host" bash -s -- "$remote" "$eacp_ref" <<'REMOTE'
set -euo pipefail
cd ~/projects
[[ -d Cows-eacp ]] || git clone -q --filter=blob:none https://github.com/eyalamirmusic/eacp.git Cows-eacp
git -C Cows-eacp fetch -q origin
eacp_ref="$2"
if git -C Cows-eacp rev-parse --verify --quiet "origin/$eacp_ref^{commit}" >/dev/null; then
    eacp_ref="origin/$eacp_ref"
fi
git -C Cows-eacp checkout -q --detach "$eacp_ref"
export COWS_EACP="$(cygpath -m ~/projects/Cows-eacp)"
cd ~/"$1"
tools/release-microsoft.sh > release-microsoft.log 2>&1 || {
    tail -80 release-microsoft.log
    exit 1
}
tail -12 release-microsoft.log
REMOTE

mkdir -p "$out" build-windows/Apps/CowsInLove build-windows-arm64/Apps/CowsInLove
scp -q "$host:$remote/$out/$artifact-x64.msix" "$out/"
scp -q "$host:$remote/$out/$artifact-arm64.msix" "$out/"
scp -q "$host:$remote/build-windows/Apps/CowsInLove/Cows.exe" \
    build-windows/Apps/CowsInLove/Cows.exe
scp -q "$host:$remote/build-windows-arm64/Apps/CowsInLove/Cows.exe" \
    build-windows-arm64/Apps/CowsInLove/Cows.exe

for arch in x64 arm64; do
    package="$out/$artifact-$arch.msix"
    if [[ "$arch" == x64 ]]; then
        exe=build-windows/Apps/CowsInLove/Cows.exe
    else
        exe=build-windows-arm64/Apps/CowsInLove/Cows.exe
    fi
    unzip -tq "$package" >/dev/null
    unzip -p "$package" AppxManifest.xml \
        | grep -q "ProcessorArchitecture=\"$arch\""
    cmp -s <(unzip -p "$package" Cows.exe) "$exe"
done
ls -lh "$out/$artifact-x64.msix" "$out/$artifact-arm64.msix" \
    build-windows/Apps/CowsInLove/Cows.exe \
    build-windows-arm64/Apps/CowsInLove/Cows.exe
