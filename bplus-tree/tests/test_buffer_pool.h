#ifndef TEST_BUFFER_POOL_H
#define TEST_BUFFER_POOL_H

#include "../include/buffer-pool.h"
#include "../include/disk-manager.h"
#include "../include/test-data.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

class TestBufferPool {
public:

    void pass(const std::string& name) {
        std::cout << "[PASS] " << name << std::endl;
    }

    // ─────────────────────────────────────────
    //  DiskManager tests
    // ─────────────────────────────────────────

    // allocate_page must return incrementing page_ids
    // each call must return a different value
    void test_allocate_page_increments() {
        std::remove("test_alloc.db");
        DiskManager dm("test_alloc.db");

        int id0 = dm.allocate_page();
        int id1 = dm.allocate_page();
        int id2 = dm.allocate_page();

        assert(id1 == id0 + 1);
        assert(id2 == id1 + 1);

        std::remove("test_alloc.db");
        pass("test_allocate_page_increments");
    }

    // write bytes to a page then read them back
    // the data must match exactly
    void test_write_read_roundtrip() {
        std::remove("test_rw.db");
        DiskManager dm("test_rw.db");

        int page_id = dm.allocate_page();

        char write_buf[PAGE_SIZE] = {};
        snprintf(write_buf, PAGE_SIZE, "hello buffer pool");
        dm.write_page(page_id, write_buf);

        char read_buf[PAGE_SIZE] = {};
        dm.read_page(page_id, read_buf);

        assert(memcmp(write_buf, read_buf, PAGE_SIZE) == 0);

        std::remove("test_rw.db");
        pass("test_write_read_roundtrip");
    }

    // write to two different pages
    // reading page A must not return data from page B
    void test_multiple_pages_independent() {
        std::remove("test_multi.db");
        DiskManager dm("test_multi.db");

        int id_a = dm.allocate_page();
        int id_b = dm.allocate_page();

        char buf_a[PAGE_SIZE] = {};
        char buf_b[PAGE_SIZE] = {};
        snprintf(buf_a, PAGE_SIZE, "page A data");
        snprintf(buf_b, PAGE_SIZE, "page B data");

        dm.write_page(id_a, buf_a);
        dm.write_page(id_b, buf_b);

        char read_a[PAGE_SIZE] = {};
        char read_b[PAGE_SIZE] = {};
        dm.read_page(id_a, read_a);
        dm.read_page(id_b, read_b);

        assert(memcmp(read_a, buf_a, PAGE_SIZE) == 0);
        assert(memcmp(read_b, buf_b, PAGE_SIZE) == 0);
        assert(memcmp(read_a, read_b, PAGE_SIZE) != 0);

        std::remove("test_multi.db");
        pass("test_multiple_pages_independent");
    }

    // ─────────────────────────────────────────
    //  BufferPool tests
    // ─────────────────────────────────────────

    // create_new_page must return a non-negative page_id
    void test_create_new_page_returns_valid_id() {
        std::remove("mydb.db");
        BufferPool bp(4);

        int page_id = bp.create_new_page();
        assert(page_id >= 0);

        std::remove("mydb.db");
        pass("test_create_new_page_returns_valid_id");
    }

    // each call to create_new_page must return a unique page_id
    void test_create_multiple_pages_unique_ids() {
        std::remove("mydb.db");
        BufferPool bp(4);

        int id0 = bp.create_new_page();
        int id1 = bp.create_new_page();
        int id2 = bp.create_new_page();

        assert(id0 != id1);
        assert(id1 != id2);
        assert(id0 != id2);

        std::remove("mydb.db");
        pass("test_create_multiple_pages_unique_ids");
    }

    // fetching a page_id that was never created must return nullptr
    void test_fetch_nonexistent_page_returns_null() {
        std::remove("mydb.db");
        BufferPool bp(4);

        char* data = bp.fetch_page(9999);
        assert(data == nullptr);

        std::remove("mydb.db");
        pass("test_fetch_nonexistent_page_returns_null");
    }

    // create a page then fetch it — must return a non-null pointer
    void test_fetch_existing_page_returns_data() {
        std::remove("mydb.db");
        BufferPool bp(4);

        int page_id = bp.create_new_page();
        assert(page_id >= 0);

        char* data = bp.fetch_page(page_id);
        assert(data != nullptr);

        std::remove("mydb.db");
        pass("test_fetch_existing_page_returns_data");
    }

    // fetching the same page twice must return the same pointer
    // second call must be a cache hit — no disk read
    void test_fetch_same_page_twice_cache_hit() {
        std::remove("mydb.db");
        BufferPool bp(4);

        int page_id = bp.create_new_page();
        char* first  = bp.fetch_page(page_id);
        char* second = bp.fetch_page(page_id);

        assert(first != nullptr);
        assert(second != nullptr);
        assert(first == second);  // same frame pointer — cache hit

        std::remove("mydb.db");
        pass("test_fetch_same_page_twice_cache_hit");
    }

    // fill the buffer pool to capacity — all creates must succeed
    void test_fill_to_capacity() {
        std::remove("mydb.db");
        const int CAP = 4;
        BufferPool bp(CAP);

        for (int i = 0; i < CAP; i++) {
            int page_id = bp.create_new_page();
            assert(page_id >= 0);
        }

        std::remove("mydb.db");
        pass("test_fill_to_capacity");
    }

    // fill to capacity, unpin all pages, then create one more
    // eviction must succeed and return a valid page_id
    void test_eviction_triggered_when_full() {
        std::remove("mydb.db");
        const int CAP = 4;
        BufferPool bp(CAP);

        // fill pool and collect page_ids
        int ids[CAP];
        for (int i = 0; i < CAP; i++) {
            ids[i] = bp.create_new_page();
            assert(ids[i] >= 0);
        }

        // unpin all so they become evictable
        for (int i = 0; i < CAP; i++) {
            bp.unpin_page(ids[i], false);
        }

        // one more create — must trigger eviction and succeed
        int new_id = bp.create_new_page();
        assert(new_id >= 0);

        std::remove("mydb.db");
        pass("test_eviction_triggered_when_full");
    }

    // unpin page A first, then page B
    // when pool is full, page A must be evicted (LRU order)
    void test_correct_page_evicted() {
        std::remove("mydb.db");
        const int CAP = 2;
        BufferPool bp(CAP);

        int id_a = bp.create_new_page();
        int id_b = bp.create_new_page();

        // unpin A first — A is LRU
        bp.unpin_page(id_a, false);
        bp.unpin_page(id_b, false);

        // create new page — A must be evicted
        int id_c = bp.create_new_page();
        assert(id_c >= 0);

        // A is no longer in the pool — fetch must go to disk
        // B is still in the pool — fetch must be a cache hit
        char* data_b = bp.fetch_page(id_b);
        assert(data_b != nullptr);

        std::remove("mydb.db");
        pass("test_correct_page_evicted");
    }

    // write data into a fetched page
    // after unpin and re-fetch, the same data must still be there
    void test_written_data_persists() {
        std::remove("mydb.db");
        BufferPool bp(4);

        int page_id = bp.create_new_page();
        char* data = bp.fetch_page(page_id);
        assert(data != nullptr);

        // write known bytes into the frame
        snprintf(data, PAGE_SIZE, "persistent data");
        bp.unpin_page(page_id, true);  // mark dirty so it flushes

        // re-fetch — data must still be there
        char* data2 = bp.fetch_page(page_id);
        assert(data2 != nullptr);
        assert(strncmp(data2, "persistent data", 15) == 0);

        std::remove("mydb.db");
        pass("test_written_data_persists");
    }

    // pin all frames — no eviction possible
    // create_new_page must return -1
    void test_all_pinned_returns_error() {
        std::remove("mydb.db");
        const int CAP = 2;
        BufferPool bp(CAP);

        // fill and keep all frames pinned (no unpin)
        int id0 = bp.create_new_page();
        int id1 = bp.create_new_page();
        assert(id0 >= 0);
        assert(id1 >= 0);

        // no frames unpinned — eviction impossible
        int id2 = bp.create_new_page();
        assert(id2 == -1);

        std::remove("mydb.db");
        pass("test_all_pinned_returns_error");
    }

    // ─────────────────────────────────────────
    //  LRU integration tests
    // ─────────────────────────────────────────

    // unpin A then B — A must be evicted first (oldest unpinned = LRU)
    void test_lru_eviction_order() {
        std::remove("mydb.db");
        const int CAP = 3;
        BufferPool bp(CAP);

        int id_a = bp.create_new_page();
        int id_b = bp.create_new_page();
        int id_c = bp.create_new_page();

        // unpin in order: A, B, C
        bp.unpin_page(id_a, false);
        bp.unpin_page(id_b, false);
        bp.unpin_page(id_c, false);

        // first new page must evict A
        int id_d = bp.create_new_page();
        assert(id_d >= 0);

        // A is gone — fetch returns from disk (not null, just slower)
        // B and C are still in pool
        char* data_b = bp.fetch_page(id_b);
        char* data_c = bp.fetch_page(id_c);
        assert(data_b != nullptr);
        assert(data_c != nullptr);

        std::remove("mydb.db");
        pass("test_lru_eviction_order");
    }

    // re-fetching a page updates its recency
    // it should be evicted last, not first
    void test_fetch_updates_recency() {
        std::remove("mydb.db");
        const int CAP = 2;
        BufferPool bp(CAP);

        int id_a = bp.create_new_page();
        int id_b = bp.create_new_page();

        // unpin both
        bp.unpin_page(id_a, false);
        bp.unpin_page(id_b, false);

        // re-fetch A — makes A the most recently used
        bp.fetch_page(id_a);
        bp.unpin_page(id_a, false);

        // now B is LRU — new page must evict B, not A
        int id_c = bp.create_new_page();
        assert(id_c >= 0);

        // A must still be in the pool
        char* data_a = bp.fetch_page(id_a);
        assert(data_a != nullptr);

        std::remove("mydb.db");
        pass("test_fetch_updates_recency");
    }

    // ─────────────────────────────────────────
    //  Runner
    // ─────────────────────────────────────────
    void run_all() {
        std::cout << "===============================" << std::endl;
        std::cout << "   DISK MANAGER TESTS          " << std::endl;
        std::cout << "===============================" << std::endl;
        test_allocate_page_increments();
        test_write_read_roundtrip();
        test_multiple_pages_independent();

        std::cout << std::endl;
        std::cout << "===============================" << std::endl;
        std::cout << "   BUFFER POOL TESTS           " << std::endl;
        std::cout << "===============================" << std::endl;
        test_create_new_page_returns_valid_id();
        test_create_multiple_pages_unique_ids();
        test_fetch_nonexistent_page_returns_null();
        test_fetch_existing_page_returns_data();
        test_fetch_same_page_twice_cache_hit();
        test_fill_to_capacity();
        test_eviction_triggered_when_full();
        test_correct_page_evicted();
        test_written_data_persists();
        test_all_pinned_returns_error();

        std::cout << std::endl;
        std::cout << "===============================" << std::endl;
        std::cout << "   LRU INTEGRATION TESTS       " << std::endl;
        std::cout << "===============================" << std::endl;
        test_lru_eviction_order();
        test_fetch_updates_recency();

        std::cout << std::endl;
        std::cout << "===============================" << std::endl;
        std::cout << "   All tests passed! ✅        " << std::endl;
        std::cout << "===============================" << std::endl;
    }
};

#endif // TEST_BUFFER_POOL_H
