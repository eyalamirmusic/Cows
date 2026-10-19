#!/usr/bin/env python3
# Usage: tools/perf-probe.py <pid>
import ctypes
import json
import sys
import time

libc = ctypes.CDLL("/usr/lib/libSystem.B.dylib", use_errno=True)

RUSAGE_INFO_V6 = 6
PROC_PIDLISTTHREADS = 6
PROC_PIDTHREADINFO = 5


class RusageV6(ctypes.Structure):
    _fields_ = [("ri_uuid", ctypes.c_uint8 * 16)] + [
        (name, ctypes.c_uint64)
        for name in (
            "user_time system_time pkg_idle_wkups interrupt_wkups pageins "
            "wired_size resident_size phys_footprint proc_start_abstime "
            "proc_exit_abstime child_user_time child_system_time "
            "child_pkg_idle_wkups child_interrupt_wkups child_pageins "
            "child_elapsed_abstime diskio_bytesread diskio_byteswritten "
            "qos_default qos_maintenance qos_background qos_utility qos_legacy "
            "qos_user_initiated qos_user_interactive billed_system_time "
            "serviced_system_time logical_writes lifetime_max_phys_footprint "
            "instructions cycles billed_energy serviced_energy "
            "interval_max_phys_footprint runnable_time flags user_ptime "
            "system_ptime pinstructions pcycles energy_nj penergy_nj"
        ).split()
    ] + [("rest", ctypes.c_uint64 * 14)]


class ThreadInfo(ctypes.Structure):
    _fields_ = [
        ("user_time", ctypes.c_uint64),
        ("system_time", ctypes.c_uint64),
        ("cpu_usage", ctypes.c_int32),
        ("policy", ctypes.c_int32),
        ("run_state", ctypes.c_int32),
        ("flags", ctypes.c_int32),
        ("sleep_time", ctypes.c_int32),
        ("curpri", ctypes.c_int32),
        ("priority", ctypes.c_int32),
        ("maxpriority", ctypes.c_int32),
        ("name", ctypes.c_char * 64),
    ]


class Timebase(ctypes.Structure):
    _fields_ = [("numer", ctypes.c_uint32), ("denom", ctypes.c_uint32)]


def ticks_to_ns():
    base = Timebase()
    libc.mach_timebase_info(ctypes.byref(base))
    return base.numer / base.denom


def probe(pid):
    usage = RusageV6()
    if libc.proc_pid_rusage(pid, RUSAGE_INFO_V6, ctypes.byref(usage)) != 0:
        raise SystemExit(f"proc_pid_rusage({pid}) failed")

    # rusage times are mach ticks on Apple silicon, not nanoseconds.
    scale = ticks_to_ns()
    out = {
        "wall": time.monotonic(),
        "user_s": usage.user_time * scale / 1e9,
        "system_s": usage.system_time * scale / 1e9,
        "energy_j": usage.energy_nj / 1e9,
        "instructions": usage.instructions,
        "cycles": usage.cycles,
        "wakeups": usage.pkg_idle_wkups + usage.interrupt_wkups,
        "footprint_mb": usage.phys_footprint / 2**20,
        "threads": {},
    }

    handles = (ctypes.c_uint64 * 512)()
    size = libc.proc_pidinfo(
        pid, PROC_PIDLISTTHREADS, 0, ctypes.byref(handles), ctypes.sizeof(handles)
    )
    for handle in handles[: max(size, 0) // 8]:
        info = ThreadInfo()
        got = libc.proc_pidinfo(
            pid,
            PROC_PIDTHREADINFO,
            ctypes.c_uint64(handle),
            ctypes.byref(info),
            ctypes.sizeof(info),
        )
        if got == ctypes.sizeof(info):
            out["threads"][str(handle)] = {
                "name": info.name.decode(errors="replace"),
                "cpu_s": (info.user_time + info.system_time) / 1e9,
            }

    return out


if __name__ == "__main__":
    print(json.dumps(probe(int(sys.argv[1]))))
