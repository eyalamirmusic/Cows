#!/usr/bin/env python3
# Usage: tools/perf-power.py <powermetrics.txt>

import re
import sys
from collections import OrderedDict

SAMPLE = re.compile(r"\*\*\* Sampled system activity \((.+?)\)")
ROW = re.compile(
    r"^Cows In Love\s+(\d+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+"
    r"([\d.]+)\s+([\d.]+)\s+([\d.]+)"
)


def main():
    runs = OrderedDict()
    when, cpu_w, gpu_w, rows = None, None, None, []

    def close():
        for pid, values in rows:
            run = runs.setdefault(pid, {"times": [], "cpu_w": [], "gpu_w": [], "rows": []})
            run["times"].append(when)
            if cpu_w is not None:
                run["cpu_w"].append(cpu_w)
            if gpu_w is not None:
                run["gpu_w"].append(gpu_w)
            run["rows"].append(values)

    for line in open(sys.argv[1], errors="replace"):
        found = SAMPLE.search(line)
        if found:
            if when is not None:
                close()
            when = found.group(1).split()[3]
            cpu_w, gpu_w, rows = None, None, []
            continue
        if line.startswith("CPU Power:"):
            cpu_w = float(line.split()[2]) / 1000
        elif line.startswith("GPU Power:") and gpu_w is None:
            gpu_w = float(line.split()[2]) / 1000
        found = ROW.match(line)
        if found:
            rows.append((found.group(1), [float(x) for x in found.groups()[1:]]))
    if when is not None:
        close()

    def mean(values):
        return sum(values) / len(values) if values else float("nan")

    print("pid    from      to        n  pkgCPU_W pkgGPU_W  cows_cpu_ms/s deadlines<2ms/s intr_wakeups/s energy_impact")
    for pid, run in runs.items():
        columns = list(zip(*run["rows"]))
        print(f"{pid:6} {run['times'][0]} {run['times'][-1]} {len(run['times']):3} "
              f"{mean(run['cpu_w']):8.2f} {mean(run['gpu_w']):8.2f} "
              f"{mean(columns[0]):14.1f} {mean(columns[2]):15.1f} "
              f"{mean(columns[4]):14.1f} {mean(columns[6]):13.1f}")


if __name__ == "__main__":
    main()
