#pragma once
#include <atomic>
#include <chrono>
#include <stddef.h>
#include <stdint.h>
#include <stdexcept>
#include <thread>
#include "esp_heap_caps.h"
#ifdef _MSC_VER
#include <stdlib.h>
#define __builtin_bswap16(x) _byteswap_ushort(x)
#endif

typedef int esp_err_t;
#define ESP_OK 0

enum {
    PIXFORMAT_RGB565 = 1,
    PIXFORMAT_YUV422,
    PIXFORMAT_YUV420,
    PIXFORMAT_GRAYSCALE,
    PIXFORMAT_JPEG,
    PIXFORMAT_RGB888
};

#define GC0308_PID 0x9b

struct camera_config_t {
    int pixel_format;
};

struct camera_fb_t {
    uint8_t* buf;
    size_t len;
    int width;
    int height;
    int format;
};

struct sensor_t {
    struct {
        int PID;
    } id;
    int (*set_hmirror)(sensor_t*, int);
    int (*set_vflip)(sensor_t*, int);
};

inline int g_camera_init_calls = 0;
inline int g_camera_deinit_calls = 0;
inline int g_camera_sensor_get_calls = 0;
inline int g_camera_fb_get_calls = 0;
inline int g_camera_fb_return_calls = 0;
inline std::atomic<int> g_stall_deinit{0};
inline std::atomic<int> g_deinit_stalled{0};
inline std::atomic<int> g_release_deinit{0};
inline std::atomic<int> g_sensor_get_during_deinit{0};
inline bool g_fb_get_returns_frame = false;
inline camera_fb_t g_host_fb{};
inline uint8_t g_host_fb_buf[8]{};

inline esp_err_t esp_camera_init(const camera_config_t*) {
    g_camera_init_calls++;
    return ESP_OK;
}
inline sensor_t* esp_camera_sensor_get(void) {
    g_camera_sensor_get_calls++;
    if (g_deinit_stalled.load() != 0 && g_release_deinit.load() == 0) {
        g_sensor_get_during_deinit.fetch_add(1);
    }
    return nullptr;
}
inline void esp_camera_deinit(void) {
    g_camera_deinit_calls++;
    if (g_stall_deinit.exchange(0) == 1) {
        g_deinit_stalled.store(1);
        while (g_release_deinit.load() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
inline camera_fb_t* esp_camera_fb_get(void) {
    g_camera_fb_get_calls++;
    if (!g_fb_get_returns_frame) {
        return nullptr;
    }
    g_host_fb.buf = g_host_fb_buf;
    g_host_fb.len = sizeof(g_host_fb_buf);
    g_host_fb.width = 2;
    g_host_fb.height = 2;
    g_host_fb.format = PIXFORMAT_RGB565;
    return &g_host_fb;
}
inline void esp_camera_fb_return(camera_fb_t*) {
    g_camera_fb_return_calls++;
    if (g_malloc_stalled.load() != 0 && g_release_malloc.load() == 0) {
        g_fb_return_while_stalled.fetch_add(1);
    }
}
