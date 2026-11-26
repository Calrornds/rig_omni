#pragma once
typedef void* QueueHandle_t;
#define portMAX_DELAY 0xffffffffu
#define pdPASS 1
inline QueueHandle_t xQueueCreate(int, int) { return (QueueHandle_t)1; }
inline int xQueueSend(QueueHandle_t, const void*, unsigned) { return pdPASS; }
inline int xQueueReceive(QueueHandle_t, void*, unsigned) { return 0; }
inline void vQueueDelete(QueueHandle_t) {}
inline int uxTaskGetStackHighWaterMark(void*) { return 0; }
