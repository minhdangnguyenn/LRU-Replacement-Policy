import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.style.use("seaborn-v0_8-whitegrid")

FILE = "./benchmark_results.csv"

csv_path = sys.argv[1] if len(sys.argv) > 1 else FILE
df = pd.read_csv(csv_path)

# normalize
df["type"] = df["type"].str.upper().str.strip()

# ─────────────────────────────────────────
# AGGREGATION
# ─────────────────────────────────────────
agg = (
    df.groupby(["name", "type", "capacity"])
    .agg(mean_ns=("ns_per_op", "mean"), std_ns=("ns_per_op", "std"))
    .reset_index()
)

workloads = agg["name"].unique()
types = agg["type"].unique()

COLORS = {"LRU": "#4C72B0", "FIFO": "#55A868", "NAIVE": "#DD8452"}


# ─────────────────────────────────────────
# LINE PLOTS (per workload)
# ─────────────────────────────────────────
def plot_workload(name):
    sub = agg[agg["name"] == name]

    plt.figure(figsize=(8, 5))

    for t in types:
        d = sub[sub["type"] == t].sort_values("capacity")
        if d.empty:
            continue

        plt.errorbar(
            d["capacity"],
            d["mean_ns"],
            yerr=d["std_ns"],
            marker="o",
            capsize=3,
            label=t,
            color=COLORS.get(t),
        )

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Capacity (log)")
    plt.ylabel("ns / operation (log)")
    plt.title(name)
    plt.legend()
    plt.tight_layout()

    fname = f"{name.replace(' ', '_')}.png"
    plt.savefig(fname)
    plt.close()
    print("[saved]", fname)


# ─────────────────────────────────────────
# SPEEDUP PLOT
# ─────────────────────────────────────────
def plot_speedup():
    pivot = agg.pivot_table(
        index=["name", "capacity"], columns="type", values="mean_ns"
    ).reset_index()

    if "NAIVE" not in pivot or "LRU" not in pivot:
        print("Skipping speedup (missing NAIVE or LRU)")
        return

    pivot["speedup"] = pivot["NAIVE"] / pivot["LRU"]

    for name in pivot["name"].unique():
        sub = pivot[pivot["name"] == name]

        plt.figure(figsize=(8, 5))
        plt.plot(sub["capacity"], sub["speedup"], marker="o", color="#C44E52")

        plt.xscale("log")
        plt.xlabel("Capacity (log)")
        plt.ylabel("Speedup (NAIVE / LRU)")
        plt.title(f"Speedup — {name}")
        plt.axhline(1, linestyle="--", color="gray")

        plt.tight_layout()
        fname = f"speedup_{name.replace(' ', '_')}.png"
        plt.savefig(fname)
        plt.close()
        print("[saved]", fname)


# ─────────────────────────────────────────
# RUN ALL
# ─────────────────────────────────────────
for w in workloads:
    plot_workload(w)

plot_speedup()

print("\nDone.")
