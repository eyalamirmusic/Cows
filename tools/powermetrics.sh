#!/usr/bin/env bash
# Usage: sudo tools/powermetrics.sh [seconds] [scenarios...]

set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    echo "powermetrics needs root: sudo $0 $*" >&2
    exit 1
fi

seconds="${1:-30}"
shift || true
scenarios=("$@")
[ ${#scenarios[@]} -eq 0 ] && scenarios=(S1 S2 S3 S4 S5)

root="$(cd "$(dirname "$0")/.." && pwd)"
build="${PERF_BUILD:-build-release}"
app="${PERF_APP:-$root/$build/Apps/CowsInLove/Cows In Love.app}"
bin="$app/Contents/MacOS/Cows In Love"
out="${COWS_PERF_DIR:-/tmp/cows-perf}"
user="${SUDO_USER:?run through sudo so the app can run as you}"
warmup=5
mkdir -p "$out"
chown "$user" "$out"

if pgrep -f "Contents/MacOS/Cows In Love" >/dev/null; then
    echo "a Cows In Love is already running; quit it first" >&2
    exit 1
fi

for scenario in "${scenarios[@]}"; do
    case "$scenario" in
        S1) scenario_env=() ;;
        S2) scenario_env=(COWS_DRESS=1) ;;
        S3) scenario_env=(COWS_MENU=0) ;;
        S4) scenario_env=(COWS_MENU=0 COWS_WALK=1) ;;
        S5) scenario_env=(COWS_FOUND=1) ;;
        *) echo "unknown scenario $scenario" >&2; exit 1 ;;
    esac

    file="$out/power-$scenario-$(date +%H%M%S).txt"
    sudo -u "$user" env COWS_SEED=3 COWS_WINDOW=1280x800 COWS_QUALITY=high COWS_FPS="${COWS_FPS:-auto}" \
        ${scenario_env[@]+"${scenario_env[@]}"} "$bin" >/dev/null 2>&1 &
    app=$!
    sleep "$warmup"

    {
        echo "# scenario $scenario ${scenario_env[*]+${scenario_env[*]}} app $app"
        echo "# commit $(git -C "$root" rev-parse --short HEAD)"
        powermetrics --samplers cpu_power,gpu_power,tasks --show-process-energy \
            -i 1000 -n "$seconds"
    } >"$file"

    kill "$app" 2>/dev/null || true
    wait "$app" 2>/dev/null || true
    chown "$user" "$file"
    echo "$file"
done
