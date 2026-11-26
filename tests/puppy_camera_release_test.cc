#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define private public
#include "esp32_camera.h"
#undef private

#include "esp_heap_caps.h"

static int FileHas(const char* path, const char* needle) {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        return -1;
    }
    std::string data;
    char buf[1024];
    size_t n = 0;
    while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
        data.append(buf, n);
    }
    fclose(file);
    if (data.find(needle) != std::string::npos) {
        return 1;
    }
    return 0;
}

static void FailRaceAndStop(const char* name) {
    std::cout << "FAIL " << name << " returned the frame while capture still holds it\n";
    std::_Exit(1);
}

static int FrameRace(Esp32Camera& camera, bool disable, const char* name) {
    g_fb_get_returns_frame = true;
    g_stall_malloc_once.store(1);
    g_malloc_stalled.store(0);
    g_release_malloc.store(0);
    g_fb_return_while_stalled.store(0);

    std::thread capture([&camera]() { camera.Capture(); });
    for (int i = 0; i < 2000 && g_malloc_stalled.load() == 0; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (g_malloc_stalled.load() == 0) {
        g_release_malloc.store(1);
        capture.join();
        g_fb_get_returns_frame = false;
        std::cout << "FAIL " << name << " never reached the frame hold\n";
        return 1;
    }

    std::thread other([&camera, disable]() {
        if (disable) {
            camera.SetEnabled(false);
        } else {
            camera.ReleaseFrame();
        }
    });
    for (int i = 0; i < 100; i++) {
        if (g_fb_return_while_stalled.load() != 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (g_fb_return_while_stalled.load() != 0) {
        FailRaceAndStop(name);
    }

    g_release_malloc.store(1);
    capture.join();
    other.join();
    g_fb_get_returns_frame = false;
    bool enabled = camera.IsEnabled();
    if (disable) {
        if (enabled) {
            std::cout << "FAIL " << name << " left the camera on\n";
            return 1;
        }
    } else if (!enabled) {
        std::cout << "FAIL " << name << " turned the camera off\n";
        return 1;
    }
    std::cout << "ok " << name << " waits until capture finishes with the frame\n";
    return 0;
}

int main() {
    std::cout << std::unitbuf;
    int failures = 0;
    camera_config_t config{};
    {
        Esp32Camera off(config);
        bool captured = off.Capture();
        if (off.IsEnabled() || captured || g_camera_init_calls != 0 || g_camera_fb_get_calls != 0) {
            std::cout << "FAIL camera starts off and capture does not touch the sensor\n";
            failures++;
        } else {
            std::cout << "ok camera starts off and capture does not touch the sensor\n";
        }
        off.SetEnabled(false);
        if (g_camera_deinit_calls != 0 || g_camera_fb_get_calls != 0) {
            std::cout << "FAIL disable while off does not touch the driver\n";
            failures++;
        } else {
            std::cout << "ok disable while off does not touch the driver\n";
        }
    }

    const char* sources[] = {
        "main/boards/common/esp32_camera.h",
        "main/boards/common/esp32_camera.cc",
    };
    for (const char* path : sources) {
        int found = FileHas(path, "Esp32CameraTest_ArmEncoder");
        if (found != 0) {
            std::cout << "FAIL production source still has Esp32CameraTest_ArmEncoder in " << path << "\n";
            failures++;
        } else {
            std::cout << "ok production source has no Esp32CameraTest_ArmEncoder in " << path << "\n";
        }
    }

    g_camera_init_calls = 0;
    g_freed_while_encoder_running.store(0);
    {
        Esp32Camera camera(config);
        if (!camera.SetEnabled(true) || g_camera_init_calls != 1) {
            std::cout << "FAIL set_enabled turns the camera on\n";
            failures++;
        } else {
            std::cout << "ok set_enabled turns the camera on\n";
        }
        uint8_t* encode_buf = static_cast<uint8_t*>(malloc(4));
        g_encoder_still_running.store(1);
        camera.encode_buf_ = encode_buf;
        camera.encode_buf_size_ = 4;
        camera.encoder_thread_ = std::thread([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            g_encoder_still_running.store(0);
        });
        camera.SetEnabled(false);
        if (g_freed_while_encoder_running.load() != 0 || camera.IsEnabled()) {
            std::cout << "FAIL disable joins the encoder before freeing the buffer\n";
            failures++;
        } else {
            std::cout << "ok disable joins the encoder before freeing the buffer\n";
        }
    }

    {
        Esp32Camera camera(config);
        if (!camera.SetEnabled(true)) {
            std::cout << "FAIL race setup could not enable the camera\n";
            failures++;
        } else {
            failures += FrameRace(camera, false, "release_frame");
        }
    }
    {
        Esp32Camera camera(config);
        if (!camera.SetEnabled(true)) {
            std::cout << "FAIL race setup could not enable the camera\n";
            failures++;
        } else {
            failures += FrameRace(camera, true, "set_enabled");
        }
    }

    if (failures != 0) {
        std::cout << failures << " failed\n";
        return 1;
    }
    std::cout << "puppy_camera_release_test passed\n";
    return 0;
}
