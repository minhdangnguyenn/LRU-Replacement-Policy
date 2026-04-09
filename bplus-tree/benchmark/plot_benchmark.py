""

"""
plot_benchmark.py
─────────────────────────────────────────────────────────────────────────────
Reads  benchmark_results.csv  and generates four chart files:

  1. lru_test_ns_per_op.png       — ns/op for every LRU unit test
  2. benchmark_time_ms.png        — total time (ms) for O1 vs NAIVE workloads
  3. benchmark_ns_per_op.png      — ns/op for O1 vs NAIVE workloads
  4. benchmark_time_vs_capacity.png — time (ms) vs capacity line chart

Usage:
  python3 plot_benchmark.py                        # default: benchmark_results.csv
  python3 plot_benchmark.py my_results.csv         # custom path

CSV schema expected:
  name,type,capacity,operations,key_range,time_ms,ns_per_op
  type is one of: "LRU-TEST", "O1", "NAIVE"
─────────────────────────────────────────────────────────────────────────────
"""

import os
import sys

import cycler
import matplotlib.pyplot as plt
import pandas as pd

# Enable grid and update its appearance
plt.rcParams.update({"axes.grid": True})
plt.rcParams.update({"grid.color": "silver"})
plt.rcParams.update({"grid.linestyle": "--"})

# Set figure resolution
plt.rcParams.update({"figure.dpi": 150})

# Hide the top and right spines
plt.rcParams.update({"axes.spines.top": False})
plt.rcParams.update({"axes.spines.right": False})

# Increase font sizes
plt.rcParams.update({"font.size": 12})  # General font size
plt.rcParams.update({"axes.titlesize": 14})  # Title font size
plt.rcParams.update({"axes.labelsize": 12})  # Axis label font size


import matplotlib.ticker as mticker
import numpy as np

# ── style ──────────────────────────────────────────────────────────────────
plt.style.use("seaborn-v0_8-whitegrid")
plt.rcParams.update(
    {
        "figure.dpi": 150,
        "font.size": 11,
        "axes.titlesize": 13,
        "axes.titleweight": "bold",
        "axes.labelsize": 11,
        "xtick.labelsize": 9,
        "ytick.labelsize": 10,
        "legend.fontsize": 10,
    }
)

COLORS = {
    "O1": "#4C72B0",
    "NAIVE": "#DD8452",
    "LRU-TEST": "#55A868",
}
LABELS = {
    "O1": "O(1) Optimised",
    "NAIVE": "Naïve",
    "LRU-TEST": "LRU Unit Test",
}

# ── load ────────────────────────────────────────────────────────────────────
csv_path = "benchmark_results.csv"

if not os.path.exists(csv_path):
    print(f"[ERROR] File not found: {csv_path}")
    sys.exit(1)

df = pd.read_csv(csv_path)
df["type"] = df["type"].str.strip().str.upper().str.replace("-", "-")  # normalise

# split into two sub-datasets
lru_df = df[df["type"] == "LRU-TEST"].copy()
bench_df = df[df["type"].isin(["O1", "NAIVE"])].copy()

print(f"Loaded {len(df)} rows — {len(lru_df)} unit tests, {len(bench_df)} benchmarks")


# ── Chart 1: LRU unit tests — ns/op horizontal bar ─────────────────────────
def chart_lru_unit_tests():
    data = lru_df.sort_values("ns_per_op", ascending=True)
    names = data["name"].tolist()
    values = data["ns_per_op"].tolist()
    ops = data["operations"].tolist()

    fig, ax = plt.subplots(figsize=(10, max(5, len(names) * 0.45)))

    bars = ax.barh(
        names,
        values,
        color=COLORS["LRU-TEST"],
        alpha=0.85,
        edgecolor="white",
        linewidth=0.6,
    )

    # annotate: ns/op and operation count
    for bar, v, op in zip(bars, values, ops):
        ax.text(
            v + max(values) * 0.005,
            bar.get_y() + bar.get_height() / 2,
            f"{v:,} ns/op  ({op:,} ops)",
            va="center",
            fontsize=8,
            color="#333333",
        )

    ax.set_xlabel("Latency per operation  (ns)")
    ax.set_title("LRU Unit Tests — Latency per Operation (ns/op)")
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.xaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{v:,.0f}"))
    ax.set_xlim(0, max(values) * 1.30)

    plt.tight_layout()
    plt.savefig("lru_test_ns_per_op.png", dpi=150, bbox_inches="tight")
    plt.close()
    print("[saved] lru_test_ns_per_op.png")


# ── Chart 2 & 3: O1 vs NAIVE grouped bar ───────────────────────────────────
def grouped_bar(metric: str, ylabel: str, title: str, out_file: str):
    types = sorted(bench_df["type"].unique())
    workloads = bench_df["name"].unique()

    x_labels = []
    type_vals = {t: [] for t in types}

    for name in workloads:
        sub = bench_df[bench_df["name"] == name]
        cap = sub["capacity"].iloc[0]
        x_labels.append(f"{name}\n(cap={cap:,})")
        for t in types:
            row = sub[sub["type"] == t]
            type_vals[t].append(row[metric].iloc[0] if not row.empty else 0)

    n_groups = len(x_labels)
    n_types = len(types)
    bar_w = 0.35
    offsets = np.arange(n_types) * bar_w - (n_types - 1) * bar_w / 2
    x = np.arange(n_groups)

    fig, ax = plt.subplots(figsize=(max(10, n_groups * 2.5), 6))

    for i, t in enumerate(types):
        vals = type_vals[t]
        bars = ax.bar(
            x + offsets[i],
            vals,
            bar_w,
            label=LABELS.get(t, t),
            color=COLORS.get(t, f"C{i}"),
            alpha=0.88,
            edgecolor="white",
            linewidth=0.6,
        )
        for bar, v in zip(bars, vals):
            if v > 0:
                ax.text(
                    bar.get_x() + bar.get_width() / 2,
                    bar.get_height() * 1.012,
                    f"{v:,.0f}",
                    ha="center",
                    va="bottom",
                    fontsize=8,
                    color="#333333",
                )

    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.set_xticks(x)
    ax.set_xticklabels(x_labels, ha="center")
    ax.legend(loc="upper left", frameon=True)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{v:,.0f}"))

    skipped = [
        name
        for name in workloads
        if bench_df[(bench_df["name"] == name) & (bench_df["type"] == "NAIVE")].empty
    ]
    if skipped:
        note = "* NAIVE not run for: " + ", ".join(skipped)
        fig.text(
            0.5, -0.04, note, ha="center", fontsize=8, color="#888888", style="italic"
        )

    plt.tight_layout()
    plt.savefig(out_file, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"[saved] {out_file}")


# ── Chart 4: line — time_ms vs capacity ────────────────────────────────────
def line_chart(metric: str, ylabel: str, title: str, out_file: str):
    types = sorted(bench_df["type"].unique())
    fig, ax = plt.subplots(figsize=(10, 6))

    for t in types:
        sub = bench_df[bench_df["type"] == t].sort_values("capacity")
        if sub.empty:
            continue
        ax.plot(
            sub["capacity"],
            sub[metric],
            marker="o",
            linewidth=2,
            markersize=7,
            label=LABELS.get(t, t),
            color=COLORS.get(t, None),
        )
        for _, row in sub.iterrows():
            ax.annotate(
                row["name"],
                xy=(row["capacity"], row[metric]),
                xytext=(4, 4),
                textcoords="offset points",
                fontsize=7,
                color="#444444",
            )

    ax.set_xscale("log")
    ax.set_title(title)
    ax.set_xlabel("Capacity  (log scale)")
    ax.set_ylabel(ylabel)
    ax.legend(loc="upper left", frameon=True)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda v, _: f"{v:,.0f}"))

    plt.tight_layout()
    plt.savefig(out_file, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"[saved] {out_file}")


# ── run ─────────────────────────────────────────────────────────────────────
chart_lru_unit_tests()

grouped_bar(
    metric="time_ms",
    ylabel="Wall-clock time  (ms)",
    title="LRU Cache Benchmark — Total Time (ms) per Workload",
    out_file="benchmark_time_ms.png",
)

grouped_bar(
    metric="ns_per_op",
    ylabel="Latency per operation  (ns)",
    title="LRU Cache Benchmark — Latency per Operation (ns/op)",
    out_file="benchmark_ns_per_op.png",
)

line_chart(
    metric="time_ms",
    ylabel="Wall-clock time  (ms)",
    title="Time (ms) vs Cache Capacity — O(1) vs Naïve",
    out_file="benchmark_time_vs_capacity.png",
)

print("\nDone. Four PNG files written to the current directory.")
