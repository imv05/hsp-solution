#!/usr/bin/env python3
# ...existing code...
import sys
import argparse
from pathlib import Path
import random

def clamp(x, a, b):
    return max(a, min(b, x))

def generate(N=5, L=100, seed=None, max_size=50, max_group=4, max_gap=20):
    rnd = random.Random(seed)
    N = clamp(N, 1, 10000)
    L = clamp(L, 1, 100000)
    max_size = max(1, min(max_size, L))
    tasks = []

    # generate groups first (so we can enforce per-start sum <= M later)
    starts = []
    times = []
    groups = []  # list of group sizes
    remaining = N
    last_start = 0
    last_tim = None
    while remaining > 0:
        # ensure group size <= L so that sum of sizes (>= group) can fit into M<=L
        group = rnd.randint(1, min(max_group, remaining, L))
        gap = rnd.randint(0, max_gap)
        start = last_start + gap
        if gap == 0 and last_tim is not None:
            tim = last_tim
        else:
            tim = rnd.randint(1, 1000)
        groups.append((group, start, tim))
        for _ in range(group):
            starts.append(start)
            times.append(tim)
        last_start = start
        last_tim = tim
        remaining -= group

    # generate sizes per group ensuring group's total size <= L
    sizes = []
    addrs = []
    max_group_sum = 0
    for (group_count, _, _) in groups:
        # initial random sizes
        g_sizes = [rnd.randint(1, max_size) for _ in range(group_count)]
        g_sum = sum(g_sizes)
        # if sum exceeds L, reduce largest sizes until fits
        if g_sum > L:
            # repeatedly decrement the largest element until g_sum <= L
            # (this preserves positivity)
            # use a heap-like approach via sorting indices
            while g_sum > L:
                # find index of maximum
                idx = max(range(group_count), key=lambda i: g_sizes[i])
                # decrement by 1 (but keep >=1)
                if g_sizes[idx] > 1:
                    g_sizes[idx] -= 1
                    g_sum -= 1
                else:
                    # all ones? then impossible to fit (but group_count <= L prevents this)
                    break
        max_group_sum = max(max_group_sum, g_sum)
        sizes.extend(g_sizes)

    # generate addresses independently, ensuring addr + size <= L
    for s in sizes:
        a = rnd.randint(0, L - s)
        addrs.append(a)

    # compute M: must be >= max single task size and >= max_group_sum, and <= L
    max_task_size = max(sizes)
    M_min = max(max_task_size, max_group_sum)
    # pick M between M_min and L
    if M_min > L:
        # fallback: shrink M_min to L (shouldn't happen because we constrained groups)
        M = L
    else:
        extra = rnd.randint(0, max(0, L - M_min))
        M = M_min + extra
    M = clamp(M, 1, L)

    # build infile content
    lines = []
    lines.append(f"{L} {M} {N}")
    for i in range(N):
        lines.append(f"{addrs[i]} {sizes[i]} {starts[i]} {times[i]}")

    return "\n".join(lines) + "\n"

def main():
    p = argparse.ArgumentParser()
    p.add_argument('N', nargs='?', type=int, default=5)
    p.add_argument('L', nargs='?', type=int, default=100)
    p.add_argument('--seed', type=int, default=None)
    p.add_argument('--max-size', type=int, default=50, help='最大 size（会被截到 L）')
    p.add_argument('--max-group', type=int, default=4, help='最多多少任务共享同一 start')
    p.add_argument('--max-gap', type=int, default=20, help='相邻 start 的最大时间差（可为0）')
    args = p.parse_args()

    content = generate(args.N, args.L, seed=args.seed, max_size=args.max_size,
                       max_group=args.max_group, max_gap=args.max_gap)
    d = Path(__file__).parent
    (d / "infile.txt").write_text(content)
    print(f"Wrote {d/'infile.txt'} (N={args.N}, L={args.L}, seed={args.seed})")

if __name__ == "__main__":
    main()
# ...existing code...