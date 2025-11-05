#include <Arduino.h>
#include <INA3221.h>
#include <Web_Ui.h>
#include <LCD_ST7735.h>
// ---------- Config ----------
static constexpr uint32_t SENSOR_PERIOD_MS = 100; // 10 Hz
static constexpr uint32_t SCREEN_PERIOD_MS = 8; // ~30 FPS (adjust as needed)
static constexpr uint32_t UART_PERIOD_MS = 1000; // 1 Hz
static constexpr uint32_t WEB_LOOP_MS = 10; // service HTTP frequently
static constexpr uint32_t Mutex_Max_Wait_MS = 1000;                 


struct Start_Check
{
 bool Sensor_Started = false;
 bool Wifi_Connected = false;
 String Wifi_ip;
};

extern Start_Check Check;