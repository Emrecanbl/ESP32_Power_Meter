#include <Arduino.h>
#include <INA3221.h>
#include <Web_Ui.h>

// ---------- Config ----------
static constexpr uint32_t SENSOR_PERIOD_MS = 100; // 10 Hz
static constexpr uint32_t SCREEN_PERIOD_MS = 33; // ~30 FPS (adjust as needed)
static constexpr uint32_t UART_PERIOD_MS = 1000; // 1 Hz
static constexpr uint32_t WEB_LOOP_MS = 10; // service HTTP frequently
static constexpr uint32_t Mutex_Max_Wait_MS = 1000;                 


