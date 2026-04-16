#include "../include/buffer-pool.h"
#include "../include/fifo-replacer.h"
#include "../include/lru-cache-naive.h"
#include "../include/lru-replacer.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// ─────────────────────────────────────────
// CSV
// ─────────────────────────────────────────
void init_csv(std::ofstream &csv, const std::string &filename) {
    csv.open(filename);
    csv << "name,type,capacity,operations,key_range,run,time_ms,ns_per_op\n";
}

void write_csv_row(std::ofstream &csv, const std::string &name,
                   const std::string &type, int capacity, int operations,
                   int key_range, int run, long long ms, long long ns_per_op) {
    csv << name << "," << type << "," << capacity << "," << operations << ","
        << key_range << "," << run << "," << ms << "," << ns_per_op << "\n";
}

// ─────────────────────────────────────────
// BENCHMARK CORE
// ─────────────────────────────────────────
void run_once(const std::string &name, const std::string &type,
              BufferPool &cache, int operations, int key_range, int capacity,
              int run, std::ofstream &csv) {

    std::mt19937 gen(42 + run);
    std::uniform_int_distribution<int> keyDist(0, key_range - 1);
    std::uniform_int_distribution<int> opDist(0, 1);
    std::uniform_int_distribution<int> valDist(0, 9999);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < operations; i++) {
        int key = keyDist(gen);
        if (opDist(gen) == 0)
            cache.get(key);
        else
            cache.pin(key, valDist(gen));
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
                  .count();
    auto npo = ns / operations;

    write_csv_row(csv, name, type, capacity, operations, key_range, run, ms,
                  npo);
}

// ─────────────────────────────────────────
// WRAPPERS
// ─────────────────────────────────────────
void benchmark_lru(const std::string &name, int cap, int ops, int range,
                   int run, std::ofstream &csv) {
    LRUReplacer replacer;
    BufferPool cache(cap, &replacer);
    run_once(name, "LRU", cache, ops, range, cap, run, csv);
}

void benchmark_fifo(const std::string &name, int cap, int ops, int range,
                    int run, std::ofstream &csv) {
    FIFOReplacer replacer;
    BufferPool cache(cap, &replacer);
    run_once(name, "FIFO", cache, ops, range, cap, run, csv);
}

void benchmark_naive(const std::string &name, int cap, int ops, int range,
                     int run, std::ofstream &csv) {
    LRUReplacerNaive replacer;
    BufferPool cache(cap, &replacer);
    run_once(name, "NAIVE", cache, ops, range, cap, run, csv);
}

// ─────────────────────────────────────────
// EXPERIMENT
// ─────────────────────────────────────────
void run_experiment(const std::string &name, const std::vector<int> &capacities,
                    int operations, int key_range, int runs,
                    std::ofstream &csv) {

    std::cout << "\n=== " << name << " ===\n";

    for (int cap : capacities) {
        std::cout << "Capacity: " << cap << std::endl;

        for (int r = 0; r < runs; r++) {
            benchmark_lru(name, cap, operations, key_range, r, csv);
            benchmark_fifo(name, cap, operations, key_range, r, csv);

            // skip NAIVE for very large
            if (cap <= 10000)
                benchmark_naive(name, cap, operations, key_range, r, csv);
        }
    }
}

// ─────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────
int main() {
    std::ofstream csv;
    init_csv(csv, "benchmark_results.csv");

    int ops = 3'000'000;
    int runs = 3;

    std::vector<int> caps = {10, 100, 1000, 10000, 100000};

    // isolate workloads properly
    run_experiment("High contention", caps, ops, 20, runs, csv);
    run_experiment("Balanced workload", caps, ops, 2000, runs, csv);
    run_experiment("Low contention", caps, ops, 100000, runs, csv);

    csv.close();
    std::cout << "\nDone → benchmark_results.csv\n";
}
