#pragma once
class Display {
public:
    virtual ~Display() = default;
    virtual void SetPreviewRgb565(const void*, int, int, int) {}
};
