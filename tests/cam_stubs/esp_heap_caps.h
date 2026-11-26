#pragma once
#include <atomic>
#include <stdlib.h>
#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT 2
inline std::atomic<int> g_encoder_still_running{0};
inline std::atomic<int> g_freed_while_encoder_running{0};
inline void* heap_caps_malloc(size_t size, int) { return malloc(size); }
inline void* heap_caps_aligned_alloc(size_t, size_t size, int) { return malloc(size); }
inline void heap_caps_free(void* p) {
    if (g_encoder_still_running.load() != 0) {
        g_freed_while_encoder_running.fetch_add(1);
    }
    free(p);
}
