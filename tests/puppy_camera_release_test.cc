#include "esp32_camera.h"
#include "esp_heap_caps.h"

#include <chrono>
#include <iostream>
#include <thread>

void Esp32CameraTest_ArmEncoder(Esp32Camera* camera, std::thread worker, uint8_t* encode_buf, size_t size);

int main() {
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
        Esp32CameraTest_ArmEncoder(&camera, std::thread([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            g_encoder_still_running.store(0);
        }), encode_buf, 4);
        camera.SetEnabled(false);
        if (g_freed_while_encoder_running.load() != 0 || camera.IsEnabled()) {
            std::cout << "FAIL disable joins the encoder before freeing the buffer\n";
            failures++;
        } else {
            std::cout << "ok disable joins the encoder before freeing the buffer\n";
        }
    }

    if (failures != 0) {
        std::cout << failures << " failed\n";
        return 1;
    }
    std::cout << "puppy_camera_release_test passed\n";
    return 0;
}
