#pragma once
#include <stddef.h>
#include <stdint.h>
enum v4l2_pix_fmt_t {
    V4L2_PIX_FMT_RGB565,
    V4L2_PIX_FMT_YUYV,
    V4L2_PIX_FMT_YUV420,
    V4L2_PIX_FMT_GREY,
    V4L2_PIX_FMT_JPEG,
    V4L2_PIX_FMT_RGB24
};
typedef size_t (*jpeg_cb_t)(void*, size_t, const void*, size_t);
inline bool image_to_jpeg_cb(uint8_t*, size_t, uint16_t, uint16_t, v4l2_pix_fmt_t, int, jpeg_cb_t, void*) {
    return false;
}
