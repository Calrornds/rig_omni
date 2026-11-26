#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdexcept>
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

inline esp_err_t esp_camera_init(const camera_config_t*) {
    g_camera_init_calls++;
    return ESP_OK;
}
inline sensor_t* esp_camera_sensor_get(void) {
    g_camera_sensor_get_calls++;
    return nullptr;
}
inline void esp_camera_deinit(void) { g_camera_deinit_calls++; }
inline camera_fb_t* esp_camera_fb_get(void) {
    g_camera_fb_get_calls++;
    return nullptr;
}
inline void esp_camera_fb_return(camera_fb_t*) { g_camera_fb_return_calls++; }
