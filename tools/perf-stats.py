#!/usr/bin/env python3
# Usage: tools/perf-stats.py <a.csv> [b.csv] [--metrics m1,m2,...]
import csv
import math
import sys

SKIP = {"label", "scenario", "run", "build", "commit", "pid", "from", "to", "warmup_s", "window_s", "env"}


def read(path):
    with open(path) as f:
        rows = list(csv.DictReader(f))
    columns = {}
    for row in rows:
        for key, value in row.items():
            if key in SKIP or value in ("", None):
                continue
            try:
                columns.setdefault(key, []).append(float(value))
            except ValueError:
                pass
    return rows, columns


def mean(xs):
    return sum(xs) / len(xs)


def sd(xs):
    if len(xs) < 2:
        return 0.0
    m = mean(xs)
    return math.sqrt(sum((x - m) ** 2 for x in xs) / (len(xs) - 1))


# Regularised incomplete beta, by Lentz's continued fraction.
def betacf(a, b, x):
    tiny = 1e-300
    qab, qap, qam = a + b, a + 1.0, a - 1.0
    c, d = 1.0, 1.0 - qab * x / qap
    d = 1.0 / (d if abs(d) > tiny else tiny)
    h = d
    for m in range(1, 300):
        m2 = 2 * m
        aa = m * (b - m) * x / ((qam + m2) * (a + m2))
        d = 1.0 + aa * d
        d = 1.0 / (d if abs(d) > tiny else tiny)
        c = 1.0 + aa / c
        c = c if abs(c) > tiny else tiny
        h *= d * c
        aa = -(a + m) * (qab + m) * x / ((a + m2) * (qap + m2))
        d = 1.0 + aa * d
        d = 1.0 / (d if abs(d) > tiny else tiny)
        c = 1.0 + aa / c
        c = c if abs(c) > tiny else tiny
        delta = d * c
        h *= delta
        if abs(delta - 1.0) < 1e-12:
            break
    return h


def betai(a, b, x):
    if x <= 0.0:
        return 0.0
    if x >= 1.0:
        return 1.0
    front = math.exp(
        math.lgamma(a + b) - math.lgamma(a) - math.lgamma(b)
        + a * math.log(x) + b * math.log(1.0 - x)
    )
    if x < (a + 1.0) / (a + b + 2.0):
        return front * betacf(a, b, x) / a
    return 1.0 - front * betacf(b, a, 1.0 - x) / b


def t_two_sided_p(t, df):
    return betai(df / 2.0, 0.5, df / (df + t * t))


def t_critical(df, alpha=0.05):
    low, high = 0.0, 1000.0
    for _ in range(200):
        middle = (low + high) / 2.0
        if t_two_sided_p(middle, df) > alpha:
            low = middle
        else:
            high = middle
    return (low + high) / 2.0


def ci95(xs):
    if len(xs) < 2:
        return 0.0
    return t_critical(len(xs) - 1) * sd(xs) / math.sqrt(len(xs))


def welch(a, b):
    va, vb = sd(a) ** 2 / len(a), sd(b) ** 2 / len(b)
    if va + vb == 0.0:
        return float("nan") if mean(a) == mean(b) else 0.0
    t = (mean(b) - mean(a)) / math.sqrt(va + vb)
    df = (va + vb) ** 2 / (
        va**2 / (len(a) - 1) + vb**2 / (len(b) - 1)
    )
    return t_two_sided_p(t, df)


def cohens_d(a, b):
    pooled = math.sqrt(
        ((len(a) - 1) * sd(a) ** 2 + (len(b) - 1) * sd(b) ** 2)
        / (len(a) + len(b) - 2)
    )
    return (mean(b) - mean(a)) / pooled if pooled > 0 else float("nan")


def ranks(values):
    order = sorted(range(len(values)), key=lambda i: values[i])
    result = [0.0] * len(values)
    i = 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and values[order[j + 1]] == values[order[i]]:
            j += 1
        for k in range(i, j + 1):
            result[order[k]] = (i + j) / 2.0 + 1.0
        i = j + 1
    return result


# Exact without ties, else the normal approximation with the tie correction.
def mann_whitney(a, b):
    n1, n2 = len(a), len(b)
    together = a + b
    r = ranks(together)
    u = sum(r[:n1]) - n1 * (n1 + 1) / 2.0
    u = min(u, n1 * n2 - u)

    if len(set(together)) == len(together) and n1 * n2 <= 2500:
        counts = {(0, 0): [1]}

        def ways(i, j):
            if (i, j) in counts:
                return counts[(i, j)]
            size = i * j + 1
            out = [0] * size
            if i > 0:
                for k, w in enumerate(ways(i - 1, j)):
                    out[k + j] += w
            if j > 0:
                for k, w in enumerate(ways(i, j - 1)):
                    out[k] += w
            counts[(i, j)] = out
            return out

        dist = ways(n1, n2)
        total = sum(dist)
        tail = sum(dist[: int(math.floor(u)) + 1]) / total
        return min(1.0, 2.0 * tail)

    n = n1 + n2
    ties = {}
    for x in together:
        ties[x] = ties.get(x, 0) + 1
    tie_term = sum(t**3 - t for t in ties.values()) / (n * (n - 1))
    sigma = math.sqrt(n1 * n2 / 12.0 * ((n + 1) - tie_term))
    if sigma == 0:
        return float("nan")
    z = (abs(u - n1 * n2 / 2.0) - 0.5) / sigma
    return math.erfc(max(z, 0.0) / math.sqrt(2.0))


def describe(xs):
    return f"{mean(xs):10.4f} ±{ci95(xs):8.4f} (sd {sd(xs):.4f}, n {len(xs)})"


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    wanted = None
    for a in sys.argv[1:]:
        if a.startswith("--metrics="):
            wanted = a.split("=", 1)[1].split(",")
    if not args:
        raise SystemExit(__doc__ or "usage: perf-stats.py a.csv [b.csv]")

    rows_a, a = read(args[0])
    metrics = wanted or list(a)

    if len(args) == 1:
        print(f"{args[0]}  ({rows_a[0]['label']} {rows_a[0]['scenario']})")
        for m in metrics:
            if m in a:
                print(f"  {m:16} {describe(a[m])}")
        return

    rows_b, b = read(args[1])
    print(f"A = {rows_a[0]['label']} {rows_a[0]['scenario']} ({rows_a[0]['commit']})")
    print(f"B = {rows_b[0]['label']} {rows_b[0]['scenario']} ({rows_b[0]['commit']})")
    print(f"  {'metric':16} {'A mean ±95% CI':>22} {'B mean ±95% CI':>22} "
          f"{'Δ':>9} {'Δ%':>7} {'Welch p':>9} {'d':>7} {'MWU p':>9}")
    for m in metrics:
        if m not in a or m not in b or len(a[m]) < 2 or len(b[m]) < 2:
            continue
        xa, xb = a[m], b[m]
        delta = mean(xb) - mean(xa)
        share = 100.0 * delta / mean(xa) if mean(xa) else float("nan")
        print(f"  {m:16} {mean(xa):11.4f} ±{ci95(xa):8.4f} "
              f"{mean(xb):11.4f} ±{ci95(xb):8.4f} {delta:9.4f} {share:6.1f}% "
              f"{welch(xa, xb):9.2g} {cohens_d(xa, xb):7.2f} "
              f"{mann_whitney(xa, xb):9.2g}")


if __name__ == "__main__":
    main()
