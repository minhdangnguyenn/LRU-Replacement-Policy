# Database Management System (DBMS) Techniques

> A from-scratch C++ exploration of how real database engines manage
> memory, storage, and indexes under the hood.

This repository implements core DBMS internals one module at a time,
with benchmarks and visual analysis for each technique.

**Modules**

- **LRU Buffer Pool** — O(1) page eviction using a doubly linked list and hash map. Benchmarked against a naïve O(n) implementation.
- **B+ Tree** _(in development)_ — disk-backed index with buffer pool integration, supporting insert, search, and range scan.

---

## How It Works — LRU Buffer Pool

### Core Data Structures

The `BufferPool` uses two doubly linked lists and two hash maps to
achieve O(1) for every operation.

```
Main list (recency order — all pages):
head ── [MRU] ── page ── page ── [LRU] ── tail

Unpinned list (eviction candidates only):
uhead ── [MRU unpinned] ── page ── [LRU unpinned] ── utail
```

| Structure                   | Purpose                                |
| --------------------------- | -------------------------------------- |
| Main doubly linked list     | Tracks recency order of all pages      |
| Unpinned doubly linked list | Tracks only evictable (unpinned) pages |
| `unordered_map`             | O(1) key → page pointer lookup         |
| `unpinned_map`              | O(1) key → unpinned pointer lookup     |

### Page Lifecycle

- `pin(key, value)` — insert or update a page, mark it **pinned** (not evictable), move to MRU position.
- `get(key)` — read a page, move to MRU position. Returns `-1` on miss.
- `unpin(page*)` — mark a page as evictable, add to unpinned list.

When the cache is full, the **LRU unpinned page** is evicted.
Pinned pages are never evicted.

### Complexity

| Operation         | Time | Space      |
| ----------------- | ---- | ---------- |
| `get(key)`        | O(1) | O(1)       |
| `pin(key, value)` | O(1) | O(n) total |
| `unpin(page*)`    | O(1) | O(n) total |

---

## Prerequisites

```bash
cmake     # >= 3.20
g++       # C++20 or higher
python3   # for benchmark visualisation (matplotlib, pandas)
```

---

## Build & Run

```bash
# Compile
bash scripts/compile.sh

# Run tests and benchmarks
bash scripts/run.sh

# Visualise benchmark results
bash scripts/benchmark.sh
```

---

## Learning Goals

This project is built to understand how real database engines work
at the component level — not just to implement algorithms, but to
measure, compare, and reason about tradeoffs.

Each module includes:

- A working implementation with tests
- A benchmark comparing design choices
- Visual output to make the numbers concrete
