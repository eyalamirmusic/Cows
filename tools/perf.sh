#!/usr/bin/env bash
# Usage: tools/perf.sh <scenario> <label> [runs]
#
# Runs the macOS app `runs` times (default 10), a fresh process each, in one
# fixed scenario, and appends a row per run to $COWS_PERF_DIR/<label>/<scenario>.csv
# (default /tmp/cows-perf). Each run waits out a warm-up, then measures a
# window: CPU seconds per wall second (process and main thread), the energy the
# kernel bills the process (rusage ri_energy_nj), top's %CPU and POWER, the
# system's GPU utilisation, and the averages of the COWS_PROFILE lines logged
# in the window. Per-thread CPU goes to <scenario>-threads.jsonl beside it.
#
# Scenarios (all COWS_SEED=3, COWS_WINDOW=1280x800, COWS_QUALITY=high):
#   S1 menu idle          S2 editor open (COWS_DRESS=1)
#   S3 play, standing     S4 play, walking ahead (COWS_WALK=1)
#   S5 the ending (COWS_FOUND=1; it loops to the next level 4.5 s in, so its
#      warm-up is 1 s and its window 3 s)
#
# PERF_BUILD picks the build directory (default build-release); PERF_WARMUP and
# PERF_WINDOW override the seconds. Extra COWS_* settings in the environment
# pass through, so `COWS_MSAA=1 tools/perf.sh S3 msaa1` varies one knob.
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
scenario="${1:?scenario S1..S5}"
label="${2:?label}"
runs="${3:-10}"
build="${PERF_BUILD:-build-release}"
bin="$root/$build/Apps/CowsInLove/Cows In Love.app/Contents/MacOS/Cows In Love"
[[ -x "$bin" ]] || { echo "no binary at $bin" >&2; exit 1; }

warmup=5
window=30
scenario_env=()
case "$scenario" in
    S1) ;;
    S2) scenario_env=(COWS_DRESS=1) ;;
    S3) scenario_env=(COWS_MENU=0) ;;
    S4) scenario_env=(COWS_MENU=0 COWS_WALK=1) ;;
    S5) scenario_env=(COWS_FOUND=1); warmup=1; window=3 ;;
    *) echo "unknown scenario $scenario" >&2; exit 1 ;;
esac
warmup="${PERF_WARMUP:-$warmup}"
window="${PERF_WINDOW:-$window}"

passthrough=()
while IFS='=' read -r name value; do
    case "$name" in
        COWS_SEED | COWS_WINDOW | COWS_QUALITY | COWS_PROFILE | COWS_PERF_DIR) ;;
        COWS_*) passthrough+=("$name=$value") ;;
    esac
done < <(env)

out="${COWS_PERF_DIR:-/tmp/cows-perf}/$label"
mkdir -p "$out"
csv="$out/$scenario.csv"
header="label,scenario,run,build,commit,warmup_s,window_s,cpu_s_per_s,user_s_per_s,system_s_per_s,main_s_per_s,other_s_per_s,energy_w,ipc,wakeups_per_s,footprint_mb,top_cpu,top_power,gpu_util,fps,frame_ms,frame_p95_ms,cpu_update_ms,cpu_gather_ms,cpu_shadows_ms,cpu_scene_ms,cpu_hud_ms,gpu_ms,gpu_shadows_ms,gpu_scene_ms,env"
[[ -f "$csv" ]] || echo "$header" > "$csv"

commit="$(git -C "$root" rev-parse --short HEAD)$(git -C "$root" diff --quiet HEAD -- Apps Lib CMake || echo +)"

if pgrep -f "Cows In Love.app/Contents/MacOS/Cows In Love" >/dev/null; then
    echo "another Cows is running; the numbers would share the machine" >&2
fi

pid=""
trap '[[ -n "$pid" ]] && kill "$pid" 2>/dev/null || true' EXIT

for ((run = 1; run <= runs; run++)); do
    log="$out/$scenario-$run.log"
    env COWS_SEED=3 COWS_WINDOW=1280x800 COWS_QUALITY=high COWS_PROFILE=1 \
        ${scenario_env[@]+"${scenario_env[@]}"} \
        ${passthrough[@]+"${passthrough[@]}"} \
        "$bin" >"$log" 2>&1 &
    pid=$!

    sleep "$warmup"
    toplog="$out/$scenario-$run.top"
    gpulog="$out/$scenario-$run.gpu"
    top -l $((window + 1)) -s 1 -pid "$pid" -stats pid,cpu,power >"$toplog" 2>/dev/null &
    toppid=$!
    (for ((s = 0; s < window; s++)); do
        ioreg -r -c IOAccelerator -d 1 -w 0 \
            | grep -o '"Device Utilization %"=[0-9]*' | head -1 | cut -d= -f2
        sleep 1
    done) >"$gpulog" &
    gpupid=$!

    linesBefore=$(wc -l <"$log")
    before="$(python3 "$root/tools/perf-probe.py" "$pid")"
    sleep "$window"
    after="$(python3 "$root/tools/perf-probe.py" "$pid")"
    linesAfter=$(wc -l <"$log")

    kill "$pid" 2>/dev/null || true
    wait "$pid" 2>/dev/null || true
    pid=""
    wait "$toppid" 2>/dev/null || true
    wait "$gpupid" 2>/dev/null || true

    sed -n "$((linesBefore + 1)),${linesAfter}p" "$log" | grep "profile fps" >"$out/$scenario-$run.profile" || true

    python3 - "$before" "$after" "$out/$scenario-$run.profile" "$toplog" "$gpulog" \
        "$label,$scenario,$run,$build,$commit,$warmup,$window" \
        "${scenario_env[*]+${scenario_env[*]}} ${passthrough[*]+${passthrough[*]}}" \
        "$out/$scenario-threads.jsonl" >>"$csv" <<'PY'
import json, re, sys

before, after = json.loads(sys.argv[1]), json.loads(sys.argv[2])
profile, toplog, gpulog, lead, envtext, threads_out = sys.argv[3:9]
wall = after["wall"] - before["wall"]

def rate(key):
    return (after[key] - before[key]) / wall

cpu = rate("user_s") + rate("system_s")
# The main thread's pthread lives in libpthread's data, above every other
# thread's stack, so its handle is the largest; 0 is an empty slot.
handles = sorted((h for h in after["threads"] if h != "0"), key=int)
main = handles[-1] if handles else None

def thread_rate(handle):
    was = before["threads"].get(handle, {"cpu_s": 0.0})["cpu_s"]
    return (after["threads"][handle]["cpu_s"] - was) / wall

per_thread = {
    h: {"name": after["threads"][h]["name"] or ("main" if h == main else ""),
        "cpu_s_per_s": thread_rate(h)}
    for h in handles
}
main_rate = per_thread[main]["cpu_s_per_s"] if main else 0.0
cycles = after["cycles"] - before["cycles"]
ipc = (after["instructions"] - before["instructions"]) / cycles if cycles else 0.0

with open(threads_out, "a") as f:
    f.write(json.dumps({"lead": lead, "threads": per_thread}) + "\n")

top_cpu, top_power = [], []
for line in open(toplog):
    parts = line.split()
    if len(parts) == 3 and parts[0].isdigit():
        top_cpu.append(float(parts[1]))
        top_power.append(float(parts[2]))
top_cpu, top_power = top_cpu[1:], top_power[1:]

gpu = [float(x) for x in open(gpulog).read().split() if x.strip()]

fields = {k: [] for k in "fps avg p95 update gather shadows scene hud gpu gpus gpuscene".split()}
number = r"([0-9.]+)"
patterns = {
    "fps": r"profile fps " + number,
    "avg": r"frame avg " + number,
    "p95": r" p95 " + number,
    "update": r"update " + number,
    "gather": r"gather " + number,
    "shadows": r"\| cpu ms .*? shadows " + number,
    "scene": r"\| cpu ms .*? scene " + number,
    "hud": r" hud " + number,
    "gpu": r"\| gpu ms " + number,
    "gpus": r"\| gpu ms [0-9.]+ shadows " + number,
    "gpuscene": r"\| gpu ms [0-9.]+(?: shadows [0-9.]+)? scene " + number,
}
for line in open(profile):
    for key, pattern in patterns.items():
        found = re.search(pattern, line)
        if found:
            fields[key].append(float(found.group(1)))

def mean(values):
    return f"{sum(values) / len(values):.4f}" if values else ""

row = [lead, f"{cpu:.4f}", f"{rate('user_s'):.4f}", f"{rate('system_s'):.4f}",
       f"{main_rate:.4f}", f"{cpu - main_rate:.4f}", f"{rate('energy_j'):.4f}",
       f"{ipc:.3f}", f"{rate('wakeups'):.1f}", f"{after['footprint_mb']:.1f}",
       mean(top_cpu), mean(top_power), mean(gpu)]
row += [mean(fields[k]) for k in
        "fps avg p95 update gather shadows scene hud gpu gpus gpuscene".split()]
row.append(envtext.strip().replace(",", ";"))
print(",".join(row))
PY
    tail -1 "$csv" | cut -d, -f2,3,8,13,20,28 | sed 's/^/  /' >&2
    sleep 1
done

echo "$csv"
