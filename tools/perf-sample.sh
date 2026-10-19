#!/usr/bin/env bash
# Usage: tools/perf-sample.sh <scenario> <label> [runs] [seconds]

set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
scenario="${1:?scenario S1..S5}"
label="${2:?label}"
runs="${3:-3}"
build="${PERF_BUILD:-build-relwithdebinfo}"
bin="$root/$build/Apps/CowsInLove/Cows In Love.app/Contents/MacOS/Cows In Love"
[[ -x "$bin" ]] || { echo "no binary at $bin" >&2; exit 1; }

warmup=5
seconds="${4:-20}"
scenario_env=()
case "$scenario" in
    S1) ;;
    S2) scenario_env=(COWS_DRESS=1) ;;
    S3) scenario_env=(COWS_MENU=0) ;;
    S4) scenario_env=(COWS_MENU=0 COWS_WALK=1) ;;
    S5) scenario_env=(COWS_FOUND=1); warmup=1; seconds="${4:-3}" ;;
    *) echo "unknown scenario $scenario" >&2; exit 1 ;;
esac

out="${COWS_PERF_DIR:-/tmp/cows-perf}/$label"
mkdir -p "$out"
pid=""
trap '[[ -n "$pid" ]] && kill "$pid" 2>/dev/null || true' EXIT

for ((run = 1; run <= runs; run++)); do
    env COWS_SEED=3 COWS_WINDOW=1280x800 COWS_QUALITY=high COWS_FPS="${COWS_FPS:-auto}" \
        ${scenario_env[@]+"${scenario_env[@]}"} "$bin" >/dev/null 2>&1 &
    pid=$!
    sleep "$warmup"
    sample "$pid" "$seconds" 1 -mayDie -file "$out/$scenario-$run.sample.txt" >/dev/null 2>&1
    kill "$pid" 2>/dev/null || true
    wait "$pid" 2>/dev/null || true
    pid=""
    echo "$out/$scenario-$run.sample.txt"
    sleep 1
done
