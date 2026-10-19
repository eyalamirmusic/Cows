#!/usr/bin/env python3
# Usage: tools/perf-sample-top.py [--match=REGEX] [--top=N] <sample.txt>...

import re
import sys
from collections import defaultdict

LINE = re.compile(r"^([\s+!:|]*)(\d+) (.+?)  \(in ([^)]+)\)")
THREAD = re.compile(r"^\s+(\d+) (Thread_\d+.*)$")


def short(name):
    depth, out = 0, []
    for ch in name:
        if ch == "(":
            depth += 1
        if depth == 0:
            out.append(ch)
        if ch == ")":
            depth -= 1
    return "".join(out).strip()


WAITS = {
    "mach_msg2_trap", "__workq_kernreturn", "semaphore_wait_trap",
    "semaphore_timedwait_trap", "semaphore_wait_signal_trap",
    "semaphore_timedwait_signal_trap", "__psynch_cvwait", "__psynch_mutexwait",
    "kevent_id", "kevent", "kevent64", "__semwait_signal", "__ulock_wait",
    "__ulock_wait2", "__select", "poll", "__sigsuspend", "__wait4",
    # _pthread_cond_wait reads the clock on its way into a timed wait.
    "__gettimeofday",
}


def tally(thread, nodes):
    for index, (depth, count, path) in enumerate(nodes):
        children, child_depth = 0, None
        for later_depth, later_count, _ in nodes[index + 1:]:
            if later_depth <= depth:
                break
            child_depth = child_depth or later_depth
            if later_depth == child_depth:
                children += later_count
        own = count - children
        if own <= 0:
            continue
        if path[-1].split(" [")[0] in WAITS:
            thread["waiting"] += own
            continue
        thread["running"] += own
        for frame in set(path):
            thread["frames"][frame] += own


def parse(path, threads):
    current, nodes, stack, in_graph = None, [], [], False

    for line in open(path, errors="replace"):
        if line.startswith("Call graph:"):
            in_graph = True
            continue
        if not in_graph:
            continue
        if line.startswith("Total number in stack") or line.startswith("Binary Images"):
            break
        found = THREAD.match(line)
        if found:
            if current is not None:
                tally(current, nodes)
            label = re.sub(r"Thread_\d+", "", found.group(2)).strip(" :")
            current, nodes, stack = threads[label or "(unnamed)"], [], []
            continue
        found = LINE.match(line)
        if not found or current is None:
            continue
        depth = len(found.group(1))
        name = f"{short(found.group(3))} [{found.group(4)}]"
        while stack and stack[-1][0] >= depth:
            stack.pop()
        stack.append((depth, name))
        nodes.append((depth, int(found.group(2)), [n for _, n in stack]))

    if current is not None:
        tally(current, nodes)


def main():
    match = None
    top = 25
    files = []
    for arg in sys.argv[1:]:
        if arg.startswith("--match="):
            match = re.compile(arg.split("=", 1)[1])
        elif arg.startswith("--top="):
            top = int(arg.split("=", 1)[1])
        else:
            files.append(arg)

    threads = defaultdict(
        lambda: {"running": 0, "waiting": 0, "frames": defaultdict(int)})
    for path in files:
        parse(path, threads)

    running = sum(t["running"] for t in threads.values())
    print(f"on-CPU samples: {running}")
    for label, t in sorted(threads.items(), key=lambda kv: -kv[1]["running"]):
        if t["running"]:
            print(f"  {100 * t['running'] / running:5.1f}%  {label}"
                  f"  ({t['running']} on CPU, {t['waiting']} waiting)")

    frames = defaultdict(int)
    for t in threads.values():
        for name, count in t["frames"].items():
            frames[name] += count
    print("inclusive share of on-CPU samples:")
    shown = 0
    for name, count in sorted(frames.items(), key=lambda kv: -kv[1]):
        if match and not match.search(name):
            continue
        print(f"  {100 * count / running:5.1f}%  {name}")
        shown += 1
        if shown >= top:
            break


if __name__ == "__main__":
    main()
