#pragma once
#include "display.h"
#include <memory>
#include <stddef.h>
#define LV_COLOR_FORMAT_RGB565 0
class LvglAllocatedImage {
public:
    LvglAllocatedImage(uint8_t*, size_t, int, int, int, int) {}
};
class LvglDisplay : public Display {
public:
    virtual void SetPreviewImage(std::unique_ptr<LvglAllocatedImage>) {}
};
