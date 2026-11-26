#pragma once
#include <atomic>
#include <chrono>
#include <stdlib.h>
#include <thread>
#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT 2
inline std::atomic<int> g_encoder_still_running{0};
inline std::atomic<int> g_freed_while_encoder_running{0};
inline std::atomic<int> g_stall_malloc_once{0};
inline std::atomic<int> g_malloc_stalled{0};
inline std::atomic<int> g_release_malloc{0};
inline std::atomic<int> g_fb_return_while_stalled{0};
inline void* heap_caps_malloc(size_t size, int) {
    if (g_stall_malloc_once.exchange(0) == 1) {
        g_malloc_stalled.store(1);
        while (g_release_malloc.load() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    return malloc(size);
}
inline void* heap_caps_aligned_alloc(size_t, size_t size, int) { return malloc(size); }
inline void heap_caps_free(void* p) {
    if (g_encoder_still_running.load() != 0) {
        g_freed_while_encoder_running.fetch_add(1);
    }
    free(p);
}
